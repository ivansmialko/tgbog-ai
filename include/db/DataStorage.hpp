#pragma once
#include "db/SQLite3DB.hpp"
#include "model_context/ModelContext.hpp"
#include "telegram/MessageData.hpp"
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

		model_context::ModelContext getModelContext(const tg::MessageData& in_msg_data);
		void saveMessage(const tg::MessageData& in_msg_data);

		bool isAvailable();
	};
}