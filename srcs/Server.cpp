/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/15 00:45:30 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::Server(int port) {
	_fds = new struct pollfd[10];
	_fdSrv = 0;
	_port = port;

	std::stringstream ss;
    ss << _port;
	
	_Port = ss.str();
	
	int yes = 1;

	struct addrinfo hint, *serverinfo, *tmp;

	memset(&hint, 0, sizeof(hint));

	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = getprotobyname("TCP")->p_proto;

	if (getaddrinfo("0.0.0.0", _Port.c_str(), &hint, &serverinfo) != 0) {
		throw SrvError();
	}

	for (tmp = serverinfo; tmp != NULL; tmp = tmp->ai_next)
	{
		_fdSrv= socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);

		if (_fdSrv < 0)
			continue;

		setsockopt(_fdSrv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if (bind(_fdSrv, tmp->ai_addr, tmp->ai_addrlen) < 0)
		{
			close(_fdSrv);
			continue;
		}
		break;
	}

	freeaddrinfo(serverinfo);

	if (listen(_fdSrv, 10) == -1)
		throw SrvError();

	_fds[0].fd = _fdSrv;
	_fds[0].events = POLLIN;
}

Server::~Server(){
	delete[] _fds;
	close(_fdSrv);
}

void Server::startSrv() {
	int pollTest = 0;
	int fdTest = 0;
	struct sockaddr_storage	remote;
	socklen_t				addrlen;

	std::cout << GREEN << ITALIC << "Listening on port " << BOLD << _Port << RESET << std::endl;
	std::cout << "Try `" << BOLD << "nc 127.0.0.1 " << _Port << "`" << RESET << std::endl;
	
	// Boucle principale
	while (1) {
		std::cout << std::endl << CYAN << getNumberUsers() << " connected." << BOLD << MAGENTA << " Waiting for new connection..." << RESET << std::endl;
		std::cout << "\033[1A\033[2K";
		
		pollTest = poll(_fds, 1, -1);

		if (pollTest == -1)
			throw SrvError();

		addrlen = sizeof remote;
		fdTest = accept(_fds[0].fd, (struct sockaddr*)&remote, &addrlen);
		
		if (fdTest == -1)
			throw SrvError();
		
		// close(fdTest);

		struct sockaddr_in* s = (struct sockaddr_in*)&remote;
		char ip_str[INET_ADDRSTRLEN];

		std::cout << BLUE << BOLD << "New connection" << RESET;
		std::cout << " from " << UNDERLINE << inet_ntop(AF_INET, &(s->sin_addr), ip_str, INET_ADDRSTRLEN) << RESET;
		std::cout << " on socket " << CYAN << BOLD << fdTest << RESET << std::endl;

		struct pollfd client;
		client.fd = fdTest;
		client.events = POLLIN | POLLOUT;
		client.revents = 0;

		User* user = new User(client, client.fd);
		
		user->setNickname("Anonymous");
		
		this->addUser(*user);
		
		if (!(user->getVerification())) {
			if (checkWritable(user->getFd())) {
				send(user->getFd(), WELCOME, 27, 0);
			}
		}
	}
}

void Server::addUser(const User& user) {
	std::cout << "Adding User " << user.getFd() << " aka `" << user.getNickname() << "` to the server." << std::endl;
	_usrs.push_back(user);
}

int Server::getNumberUsers() const {
	return _usrs.size();
}

int Server::checkWritable(int fd) {
	struct pollfd pfd;
	
	pfd.fd = fd;
	pfd.events = POLLOUT;
	pfd.revents = 0;

	int ret = poll(&pfd, 1, 0);

	if (ret < 0) {
		std::cout << RED << BOLD << "Error POLLOUT for User " << fd << RESET << std::endl;
		return 0;
	} else if (ret == 0) {
		std::cout << RED << BOLD << "Fd " << fd << " for User not ready for writing." << RESET << std::endl;
	}

	if (pfd.revents & POLLOUT)
		return 1;

	return 0;
}

const char* Server::SrvError::what() const throw(){
	return "Server Error !";
}
