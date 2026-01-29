#include "db/DataStorage.hpp"

db::DataStorage::DataStorage(const std::string& in_db_path):
	_db{ std::make_unique<SQLite3DB>(in_db_path) }
{
}

model_context::ModelContext db::DataStorage::getModelContext(const tg::MessageData& in_msg_data)
{
	model_context::ModelContext model_context;
	if (!_db)
		return model_context;


	data_models::User user = _db->getUser(static_cast<UINT64>(in_msg_data._tg_chat_id));
	if (user.isEmpty())
	{
		_db->insertUser(static_cast<UINT64>(in_msg_data._tg_chat_id), in_msg_data._chat_name, "user_nickname");
		user = _db->getUser(static_cast<UINT64>(in_msg_data._tg_chat_id));
	}

	data_models::Chat chat = _db->getChat(static_cast<UINT64>(in_msg_data._tg_chat_id));
	if (chat.isEmpty())
	{
		_db->insertChat(static_cast<UINT64>(in_msg_data._tg_chat_id), in_msg_data._chat_name, in_msg_data._chat_type);
		chat = _db->getChat(static_cast<UINT64>(in_msg_data._tg_chat_id));
	}

	data_models::Thread thread = _db->getThread(static_cast<UINT64>(in_msg_data._tg_thread_id), static_cast<UINT64>(in_msg_data._tg_chat_id));
	if (thread.isEmpty())
	{
		_db->insertThread(static_cast<UINT64>(in_msg_data._tg_thread_id), static_cast<UINT64>(in_msg_data._tg_chat_id), "Default Topic");
		thread = _db->getThread(static_cast<UINT64>(in_msg_data._tg_thread_id), static_cast<UINT64>(in_msg_data._tg_chat_id));
	}

	model_context._topic_name = thread._name;
	model_context._chat_name = chat._name;
	model_context._nickname = user._nickname;
	model_context._user_name = user._name;

	std::vector<data_models::Message> chat_history = _db->getMessages(chat._id, thread._id, 100ULL);
	for (const auto& message : chat_history)
	{
		data_models::User author_user = _db->getUser(static_cast<UINT64>(in_msg_data._tg_chat_id));
		if(author_user.isEmpty())
			continue;

		model_context::Message context_message;
		context_message._nickname = author_user._nickname;
		context_message._user_name = author_user._name;
		context_message._message = message._content;
		model_context._chat_history.push_back(std::move(context_message));
	}

	return model_context;
}

bool db::DataStorage::isAvailable()
{
	if (!_db)
		return false;

	return _db->checkHealth();
}
