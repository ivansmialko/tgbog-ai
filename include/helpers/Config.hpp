#pragma once
#include <string>
#include <map>

namespace helpers
{
	class Config
	{
	private:
		std::map<std::string, std::string> _env_vars;
		void load(const std::string& in_filename);

	public:
		explicit Config(const std::string& in_filename);

		std::string get(const std::string& in_key) const;

		bool isValid() const { return !_env_vars.empty(); }
	};
};