/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:09 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/22 17:55:58 by dvergobb         ###   ########.fr       */
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
#define MAX_CLIENTS 10
#define MAX_BUFFER 1024

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
	Server(int port, std::string password);
	~Server();


	/* ===== BASICS ===== */
	void	startSrv();
	void	addUser();
	void	parseCmd(int fd);
	void	disconnectUser(User *user);
	void	execCmd(User *user, std::string cmd);
	void	sendToUser(User *user, std::string msg);
	
	
	/* ===== UTILS ===== */
	int checkWritable(int fd);
	
	void	displayWelcome(User *user);
	void	displayWrongPass(User *user);


	/* ===== GETTERS ===== */
	User	*getUser(int fd);
	
	int	getNumberUsers() const;

	std::string	getTime() const;
	std::string	getPassword() const;


	/* ===== COMMANDS ===== */
	void cmdList(User *user, std::string cmd);
	void cmdPass(User *user, std::string cmd);
	void cmdNick(User *user, std::string cmd);
	void cmdUser(User *user, std::string cmd);
	void cmdQuit(User *user, std::string cmd);
	void cmdJoin(User *user, std::string cmd);
	void cmdPing(User *user, std::string cmd);
	void cmdCap(User *user, std::string cmd);
	void cmdNames(User *user, std::string cmd);
	

	/* ===== ERRORS ===== */
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
	int								_fdSrv;
	int								_port;
	int								_nbUsers;
	std::string				_Port;
	std::string				_password;
	std::vector<User>		_usrs;
	std::vector<User>		_chans; // Update User to Chanel

};
