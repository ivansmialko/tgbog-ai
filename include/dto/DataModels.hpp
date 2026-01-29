#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace data_models
{
	struct Chat
	{
		int _id;
		int64_t _tg_chat_id;
		std::string _title;
		std::string _type;
	};

	struct Topic
	{
		int _id;
		int _chat_id;
		int64_t _tg_thread_id;
		std::string _name;
	};

	struct ChatMessage
	{
		std::string _role;
		std::string _content;
	};
}