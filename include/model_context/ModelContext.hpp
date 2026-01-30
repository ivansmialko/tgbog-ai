#pragma once
#include "pch.hpp"

namespace model_context
{
	struct Message
	{
		std::string _nickname;
		std::string _user_name;
		std::string _message;
		std::string _role;

		nlohmann::json toJson() const
		{
			nlohmann::json j;
			j["nickname"] = _nickname;
			j["user_name"] = _user_name;
			j["role"] = _role;
			j["message"] = _message;
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

		nlohmann::json getJsonHistory() const
		{
			nlohmann::json chat_history_json = nlohmann::json::array();
			for (const auto& message : _chat_history)
			{
				nlohmann::json text;
				text["text"] = fmt::format("[{}]: {}", message._nickname, message._message);

				nlohmann::json parts_arr = nlohmann::json::array();
				parts_arr.push_back(text);

				nlohmann::json content;
				content["role"] = message._role;
				content["parts"] = parts_arr;
				chat_history_json.push_back(content);
			}

			return chat_history_json;
		}

		nlohmann::json getJsonChatOverview() const
		{
			nlohmann::json j;
			j["topic_name"] = _topic_name;
			j["chat_name"] = _chat_name;
			j["nickname"] = _nickname;
			j["user_name"] = _user_name;
			return j;
		}
	};
}
