#include "db/SQLite3DB.hpp"
#include <iostream>

db::SQLite3DB::SQLite3DB(const std::string& in_db_path)
{
	if (sqlite3_open(in_db_path.c_str(), &_db) != SQLITE_OK)
	{
		std::cerr << "DB Error: " << sqlite3_errmsg(_db) << std::endl;
		return;
	}

	createTables();
}

db::SQLite3DB::~SQLite3DB()
{
	if (!_db)
		return;

	sqlite3_close(_db);
}

data_models::Chat db::SQLite3DB::getOrCreateChat(int64_t in_tg_chat_id, const std::string& in_title, const std::string& in_type)
{
	sqlite3_stmt* stmt;
	std::string sql = "INSERT INTO chats (tg_chat_id, title, type) VALUES (?, ?, ?) "
		"ON CONFLICT(tg_chat_id) DO UPDATE SET title=excluded.title, type=excluded.type;";

	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
	{
		sqlite3_bind_int64(stmt, 1, in_tg_chat_id);
		sqlite3_bind_text(stmt, 2, in_title.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, in_type.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}

	return fetchChatByTgId(in_tg_chat_id);
}

data_models::Topic db::SQLite3DB::getOrCreateTopic(int in_chat_id, int64_t in_tg_thread_id, const std::string& default_name)
{
	std::string insert_sql = "INSERT INTO topics (chat_id, tg_thread_id, name) VALUES (?, ?, ?) "
		"ON CONFLICT(chat_id, tg_thread_id) DO NOTHING;";

	sqlite3_stmt* insert_stmt;
	if (sqlite3_prepare_v2(_db, insert_sql.c_str(), -1, &insert_stmt, nullptr) == SQLITE_OK)
	{
		sqlite3_bind_int(insert_stmt, 1, in_chat_id);
		sqlite3_bind_int64(insert_stmt, 2, in_tg_thread_id);
		sqlite3_bind_text(insert_stmt, 3, default_name.c_str(), -1, SQLITE_TRANSIENT);

		sqlite3_step(insert_stmt);
		sqlite3_finalize(insert_stmt);
	}

	data_models::Topic topic{ 0, in_chat_id, in_tg_thread_id, "" };
	std::string select_sql = "SELECT id, name FROM topics WHERE chat_id = ? AND tg_thread_id = ?;";

	sqlite3_stmt* select_stmt;
	if (sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &select_stmt, nullptr) != SQLITE_OK)
		return topic;

	sqlite3_bind_int(select_stmt, 1, in_chat_id);
	sqlite3_bind_int64(select_stmt, 2, in_tg_thread_id);

	if (sqlite3_step(select_stmt) == SQLITE_ROW)
	{
		topic._id = sqlite3_column_int(select_stmt, 0);
		if (const unsigned char* name = sqlite3_column_text(select_stmt, 1))
		{
			topic._name = reinterpret_cast<const char*>(name);
		}
	}
	
	sqlite3_finalize(select_stmt);

	return topic;
}

void db::SQLite3DB::saveMessage(int in_topic_id, const data_models::ChatMessage& msg)
{
	sqlite3_stmt* stmt;
	std::string sql = "INSERT INTO chat_history (topic_id, role, content) VALUES (?, ?, ?);";

	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return;

	sqlite3_bind_int(stmt, 1, in_topic_id);
	sqlite3_bind_text(stmt, 2, msg._role.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, msg._content.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

std::vector<data_models::ChatMessage> db::SQLite3DB::getChatHistory(int in_topic_id, size_t in_limit /*= 10*/)
{
	std::vector<data_models::ChatMessage> history;
	sqlite3_stmt* stmt;

	std::string sql = "SELECT role, content FROM (SELECT * FROM chat_history "
		"WHERE topic_id = ? ORDER BY timestamp DESC LIMIT ?) "
		"ORDER BY timestamp ASC;";

	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return history;

	sqlite3_bind_int(stmt, 1, in_topic_id);
	sqlite3_bind_int(stmt, 2, in_limit);

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		data_models::ChatMessage message;
		if (const unsigned char* role = sqlite3_column_text(stmt, 0); role != nullptr)
		{
			message._role = reinterpret_cast<const char*>(role);
		}

		if (const unsigned char* text = sqlite3_column_text(stmt, 1); text != nullptr)
		{
			message._content = reinterpret_cast<const char*>(text);
		}

		history.push_back(message);
	}

	sqlite3_finalize(stmt);
	return history;
}

void db::SQLite3DB::createTables()
{
	std::string sql =
		"CREATE TABLE IF NOT EXISTS chats (id INTEGER PRIMARY KEY, tg_chat_id INTEGER UNIQUE, title TEXT, type TEXT);"
		"CREATE TABLE IF NOT EXISTS topics (id INTEGER PRIMARY KEY, chat_id INTEGER, tg_thread_id INTEGER, name TEXT, "
		"UNIQUE(chat_id, tg_thread_id), FOREIGN KEY(chat_id) REFERENCES chats(id));"
		"CREATE TABLE IF NOT EXISTS chat_history (id INTEGER PRIMARY KEY, topic_id INTEGER, role TEXT, content TEXT, "
		"timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, FOREIGN KEY(topic_id) REFERENCES topics(id));";

	execute(sql);
}

bool db::SQLite3DB::execute(const std::string& in_sql)
{
	return sqlite3_exec(_db, in_sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK;
}

data_models::Chat db::SQLite3DB::fetchChatByTgId(int64_t in_tg_chat_id)
{
	data_models::Chat chat{ 0, in_tg_chat_id, "", "" };
	sqlite3_stmt* stmt;

	std::string sql = "SELECT id, title, type FROM chats WHERE tg_chat_id = ?;";
	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
	{
		return chat;
	}

	sqlite3_bind_int64(stmt, 1, in_tg_chat_id);
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		chat._id = sqlite3_column_int(stmt, 0);
		if (const unsigned char* title = sqlite3_column_text(stmt, 1); title != nullptr)
		{
			chat._title = reinterpret_cast<const char*>(title);
		}

		if (const unsigned char* type = sqlite3_column_text(stmt, 2); type != nullptr)
		{
			chat._type = reinterpret_cast<const char*>(type);
		}
	}

	sqlite3_finalize(stmt);
	return chat;
}
