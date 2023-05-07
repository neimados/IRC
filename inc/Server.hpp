/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:09 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/08 00:21:15 by dvergobb         ###   ########.fr       */
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
#include "Channel.hpp"

// Const define
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
	void	disconnectUser(User *user, int fd);
	void	execCmd(User *user, std::string cmd, int fd);
	void	sendToUser(User *user, std::string msg);
	void	sendUserInChan(User *user, std::string chan);
	void	sendAllUsersInChan(std::string chan, std::string msg);
	void	sendPrivMsgInChan(std::string chan, std::string msg, std::string user);
	void	sendToUserInChan(User *user, int code, std::string chan, std::string msg);
	
	
	/* ===== UTILS ===== */
	int checkWritable(int fd);
	int findChan(std::string const name) const;
	int findUser(std::string const name) const;
	
	void	displayWelcome(User *user);
	void	displayWrongPass(User *user);
	void	sendAllIfInChannel(User *user, std::string msg);


	/* ===== GETTERS ===== */
	User	*getUser(int fd);
	
	int	getNumberUsers() const;

	std::string	getTime() const;
	std::string	getPassword() const;


	/* ===== COMMANDS ===== */
	void cmdList(User *user, std::string cmd);
	void cmdPass(User *user, std::string cmd, int fd);
	void cmdNick(User *user, std::string cmd);
	void cmdUser(User *user, std::string cmd);
	void cmdQuit(User *user, std::string cmd, int fd);
	void cmdJoin(User *user, std::string cmd);
	void cmdPing(User *user, std::string cmd);
	void cmdCap(User *user, std::string cmd);
	void cmdTopic(User *user, std::string cmd);
	void cmdNames(User *user, std::string cmd);
	void cmdPart(User *user, std::string cmd);
	void cmdInvite(User *user, std::string cmd);
	void cmdKick(User *user, std::string cmd);
	void cmdPrivmsg(User *user, std::string cmd);
	void cmdNotice(User *user, std::string cmd);
	void cmdMode(User *user, std::string cmd);
	

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
	std::vector<Channel>	_channels;

	static const int								_maxFds = 10;
};
