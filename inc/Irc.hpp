#pragma once

#include <iostream>

class Irc {
	public:
		Irc();
		Irc(std::string &port, std::string &password);
		Irc(Irc const &i);
		Irc &operator=(Irc const &i);
		~Irc();

	private:
		int			_port;
		std::string	_password;
};