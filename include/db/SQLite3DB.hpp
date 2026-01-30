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

		void insertMessage(int64_t in_tg_chat_id, uint64_t_t in_tg_thread_id, uint64_t_t in_tg_user_id,
			uint64_t_t in_tg_msg_id, std::string in_role, const std::string& in_content);
		void insertUser(uint64_t_t in_tg_user_id, const std::string& in_name, const std::string& in_nickname);
		void insertChat(int64_t in_tg_chat_id, const std::string& in_name, const std::string& in_type);
		void insertThread(uint64_t_t in_tg_thread_id, int64_t in_tg_chat_id, const std::string& in_name);

		data_models::Chat getChat(int64_t in_tg_chat_id);
		data_models::User getUser(uint64_t_t in_tg_user_id);
		data_models::User getUserById(uint64_t_t in_user_id);
		data_models::Thread getThread(uint64_t_t in_tg_thread_id, int64_t in_tg_chat_id);
		std::vector<data_models::Message> getMessages(uint64_t_t in_chat_id, uint64_t_t in_thread_id, uint64_t_t in_limit = 100ULL);

		bool checkHealth();
	};
}
