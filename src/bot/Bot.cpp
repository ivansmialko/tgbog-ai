#include "bot/Bot.hpp"
#include "telegram/MessageData.hpp"
#include "clients/Gemini.hpp"
#include "model_context/ModelContext.hpp"

void debugJson(const nlohmann::json& j) {
	std::ofstream file("debug_request.json");
	if (file.is_open()) {
		file << j.dump(4);
		file.close();
		std::cout << "JSON saved to debug_request.json" << std::endl;
	}
}

bool tgbot_ai::BotAi::initEnvConfig()
{
	_env_config = std::make_unique<helpers::Config>("config.env");
	if (!_env_config->isValid())
	{
		LOG_ERR("Failed to load environment configuration.");
		return false;
	}

	return true;
}

bool tgbot_ai::BotAi::initAiClient()
{
	if (!_env_config)
		return false;

	std::string api_key = _env_config->get("GEMINI_KEY");
	if (api_key.empty())
	{
		LOG_ERR("GEMINI_KEY is not set in the configuration.");
		return false;
	}

	_ai_client = std::make_unique<clients::GeminiClient>();
	_ai_client->setApiKey(api_key);
	if (!_ai_client->checkApiKey())
	{
		LOG_ERR("GEMINI_KEY is invalid.");
		return false;
	}

	std::string system_prompt = _env_config->get("SYSTEM_PROMPT");
	_ai_client->setSystemPrompt(system_prompt);

	return true;
}

bool tgbot_ai::BotAi::initStorage()
{
	_storage = std::make_unique<db::DataStorage>("chatbot.db");
	if(!_storage->isAvailable())
	{
		LOG_ERR("Database is not available or corrupted.");
		return false;
	}

	return true;
}

bool tgbot_ai::BotAi::initTgBot()
{
	std::string tg_token = _env_config->get("TELEGRAM_TOKEN");
	if (tg_token.empty())
	{
		LOG_ERR("TELEGRAM_TOKEN is not set in the configuration.");
		return false;
	}

	_bot = std::make_unique<TgBot::Bot>(tg_token);
	_username = _bot->getApi().getMe()->username;

	return true;
}

bool tgbot_ai::BotAi::init()
{
	if (!initEnvConfig())
		return false;

	if (!initAiClient())
		return false;

	if (!initStorage())
		return false;

	if (!initTgBot())
		return false;

	return true;
}

void tgbot_ai::BotAi::tgOnAnyMessage(TgBot::Message::Ptr in_message)
{
	if (in_message->text.empty())
		return;

	LOG_INFO("Received message from chat_id {}: {}", in_message->chat->id, in_message->text);

	bool isGroup = (in_message->chat->type == TgBot::Chat::Type::Group ||
		in_message->chat->type == TgBot::Chat::Type::Supergroup);

	if (isGroup)
	{
		std::string mention = "@" + _username;
		if (in_message->text.find(mention) == std::string::npos)
		{
			tg::MessageData msg_data;
			msg_data._chat_name = in_message->chat->title.empty() ? in_message->chat->username : in_message->chat->title;
			msg_data._chat_type = isGroup ? "group" : "private";
			msg_data._message_text = in_message->text;
			msg_data._tg_chat_id = in_message->chat->id;
			msg_data._tg_thread_id = in_message->messageThreadId;
			msg_data._tg_user_id = in_message->from->id;
			msg_data._tg_msg_id = in_message->messageId;
			msg_data._role = "user";
			msg_data._nickname = in_message->from->username.empty() ? in_message->from->firstName : in_message->from->username;
			_storage->saveMessage(msg_data);
			return;
		}
	}

	streamReply(in_message);

	LOG_INFO("Processed message from chat_id {}: {}", in_message->chat->id, in_message->text);
}

void tgbot_ai::BotAi::streamReply(TgBot::Message::Ptr in_message)
{
	if (!_ai_client)
		return;

	if (!_bot)
		return;

	_bot->getApi().sendChatAction(in_message->chat->id, "typing");

	bool is_group = (in_message->chat->type == TgBot::Chat::Type::Group ||
		in_message->chat->type == TgBot::Chat::Type::Supergroup);

	std::string full_response;
	auto last_update = std::chrono::steady_clock::now();
	UINT64 sent_message_id = 0;
	UINT64 request_msg_id = static_cast<UINT64>(in_message->messageId);

	tg::MessageData msg_data;
	msg_data._chat_name = in_message->chat->title.empty() ? in_message->chat->username : in_message->chat->title;
	msg_data._chat_type = is_group ? "group" : "private";
	msg_data._message_text = in_message->text;
	msg_data._tg_chat_id = in_message->chat->id;
	msg_data._tg_thread_id = in_message->messageThreadId;
	msg_data._tg_user_id = in_message->from->id;
	msg_data._tg_msg_id = in_message->messageId;
	msg_data._role = "user";
	msg_data._nickname = in_message->from->username.empty() ? in_message->from->firstName : in_message->from->username;

	_storage->saveMessage(msg_data);

	model_context::ModelContext model_context = _storage->getModelContext(msg_data);
	debugJson(model_context.getJsonHistory());

	std::string response = _ai_client->ask(model_context);
	auto sent_message = _bot->getApi().sendMessage(in_message->chat->id, response);
	
	tg::MessageData sent_msg_data;
	msg_data._chat_name = sent_message->chat->title.empty() ? in_message->chat->username : sent_message->chat->title;
	sent_msg_data._chat_type = is_group ? "group" : "private";
	sent_msg_data._message_text = sent_message->text;
	sent_msg_data._tg_chat_id = sent_message->chat->id;
	sent_msg_data._tg_thread_id = sent_message->messageThreadId;
	sent_msg_data._tg_user_id = _bot->getApi().getMe()->id;
	sent_msg_data._tg_msg_id = sent_message->messageId;
	sent_msg_data._role = "model";
	sent_msg_data._nickname = _bot->getApi().getMe()->username;

	_storage->saveMessage(sent_msg_data);

	//_ai_client->askStream(history, [this, tg_chat_id, &sent_message_id, &full_response, &last_update, &is_group, &request_msg_id](const std::string& in_chunk)
	//{
	//	full_response += in_chunk;

	//	auto now = std::chrono::steady_clock::now();
	//	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update).count();

	//	if (sent_message_id == 0)
	//	{
	//		auto reply_params = std::make_shared<TgBot::ReplyParameters>();
	//		reply_params->messageId = request_msg_id;
	//		reply_params->chatId = tg_chat_id;
	//		reply_params->allowSendingWithoutReply = true;

	//		TgBot::ReplyParameters::Ptr current_reply = is_group ? reply_params : nullptr;

	//		auto msg = _bot->getApi().sendMessage(
	//			tg_chat_id,
	//			full_response + "|",
	//			nullptr,
	//			current_reply,
	//			nullptr,
	//			"HTML"
	//		);

	//		sent_message_id = msg->messageId;
	//		last_update = now;
	//	}
	//	else if (diff > 500)
	//	{
	//		try
	//		{
	//			_bot->getApi().editMessageText(full_response + "|", tg_chat_id, sent_message_id);
	//			last_update = now;
	//		}
	//		catch (const std::exception& e)
	//		{
	//			LOG_ERR("Edit error: {}", std::string(e.what()));
	//		}
	//	}
	//});

	//if (full_response.empty() || sent_message_id == 0)
	//	return;

	//_storage->saveModelReponse(tg_chat_id, in_message->messageThreadId, full_response);

	//_bot->getApi().editMessageText(full_response, tg_chat_id, sent_message_id);
}

void tgbot_ai::BotAi::start()
{
	if (!_bot)
		return;

	_bot->getEvents().onAnyMessage([this](TgBot::Message::Ptr in_message) {
		tgOnAnyMessage(in_message);
	});

	try
	{
		TgBot::TgLongPoll long_poll((*_bot));
		LOG_INFO("Bot started");
		while (true)
		{
			long_poll.start();
		}
	}
	catch (std::exception& e)
	{
		LOG_ERR("Exception occurred: {}", e.what());
	}
}
