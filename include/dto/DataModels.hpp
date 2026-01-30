#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace data_models
{
	struct User
	{
		uint64_t _id;
		std::string _name;
		std::string _nickname;

		bool isEmpty() { return _name.empty() && _nickname.empty(); }
	};

	struct Chat
	{
		uint64_t _id;
		std::string _name;
		std::string _type;

		bool isEmpty() { return _name.empty() && _type.empty(); }
	};

	struct Thread
	{
		uint64_t _id;
		std::string _name;

		bool isEmpty() { return _name.empty(); }
	};

	struct Message
	{
		uint64_t _id;
		std::string _content;
		std::string _role;
		uint64_t _timestamp;
		uint64_t _tg_msg_id;
		uint64_t _user_id;

		bool isEmpty() { return _content.empty() && _role.empty(); }
	};
}