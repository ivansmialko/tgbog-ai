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

clients::GeminiClient::GeminiClient(const std::string& in_api_key)
	: _api_key(in_api_key)
{
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

std::string clients::GeminiClient::ask(const std::vector<data_models::ChatMessage>& in_history)
{
	CURL* curl = curl_easy_init();

	if (!curl)
		return "Error";

	std::string api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + _api_key;

	nlohmann::json json_body;
	json_body["system_instruction"]["parts"]["text"] = _system_prompt;

	nlohmann::json contents = nlohmann::json::array();
	for (const auto& message : in_history)
	{
		contents.push_back({
				{"role", message._role},
				{"parts", {{{"text", message._content}}}} });
	}
	json_body["contents"] = contents;

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

void clients::GeminiClient::setSystemPrompt(const std::string& in_system_prompt)
{
	_system_prompt = in_system_prompt;
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

