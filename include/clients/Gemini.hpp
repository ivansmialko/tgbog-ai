#pragma once
#include "pch.hpp"
#include <vector>
#include "dto/DataModels.hpp"
#include "AiClient.hpp"

namespace clients
{
	class GeminiClient : public clients::AiClient
	{
	private:
		static size_t writeCallback(void* in_contents, size_t in_size, size_t in_nmemb, void* out_userp);

	public:
		virtual std::string ask(const std::string& in_prompt) override;
		virtual std::string ask(const std::vector<data_models::ChatMessage>& in_history) override;
		virtual void askStream(const std::vector<data_models::ChatMessage>& in_history, OnStreamChunk in_chunk_dlg);

		virtual bool checkApiKey() override;
	};
}