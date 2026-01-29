#pragma once
#include "pch.hpp"

namespace model_context
{
	struct Message
	{
		std::string _nickname;
		std::string _user_name;
		std::string _message;

		nlohmann::json toJson() const
		{
			nlohmann::json j;
			j["_nickname"] = _nickname;
			j["_user_name"] = _user_name;
			j["_message"] = _message;
			return j;
		}
	};

	struct ModelContext
	{
		std::string _topic_name;
		std::string _chat_name;
		std::string _nickname;
		std::string _user_name;
		std::vector<model_context::Message> _chat_history;

		nlohmann::json toJson() const
		{
			nlohmann::json j;
			j["topic_name"] = _topic_name;
			j["chat_name"] = _chat_name;
			j["nickname"] = _nickname;
			j["user_name"] = _user_name;

			nlohmann::json chat_history_json = nlohmann::json::array();
			for (const auto& message : _chat_history)
			{
				chat_history_json.push_back(message.toJson());
			}
			j["chat_history"] = chat_history_json;

			return j;
		}
	};
}
