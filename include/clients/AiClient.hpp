#pragma once
#include <functional>
#include "dto/DataModels.hpp"
#include "db/UserContext.hpp"

namespace clients
{
	using OnStreamChunk = std::function<void(const std::string& in_chunk)>;

	class AiClient
	{
	protected:
		std::string _api_key;
		std::string _system_prompt;

		virtual nlohmann::json chatHistoryToJson(const std::vector<data_models::ChatMessage>& in_history) const = 0;

	public:
		virtual ~AiClient() = default;

		virtual std::string ask(const std::string& in_prompt) = 0;
		virtual std::string ask(const db::UserContext& in_context) = 0;
		virtual void askStream(const db::UserContext& in_context, OnStreamChunk in_chunk_dlg) = 0;

		virtual bool checkApiKey() = 0;
		void setSystemPrompt(const std::string& in_system_prompt) { _system_prompt = in_system_prompt; }
		void setApiKey(const std::string& in_api_key) { _api_key = in_api_key; }
	};
}