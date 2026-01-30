#pragma once

namespace tg
{
	struct MessageData
	{
		int64_t _tg_chat_id;
		uint64_t _tg_thread_id;
		uint64_t _tg_msg_id;
		uint64_t _tg_user_id;
		std::string _chat_name;
		std::string _chat_type;
		std::string _message_text;
		std::string _role;
		std::string _nickname;
	};
}