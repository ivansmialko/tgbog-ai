#include "db/DataStorage.hpp"

db::DataStorage::DataStorage(const std::string& in_db_path):
	_db{ std::make_unique<SQLite3DB>(in_db_path) }
{
}

db::UserContext db::DataStorage::getUserContext(const db::MessageData& in_msg_data)
{
	db::UserContext user_context;
	user_context._chat_data = _db->getOrCreateChat(in_msg_data._tg_chat_id);
	user_context._topic_data = _db->getOrCreateTopic(in_msg_data._tg_thread_id);
	user_context._chat_history = _db->getChatHistory(in_msg_data._tg_chat_id, in_msg_data._tg_thread_id);

	return user_context;
}

std::vector<data_models::ChatMessage> db::DataStorage::getChatHistory(int64_t in_tg_chat_id,
	int64_t in_tg_thread_id,
	const std::string& in_chat_title,
	const std::string& in_chat_type,
	const std::string& in_user_text,
	size_t in_limit /*= 10*/)
{
	data_models::Chat chat = _db->getOrCreateChat(in_tg_chat_id, in_chat_title, in_chat_type);
	std::string topic_name = (in_tg_thread_id == 0 ? "General" : "Thread_" + std::to_string(in_tg_thread_id));

	data_models::Topic topic = _db->getOrCreateTopic(chat._id, in_tg_thread_id, topic_name);

	_db->saveMessage(topic._id, { "user", in_user_text });

	return _db->getChatHistory(topic._id, in_limit);
}

void db::DataStorage::saveModelReponse(int64_t in_tg_chat_id, int64_t in_tg_thread_id, const std::string& in_response)
{
	data_models::Chat chat = _db->getOrCreateChat(in_tg_chat_id, "", "");
	data_models::Topic topic = _db->getOrCreateTopic(chat._id, in_tg_thread_id, "");

	_db->saveMessage(topic._id, { "model", in_response });
}

bool db::DataStorage::isAvailable()
{
	return _db->checkHealth();
}
