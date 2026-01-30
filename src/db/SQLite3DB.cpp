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

void db::SQLite3DB::insertMessage(int64_t in_tg_chat_id, uint64_t_t in_tg_thread_id, uint64_t_t in_tg_user_id,
	uint64_t_t in_tg_msg_id, std::string in_role, const std::string& in_content)
{
	std::string sql =
		"INSERT INTO messages (thread_id, chat_id, user_id, tg_msg_id, content, role, timestamp) "
		"VALUES ("
		"  (SELECT id FROM threads WHERE tg_thread_id = ? LIMIT 1), "
		"  (SELECT id FROM chats WHERE tg_chat_id = ? LIMIT 1), "
		"  (SELECT id FROM users WHERE tg_user_id = ? LIMIT 1), "
		"  ?, ?, ?, strftime('%s','now')"
		");";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int64_t(stmt, 1, in_tg_thread_id);
	sqlite3_bind_int64_t(stmt, 2, in_tg_chat_id);
	sqlite3_bind_int64_t(stmt, 3, in_tg_user_id);

	sqlite3_bind_int64_t(stmt, 4, in_tg_msg_id);
	sqlite3_bind_text(stmt, 5, in_content.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 6, in_role.c_str(), -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

void db::SQLite3DB::insertUser(uint64_t_t in_tg_user_id, const std::string& in_name, const std::string& in_nickname)
{
	std::string sql = "INSERT INTO users (tg_user_id, name, nickname) VALUES (?, ?, ?) "
		"ON CONFLICT(tg_user_id) DO UPDATE SET name=excluded.name, nickname=excluded.nickname;";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		LOG_ERR("SQL Error: {}", sqlite3_errmsg(_db));
		return;
	}

	sqlite3_bind_int64_t(stmt, 1, in_tg_user_id);
	sqlite3_bind_text(stmt, 2, in_name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, in_nickname.c_str(), -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);
}

void db::SQLite3DB::insertChat(int64_t in_tg_chat_id, const std::string& in_name, const std::string& in_type)
{
	std::string sql = "INSERT INTO chats (tg_chat_id, name, type) VALUES (?, ?, ?) "
		"ON CONFLICT(tg_chat_id) DO UPDATE SET name=excluded.name, type=excluded.type;";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return;

	sqlite3_bind_int64_t(stmt, 1, in_tg_chat_id);
	sqlite3_bind_text(stmt, 2, in_name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, in_type.c_str(), -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);
}

void db::SQLite3DB::insertThread(uint64_t_t in_tg_thread_id, int64_t in_tg_chat_id, const std::string& in_name)
{
	std::string sql = "INSERT INTO threads (tg_thread_id, chat_id, name) VALUES ("
		"?, (SELECT id FROM chats WHERE tg_chat_id = ?), ?) "
		"ON CONFLICT(tg_thread_id, chat_id) DO UPDATE SET name=excluded.name;";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return;

	sqlite3_bind_int64_t(stmt, 1, in_tg_thread_id);
	sqlite3_bind_int64_t(stmt, 2, in_tg_chat_id);
	sqlite3_bind_text(stmt, 3, in_name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
}

data_models::Chat db::SQLite3DB::getChat(int64_t in_tg_chat_id)
{
	data_models::Chat chat;

	std::string select_sql = "SELECT id, name, type FROM chats"
		" WHERE tg_chat_id = ? LIMIT 1";

	sqlite3_stmt* stmt{ nullptr };
	if (sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return chat;

	sqlite3_bind_int64_t(stmt, 1, in_tg_chat_id);

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		chat._id = sqlite3_column_int64_t(stmt, 0);

		if (const unsigned char* name = sqlite3_column_text(stmt, 1); name != nullptr)
		{
			chat._name = reinterpret_cast<const char*>(name);
		}

		if (const unsigned char* type = sqlite3_column_text(stmt, 2); type != nullptr)
		{
			chat._type = reinterpret_cast<const char*>(type);
		}
	}

	return chat;
}

data_models::User db::SQLite3DB::getUser(uint64_t_t in_tg_user_id)
{
	data_models::User user;

	std::string select_sql = "SELECT id, name, nickname FROM users"
		" WHERE tg_user_id = ? LIMIT 1";

	sqlite3_stmt* stmt{ nullptr };
	if (sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return user;

	sqlite3_bind_int64_t(stmt, 1, in_tg_user_id);

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		user._id = sqlite3_column_int64_t(stmt, 0);

		if (const unsigned char* name = sqlite3_column_text(stmt, 1); name != nullptr)
		{
			user._name = reinterpret_cast<const char*>(name);
		}

		if (const unsigned char* nickname = sqlite3_column_text(stmt, 2); nickname != nullptr)
		{
			user._nickname = reinterpret_cast<const char*>(nickname);
		}
	}

	return user;
}

data_models::User db::SQLite3DB::getUserById(uint64_t_t in_user_id)
{
	data_models::User user;
	std::string select_sql = "SELECT id, name, nickname FROM users"
		" WHERE id = ? LIMIT 1";

	sqlite3_stmt* stmt{ nullptr };
	if (sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return user;

	sqlite3_bind_int64_t(stmt, 1, in_user_id);

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		user._id = sqlite3_column_int64_t(stmt, 0);
		if (const unsigned char* name = sqlite3_column_text(stmt, 1); name != nullptr)
		{
			user._name = reinterpret_cast<const char*>(name);
		}
		if (const unsigned char* nickname = sqlite3_column_text(stmt, 2); nickname != nullptr)
		{
			user._nickname = reinterpret_cast<const char*>(nickname);
		}
	}
	return user;
}

data_models::Thread db::SQLite3DB::getThread(uint64_t_t in_tg_thread_id, int64_t in_tg_chat_id)
{
	data_models::Thread thread;

	std::string select_sql = "SELECT id, name FROM threads"
		" WHERE tg_thread_id = ? AND chat_id = (SELECT id FROM chats WHERE tg_chat_id = ? LIMIT 1) LIMIT 1";

	sqlite3_stmt* stmt{ nullptr };
	if (sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return thread;

	sqlite3_bind_int64_t(stmt, 1, in_tg_thread_id);
	sqlite3_bind_int64_t(stmt, 2, in_tg_chat_id);

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		thread._id = sqlite3_column_int64_t(stmt, 0);

		if (const unsigned char* name = sqlite3_column_text(stmt, 1); name != nullptr)
		{
			thread._name = reinterpret_cast<const char*>(name);
		}
	}

	return thread;
}

std::vector<data_models::Message> db::SQLite3DB::getMessages(uint64_t_t in_chat_id, uint64_t_t in_thread_id, uint64_t_t in_limit /*= 100*/)
{
	std::vector<data_models::Message> messages;
	messages.reserve(in_limit);

	std::string select_sql = "SELECT content, role, timestamp, tg_msg_id, user_id FROM messages"
		" WHERE chat_id = ? ORDER BY timestamp ASC LIMIT ?";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
	{
		LOG_ERR("SQL Error: {}", sqlite3_errmsg(_db));
		return messages;
	}

	sqlite3_bind_int64_t(stmt, 1, in_chat_id);
	sqlite3_bind_int64_t(stmt, 2, in_limit);
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		data_models::Message new_message;
		if (const unsigned char* content = sqlite3_column_text(stmt, 0); content != nullptr)
		{
			new_message._content = reinterpret_cast<const char*>(content);
		}

		if (const unsigned char* role = sqlite3_column_text(stmt, 1); role != nullptr)
		{
			new_message._role = reinterpret_cast<const char*>(role);
		}

		new_message._timestamp = sqlite3_column_int64_t(stmt, 2);
		new_message._tg_msg_id = sqlite3_column_int64_t(stmt, 3);
		new_message._user_id = sqlite3_column_int64_t(stmt, 4);
		messages.push_back(std::move(new_message));
	}

	sqlite3_finalize(stmt);
	return messages;
}

bool db::SQLite3DB::checkHealth()
{
	const char* sql = "PRAGMA integrity_check;";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	bool isOk = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		if (const unsigned char* result = sqlite3_column_text(stmt, 0);
			result != nullptr && std::string(reinterpret_cast<const char*>(result)) == "ok")
		{
			isOk = true;
		}
	}

	sqlite3_finalize(stmt);
	return isOk;
}

void db::SQLite3DB::createTables()
{
	std::string create_users_sql = "CREATE TABLE IF NOT EXISTS users ("
		"id INTEGER PRIMARY KEY, "
		"tg_user_id INTEGER UNIQUE, "
		"name TEXT, "
		"nickname TEXT);";
	execute(create_users_sql);

	std::string chats_create_sql = "CREATE TABLE IF NOT EXISTS chats ("
		"id INTEGER PRIMARY KEY, "
		"tg_chat_id INTEGER UNIQUE, "
		"name TEXT, "
		"type TEXT"
		");";
	execute(chats_create_sql);

	std::string threads_create_sql = "CREATE TABLE IF NOT EXISTS threads ("
		"id INTEGER PRIMARY KEY, "
		"chat_id INTEGER, "
		"tg_thread_id INTEGER, "
		"name TEXT, "
		"UNIQUE(chat_id, tg_thread_id), "
		"FOREIGN KEY(chat_id) REFERENCES chats(id)"
		");";
	execute(threads_create_sql);

	std::string messages_create_sql = "CREATE TABLE IF NOT EXISTS messages ("
		"id INTEGER PRIMARY KEY, "
		"thread_id INTEGER, "
		"chat_id INTEGER, "
		"user_id INTEGER, "
		"tg_msg_id INTEGER, "
		"content TEXT, "
		"role TEXT, "
		"timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
		"FOREIGN KEY(thread_id) REFERENCES threads(id), "
		"FOREIGN KEY(chat_id) REFERENCES chats(id), "
		"FOREIGN KEY(user_id) REFERENCeS users(id)"
		");";
	execute(messages_create_sql);
}

bool db::SQLite3DB::execute(const std::string& in_sql)
{
	return sqlite3_exec(_db, in_sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK;
}