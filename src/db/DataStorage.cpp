#include "db/DataStorage.hpp"

db::DataStorage::DataStorage(const std::string& in_db_path):
	_db{ std::make_unique<SQLite3DB>(in_db_path) }
{
}

std::vector<data_models::ChatMessage> db::DataStorage::getUserContext(int64_t in_tg_chat_id,
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
