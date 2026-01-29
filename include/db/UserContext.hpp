#pragma once
#include "dto/DataModels.hpp"


namespace db
{
	struct UserContext
	{
		data_models::Chat _chat_data;
		data_models::Topic _topic_data;
		std::vector<data_models::ChatMessage> _chat_history;
	};
}
