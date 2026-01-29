#pragma once
#include <functional>
#include "model_context/ModelContext.hpp"

namespace clients
{
	using OnStreamChunk = std::function<void(const std::string& in_chunk)>;

	class AiClient
	{
	protected:
		std::string _api_key;
		std::string _system_prompt;

	public:
		virtual ~AiClient() = default;

		virtual std::string ask(const std::string& in_prompt) = 0;
		virtual std::string ask(const model_context::ModelContext& in_context) = 0;
		virtual void askStream(const model_context::ModelContext&, OnStreamChunk in_chunk_dlg) = 0;

		virtual bool checkApiKey() = 0;
		void setSystemPrompt(const std::string& in_system_prompt) { _system_prompt = in_system_prompt; }
		void setApiKey(const std::string& in_api_key) { _api_key = in_api_key; }
	};
}