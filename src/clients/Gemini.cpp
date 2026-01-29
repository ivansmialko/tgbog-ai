#include "clients/Gemini.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>

void debugJson(const nlohmann::json& j) {
	std::ofstream file("debug_request.json");
	if (file.is_open()) {
		file << j.dump(4);
		file.close();
		std::cout << "JSON saved to debug_request.json" << std::endl;
	}
}

size_t clients::GeminiClient::writeCallback(void* in_contents, size_t in_size, size_t in_nmemb, void* out_userp)
{
	static_cast<std::string*>(out_userp)->append(static_cast<char*>(in_contents), in_size * in_nmemb);
	return in_size * in_nmemb;
}

void clients::GeminiClient::cleanUpJsonChunk(std::string& in_json)
{
	size_t first_brace = in_json.find('{');

	if (first_brace == std::string::npos) {
		if (in_json.find_first_of("], \r\t") != std::string::npos) {
			in_json.clear();
		}
		return;
	}

	in_json.erase(0, first_brace);

	size_t last_brace = in_json.rfind('}');
	if (last_brace != std::string::npos) {
		in_json.erase(last_brace + 1);
	}
}

nlohmann::json clients::GeminiClient::chatHistoryToJson(const std::vector<data_models::ChatMessage>& in_history) const
{
	nlohmann::json contents = nlohmann::json::array();
	for (const auto& message : in_history)
	{
		contents.push_back({
				{"role", message._role},
				{"parts", {{{"text", message._content}}}} });
	}

	return contents;
}

std::string clients::GeminiClient::ask(const std::string& in_prompt)
{
	CURL* curl = curl_easy_init();
	if (!curl)
		return "Error";

	std::string response;

	std::string api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + _api_key;
	nlohmann::json json_body = {
		{"system_instruction", {
			{"parts", {{{"text", _system_prompt}}}}
		}},
		{"contents", {
			{"parts", {{{"text", in_prompt}}}}
		}}
	};
	std::string request_data = json_body.dump();

	struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	auto response_json = nlohmann::json::parse(response);
	return response_json["candidates"][0]["content"]["parts"][0]["text"];
}

void clients::GeminiClient::askStream(const db::UserContext& in_context, OnStreamChunk in_chunk_dlg)
{
	CURL* curl = curl_easy_init();
	if (!curl)
		return;

	std::string api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:streamGenerateContent?key=" + _api_key;

	nlohmann::json json_body;
	json_body["system_instruction"]["parts"]["text"] = _system_prompt;
	json_body["contents"] = chatHistoryToJson(in_context._chat_history);

	struct curl_slist* headers{ nullptr };
	headers = curl_slist_append(headers, "Content-Type: application/json");

	auto curl_write_function = [](char* in_ptr, size_t in_size, size_t in_nmemb, void* in_user_data)
	{
		auto* callback_ptr = static_cast<OnStreamChunk*>(in_user_data);
		if (!callback_ptr || !(*callback_ptr))
			return in_size * in_nmemb;
		
		std::string buffer(in_ptr, in_size * in_nmemb);
		cleanUpJsonChunk(buffer);

		if(buffer.empty())
			return in_size * in_nmemb;

		try
		{
			auto json_chunk = nlohmann::json::parse(buffer);
			if (!json_chunk.contains("candidates"))
				return in_size * in_nmemb;

			if (json_chunk["candidates"].empty())
				return in_size * in_nmemb;

			std::string text_chunk = json_chunk["candidates"][0]["content"]["parts"][0]["text"];
			LOG_INFO(text_chunk);
			(*callback_ptr)(text_chunk);
		}
		catch (std::exception& e)
		{
			LOG_WARN("Failed to parse json chunk");
		}

		return in_size * in_nmemb;
	};

	std::string request_data = json_body.dump();

	curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +curl_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &in_chunk_dlg);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		LOG_WARN("Stream request failed: {}", std::string(curl_easy_strerror(res)));
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
}

std::string clients::GeminiClient::ask(const db::UserContext& in_context)
{
	CURL* curl = curl_easy_init();

	if (!curl)
		return "Error";

	std::string api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + _api_key;

	nlohmann::json json_body;
	json_body["system_instruction"]["parts"]["text"] = _system_prompt;
	json_body["contents"] = chatHistoryToJson(in_context._chat_history);

	std::string request_data = json_body.dump();
	std::string response_data;

	curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
	curl_easy_setopt(curl , CURLOPT_POSTFIELDS, request_data.c_str());

	struct curl_slist* headers{ nullptr };
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

	curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	try {
		auto json_res = nlohmann::json::parse(response_data);
		return json_res["candidates"][0]["content"]["parts"][0]["text"];
	}
	catch (...) {
		return "Error trying to parse Gemini's response";
	}
}

bool clients::GeminiClient::checkApiKey()
{
	CURL* curl = curl_easy_init();
	long http_code = 0;

	if (curl) {
		std::string url = "https://generativelanguage.googleapis.com/v1beta/models?key=" + _api_key;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void*, size_t size, size_t nmemb, void*) {
			return size * nmemb; // Просто ігноруємо дані
		});

		CURLcode res = curl_easy_perform(curl);
		if (res == CURLE_OK) {
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		}

		curl_easy_cleanup(curl);
	}

	return http_code == 200;
}