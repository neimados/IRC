#pragma once

#include <iostream>


class Irc {
	public:
		Irc(std::string port, std::string password);
		~Irc();

	private:
		Irc();
		Irc(Irc const &i);
		Irc &operator=(Irc const &i);

		std::string	_port;
		std::string	_password;

};
