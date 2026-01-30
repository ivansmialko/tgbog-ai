#pragma once

namespace tg
{
	struct MessageData
	{
		INT64 _tg_chat_id;
		UINT64 _tg_thread_id;
		UINT64 _tg_msg_id;
		UINT64 _tg_user_id;
		std::string _chat_name;
		std::string _chat_type;
		std::string _message_text;
		std::string _role;
		std::string _nickname;
	};
}