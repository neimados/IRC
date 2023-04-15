/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/15 22:08:55 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::Server(int port) {
	_fds = new struct pollfd[10];
	_fdSrv = 0;
	_nbUsers = 1;
	_port = port;

	std::stringstream ss;
    ss << _port;
	
	_Port = ss.str();
	
	int optval = 1;

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
		_fdSrv = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);

		if (_fdSrv < 0)
			continue;

		setsockopt(_fdSrv, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		if (bind(_fdSrv, tmp->ai_addr, tmp->ai_addrlen) < 0)
		{
			close(_fdSrv);
			continue;
		}
		break;
	}

	freeaddrinfo(serverinfo);

	if (listen(_fdSrv, 10) == -1 || !tmp)
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

	std::cout << GREEN << ITALIC << "Listening on port " << BOLD << _Port << RESET << std::endl;
	std::cout << "Try `" << BOLD << "nc 127.0.0.1 " << _Port << "`" << RESET << std::endl << std::endl;
	
	// Boucle principale
	while (1) {
		std::cout << std::endl << CYAN << _nbUsers - 1 << " connected." << BOLD << MAGENTA << " Waiting for new connection..." << RESET << std::endl;
		std::cout << "\033[1A\033[2K";
		std::cout << "\033[1A\033[2K";
		
		pollTest = poll(_fds, _nbUsers, -1);

		if (pollTest == -1)
			throw SrvError();
		for (int i = 0; i < _nbUsers; i++) {
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _fdSrv)
					addUser(); // New user
				else
					cmdUser(i); // recup la commande de l'user deja chez nous
			}
		}
	}
}

void Server::addUser() {
	struct sockaddr_storage	remote;
	socklen_t				addrlen;
	int						fd;

	addrlen = sizeof remote;
	fd = accept(_fdSrv, (struct sockaddr*)&remote, &addrlen);

	if (fd == -1)
		throw SrvError();

	struct sockaddr_in* s = (struct sockaddr_in*)&remote;
	char ip_str[INET_ADDRSTRLEN];

	std::cout << std::endl << BLUE << BOLD << "New connection" << RESET;
	std::cout << " from " << UNDERLINE << inet_ntop(AF_INET, &(s->sin_addr), ip_str, INET_ADDRSTRLEN) << RESET;
	std::cout << " on socket " << CYAN << BOLD << fd << RESET << std::endl;

	struct pollfd client;
	client.fd = fd;
	client.events = POLLIN | POLLOUT;
	client.revents = 0;
	_fds[_nbUsers].fd = fd;
	_fds[_nbUsers].events = POLLIN;

	User* user = new User(client, client.fd);
		
	user->setNickname("Anonymous");

	std::cout << "Adding User " << user->getFd() << " aka `" << user->getNickname() << "` to the server." << std::endl << std::endl;
	_usrs.push_back(*user);
	_nbUsers++;

	if (!(user->getVerification())) {
		if (checkWritable(user->getFd())) {
			send(user->getFd(), WELCOME, 27, 0);
			send(user->getFd(), PROMPT, 6, 0);
		}
	}
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

void	Server::cmdUser(int fd){
	int							len = 1024;
	int							client;
	char						buf[len];
	std::vector<User>::iterator it;

	client = _fds[fd].fd;
	send(client, PROMPT, 6, 0);

	if (recv(client, buf, len, 0) <= 0) {
		// Connection closed
		std::cout << std::endl << YELLOW << "Client " << BOLD << client << NORMAL " disconnected!" << RESET << std::endl;
		close(client);
		
		_fds[fd] = _fds[_nbUsers -1];
		it = _usrs.begin();
		
		while (it != _usrs.end()){
			if (it->getFd() == fd)
				_usrs.erase(it);
			it++;
		}
		_nbUsers--;
	}
	else {
		std::string cmd(buf, strlen(buf) - 1);
		std::cout << BOLD "User " << client << ": " << RESET <<cmd<<std::endl;
		//on parse la commande et on execute
		//a refaire, je recupere que 1024 mais faudra faire un truc genre gnl
	}
	memset(&buf, 0, len);//reset du buffer

}

const char* Server::SrvError::what() const throw(){
	return "Server Error !";
}
