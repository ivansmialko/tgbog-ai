#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include "dto/DataModels.hpp"

namespace db
{
	class SQLite3DB
	{
	public:
		explicit SQLite3DB(const std::string& in_db_path);
		~SQLite3DB();

		data_models::Chat getOrCreateChat(int64_t in_tg_chat_id, const std::string& in_title, const std::string& in_type);
		data_models::Topic getOrCreateTopic(int in_chat_id, int64_t in_tg_thread_id, const std::string& default_name);

		void saveMessage(int in_topic_id, const data_models::ChatMessage& msg);
		std::vector<data_models::ChatMessage> getChatHistory(int in_topic_id, size_t in_limit = 10);

		bool checkHealth();

	private:
		sqlite3* _db{ nullptr };
		void createTables();
		bool execute(const std::string& in_sql);

		data_models::Chat fetchChatByTgId(int64_t in_tg_chat_id);
	};
}
