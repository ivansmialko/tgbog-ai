#pragma once
#include "pch.hpp"
#include <vector>
#include "model_context/ModelContext.hpp"
#include "AiClient.hpp"

namespace clients
{
	class GeminiClient : public clients::AiClient
	{
	private:
		static size_t writeCallback(void* in_contents, size_t in_size, size_t in_nmemb, void* out_userp);
		static void cleanUpJsonChunk(std::string& in_json);

	public:
		virtual std::string ask(const std::string& in_prompt) override;
		virtual std::string ask(const model_context::ModelContext& in_context) override;
		virtual void askStream(const model_context::ModelContext& in_context, OnStreamChunk in_chunk_dlg);

		virtual bool checkApiKey() override;
	};
}