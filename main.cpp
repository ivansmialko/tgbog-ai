#include "pch.hpp"
#include "bot/Bot.hpp"

int main() {
	logging::Logger::initConsole();

	tgbot_ai::BotAi bot_ai;
	if(!bot_ai.init())
	{
		LOG_ERR("Failed to initialize Bot AI.");
		return 1;
	}

	bot_ai.start();
	return 0;
}