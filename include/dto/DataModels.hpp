#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace data_models
{
	struct User
	{
		UINT64 _id;
		std::string _name;
		std::string _nickname;

		bool isEmpty() { return _name.empty() && _nickname.empty(); }
	};

	struct Chat
	{
		UINT64 _id;
		std::string _name;
		std::string _type;

		bool isEmpty() { return _name.empty() && _type.empty(); }
	};

	struct Thread
	{
		UINT64 _id;
		std::string _name;

		bool isEmpty() { return _name.empty(); }
	};

	struct Message
	{
		UINT64 _id;
		std::string _content;
		std::string _role;
		UINT64 _timestamp;
		UINT64 _tg_msg_id;
		UINT64 _user_id;

		bool isEmpty() { return _content.empty() && _role.empty(); }
	};
}