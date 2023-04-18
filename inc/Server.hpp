/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:09 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/18 00:18:41 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include "poll.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <vector>
#include <fcntl.h>
#include <signal.h>

#include "User.hpp"

// Const defines
#define WELCOME "Welcome to the IRC server!\n"
#define PROMPT "$irc> "
#define MAX_CLIENTS 10
#define MAX_BUFFER 1024
#define NICKNAME_ALREADY_USED "\033[0;31mError:\033[0m Nickname already in use.\n"
#define NICKNAME_NOT_FOUND "\033[0;31mError:\033[0m <nickname> not found.\n"
#define NICKNAME_TOO_LONG "\033[0;31mError:\033[0m Nickname too long.\n"
#define NICKNAME_FORMAT "\033[0;31mError:\033[0m Nickname must contain only letters, numbers and underscores.\n"
#define USE_HELP "\033[0;31mError:\033[0m Command not found.\n\rUse /HELP to see the list of available commands.\n"

// Defines colors
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define YELLOW "\033[0;33m"
#define CYAN "\033[0;36m"
#define ORANGE "\033[0;33m"
#define GREY "\033[0;37m"
#define WHITE "\033[0;37m"
#define RESET "\033[0m"

// Defines format
#define BOLD "\033[1m"
#define ITALIC "\033[3m"
#define NORMAL "\033[22m"
#define UNDERLINE "\033[4m"
#define REVERSE "\033[7m"

class User;

class Server {
public:
	Server(int port);
	~Server();

	void startSrv();
	void addUser();
	void cmdUser(int fd);

	int checkWritable(int fd);

	// Getters
	int	getNumberUsers() const;
	User *getUser(int fd);
	std::string getTime() const;

	// Supported commands
	void cmdNick(User *user, std::string cmd);
	void cmdHelp(User *user);
	// void cmdUser(User *user, std::string &cmd);
	void cmdList(User *user);
	// void cmdJoin(User *user, std::string &cmd);
	// void cmdPart(User *user, std::string &cmd);
	// void cmdNames(User *user, std::string &cmd);
	// void cmdPrivmsg(User *user, std::string &cmd);
	// void cmdQuit(User *user, std::string &cmd);
	

	class SrvError : public std::exception {
		public:
			virtual const char* what() const throw();
	};

	class SrvErrorClient : public std::exception {
		public:
			virtual const char* what() const throw();
	};

private:
	Server();
	Server(Server const &srv);
	Server &operator= (Server const &srv);

	struct pollfd			*_fds;
	int						_fdSrv;
	int						_port;
	int						_nbUsers;
	std::string				_Port;
	std::vector<User>		_usrs;

};
