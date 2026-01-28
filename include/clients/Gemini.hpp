#pragma once
#include "pch.hpp"
#include <vector>
#include "dto/DataModels.hpp"

namespace clients
{
	class GeminiClient
	{
	private:
		std::string _api_key;
		std::string _system_prompt;
		static size_t writeCallback(void* in_contents, size_t in_size, size_t in_nmemb, void* out_userp);

	public:
		explicit GeminiClient(const std::string& in_api_key);
		std::string ask(const std::string& in_prompt);
		std::string ask(const std::vector<data_models::ChatMessage>& in_history);
		void setSystemPrompt(const std::string& in_system_prompt);
		bool checkApiKey();
	};
}