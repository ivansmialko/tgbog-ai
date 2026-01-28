#include "helpers/Config.hpp"
#include <fstream>
#include <iostream>
#include <logging/Logger.hpp>

namespace helpers
{
	Config::Config(const std::string& in_filename)
	{
		load(in_filename);
	}

	void Config::load(const std::string& in_filename)
	{
		std::ifstream file(in_filename);
		if (!file.is_open())
		{
			LOG_ERR("Unable to load {} file.", in_filename);
			return;
		}

		std::string line;
		while (std::getline(file, line))
		{
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			if (line.empty() || line[0] == '#')
				continue;

			size_t delimiter_pos = line.find('=');
			if (delimiter_pos == std::string::npos)
				continue;

			std::string key = line.substr(0, delimiter_pos);
			std::string value = line.substr(delimiter_pos + 1);

			key.erase(0, key.find_first_not_of(" \t\r\n"));
			value.erase(0, key.find_first_not_of(" \t\r\n"));

			_env_vars[key] = value;
		}
	}

	std::string Config::get(const std::string& in_key) const
	{
		auto it = _env_vars.find(in_key);
		if (it == std::end(_env_vars))
			return "";

		return it->second;
	}
}
