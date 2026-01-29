#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include "dto/DataModels.hpp"

namespace db
{
	class SQLite3DB
	{
	private:
		sqlite3* _db{ nullptr };
		void createTables();
		bool execute(const std::string& in_sql);

	public:
		explicit SQLite3DB(const std::string& in_db_path);
		~SQLite3DB();

		void insertMessage(UINT64 in_tg_chat_id, UINT64 in_tg_user_id, UINT64 in_tg_thread_id, UINT64 in_tg_msg_id, std::string in_role, const std::string& in_content);
		void insertUser(UINT64 in_tg_user_id, const std::string& in_name, const std::string& in_nickname);
		void insertChat(UINT64 in_tg_chat_id, const std::string& in_name, const std::string& in_type);
		void insertThread(UINT64 in_tg_thread_id, UINT64 in_tg_chat_id, const std::string& in_name);

		data_models::Chat getChat(UINT64 in_tg_chat_id);
		data_models::User getUser(UINT64 in_tg_user_id);
		data_models::Thread getThread(UINT64 in_tg_thread_id, UINT64 in_tg_chat_id);
		std::vector<data_models::Message> getMessages(UINT64 in_chat_id, UINT64 in_thread_id, UINT64 in_limit = 100ULL);

		bool checkHealth();
	};
}
