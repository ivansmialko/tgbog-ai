#include <iostream>
#include <filesystem>
#include <csignal>
#include <tgbot/tgbot.h>
#include "helpers/Config.hpp"
#include "clients/Gemini.hpp"
#include "db/DataStorage.hpp"
#include <windows.h>

int main() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	helpers::Config env_cfg("config.env");
	if (!env_cfg.isValid())
		return 0;

	std::string ai_token = env_cfg.get("GEMINI_KEY");
	std::string system_prompt = env_cfg.get("SYSTEM_PROMPT");
	clients::GeminiClient gemini(ai_token);
	gemini.setSystemPrompt(system_prompt);
	
	std::string tg_token = env_cfg.get("TELEGRAM_TOKEN");
	TgBot::Bot bot(tg_token);

	db::DataStorage storage("chatbot.db");

	bot.getEvents().onAnyMessage([&bot, &gemini, &storage](TgBot::Message::Ptr message) {
        if (message->text.empty())
			return;
        
		auto history = storage.getUserContext(
			message->chat->id,
			message->messageThreadId,
			(message->chat->title.empty() ? message->chat->firstName : message->chat->title),
			"private",
			message->text);

		std::string ai_response = gemini.ask(history);
		storage.saveModelReponse(message->chat->id, message->messageThreadId, ai_response);

		auto replyParams = std::make_shared<TgBot::ReplyParameters>();
		replyParams->messageId = message->messageId;
		replyParams->chatId = message->chat->id;

        std::cout << "User" << message->chat->username << " wrote: " << message->text << std::endl;
        bot.getApi().sendMessage(message->chat->id, ai_response, nullptr, replyParams, std::make_shared<TgBot::GenericReply>(), "HTML");
    });

	try
	{
		TgBot::TgLongPoll long_poll(bot);
		while (true)
		{
			long_poll.start();
		}
	}
	catch (std::exception& e)
	{
		printf("Error: %s\n", e.what());
	}

	return 0;
}