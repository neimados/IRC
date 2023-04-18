/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/18 13:31:37 by dvergobb         ###   ########.fr       */
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
		std::cout << std::endl << CYAN << _nbUsers - 1 << " connected." << BOLD << MAGENTA << " Waiting for new connection..." << RESET << std::endl << std::endl;
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
	std::stringstream 		ss;

	addrlen = sizeof remote;
	fd = accept(_fdSrv, (struct sockaddr*)&remote, &addrlen);

	if (fd == -1)
		throw SrvError();

	struct sockaddr_in* s = (struct sockaddr_in*)&remote;
	char ip_str[INET_ADDRSTRLEN];

	struct pollfd client;
	client.fd = fd;
	client.events = POLLIN | POLLOUT;
	client.revents = 0;
	_fds[_nbUsers].fd = fd;
	_fds[_nbUsers].events = POLLIN;

	User* user = new User(client, client.fd, fd);

	std::string nickname = "User_";
	ss << fd;
	nickname += ss.str();
		
	user->setNickname(nickname);

	std::cout << std::endl <<  BLUE << BOLD << "New connection" << RESET << " of " << ITALIC << CYAN << user->getNickname() << RESET;
	std::cout << " from " << UNDERLINE << inet_ntop(AF_INET, &(s->sin_addr), ip_str, INET_ADDRSTRLEN) << RESET;
	std::cout << " on socket " << CYAN << BOLD << fd << RESET << std::endl;

	_usrs.push_back(*user);
	_nbUsers++;

	std::string msg = ":";
	
	msg += "IRC";
	msg += " 001 ";
	msg += nickname;
	msg += " :Welcome ";
	msg += nickname;
	msg += " to serv IRC\n\r";

	const void *cmsg = msg.c_str();

	if (!(user->getVerification())) {
		if (checkWritable(user->getFd())) {
			send(user->getFd(), cmsg, msg.size(), 0);
			// user->sendPrompt();
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
		std::cout <<  RED << BOLD << "Error POLLOUT for User " << fd << RESET << std::endl;
		return 0;
	} else if (ret == 0) {
		std::cout <<  RED << BOLD << "Fd " << fd << " for User not ready for writing." << RESET << std::endl;
	}

	if (pfd.revents & POLLOUT)
		return 1;

	return 0;
}

void	Server::cmdUser(int fd){
	char						buf[MAX_BUFFER];
	std::vector<User>::iterator it;

	User 						*user;
	std::string					nickname;
	int							clientFd;
	int							socket;
	
	user = this->getUser(_fds[fd].fd);
	if (!user)
		throw SrvErrorClient();

	clientFd = user->getFd();
	socket = user->getSocket();
	nickname = user->getNickname();

	// send(clientFd, PROMPT, 6, 0);

	if (recv(clientFd, buf, MAX_BUFFER, 0) <= 0) {
		// Connection closed
		std::cout << std::endl <<  YELLOW << "Client " << BOLD << nickname << NORMAL " on socket " << ITALIC << socket << RESET << YELLOW << " disconnected!" << RESET << std::endl;
		close(clientFd);
		
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
		std::string cmd(buf);
		std::string delimiter = "\n";
		
		cmd = cmd.substr(0, cmd.size() - 1);

		size_t pos = 0;
		std::string token;
		while ((pos = cmd.find(delimiter)) != std::string::npos) {
			token = cmd.substr(0, pos);
			execCmd(user, token);
			cmd.erase(0, pos + delimiter.length());
		}
	}
	
	memset(&buf, 0, MAX_BUFFER); // Reset du buffer
}

User *Server::getUser(int fd) {
	std::vector<User>::iterator it;

	it = _usrs.begin();
	while (it != _usrs.end()) {
		if (it->getFd() == fd)
			return &(*it);
		it++;
	}
	return NULL;
}

std::string Server::getTime() const {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[8];
	
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (buffer,8,"%Hh%Mm%S",timeinfo);
	
	std::string str(buffer);
	str += " ";
	return str;
}


// ========= Supported commands =========
void Server::execCmd(User *user, std::string cmd) {
	std::cout <<  BOLD << user->getNickname() << ": " << RESET << cmd <<std::endl;
	if (cmd.substr(0, 4) == "NICK") {
		this->cmdNick(user, cmd);
		// user->sendPrompt();
	} else if (cmd.substr(0, 4) == "USER") {
		this->cmdUser(user, cmd);
		// user->sendPrompt();
	} else if (cmd.substr(0, 3) == "CAP") {
		std::cout <<  CYAN << ITALIC << "CAP command called." << RESET << std::endl << std::endl;
	} else if (cmd.substr(0, 3) == "PING") {
		std::cout <<  CYAN << ITALIC << "PING command called." << RESET << std::endl << std::endl;
	} else if (cmd.substr(0, 4) == "LIST") {
		std::cout <<  CYAN << ITALIC << "Showing users and channels." << RESET << std::endl << std::endl;
		this->cmdList(user);
		user->sendPrompt();
	} else if (cmd.substr(0, 4) == "QUIT") {
		std::cout <<  "Command QUIT received from " << BOLD << user->getNickname() << RESET << std::endl;
		std::cout <<  "Leaving server..." << std::endl;
		
		// Connection closed
		// std::cout << std::endl <<  YELLOW << "Client " << BOLD << nickname << NORMAL " on socket " << ITALIC << socket << RESET << YELLOW << " disconnected!" << RESET << std::endl;
		// close(user->getFd());
	} else {
		std::cout <<  RED << BOLD << "Command not found." << RESET << std::endl << std::endl;
		send(user->getFd(), USE_HELP, sizeof(USE_HELP), 0);
		user->sendPrompt();
	}
}

void Server::cmdNick(User *user, std::string cmd) {
	for (size_t i = 0; i < cmd.size(); i++) {
		if (cmd[i] == ' ') {
			cmd.erase(i, 1);
			i--;
		}
		else
			break;
	}

	for (int i = cmd.size() - 1; i >= 0; i--) {
		if (cmd[i] == ' ') {
			cmd.erase(i, 1);
			i--;
		}
		else
			break;
	}

	if (cmd.size() <= 4) {
		std::cout <<  RED << BOLD << "<nickname> not found." << RESET << std::endl << std::endl;
		send(user->getFd(), NICKNAME_NOT_FOUND, sizeof(NICKNAME_NOT_FOUND), 0);
		user->sendPrompt();
		return;
	}
	
	std::string new_nickname = cmd.substr(5, cmd.size() - 5);

	// Remove spaces from new_nickname
	for (size_t i = 0; i < new_nickname.size(); i++) {
		if (new_nickname[i] == ' ' || new_nickname[i] == '\t' || new_nickname[i] == '\n' || new_nickname[i] == '\r') {
			new_nickname.erase(i, 1);
			i--;
		}
	}

	if (new_nickname == user->getNickname()) {
		std::cout <<  ORANGE << ITALIC << user->getNickname() << RESET << " is still " << CYAN << BOLD << new_nickname << RESET << std::endl << std::endl;
		return;
	}

	if (new_nickname.size() == 0) {
		std::cout <<  RED << BOLD << "Nickname is not valid." << RESET << std::endl << std::endl;
		send(user->getFd(), NICKNAME_FORMAT, sizeof(NICKNAME_FORMAT), 0);
		user->sendPrompt();
		new_nickname.clear();
		return;
	}

	// Check if nickname contains only letters, numbers and underscores
	for (size_t i = 0; i < new_nickname.size(); i++) {
		if (!isalnum(new_nickname[i]) && new_nickname[i] != '_') {
			std::cout <<  RED << BOLD << "Nickname contains invalid characters : '" << new_nickname[i] << "'" << RESET << std::endl << std::endl;
			
			send(user->getFd(), NICKNAME_FORMAT, sizeof(NICKNAME_FORMAT), 0);
			
			user->sendPrompt();
			new_nickname.clear();
			return;
		}
	}

	if (new_nickname.size() == 0) {
		std::cout <<  RED << BOLD << "Nickname too short!" << RESET << std::endl << std::endl;
		send(user->getFd(), NICKNAME_NOT_FOUND, sizeof(NICKNAME_NOT_FOUND), 0);
	} else if (new_nickname.size() > 9) {
		std::cout <<  RED << BOLD << "Nickname too long!" << RESET << std::endl << std::endl;
		send(user->getFd(), NICKNAME_TOO_LONG, sizeof(NICKNAME_TOO_LONG), 0);
	}
	else {
		// Check if nickname is already taken
		std::vector<User>::iterator it = _usrs.begin();
		while (it != _usrs.end()) {
			if (it->getNickname() == new_nickname) {
				std::cout << RED << BOLD << "Nickname already taken." << RESET << std::endl << std::endl;
				send(user->getFd(), NICKNAME_ALREADY_USED, sizeof(NICKNAME_ALREADY_USED), 0);
				return;
			}
			it++;
		}

		std::cout <<  ORANGE << ITALIC << user->getNickname() << RESET << " is now " << CYAN << BOLD << new_nickname << RESET << std::endl << std::endl;
		user->setNickname(new_nickname);
	}
			
}

void Server::cmdUser(User *user, std::string cmd) {
	(void)user;
	(void)cmd;
	std::cout <<  CYAN << ITALIC << "USER command called." << RESET << std::endl << std::endl;
}

void Server::cmdHelp(User *user) {
	std::string HELP = "";

	HELP += "\n\r";
	HELP += BLUE;
	HELP += BOLD;
	HELP += "Available commands:";
	HELP += RESET;

	HELP += "\n\r";
	HELP += BOLD;
	HELP += "  /NICK <nickname> : ";
	HELP += RESET;
	HELP += "Change your nickname";

	HELP += "\n\r";
	HELP += BOLD;
	HELP += "  /JOIN <channel> : ";
	HELP += RESET;
	HELP += "Join a channel";

	HELP += "\n\r";
	HELP += BOLD;
	HELP += "  /MSG <message> : ";
	HELP += RESET;
	HELP += "Send a message to the channel";

	HELP += "\n\r";
	HELP += BOLD;
	HELP += "  /LIST : ";
	HELP += RESET;
	HELP += "List channels and users";

	HELP += "\n\r";
	HELP += BOLD;
	HELP += "  /PART : ";
	HELP += RESET;
	HELP += "Leave the channel";

	HELP += "\n\r";
	HELP += BOLD;
	HELP += "  /QUIT : ";
	HELP += RESET;
	HELP += "Leave the server";

	HELP += "\n\r";
	HELP += BOLD;
	HELP += "  /HELP : ";
	HELP += RESET;
	HELP += "Display this help";
	
	
	HELP += "\n\r";
	HELP += "\n\r";
	// Send the help message to the client
	send(user->getFd(), (void *)HELP.c_str(), HELP.size(), 0);
}

void Server::cmdList(User *user) {
	std::string LIST = "";

	LIST += "\n\r";
	LIST += BLUE;
	LIST += BOLD;
	LIST += "Users:\n\r";
	LIST += RESET;

	std::vector<User>::iterator it = _usrs.begin();
	while (it != _usrs.end()) {
		LIST += "  - ";
		LIST += it->getNickname();
		LIST += "\n\r";
		it++;
	}

	LIST += "\n\r";
	LIST += "\n\r";

	LIST += ORANGE;
	LIST += BOLD;
	LIST += "Channels: none\n\r";
	LIST += RESET;

	// std::vector<User>::iterator it = _usrs.begin();
	// while (it != _usrs.end()) {
	// 	LIST += "  - ";
	// 	LIST += it->getNickname();
	// 	LIST += "\n\r";
	// 	it++;
	// }

	LIST += "\n\r";
	
	// Send the list to the client
	send(user->getFd(), (void *)LIST.c_str(), LIST.size(), 0);
}
// ========= Exceptions ==========

const char* Server::SrvError::what() const throw(){
	return "\033[1;31mServer error.\033[0m";
}

const char* Server::SrvErrorClient::what() const throw(){
	// Return the error message in bold red
	return "\033[1;31mError: user not found.\033[0m";
}
