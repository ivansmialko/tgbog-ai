#pragma once
#include "db/SQLite3DB.hpp"
#include <memory>
#include <vector>
#include <string>

namespace db
{
	class DataStorage
	{
	private:
		std::unique_ptr<SQLite3DB> _db;

	public:
		explicit DataStorage(const std::string& in_db_path);

		std::vector<data_models::ChatMessage> getUserContext(int64_t in_tg_chat_id, int64_t in_tg_thread_id, const std::string& in_chat_title, const std::string& in_chat_type, const std::string& in_user_text, size_t in_limit = 10);
		void saveModelReponse(int64_t in_tg_chat_id, int64_t in_tg_thread_id, const std::string& in_response);
	};
}