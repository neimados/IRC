/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:09 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/15 22:02:59 by dvergobb         ###   ########.fr       */
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

#include "User.hpp"

// Const defines
#define WELCOME "Welcome to the IRC server!\n"
#define PROMPT "$IRC> "

// Defines colors
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define YELLOW "\033[0;33m"
#define CYAN "\033[0;36m"
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

	class SrvError : public std::exception {
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
