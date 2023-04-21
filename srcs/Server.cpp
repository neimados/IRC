/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/21 19:23:34 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::Server(int port, std::string password) {
	_fds = new struct pollfd[10];
	_fdSrv = 0;
	_nbUsers = 1;
	_port = port;
	_password = password;

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
	std::cout << " on socket " << CYAN << BOLD << fd << RESET << " at " << this->getTime() << std::endl;

	_usrs.push_back(*user);
	_nbUsers++;

	std::string msg = ":";
	
	msg = ":IRC 001 ";
	msg += nickname;
	msg += " :Welcome ";
	msg += nickname;
	msg += " to serv IRC\n\r";

	const void *cmsg = msg.c_str();

	if (!(user->getVerification())) {
		if (checkWritable(user->getFd())) {
			send(user->getFd(), cmsg, msg.size(), 0);
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

void	Server::disconnectUser(User *user) {
	std::vector<User>::iterator it;

	for (it = _usrs.begin(); it != _usrs.end(); it++) {
		if (it->getFd() == user->getFd()) {
			close(user->getFd());
			_usrs.erase(it);
			break;
		}
	}

	_nbUsers--;
	std::cout << std::endl <<  YELLOW << "Client " << BOLD << user->getNickname() << NORMAL;
	std::cout << " on socket " << ITALIC << user->getSocket() << RESET << YELLOW << " disconnected at " << this->getTime() << RESET << std::endl;
}

void	Server::cmdUser(int fd){
	char						buf[MAX_BUFFER];
	std::vector<User>::iterator it;

	User 						*user;
	std::string			nickname;
	int							clientFd;
	
	user = this->getUser(_fds[fd].fd);

	if (!user)
		throw SrvErrorClient();

	clientFd = user->getFd();
	nickname = user->getNickname();

	if (recv(clientFd, buf, MAX_BUFFER, 0) <= 0) {
		// Connection closed
		disconnectUser(user);
	}
	else {
		std::string cmd(buf);

		if (cmd.find("\n") != std::string::npos) {
			std::string delimiter = "\n";
			size_t pos = 0;
			std::string token;
			
			// cmd = cmd.substr(0, cmd.size() - 1);
			
			while ((pos = cmd.find(delimiter)) != std::string::npos) {
				token = cmd.substr(0, pos);
				execCmd(user, token);
				cmd.erase(0, pos + delimiter.length());
			}
		} else {
			execCmd(user, cmd);
		}
	}
	
	memset(&buf, 0, MAX_BUFFER); // Reset du buffer
}

void Server::sentUser(User *user, std::string msg) {
	std::string start = ":IRC 001 ";
	start += user->getNickname();
	start += " :";

	msg = start + msg + "\n\r";
	
	const void *cmsg = msg.c_str();

	if (!(user->getVerification())) {
		if (checkWritable(user->getFd())) {
			send(user->getFd(), cmsg, msg.size(), 0);
		}
	}
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
	str.erase(str.size() - 1);
	return str;
}

std::string Server::getPassword() const {
	return (this->_password);
}


// ========= Supported commands =========
void Server::execCmd(User *user, std::string cmd) {
	bool passOk = user->getPassOk();
	std::cout <<  BOLD << user->getNickname() << ": " << RESET << cmd <<std::endl;
	
	if (cmd.substr(0, 4) == "PASS") {
		//check password. If ok getPassOk() = true
		this->cmdPass(user, cmd);
	} else if (passOk == false) {
		// Deconnect user
		sentUser(user, "Wrong password");
		close(user->getFd());
	} else if (cmd.substr(0, 4) == "NICK") {
		this->cmdNick(user, cmd);
	} else if (cmd.substr(0, 4) == "USER") {
		this->cmdUser(user, cmd);
	} else if (cmd.substr(0, 3) == "CAP") {
		std::cout <<  CYAN << ITALIC << "CAP command called." << RESET << std::endl << std::endl;
	} else if (cmd.substr(0, 4) == "PING") {
		std::cout <<  CYAN << ITALIC << "PING command called." << RESET << std::endl << std::endl;
		sentUser(user, "PONG");
	} else if (cmd.substr(0, 4) == "LIST") {
		std::cout <<  CYAN << ITALIC << "Showing users and channels." << RESET << std::endl << std::endl;
		this->cmdList(user);
	} else if (cmd.substr(0, 4) == "QUIT") {
		std::string msg = "";
		
		if (cmd.size() > 6)
			msg = cmd.substr(6);
		
		std::cout <<  "Command QUIT received from " << BOLD << user->getNickname() << RESET << " with message: " << ITALIC << msg << RESET << std::endl;
	} else if (cmd.substr(0, 4) == "JOIN") {
		std::cout <<  CYAN << ITALIC << "JOIN command called." << RESET << std::endl << std::endl;
		sentUser(user, "Join channel `" + cmd.substr(5) + "`. (non en vrai ça ne fait rien)");
	} else {
		std::cout <<  RED << BOLD << "Command not found." << RESET << std::endl << std::endl;
		sentUser(user, "Command not found.");
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
		sentUser(user, NICKNAME_NOT_FOUND);
		// user->sendPrompt();
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
		sentUser(user, NICKNAME_FORMAT);
		// user->sendPrompt();
		new_nickname.clear();
		return;
	}

	// Check if nickname contains only letters, numbers and underscores
	for (size_t i = 0; i < new_nickname.size(); i++) {
		if (!isalnum(new_nickname[i]) && new_nickname[i] != '_') {
			std::cout <<  RED << BOLD << "Nickname contains invalid characters : '" << new_nickname[i] << "'" << RESET << std::endl << std::endl;
			sentUser(user, NICKNAME_FORMAT);
			new_nickname.clear();
			return;
		}
	}

	if (new_nickname.size() == 0) {
		std::cout <<  RED << BOLD << "Nickname too short!" << RESET << std::endl << std::endl;
		sentUser(user, NICKNAME_NOT_FOUND);
	} else if (new_nickname.size() > 9) {
		std::cout <<  RED << BOLD << "Nickname too long!" << RESET << std::endl << std::endl;
		sentUser(user, NICKNAME_TOO_LONG);
	}
	else {
		// Check if nickname is already taken
		std::vector<User>::iterator it = _usrs.begin();
		while (it != _usrs.end()) {
			if (it->getNickname() == new_nickname) {
				std::cout << RED << BOLD << "Nickname already taken." << RESET << std::endl << std::endl;
				sentUser(user, NICKNAME_ALREADY_USED);
				return;
			}
			it++;
		}

		std::cout <<  ORANGE << ITALIC << user->getNickname() << RESET << " is now " << CYAN << BOLD << new_nickname << RESET << std::endl << std::endl;
		user->setNickname(new_nickname);
		sentUser(user, "Nickname updated.");
	}
			
}

void Server::cmdPass(User *user, std::string cmd) {
	std::vector<User>::iterator it;
	
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
		std::cout <<  RED << BOLD << "Missing password." << std::endl;
		return;
	}
	std::string pass = cmd.substr(5, cmd.size() - 5);
	for (size_t i = 0; i < pass.size(); i++) {
		if (pass[i] == ' ' || pass[i] == '\t' || pass[i] == '\n' || pass[i] == '\r') {
			pass.erase(i, 1);
			i--;
		}
	}
	
	user->setPassword(pass);
	
	if (pass == this->getPassword())
		user->setPassOk(true);
	else {
		sentUser(user, "Wrong password.");
		std::cout <<  RED << BOLD << "Wrong password." << RESET << std::endl;
		disconnectUser(user);
	}
}

void Server::cmdUser(User *user, std::string cmd) {
	(void)user;
	(void)cmd;
	std::cout <<  CYAN << ITALIC << "USER command called." << RESET << std::endl << std::endl;
	sentUser(user, "User updated.");
}

void Server::cmdList(User *user) {
	std::string usrs = "Users: ";
	std::string chans = "Channels: ";

	std::vector<User>::iterator it = _usrs.begin();
	std::vector<User>::iterator it2 = _chans.begin();
	
	while (it != _usrs.end()) {
		usrs += it->getNickname();
		
		if (it + 1 != _usrs.end())
			usrs += ", ";
		it++;
	}

	if (_usrs.size() == 0)
		usrs += "none";

	while (it2 != _chans.end()) {
		chans += it2->getNickname(); // Update to get channel name
		
		if (it2 + 1 != _chans.end())
			chans += ", ";
		it2++;
	}

	if (_chans.size() == 0)
		chans += "none";

	sentUser(user, usrs);
	sentUser(user, chans);
}

// ========= Exceptions ==========

const char* Server::SrvError::what() const throw(){
	return "\033[1;31mServer error.\033[0m";
}

const char* Server::SrvErrorClient::what() const throw(){
	// Return the error message in bold red
	return "\033[1;31mError: user not found.\033[0m";
}
