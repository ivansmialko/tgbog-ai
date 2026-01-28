#pragma once
#include "pch.hpp"
#include "helpers/Config.hpp"
#include "clients/Gemini.hpp"
#include "db/DataStorage.hpp"

namespace tgbot_ai
{
	class BotAi
	{
	private:
		std::unique_ptr<helpers::Config> _env_config;
		std::unique_ptr<clients::GeminiClient> _ai_client;
		std::unique_ptr<db::DataStorage> _storage;
		std::unique_ptr<TgBot::Bot> _bot;

		bool initEnvConfig();
		bool initAiClient();
		bool initStorage();
		bool initTgBot();

		void tgOnAnyMessage(TgBot::Message::Ptr in_message);

	public:
		bool init();
		void start();
	};
}