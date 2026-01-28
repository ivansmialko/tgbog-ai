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

	auto history = _storage->getUserContext(
		in_message->chat->id,
		in_message->messageThreadId,
		(in_message->chat->title.empty() ? in_message->chat->firstName : in_message->chat->title),
		"private",
		in_message->text);

	std::string ai_response = _ai_client->ask(history);
	_storage->saveModelReponse(in_message->chat->id, in_message->messageThreadId, ai_response);

	auto replyParams = std::make_shared<TgBot::ReplyParameters>();
	replyParams->messageId = in_message->messageId;
	replyParams->chatId = in_message->chat->id;

	_bot->getApi().sendMessage(in_message->chat->id, ai_response, nullptr, replyParams, std::make_shared<TgBot::GenericReply>(), "HTML");

	LOG_INFO("Processed message from chat_id {}: {}", in_message->chat->id, in_message->text);
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
