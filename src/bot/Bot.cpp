#include "bot/Bot.hpp"
#include "clients/Gemini.hpp"

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
			return;
	}

	//auto history = _storage->getUserContext(
	//	in_message->chat->id,
	//	in_message->messageThreadId,
	//	(in_message->chat->title.empty() ? in_message->chat->firstName : in_message->chat->title),
	//	"private",
	//	in_message->text);

	//std::string ai_response = _ai_client->ask(history);
	//_storage->saveModelReponse(in_message->chat->id, in_message->messageThreadId, ai_response);

	//auto replyParams = std::make_shared<TgBot::ReplyParameters>();
	//replyParams->messageId = in_message->messageId;
	//replyParams->chatId = in_message->chat->id;

	//_bot->getApi().sendMessage(in_message->chat->id, ai_response, nullptr, replyParams, std::make_shared<TgBot::GenericReply>(), "HTML");
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

	bool isGroup = (in_message->chat->type == TgBot::Chat::Type::Group ||
		in_message->chat->type == TgBot::Chat::Type::Supergroup);

	std::string full_response;
	auto last_update = std::chrono::steady_clock::now();
	int32_t sent_message_id = 0;
	int64_t tg_chat_id = in_message->chat->id;
	int32_t request_msg_id = in_message->messageId;

	auto history = _storage->getUserContext(
		tg_chat_id,
		in_message->messageThreadId,
		(in_message->chat->title.empty() ? in_message->chat->firstName : in_message->chat->title),
		"private",
		in_message->text);

	_ai_client->askStream(history, [this, tg_chat_id, &sent_message_id, &full_response, &last_update, &isGroup, &request_msg_id](const std::string& in_chunk)
	{
		full_response += in_chunk;

		auto now = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update).count();

		if (sent_message_id == 0)
		{
			if (isGroup)
			{
				auto reply_params = std::make_shared<TgBot::ReplyParameters>();
				reply_params->messageId = request_msg_id;
				reply_params->chatId = tg_chat_id;
				reply_params->allowSendingWithoutReply = true;

				auto msg = _bot->getApi().sendMessage(tg_chat_id, full_response + "|", false, reply_params, nullptr, "HTML");
				sent_message_id = msg->messageId;
				last_update = now;
			}
			else
			{
				auto msg = _bot->getApi().sendMessage(tg_chat_id, full_response + "|", false, 0, nullptr, "HTML");
				sent_message_id = msg->messageId;
				last_update = now;
			}
		}
		else if (diff > 500)
		{
			try
			{
				_bot->getApi().editMessageText(full_response + "|", tg_chat_id, sent_message_id);
				last_update = now;
			}
			catch (const std::exception& e)
			{
				LOG_ERR("Edit error: {}", std::string(e.what()));
			}
		}
	});

	if (full_response.empty() || sent_message_id == 0)
		return;

	_bot->getApi().editMessageText(full_response, tg_chat_id, sent_message_id);
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
