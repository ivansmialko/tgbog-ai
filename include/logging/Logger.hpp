#pragma once
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif

namespace logging
{
	enum class LogLevel { INFO, WARN, ERR };

	class Logger {
	public:
		template <typename... Args>
		static void log(LogLevel in_level, fmt::format_string<Args...> in_str, Args&&... in_args) {
			static std::mutex logMutex;
			std::lock_guard<std::mutex> lock(logMutex);

			auto now = std::chrono::system_clock::now();

			fmt::print(fg(fmt::color::gray), "[{:%H:%M:%S}] ", now);

			switch (in_level) {
			case LogLevel::INFO:
				fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "[INFO] ");
				break;
			case LogLevel::WARN:
				fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold, "[WARN] ");
				break;
			case LogLevel::ERR:
				fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "[ERROR] ");
				break;
			}

			fmt::print(in_str, std::forward<Args>(in_args)...);
			fmt::print("\n");
		}

		static void initConsole()
		{
#ifdef _WIN32
			SetConsoleOutputCP(CP_UTF8);
			SetConsoleCP(CP_UTF8);

			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			DWORD dwMode = 0;
			GetConsoleMode(hOut, &dwMode);
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(hOut, dwMode);
#endif
		}
	};

#define LOG_INFO(fmt_str, ...) logging::Logger::log(logging::LogLevel::INFO, fmt_str, ##__VA_ARGS__)
#define LOG_WARN(fmt_str, ...) logging::Logger::log(logging::LogLevel::WARN, fmt_str, ##__VA_ARGS__)
#define LOG_ERR(fmt_str, ...)  logging::Logger::log(logging::LogLevel::ERR,  fmt_str, ##__VA_ARGS__)
}

