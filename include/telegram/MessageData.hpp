#pragma once

namespace tg
{
	struct MessageData
	{
		int _tg_chat_id;
		int _tg_thread_id;
		std::string _chat_name;
		std::string _chat_type;
		std::string _message_text;
	};
}