/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/31 11:20:36 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Server::Server(int port, std::string password) {
	for (int i = 0; i < 10; i++){
		_fds.push_back(pollfd());
	}

	_fdSrv = 0;
	_nbUsers = 1;
	_port = port;
	_password = password;

	std::stringstream ss;
	ss << _port;
	
	_Port = ss.str();
	
	int							optval = 1;
	struct addrinfo	hint, *serverinfo, *tmp;
	memset(&hint, 0, sizeof(hint));

	hint.ai_family		= AF_INET;
	hint.ai_socktype	= SOCK_STREAM;
	hint.ai_protocol	= getprotobyname("TCP")->p_proto;

	if (getaddrinfo("0.0.0.0", _Port.c_str(), &hint, &serverinfo) != 0)
		throw SrvError();

	for (tmp = serverinfo; tmp != NULL; tmp = tmp->ai_next) {
		_fdSrv = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);

		if (_fdSrv < 0)
			continue;

		setsockopt(_fdSrv, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		if (bind(_fdSrv, tmp->ai_addr, tmp->ai_addrlen) < 0) {
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
	close(_fdSrv);
}

/* ===== BASICS ===== */

void Server::startSrv() {
	int pollTest = 0;

	std::cout << GREEN << ITALIC << "Listening on port " << BOLD << _Port << RESET << std::endl;
	std::cout << "Try `" << BOLD << "nc 127.0.0.1 " << _Port << "`" << RESET << std::endl << std::endl;
	// Boucle principale
	while (1) {
		// Print message
		std::cout << std::endl << CYAN << _nbUsers - 1 << " connected." << BOLD << MAGENTA << " Waiting for new connection..." << RESET << std::endl << std::endl;
		
		// Clear the message to print the new one
		std::cout << "\033[1A\033[2K";
		std::cout << "\033[1A\033[2K";
		
		pollTest = poll(&_fds[0], _nbUsers, -1);
		if (pollTest == -1) {
			// Closing Server
			return;
		}

		for (int i = 0; i < _nbUsers; i++) {
			if (_fds[i].revents & POLLIN) {
				if (_fds[i].fd == _fdSrv) {

					if ((int)this->_usrs.size() >= this->_maxFds) {
						struct sockaddr_storage	remote;
						socklen_t								addrlen;
						
						int fd = accept(_fdSrv, (struct sockaddr*)&remote, &addrlen);

						if (fd == -1)
							throw SrvError();
							
						std::cout << RED << BOLD << "Too many clients connected." << RESET << std::endl;
						
						std::string msg = "ERROR :Too many clients connected";
						send(fd, msg.c_str(), msg.length(), 0);
						close(fd);
					}
					else {
						addUser(); // Add user to the server
					}
				} else
					parseCmd(i); // Parse the command of the user
			}
		}
	}
}

void Server::addUser() {
	struct sockaddr_storage	remote;
	socklen_t								addrlen;
	std::stringstream				ss;
	int											fd;
	
	struct sockaddr_in*			s = (struct sockaddr_in*)&remote;
	struct pollfd						client;
	char										ip_str[INET_ADDRSTRLEN];

	addrlen = sizeof remote;
	fd = accept(_fdSrv, (struct sockaddr*)&remote, &addrlen);

	if (fd == -1)
		throw SrvError();

	client.fd = fd;
	client.events = POLLIN | POLLOUT;
	client.revents = 0;
	_fds[_nbUsers].fd = fd;
	_fds[_nbUsers].events = POLLIN;

	User user = User(client, client.fd, fd);

	std::string nickname = "user";
	ss << fd;
	nickname += ss.str();
	user.setNickname(nickname);
	user.setUsername(nickname);

	std::cout << std::endl << BLUE << BOLD << "New connection" << RESET << " of " << ITALIC << CYAN << user.getNickname() << RESET;
	std::cout << " from " << UNDERLINE << inet_ntop(AF_INET, &(s->sin_addr), ip_str, INET_ADDRSTRLEN) << RESET;
	std::cout << " on socket " << CYAN << BOLD << fd << RESET << "." << std::endl;
	_usrs.push_back(user);

	// Update pollfd
	_nbUsers++;

}

void	Server::parseCmd(int fd){
	char						buf[MAX_BUFFER];
	std::vector<User>::iterator it;
	(void) it;

	User 						*user;
	std::string			nickname;
	int							clientFd;
	
	user = this->getUser(_fds[fd].fd);

	if (!user)
		throw SrvErrorClient();

	clientFd = user->getFd();
	nickname = user->getNickname();

	if (recv(clientFd, buf, MAX_BUFFER, 0) <= 0) {
		// Connection closed by client
		disconnectUser(user, fd);
	} else {
		// Command received from client
		std::string cmd(buf);

		// Ignore if the command is empty
		if (cmd.size() == 0)
			return;

		if ( cmd == "\n" || cmd == "\r" || cmd == "\r\n") {
			// Execute the command in the buffer
			std::string cmdTmp = user->getBuffer();
			user->setBuffer("");

			std::cout << YELLOW << ITALIC << "Execute command `" << cmdTmp << "`." << RESET << NORMAL << std::endl;
			execCmd(user, cmdTmp, fd);
		} else if (cmd.find("\n") == std::string::npos && cmd.find("\r") == std::string::npos) {
			// Check if last char is a printable char
			if (!isprint(cmd[cmd.size() - 2])) {
				// Remove the last char
				cmd.erase(cmd.size() - 2);
			}

			// Command with `ctrl + D`
			std::cout << CYAN << ITALIC << "Added `" << cmd << "` to user's buffer (ctrl +D)." << RESET << NORMAL << std::endl;
			
			user->setBuffer(user->getBuffer() + cmd);
			cmd = user->getBuffer();
			
			std::cout << YELLOW << ITALIC << "Buffer is now `" << cmd << "`." << RESET << NORMAL << std::endl;		
		} else if (cmd.find("\n") != std::string::npos) {
			// Valid command
			std::string delimiter = "\n";
			size_t pos = 0;
			std::string token;
			
			while ((pos = cmd.find(delimiter)) != std::string::npos) {
				token = cmd.substr(0, pos);
				execCmd(user, token, fd);
				cmd.erase(0, pos + delimiter.length());
			}
		} else {
			execCmd(user, cmd, fd);
		}
	}
	memset(&buf, 0, MAX_BUFFER); // Reset du buffer
}

void	Server::disconnectUser(User *user, int fd) {
	std::vector<User>::iterator itUser;
	std::vector<std::string> chans = user->getAllChans();

	// A loop to send PART command to all channels and delete the user
	for (std::vector<std::string>::iterator it = chans.begin(); it != chans.end(); it++) {
		cmdPart(user, "PART " + *it);
	}

	
	// A loop to find the user in the vector and delete it
	for (itUser = _usrs.begin(); itUser != _usrs.end(); itUser++) {
		if (itUser->getFd() == user->getFd()) {
			// Close the socket
			_fds[fd].fd = _fds[_nbUsers - 1].fd;
			close(user->getFd());
			_usrs.erase(itUser);
			break;
		}
	}

	
	// Update the pollfd array
	_nbUsers--;
	
	// Display the disconnection
	std::cout << std::endl << YELLOW << "Client " << BOLD << user->getNickname() << NORMAL;
	std::cout << " on socket " << ITALIC << user->getSocket() << RESET << YELLOW << " disconnected." << RESET << std::endl;
}

void Server::execCmd(User *user, std::string cmd, int fd) {
	// Check is user's buffr is empty
	if (user->getBuffer().size() > 0) {
		// Append the command to the buffer
		cmd = user->getBuffer() + cmd;
		user->setBuffer("");
	}

	std::cout << BOLD << user->getNickname() << ": " << RESET << cmd <<std::endl;

	// Check if the user is logged in
	if (user->getPassOk() == false && cmd.substr(0, 4) != "PASS") {
		user->sendToUser(ERR_NOLOGIN(user->getNickname()));
		return;
	}

	// Check is the user is registered
	if (user->getIsRegistered() == false && cmd.substr(0, 4) != "NICK" && cmd.substr(0, 4) != "USER" && cmd.substr(0, 3) != "CAP" && cmd.substr(0, 4) != "PASS") {
		user->sendToUser(ERR_NOTREGISTERED(user->getNickname()));
		return;
	}
	
	if (cmd.substr(0, 4) == "PASS") {
		//check password. If ok getPassOk() = true
		this->cmdPass(user, cmd);
	} else if (cmd.substr(0, 4) == "NICK") {
		this->cmdNick(user, cmd);
	} else if (cmd.substr(0, 4) == "USER") {
		this->cmdUser(user, cmd);
	} else if (cmd.substr(0, 3) == "CAP") {
		std::cout << "Command CAP received from " << BOLD << user->getNickname() << RESET << std::endl;
	} else if (cmd.substr(0, 4) == "PING") {
		std::cout << "Command PING received from " << BOLD << user->getNickname() << RESET << std::endl;
	} else if (cmd.substr(0, 4) == "LIST") {
		this->cmdList(user, cmd);
	} else if (cmd.substr(0, 4) == "QUIT") {
		this->cmdQuit(user, cmd, fd);
	} else if (cmd.substr(0, 4) == "JOIN") {
		this->cmdJoin(user, cmd);
	} else if (cmd.substr(0, 5) == "NAMES") {
		this->cmdNames(user, cmd);
	} else if (cmd.substr(0, 4) == "PART") {
		this->cmdPart(user, cmd);
	} else if (cmd.substr(0, 5) == "TOPIC") {
		this->cmdTopic(user, cmd);
	} else if (cmd.substr(0, 4) == "KICK") {
		this->cmdKick(user, cmd);
	} else if (cmd.substr(0, 7) == "PRIVMSG") {
		this->cmdPrivmsg(user, cmd);
	} else if (cmd.substr(0, 6) == "NOTICE") {
		this->cmdNotice(user, cmd);
	} else if (cmd.substr(0, 4) == "MODE") {
		this->cmdMode(user, cmd);
	} else {
		std::cout << RED << BOLD << "Command not found." << RESET << std::endl << std::endl;
		user->sendToUser(ERR_UNKNOWNCOMMAND(user->getNickname()));
	}
}

/* ===== UTILS ===== */

int Server::findChan(std::string const name) const {
	for (size_t i = 0; i < _channels.size(); i++) {
		if (_channels[i].getName() == name)
			return i;
	}
	return -1;
}

int Server::findUser(std::string const name) const {
	for (size_t i = 0; i < _usrs.size(); i++)
	{
		if(_usrs[i].getNickname() == name)
			return i;
	}
	return -1;
}

/* ===== GETTERS ===== */
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

std::string Server::getPassword() const {
	return (this->_password);
}

/* ===== ERRORS ===== */

const char* Server::SrvError::what() const throw(){
	return "\033[1;31mServer error.\033[0m";
}

const char* Server::SrvErrorClient::what() const throw(){
	// Return the error message in bold red
	return "\033[1;31mError: user not found.\033[0m";
}
