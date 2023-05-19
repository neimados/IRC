/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/12 00:48:28 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Server::Server(int port, std::string password) {
	for (int i = 0; i < 10; i++){
		_fds.push_back(pollfd());
	}
	// _fds = new struct pollfd[10];
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

/* ===== Annexes ===== */

// Remove the \n and spaces from the channel name
std::string clearString(std::string str) {
	for (size_t j = 0; j < str.size(); j++) {
		if (str[j] == ' ' || str[j] == '\t' || str[j] == '\n' || str[j] == '\r') {
			str.erase(j, 1);
			j--;
		}
	}

	return str;
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
		if (pollTest == -1)
			throw SrvError();
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

	std::string nickname = "User_";
	ss << fd;
	nickname += ss.str();
	user.setNickname(nickname);
	user.setUsername(nickname);

	std::cout << std::endl << BLUE << BOLD << "New connection" << RESET << " of " << ITALIC << CYAN << user.getNickname() << RESET;
	std::cout << " from " << UNDERLINE << inet_ntop(AF_INET, &(s->sin_addr), ip_str, INET_ADDRSTRLEN) << RESET;
	std::cout << " on socket " << CYAN << BOLD << fd << RESET << " at " << this->getTime() << std::endl;
	user.setVerification(true);
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
		if (cmd.size() == 0 || cmd == "\n" || cmd == "\r" || cmd == "\r\n")
			return;

		if ((cmd.size() == 1 || cmd.size() > 1) && cmd.substr(0, 5) != "PASS " && cmd[cmd.size() - 1] != '\n') {
			// Command with `ctrl + D`
			std::cout << CYAN << ITALIC << "Added `" << cmd << "` to user's buffer (ctrl +D)." << RESET << NORMAL << std::endl;
			
			user->setBuffer(user->getBuffer() + cmd);
			cmd = user->getBuffer();
			
			std::cout << YELLOW << ITALIC << "Buffer is now `" << cmd << "`." << RESET << NORMAL << std::endl;

			// Check if there is a space in the command
			if (cmd.find(" ") != std::string::npos) {
				// Get the command before the space and check if it is a valid command
				std::string cmdBeforeSpace = cmd.substr(0, cmd.find(" "));

				std::cout << "Searching for command `" << cmdBeforeSpace << "`..." << std::endl << std::endl;
				
				if (cmdBeforeSpace == "PASS" || cmdBeforeSpace == "NICK" || cmdBeforeSpace == "USER" || cmdBeforeSpace == "CAP" || cmdBeforeSpace == "PING" || cmdBeforeSpace == "LIST" || cmdBeforeSpace == "QUIT" || cmdBeforeSpace == "JOIN" || cmdBeforeSpace == "NAMES" || cmdBeforeSpace == "PART" || cmdBeforeSpace == "TOPIC" || cmdBeforeSpace == "INVITE" || cmdBeforeSpace == "KICK" || cmdBeforeSpace == "PRIVMSG" || cmdBeforeSpace == "NOTICE" || cmdBeforeSpace == "MODE") {
					// Valid command
					execCmd(user, "", fd);
				} else {
					// Invalid command
					std::cout << RED << BOLD << "Command `" << cmdBeforeSpace << "` not found." << RESET << std::endl << std::endl;
					sendToUser(user, "Command not found in buffer mode.");
				}
			}
		
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
	
	//pour quitter tous les channels
	std::vector<Channel>::iterator itChan = _channels.begin();
	while (itChan != _channels.end()){
		if (itChan->delUsr(user))
			sendAllUsersInChan(itChan->getName(), user->getNickname() + " PART " + itChan->getName());
		++itChan;
	}

	
	// A loop to find the user in the vector and delete it
	for (itUser = _usrs.begin(); itUser != _usrs.end(); itUser++) {
		if (itUser->getFd() == user->getFd()) {
			// // Quit the channel
			// if (it->getisInChannel() == true && it->getWhatChannel().size() > 0) {
			// 	int chan_index = findChan(it->getWhatChannel());

			// 	if (chan_index != -1) {
			// 		_channels[chan_index].delUsr(&(*it));
			// 		sendAllUsersInChan(_channels[chan_index].getName(), user->getNickname() + " PART " + _channels[chan_index].getName());
			// 	}
			// }
			
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
	std::cout << " on socket " << ITALIC << user->getSocket() << RESET << YELLOW << " disconnected at " << this->getTime() << RESET << std::endl;
}

void Server::execCmd(User *user, std::string cmd, int fd) {
	// Concat command with buffer (ctrl + D) and reset it
	cmd = user->getBuffer() + cmd;
	user->setBuffer("");
	
	std::cout << BOLD << user->getNickname() << ": " << RESET << cmd <<std::endl;

	if (user->getPassOk() == false && cmd.substr(0, 4) != "PASS") {
		sendToUser(user, "User not connected yet. Please use PASS command first.");
		std::cout << RED << BOLD << "User " << user->getNickname() << " not connected yet ; PASS command not used." << RESET << std::endl;
		disconnectUser(user, fd);
		return;
	}
	
	if (cmd.substr(0, 4) == "PASS") {
		//check password. If ok getPassOk() = true
		this->cmdPass(user, cmd, fd);
	} else if (cmd.substr(0, 4) == "NICK") {
		this->cmdNick(user, cmd);
	} else if (cmd.substr(0, 4) == "USER") {
		this->cmdUser(user, cmd);
	} else if (cmd.substr(0, 3) == "CAP") {
		this->cmdCap(user, cmd);
	} else if (cmd.substr(0, 4) == "PING") {
		this->cmdPing(user, cmd);
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
	} else if (cmd.substr(0, 6) == "INVITE") {
		this->cmdInvite(user, cmd);
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
		sendToUser(user, "Command not found.");
	}
}

void Server::sendToUser(User *user, std::string msg) {
	// Sending a message to a user
	std::string start = ":IRC 001 ";
	start += user->getNickname();
	start += " :";

	msg = start + msg + "\n\r";
	
	const void *cmsg = msg.c_str();

	if (user->getVerification()) {
		if (checkWritable(user->getFd())) {
			send(user->getFd(), cmsg, msg.size(), 0);
		}
	}
}

void Server::sendUserInChan(User *user, std::string chan) {
	// Sending a message to a user while he is in a channel
	std::string msg = ":" + chan + "\n\r";

	send(user->getFd(), msg.c_str(), msg.length(), 0);
}

void Server::sendAllUsersInChan(std::string chan, std::string msg) {
	// Sending a message to all users in a channel
	std::vector<User>::iterator it;
	
	for (it = _usrs.begin(); it != _usrs.end(); it++) {
		if (it->getWhatChannel() == chan) {
			sendUserInChan(&(*it), msg);
		}
	}
}

void Server::sendPrivMsgInChan(std::string chan, std::string msg, std::string user) {
	// Sending a message to all users in a channel
	std::vector<User>::iterator it;
	
	for (it = _usrs.begin(); it != _usrs.end(); it++) {
		if (it->getWhatChannel() == chan && it->getNickname() != user) {
			sendUserInChan(&(*it), msg);
		}
	}
}

void Server::sendToUserInChan(User *user, int code, std::string chan, std::string msg) {
	// Sending a message to a user while he is in a channel with a custom code
	std::string start = ":IRC ";
	start += std::to_string(code);
	start += " ";
	start += user->getNickname();
	start += " ";
	start += chan;
	start += " :";
	
	msg = start + msg + "\n\r";
	
	const void *cmsg = msg.c_str();

	if (user->getVerification()) {
		if (checkWritable(user->getFd())) {
			send(user->getFd(), cmsg, msg.size(), 0);
		}
	}
}

/* ===== UTILS ===== */

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


void Server::displayWelcome(User *user) {
	sendToUser(user, " _       __     __                             ______         ________  ______");
	sendToUser(user, "| |     / /__  / /________  ____ ___  ___     /_  __/___     /  _/ __ \\/ ____/");
	sendToUser(user, "| | /| / / _ \\/ / ___/ __ \\/ __ `__ \\/ _ \\     / / / __ \\    / // /_/ / /     ");
	sendToUser(user, "| |/ |/ /  __/ / /__/ /_/ / / / / / /  __/    / / / /_/ /  _/ // _, _/ /___   ");
	sendToUser(user, "|__/|__/\\___/_/\\___/\\____/_/ /_/ /_/\\___/    /_/  \\____/  /___/_/ |_|\\____/   ");

	sendToUser(user, "");
	sendToUser(user, "    _                                                  _                            _     _    ");
	sendToUser(user, "  __| | ___ ___      __ _  _  _  _  _  __ _  _ _     __| |__ __ ___  _ _  __ _  ___ | |__ | |__ ");
	sendToUser(user, " / _` |(_-</ _ \\ _  / _` || || || || |/ _` || '_|_  / _` |\\ V // -_)| '_|/ _` |/ _ \\| '_ \\| '_ \\");
	sendToUser(user, " \\__,_|/__/\\___/( ) \\__, | \\_,_| \\_, |\\__,_||_| ( ) \\__,_| \\_/ \\___||_|  \\__, |\\___/|_.__/|_.__/");
	sendToUser(user, "                |/  |___/        |__/           |/                       |___/                  ");

	sendToUser(user, "");
	sendToUser(user, "");
}

void Server::displayWrongPass(User *user) {
	sendToUser(user, "");
	sendToUser(user, " _    _                                                                               _            __");
	sendToUser(user, "| |  | |                                                                             | |    _     / /");
	sendToUser(user, "| |  | | _ __  ___   _ __    __ _    _ __    __ _  ___  ___ __      __ ___   _ __  __| |   (_)   / / ");
	sendToUser(user, "| |/\\| || '__|/ _ \\ | '_ \\  / _` |  | '_ \\  / _` |/ __|/ __|\\ \\ /\\ / // _ \\ | '__|/ _` |        / /  ");
	sendToUser(user, "\\  /\\  /| |  | (_) || | | || (_| |  | |_) || (_| |\\__ \\__ \\ \\ V  V / | (_) || |  | (_| |    _  / /   ");
	sendToUser(user, " \\/  \\/ |_|   \\___/ |_| |_| \\__, |  | .__/  \\__,_||___/|___/  \\_/\\_/  \\___/ |_|   \\__,_|   (_)/_/    ");
	sendToUser(user, "                             __/ |  | |                                                              ");
	sendToUser(user, "                            |___/   |_|                                                              ");
	sendToUser(user, "");
}

void	Server::sendAllIfInChannel(User *user, std::string msg) {
	if (user->getisInChannel() == true && user->getWhatChannel().size() > 0) {
		int chan_index = findChan(user->getWhatChannel());

		if (chan_index != -1) {
			sendAllUsersInChan(_channels[chan_index].getName(), msg);
		}
	}
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

int Server::getNumberUsers() const {
	return _usrs.size();
}


std::string Server::getTime() const {
	time_t now = time(0);
	tm *gmt = gmtime(&now);
	std::string str = asctime(gmt);
	return str;

}

std::string Server::getPassword() const {
	return (this->_password);
}


/* ===== COMMANDS ===== */

void Server::cmdQuit(User *user, std::string cmd, int fd) {
	std::string msg = "";
	
	if (cmd.size() > 6)
		msg = cmd.substr(6);
	
	std::cout << "Command QUIT received from " << BOLD << user->getNickname() << RESET << " with message: " << ITALIC << msg << RESET << std::endl;
	disconnectUser(user, fd);
}

void Server::cmdPing(User *user, std::string cmd) {
	std::string address = "";
	if (cmd.size() > 5)
		address = cmd.substr(5);
		
	std::cout << CYAN << "Sending PONG to address : " << ORANGE << BOLD << address << RESET << NORMAL << std::endl;
	sendToUser(user, "PONG");
}

void Server::cmdCap(User *user, std::string cmd) {
	(void) cmd;

	std::cout << "Command CAP received from " << BOLD << user->getNickname() << RESET << std::endl;
}

void Server::cmdNick(User *user, std::string cmd) {
	int	tmpNick = 0;

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
		std::cout << RED << BOLD << "<nickname> not found." << RESET << std::endl << std::endl;
		sendToUser(user, "Error: <nickname> not found.");
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
		std::cout << ORANGE << ITALIC << user->getNickname() << RESET << " is still " << CYAN << BOLD << new_nickname << RESET << std::endl << std::endl;
		return;
	}

	if (new_nickname.size() == 0) {
		std::cout << RED << BOLD << "Nickname is not valid." << RESET << std::endl << std::endl;
		sendToUser(user, "Error: <nickname> must contain only letters, numbers and underscores.");
		new_nickname.clear();
		return;
	}

	// Check if nickname contains only letters, numbers and underscores
	for (size_t i = 0; i < new_nickname.size(); i++) {
		if (!isalnum(new_nickname[i]) && new_nickname[i] != '_') {
			std::cout << RED << BOLD << "Nickname contains invalid characters : '" << new_nickname[i] << "'" << RESET << std::endl << std::endl;
			sendToUser(user, "Error: <nickname> must contain only letters, numbers and underscores.");
			new_nickname.clear();
			return;
		}
	}

	if (new_nickname.size() == 0) {
		std::cout << RED << BOLD << "Nickname too short!" << RESET << std::endl << std::endl;
		sendToUser(user, "Error: <nickname> too short.");
	} else if (new_nickname.size() > 9) {
		std::cout << RED << BOLD << "Nickname too long!" << RESET << std::endl << std::endl;
		sendToUser(user, "Error: <nickname> too long.");
	}
	else {
		// Check if nickname is already taken
		std::vector<User>::iterator it = _usrs.begin();
		
		while (it != _usrs.end()) {
			if (it->getNickname() == new_nickname) {
				std::cout << RED << BOLD << "Nickname already taken." << RESET << std::endl << std::endl;
				sendToUser(user, "Error: Nickname already taken.");
				
				while (findUser(new_nickname) != -1){
					std::stringstream ss;
					ss << tmpNick;
					new_nickname += ss.str();
					tmpNick++;
				}
				
				sendToUser(user, "New nickname :" + new_nickname);
				continue;
			}
			it++;
		}

		std::cout << ORANGE << ITALIC << user->getNickname() << RESET << " is now " << CYAN << BOLD << new_nickname << RESET << std::endl << std::endl;

		// Send NICK message to all users in the channel
		sendAllIfInChannel(user, user->getNickname() + " NICK " + new_nickname);
		
		user->setNickname(new_nickname);
		user->setUsername(new_nickname);

		if (user->getisInChannel()) {
			int chan_index = findChan(user->getWhatChannel());

			if (chan_index != -1) {
				_channels[chan_index].updateUser(user);
			}
		}

		// Avert user
		sendToUser(user, "Nickname updated.");
	}
			
}

void Server::cmdPass(User *user, std::string cmd, int fd) {
	std::vector<User>::iterator it;
	(void) it;
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
		std::cout << RED << BOLD << "Missing password." << std::endl;
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
	
	if (pass == this->getPassword()) {
		user->setPassOk(true);
		displayWelcome(user);
		
		std::cout << GREEN << BOLD << "User " << user->getNickname() << " connected." << RESET << std::endl;
	} else {
		sendToUser(user, "Wrong password.");
		displayWrongPass(user);
		disconnectUser(user, fd);

		std::cout << RED << BOLD << "Wrong password ; kick user from the server." << RESET << std::endl;
	}
}

void Server::cmdUser(User *user, std::string cmd) {
	// Split the message into its components
	std::istringstream iss(cmd);
	std::vector<std::string> components;
	std::string component;

	while (std::getline(iss, component, ' ')) {
		components.push_back(component);
	}

	// Check if the message is a USER message
	if (components.size() == 5 && components[0] == "USER") {
		// Extract the relevant information from the message
		std::string username = components[1];
		std::string hostname = components[2];
		std::string realname = components[4].substr(1);

		std::string tmp = user->getNickname();

		if (tmp.compare(username) != 0) {
			sendAllIfInChannel(user, user->getNickname() + " NICK " + username);
		}

		username = user->getNickname();
		realname = user->getNickname();
		// Update the user's information
		user->setNickname(username);
		user->setHostname(hostname);
		user->setUsername(username);

		if (user->getisInChannel()) {
			int chan_index = findChan(user->getWhatChannel());

			if (chan_index != -1) {
				_channels[chan_index].updateUser(user);
			}
		}

		std::cout << CYAN << ITALIC << "Username: " << username << RESET << NORMAL << std::endl;
		std::cout << CYAN << ITALIC << "Hostname: " << hostname << RESET << NORMAL << std::endl;
		std::cout << CYAN << ITALIC << "Realname: " << realname << RESET << NORMAL << std::endl << std::endl;
		
		sendToUser(user, "User informations updated : username=`" + username + "` hostname=`" + hostname + "` realname=`" + realname + "`");
	} else {
		std::cout << RED << BOLD << "Invalid USER message." << RESET << std::endl << std::endl;
		sendToUser(user, "Invalid USER message.");
	}
}

void Server::cmdList(User *user, std::string cmd) {
	(void)cmd;

	std::cout << CYAN << ITALIC << "Showing users and channels." << RESET << std::endl << std::endl;

	// Send the 321 message
	sendUserInChan(user, "IRC 321 " + user->getNickname() + " Channel :Users  Name");
	
	// Send the 322 message for each channel
	for (size_t i = 0; i < _channels.size(); i++) {
		if (_channels[i].getIsPrivate() == false) {	
			std::string msg = "IRC 322 " + user->getNickname() + " " + _channels[i].getName() + " " + _channels[i].getNbUsers() + " :" + _channels[i].getTopic();
			sendUserInChan(user, msg);
		}
	}

	// Send the 323 message
	sendUserInChan(user, "IRC 323 " + user->getNickname() + " :End of /LIST");
}

void Server::cmdJoin(User *user, std::string cmd) {
	// Clear command
	cmd.erase(0, 5);
	
	std::cout << CYAN << "Command JOIN received from " << BOLD << user->getNickname() << RESET << " with channel(s): " << ITALIC << cmd << RESET << std::endl;

	// Parse the command to get the channel name(s)
	std::vector<std::string> channels;
	std::string::size_type channel_start = cmd.find_first_of("#");
	
	while (channel_start != std::string::npos) {
		// Set channel_end to the first space or , after channel_start
		std::string::size_type channel_end = cmd.find_first_of(" ,", channel_start);
		
		if (channel_end == std::string::npos)
			channel_end = cmd.size();
			
		std::string channel = cmd.substr(channel_start, channel_end - channel_start);
		
		channels.push_back(channel);
		channel_start = cmd.find_first_of("#", channel_end);
	}

	// Join the channel(s)
	for (size_t i = 0; i < channels.size(); i++) {
		channels[i] = clearString(channels[i]);
	
		// Check if the channel already exists
		int chan_index = findChan(channels[i]);
		
		if (chan_index == -1) {
			// Create the channel
			Channel chan(channels[i]);
			_channels.push_back(chan);
			chan_index = _channels.size() - 1;

			std::cout << "Channel " << channels[i] << " created." << std::endl;
		}

		// If the user is already in the channel, delete him
		if (user->getisInChannel() == true && user->getWhatChannel().size() > 0) {
			int chan_index = findChan(user->getWhatChannel());

			if (chan_index != -1) {
				_channels[chan_index].delUsr(user);
				sendAllUsersInChan(_channels[chan_index].getName(), user->getNickname() + " PART " + _channels[chan_index].getName());
			}
		}
		
		// Add the user to the channel
		_channels[chan_index].addUsr(user);
		user->setWhatChannel(_channels[chan_index].getName());
		user->setisInChannel(true);

		// Send the JOIN message to the channel
		sendAllUsersInChan(channels[i], user->getNickname() + " JOIN " + channels[i]);

		// Send the NAMES message to the user
		cmdNames(user, channels[i]);

		// Send the TOPIC message to the user
		cmdTopic(user, channels[i]);

		// Check if user is operator or voiced
		// ! commented because ça bug
		// if (_channels[chan_index].isOperator(user) == true)
		// 	sendUserInChan(user, user->getNickname() + " MODE " +channels[i] + " +v " + user->getNickname());
		// if (_channels[chan_index].isVoiced(user) == true)
		// 	sendUserInChan(user, user->getNickname() + " MODE " +channels[i] + " +v " + user->getNickname());
	}	

	if (channels.size() == 0) {
		std::cout << RED << BOLD << "No channel specified or wrong usage." << RESET << std::endl << std::endl;
		sendToUser(user, "Error: No channel specified or wrong usage.");
	}
}

void Server::cmdPrivmsg(User *user, std::string cmd) {
	std::string msg;
	std::string::size_type msg_start;
	
	cmd.erase(0, 8);
	msg_start = cmd.find_first_of(":");
	
	if (msg_start == std::string::npos) {
		std::cout << RED << BOLD << "Invalid PRIVMSG message." << RESET << std::endl << std::endl;
		return;
	}
	
	msg = cmd.substr(msg_start + 1, cmd.size());

	std::cout << CYAN << "Command PRIVMSG received from " << BOLD << user->getNickname() << RESET << " with message: " << ITALIC << msg << RESET << std::endl;
		
	// Get the channel name or username
	std::string destinataire;
	std::string::size_type destinataire_start = cmd.find_first_of("#");
	
	if (destinataire_start != std::string::npos) {
		// destinataire is a channel
		destinataire = clearString(cmd.substr(destinataire_start, msg_start - destinataire_start - 1));
	} else {
		// destinataire is a username
		destinataire = clearString(cmd.substr(0, msg_start));
	}

	// Check if the destinataire is a channel or a username
	if (destinataire[0] == '#') {
		// destinataire is a channel
		int chan_index = findChan(destinataire);
		
		// Check if the channel is moderated and if the user is not an operator or a voiced user
		if (
			_channels[chan_index].getIsModerated() == true
			&& _channels[chan_index].isOperator(user) == false
			&& _channels[chan_index].isVoiced(user) == false
		) {
			// Send the error message to the user
			sendToUserInChan(user, 404, destinataire, "Channel is moderated.");
			std::cout << RED << BOLD << "Channel " << destinataire << " is moderated." << RESET << std::endl << std::endl;
			return;
		}
		
		if (chan_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: channel " + destinataire + " doesn't exist.");
			std::cout << RED << BOLD << "Channel " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
		} else {
			// Check if the user is in the channel or if the user is an operator or a voiced user
			if (_channels[chan_index].getExternalMessage() == false && _channels[chan_index].isInChannel(user) == false) {
				// Send the error message to the user
				if (user->getisInChannel() == false)
					sendToUserInChan(user, 404, destinataire, "You're not in the channel " + destinataire + ".");
				else
					sendToUser(user, "Error: you're not in the channel " + destinataire + ".");
				std::cout << RED << BOLD << "User " << user->getNickname() << " is not in the channel " << destinataire << " without external messages." << RESET << std::endl << std::endl;
				return;
			}
			
			// Send the message to the channel
			sendPrivMsgInChan(destinataire, user->getNickname() + " PRIVMSG " + destinataire + " :" + msg, user->getNickname());
			std::cout << GREEN << BOLD << user->getNickname() << " sent a message to " << destinataire << RESET << std::endl << std::endl;
		}
	} else {
		// destinataire is a username
		int user_index = findUser(destinataire);
		
		if (user_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: user " + destinataire + " doesn't exist.");
			std::cout << RED << BOLD << "User " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
		} else {
			// Send the message to the user
			sendToUser(&(_usrs[user_index]), user->getNickname() + " PRIVMSG " + destinataire + " :" + msg);
			std::cout << GREEN << BOLD << user->getNickname() << " sent a message to " << destinataire << RESET << std::endl << std::endl;
		}
	}
}

void Server::cmdPart(User *user, std::string cmd) {
	// Clear command
	cmd.erase(0, 5);
	
	std::cout << CYAN << "Command PART received from " << BOLD << user->getNickname() << RESET << " with channel(s): " << ITALIC << cmd << RESET << std::endl;
	
	// Parse the command to get the channel name(s)
	std::vector<std::string> channels;
	std::string::size_type channel_start = cmd.find_first_of("#");
	
	while (channel_start != std::string::npos) {
		// Set channel_end to the first ' ' or ',' after channel_start
		std::string::size_type channel_end = cmd.find_first_of(" ,", channel_start);
		
		if (channel_end == std::string::npos)
			channel_end = cmd.size();
			
		std::string channel = cmd.substr(channel_start, channel_end - channel_start);
		channels.push_back(channel);
		channel_start = cmd.find_first_of("#", channel_end);
	}

	// Part the channel(s)
	for (size_t i = 0; i < channels.size(); i++) {
		// Check if the channel exists with a loop
		int chan_index = findChan(channels[i]);
		
		if (chan_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: channel " + channels[i] + " doesn't exist.");

			std::cout << RED << BOLD << "Channel " << channels[i] << " doesn't exist." << RESET << std::endl << std::endl;
		} else {
			// Remove the user from the channel
			_channels[chan_index].delUsr(user);
			
			// Remove privileges
			_channels[chan_index].delOperator(user, user);
			_channels[chan_index].delVoiced(user, user);
			
			user->setWhatChannel("");
			user->setisInChannel(false);
			
			// Send the PART message to the user
			sendUserInChan(user, user->getNickname() + " PART " + channels[i]);
			sendAllUsersInChan(channels[i], user->getNickname() + " PART " + channels[i]);
			
			// Send the NAMES message to the user
			cmdNames(user, channels[i]);

			std::cout << GREEN << BOLD << user->getNickname() << " left " << channels[i] << RESET << std::endl << std::endl;
		}
	}
}

void Server::cmdNames(User *user, std::string cmd) {
	// Execute the command ; `cmd` is the channel name
	std::cout << CYAN << "Command NAMES received from " << BOLD << user->getNickname() << RESET << " with channel: " << ITALIC << cmd << RESET << std::endl;
	
	if (cmd.size() == 0) {
		// Send the error message to the user
		sendToUser(user, "Error: missing channel name.");

		std::cout << RED << BOLD << "Missing channel name." << RESET << std::endl << std::endl;
		return;
	}

	// Parse the command to get the channel name(s)
	std::vector<std::string> channels;
	std::string::size_type channel_start = cmd.find_first_of("#");
	
	while (channel_start != std::string::npos) {
		// Set channel_end to the first ' ' or ',' after channel_start
		std::string::size_type channel_end = cmd.find_first_of(" ,", channel_start);
		
		if (channel_end == std::string::npos)
			channel_end = cmd.size();
			
		std::string channel = cmd.substr(channel_start, channel_end - channel_start);
		channels.push_back(channel);
		channel_start = cmd.find_first_of("#", channel_end);
	}

	// Part the channel(s)
	for (size_t i = 0; i < channels.size(); i++) {
		channels[i] = clearString(channels[i]);
		// Check if the channel exists with a loop
		int chan_index = findChan(channels[i]);
		
		if (chan_index == -1) {
			// Send the error message to the user
			sendUserInChan(user, "NAMES Error: channel doesn't exist.");
			std::cout << RED << BOLD << "Channel doesn't exist." << RESET << std::endl << std::endl;
		} else {
			// List the users in the channel
			sendToUserInChan(user, 353, channels[i], _channels[chan_index].getChanUsrs());
			std::cout << GREEN << BOLD << "List of users in " << channels[i] << ": " << _channels[chan_index].getChanUsrs() << RESET << std::endl << std::endl;
		}
	}

	if (channels.size() == 0) {
		std::cout << RED << BOLD << "No channel specified or wrong usage." << RESET << std::endl << std::endl;
		sendToUser(user, "Error: No channel specified or wrong usage.");
	}
}

void Server::cmdTopic(User *user, std::string cmd) {
	// Parse the command to get channel name and topic from inputs 'TOPIC #no' and 'TOPIC #no :er'
	std::string channel;
	std::string topic;
	std::string::size_type channel_start = cmd.find_first_of("#");
	std::string::size_type topic_start = cmd.find_first_of(":");
	
	if (channel_start == std::string::npos) {
		// Send the error message to the user
		sendToUser(user, "Error: missing channel name.");

		std::cout << RED << BOLD << "Missing channel name." << RESET << std::endl << std::endl;
		return;
	}
	
	channel = clearString(cmd.substr(channel_start, topic_start - channel_start - 1));
	
	if (topic_start != std::string::npos)
		topic = cmd.substr(topic_start + 1, cmd.size() - topic_start - 1);
	
	int chan_index = findChan(channel);

	if (
		_channels[chan_index].getIsTopicSettable() == true
		&& _channels[chan_index].isOperator(user) == false
	) {
		// Send the error message to the user
		if (user->getisInChannel())
			sendToUserInChan(user, 404, user->getWhatChannel(), " you are not allowed to change the topic of " + channel + ".");
		else
			sendToUser(user, "Error: you are not allowed to change the topic of " + channel + ".");
		std::cout << RED << BOLD << "User " << user->getNickname() << " is not allowed to change the topic of " << channel << "." << RESET << std::endl << std::endl;
		return;
	}
	
	// Print the topic of the channel is it exists and is not empty
	if (chan_index != -1 && topic.size() == 0) {
		if (_channels[chan_index].getTopic().size() != 0) {
			std::cout << GREEN << BOLD << "Topic of " << channel << ": " << _channels[chan_index].getTopic() << RESET << std::endl << std::endl;
			sendToUserInChan(user, 332, channel, _channels[chan_index].getTopic());
		} else {
			std::cout << GREEN << BOLD << "Topic of " << channel << ": No topic set." << RESET << std::endl << std::endl;
			sendToUserInChan(user, 331, channel, "No topic is set");
		}
		return;
	}

	if (user->getWhatChannel() != channel) {
		// Send the error message to the user
		sendToUser(user, "Error: you are not in channel " + channel + ".");
		std::cout << RED << BOLD << "User " << user->getNickname() << " is not in channel " << channel << "." << RESET << std::endl << std::endl;
		return;
	}
	
	if (chan_index == -1) {
		// Send the error message to the user
		sendToUser(user, "Error: channel " + channel + " doesn't exist.");
		std::cout << RED << BOLD << "Channel " << channel << " doesn't exist." << RESET << std::endl << std::endl;
	} else {
		_channels[chan_index].setTopic(topic);
		
		// Send the TOPIC message to the users
		sendAllUsersInChan(channel, user->getNickname() + " TOPIC " + channel + " " + topic);
		std::cout << GREEN << BOLD << user->getNickname() << " changed the topic of " << channel << " to " << topic << RESET << std::endl << std::endl;
	}
}

void Server::cmdInvite(User *user, std::string cmd) {
	// Get username and channel name from the command : "INVITE User_5 #nope"
	std::string username;
	std::string channel;

	std::string::size_type username_start = cmd.find_first_of(" ");
	std::string::size_type channel_start = cmd.find_first_of("#");

	if (username_start == std::string::npos || channel_start == std::string::npos) {
		// Send the error message to the user
		sendToUser(user, "Error: missing username or channel name.");

		std::cout << RED << BOLD << "Missing username or channel name." << RESET << std::endl << std::endl;
		return;
	}

	username = clearString(cmd.substr(username_start + 1, channel_start - username_start - 1));
	channel = clearString(cmd.substr(channel_start, cmd.size() - channel_start));

	// Check if the channel exists with a loop
	int chan_index = findChan(channel);
	
	if (chan_index == -1) {
		// Send the error message to the user
		sendToUser(user, "Error: channel " + channel + " doesn't exist.");

		std::cout << RED << BOLD << "Channel " << channel << " doesn't exist." << RESET << std::endl << std::endl;
		return;
	}

	// Check if user is currently in a channel
	if (user->getWhatChannel() != channel) {
		// Send the error message to the user
		sendToUser(user, "Error: you are not in the channel'" + channel + "'.");

		std::cout << RED << BOLD << "User " << user->getNickname() << " is not in the '" << channel << "' channel." << RESET << std::endl << std::endl;
		return;
	}

	// Check if the user exists with a loop
	for (size_t i = 0; i < _usrs.size(); i++) {
		if (_usrs[i].getNickname() == username) {
			// Send the INVITE message to the user
			if (_usrs[i].getisInChannel() == true) {
				sendToUser(user, "Error: user " + username + " is already in a channel.");
				std::cout << RED << BOLD << "User " << username << " is already in a channel." << RESET << std::endl << std::endl;
			} else {
				sendToUser(&_usrs[i], user->getNickname() + " invited you (" + username + ") in channel " + channel);
				std::cout << GREEN << BOLD << user->getNickname() << " invited " << username << " to " << channel << RESET << std::endl << std::endl;
			}
			return;
		}
	}

	// Send the error message to the user
	sendToUser(user, "Error: user " + username + " doesn't exist.");
	std::cout << RED << BOLD << "User " << username << " doesn't exist." << RESET << std::endl << std::endl;
}

void Server::cmdKick(User *user, std::string cmd) {
	cmd.erase(0, 5); // Remove the "KICK " part of the command
	// Split the command into 3 parts
	std::string username;
	std::string channel;
	std::string reason = "No reason specified";
	size_t i = 0;

	// A for loop to get the username and channel name
	while (i < cmd.size()) {
		if (cmd[i] == ' ') {
			channel = cmd.substr(0, i);
			break;
		}
		i++;
	}

	cmd.erase(0, i + 1); // Remove the channel name from the command

	i = 0;
	// A for loop to get the username and channel name
	while (i < cmd.size()) {
		if (cmd[i] == ' ' || i == cmd.size() - 1) {
			username = clearString(cmd.substr(0, i + 1));
			break;
		}
		i++;
	}

	cmd.erase(0, i + 1); // Remove the username from the command

	// A for loop to get the reason
	if (cmd.size() > 0) {
		if (cmd[0] == ':') {
			cmd.erase(0, 1);
		}
		reason = cmd;
	}

	if (user->getWhatChannel() != channel) {
		// Send the error message to the user
		if (user->getisInChannel() == false)
			sendToUser(user, "Error: you are not in channel " + channel + ".");
		else
			sendToUserInChan(user, 404, user->getWhatChannel(), " you are not allowed to kick " + username + " from " + channel + ".");
		
		std::cout << RED << BOLD << "User " << user->getNickname() << " is not in channel " << channel << "." << RESET << std::endl << std::endl;
		return;
	}

	// Check if the channel exists with a loop
	int chan_index = findChan(channel);
	
	if (chan_index == -1) {
		// Send the error message to the user
		if (user->getisInChannel() == false)
			sendToUser(user, "Error: channel " + channel + " doesn't exist.");
		else
			sendToUserInChan(user, 404, user->getWhatChannel(), " channel " + channel + " doesn't exist.");

		std::cout << RED << BOLD << "Channel doesn't exist." << RESET << std::endl << std::endl;
		return;
	}

	// Check if the user exists with a loop
	for (size_t i = 0; i < _usrs.size(); i++) {
		if (_usrs[i].getNickname() == username) {
			// Send the KICK message to the user
			if (_usrs[i].getisInChannel() == false || _usrs[i].getWhatChannel() != channel) {
				sendToUser(user, "Error: user " + username + " is not in the channel " + channel + ".");
				std::cout << RED << BOLD << "User " << username << " is not in the channel " << channel << "." << RESET << std::endl << std::endl;
			} else {
				sendAllUsersInChan(channel, user->getNickname() + " KICK " + channel + " " + username + " :" + reason);
				sendToUser(&_usrs[i], user->getNickname() + " KICK " + channel + " " + username + " :" + reason);
	
				// Remove the user from the channel
				_channels[chan_index].delUsr(&_usrs[i]);

				// Remove privileges
				_channels[chan_index].delOperator(user, user);
				_channels[chan_index].delVoiced(user, user);
			
				_usrs[i].setWhatChannel("");
				_usrs[i].setisInChannel(false);
				
				// Send the NAMES message to the user
				cmdNames(user, channel);
				
				std::cout << GREEN << BOLD << user->getNickname() << " kicked " << username << " from " << channel << RESET << std::endl << std::endl;
			}
			return;
		}
	}

	// Send the error message to the user
	sendToUser(user, "Error: user " + username + " doesn't exist.");
	std::cout << RED << BOLD << "User " << username << " doesn't exist." << RESET << std::endl << std::endl;
}

void Server::cmdNotice(User *user, std::string cmd) {
	// Format of the command : "NOTICE #no :er" or "NOTICE User_5 :er"
	std::string msg;
	std::string::size_type msg_start;
	
	cmd.erase(0, 7);

	msg_start = cmd.find_first_of(":");
	
	if (msg_start == std::string::npos) {
		std::cout << RED << BOLD << "Invalid NOTICE message." << RESET << std::endl << std::endl;
		return;
	}

	msg = cmd.substr(msg_start + 1, cmd.size());
	
	std::cout << CYAN << "Command NOTICE received from " << BOLD << user->getNickname() << RESET << " with message: " << ITALIC << msg << RESET << std::endl;

	// Get the channel name or username
	std::string destinataire;
	std::string::size_type destinataire_start = cmd.find_first_of("#");
	
	if (destinataire_start != std::string::npos) {
		// destinataire is a channel
		destinataire = clearString(cmd.substr(destinataire_start, msg_start - destinataire_start - 1));
	} else {
		// destinataire is a username
		destinataire = clearString(cmd.substr(0, msg_start));
	}

	// Check if the destinataire is a channel or a username
	if (destinataire[0] == '#') {
		// destinataire is a channel
		int chan_index = findChan(destinataire);


		// Check if the channel is moderated and if the user is not an operator or a voiced user
		if (
			_channels[chan_index].getIsModerated() == true
			&& _channels[chan_index].isOperator(user) == false
			&& _channels[chan_index].isVoiced(user) == false
		) {
			// Send the error message to the user
			sendToUserInChan(user, 404, destinataire, "Channel is moderated.");
			std::cout << RED << BOLD << "Channel " << destinataire << " is moderated." << RESET << std::endl << std::endl;
			return;
		}
		
		if (chan_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: channel " + destinataire + " doesn't exist.");

			std::cout << RED << BOLD << "Channel " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
		} else {

			// Check if the user is in the channel or if the user is an operator or a voiced user
			if (_channels[chan_index].getExternalMessage() == false && _channels[chan_index].isInChannel(user) == false) {
				// Send the error message to the user
				if (user->getisInChannel() == false)
					sendToUserInChan(user, 404, destinataire, "You're not in the channel " + destinataire + ".");
				else
					sendToUser(user, "Error: you're not in the channel " + destinataire + ".");
				std::cout << RED << BOLD << "User " << user->getNickname() << " is not in the channel " << destinataire << " without external messages." << RESET << std::endl << std::endl;
				return;
			}
			
			// Send the NOTICE message to the users
			sendAllUsersInChan(destinataire, user->getNickname() + " NOTICE " + destinataire + " :" + msg);
			std::cout << GREEN << BOLD << user->getNickname() << " sent a notice to " << destinataire << RESET << std::endl << std::endl;
		}
	} else {
		// destinataire is a username
		for (size_t i = 0; i < _usrs.size(); i++) {
			if (_usrs[i].getNickname() == destinataire) {
				// Send the NOTICE message to the user
				sendToUser(&_usrs[i], user->getNickname() + " NOTICE " + destinataire + " :" + msg);
				std::cout << GREEN << BOLD << user->getNickname() << " sent a notice to " << destinataire << RESET << std::endl << std::endl;
				return;
			}
		}

		// Send the error message to the user
		sendToUser(user, "Error: user " + destinataire + " doesn't exist.");
		std::cout << RED << BOLD << "User " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
	}
}

void Server::cmdMode(User *user, std::string cmd) {
	cmd.erase(0, 5); // Remove the "MODE " part of the command

	User *sender = user;

	// Get the channel name or username
	std::string destinataire;
	std::string::size_type destinataire_start = cmd.find_first_of("#");

	// destinataire_end is the first space after destinataire_start or the end of the command
	std::string::size_type destinataire_end = cmd.find_first_of(" ", destinataire_start);


	if (destinataire_start != std::string::npos) {
		// destinataire is a channel
		destinataire = clearString(cmd.substr(destinataire_start, destinataire_end - destinataire_start));
	} else {
		// destinataire is a username
		destinataire = clearString(cmd.substr(0, destinataire_end));
	}

	// Get mode from the command if exists
	std::string mode = "";
	std::string::size_type mode_start = cmd.find_first_of(" ");
	
	if (mode_start != std::string::npos) {
		mode = cmd.substr(mode_start + 1, cmd.size());
	}

	std::cout << CYAN << "Command MODE received from " << BOLD << user->getNickname() << RESET << " with mode: " << ITALIC << mode << RESET << std::endl;

	// Check if the destinataire is a channel or a username
	if (destinataire[0] == '#' && mode.size() == 0) {
		// destinataire is a channel
		int chan_index = findChan(destinataire);

		if (chan_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: channel " + destinataire + " doesn't exist.");

			std::cout << RED << BOLD << "Channel " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
		} else {

			// Check if the user is in the channel or if he is an operator or if channel as externe message
			if (
				user->getWhatChannel() != destinataire
				&& _channels[chan_index].isOperator(user) == false
				&& _channels[chan_index].getExternalMessage() == false
			) {
				// Send the error message to the user
				sendToUser(user, "Error: you are not in channel " + destinataire + ".");
				std::cout << RED << BOLD << "User " << user->getNickname() << " is not in channel " << destinataire << "." << RESET << std::endl << std::endl;
			} else {
				// Send the MODE message to the users
				sendUserInChan(user, "IRC 324 " +  destinataire + " " + _channels[chan_index].getName() + " " + _channels[chan_index].getMode());
			}
		}
		return;
	} else if (mode.size() == 0) {
		// destinataire is a username
		int user_index = findUser(destinataire);

		if (user_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: user " + destinataire + " doesn't exist.");
			std::cout << RED << BOLD << "User " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
		} else {
			int chan_index = findChan(_usrs[user_index].getWhatChannel());

			if (chan_index == -1) {
				// Send the error message to the user
				sendToUser(user, "Error: channel " + _usrs[user_index].getWhatChannel() + " doesn't exist.");
				std::cout << RED << BOLD << "Channel " << _usrs[user_index].getWhatChannel() << " doesn't exist." << RESET << std::endl << std::endl;
			} else {
				// Send the MODE message to the user
				sendToUser(user, "IRC 221 " + destinataire + " " + _channels[chan_index].getModeUser(&_usrs[user_index]));
				std::cout << GREEN << BOLD << "Mode of " << destinataire << ": " << _channels[chan_index].getModeUser(&_usrs[user_index]) << RESET << std::endl << std::endl;
			}
		}
		return;
	}

	// destinataire is a username
	if (destinataire[0] != '#') {
		// Update another user's mode

		// Get username from command : after "MODE " and before the first space
		std::string username = clearString(cmd.substr(0, mode_start));
		
		int user_index = findUser(username);
		
		if (user_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: user " + destinataire + " doesn't exist.");
			std::cout << RED << BOLD << "User " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
		} else {
			int chan_index = findChan(_usrs[user_index].getWhatChannel());

			if (chan_index == -1) {
				// Send the error message to the user
				sendToUser(user, "Error: channel " + _usrs[user_index].getWhatChannel() + " doesn't exist.");
				std::cout << RED << BOLD << "Channel " << _usrs[user_index].getWhatChannel() << " doesn't exist." << RESET << std::endl << std::endl;
				return;
			}

			destinataire = _channels[chan_index].getName();
			sender = user;
			user = &_usrs[user_index];
		}
	}

	// Mode is not empty
	if (destinataire[0] == '#') {
		int chan_index = findChan(destinataire);

		if (chan_index == -1) {
			// Send the error message to the user
			sendToUser(user, "Error: channel " + destinataire + " doesn't exist.");

			std::cout << RED << BOLD << "Channel " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
		} else {
			if (
				user->getWhatChannel() != destinataire
				&& _channels[chan_index].isOperator(user) == false
				&& _channels[chan_index].getExternalMessage() == false)
			{
				// Send the error message to the user
				sendToUser(user, "Error: you are not in channel " + destinataire + ".");
				std::cout << RED << BOLD << "User " << user->getNickname() << " is not in channel " << destinataire << "." << RESET << std::endl << std::endl;
				return;
			}
		
			// Check if the mode is valid
			if (mode.size() > 3) {
				// Send the error message to the user
				sendToUser(user, "Error: invalid mode.");
				std::cout << RED << BOLD << "Invalid mode." << RESET << std::endl << std::endl;
				return;
			}

			// Check if the mode is valid
			if (mode[0] != '+' && mode[0] != '-') {
				// Send the error message to the user
				sendToUser(user, "Error: invalid mode.");
				std::cout << RED << BOLD << "Invalid mode." << RESET << std::endl << std::endl;
				return;
			}

			// Check if the mode is valid : o v m n p t
			if (mode[1] != 'o' && mode[1] != 'v' && mode[1] != 'm' && mode[1] != 'n' && mode[1] != 'p' && mode[1] != 't') {
				// Send the error message to the user
				sendToUser(user, "Error: invalid mode.");
				std::cout << RED << BOLD << "Invalid mode." << RESET << std::endl << std::endl;
				return;
			}
			
			// User's mode
			if (mode.substr(0, 2) == "-o") {
				_channels[chan_index].delOperator(user, sender);
				std::cout << GREEN << BOLD << user->getNickname() << " is not an operator. " << RESET << std::endl << std::endl;
				return;
			} else if (mode.substr(0, 2) == "+o") {
				_channels[chan_index].addOperator(user, sender);
				std::cout << GREEN << BOLD << user->getNickname() << " is an operator." << RESET << std::endl << std::endl;
				return;
			} else if (mode.substr(0, 2) == "-v") {
				_channels[chan_index].delVoiced(user, sender);
				std::cout << GREEN << BOLD << user->getNickname() << " is not voiced. " << RESET << std::endl << std::endl;
				return;
			} else if (mode.substr(0, 2) == "+v") {
				_channels[chan_index].addVoiced(user, sender);
				std::cout << GREEN << BOLD << user->getNickname() << " is voiced." << RESET << std::endl << std::endl;
				return;
			}
			
			// Channel's mode
			if (mode.substr(0, 2) == "-m") {
				_channels[chan_index].setIsModerated(false);
				std::cout << GREEN << BOLD << destinataire << " is not moderated." << RESET << std::endl << std::endl;
			} else if (mode.substr(0, 2) == "+m") {
				_channels[chan_index].setIsModerated(true);
				std::cout << GREEN << BOLD << destinataire << " is moderated." << RESET << std::endl << std::endl;
			} else if (mode.substr(0, 2) == "-n") {
				_channels[chan_index].setExternalMessage(true);
				std::cout << GREEN << BOLD << destinataire << " is not external message." << RESET << std::endl << std::endl;
			} else if (mode.substr(0, 2) == "+n") {
				_channels[chan_index].setExternalMessage(false);
				std::cout << GREEN << BOLD << destinataire << " is external message." << RESET << std::endl << std::endl;
			} else if (mode.substr(0, 2) == "-p") {
				_channels[chan_index].setIsPrivate(false);
				std::cout << GREEN << BOLD << destinataire << " is not private." << RESET << std::endl << std::endl;
			} else if (mode.substr(0, 2) == "+p") {
				_channels[chan_index].setIsPrivate(true);
				std::cout << GREEN << BOLD << destinataire << " is private." << RESET << std::endl << std::endl;
			} else if (mode.substr(0, 2) == "-t") {
				_channels[chan_index].setIsTopicSettable(false);
				std::cout << GREEN << BOLD << destinataire << " is not topic settable." << RESET << std::endl << std::endl;
			} else if (mode.substr(0, 2) == "+t") {
				_channels[chan_index].setIsTopicSettable(true);
				std::cout << GREEN << BOLD << destinataire << " is topic settable." << RESET << std::endl << std::endl;
			} else {
				// Send the error message to the user
				sendToUser(user, "Error: invalid mode.");
				std::cout << RED << BOLD << "Invalid mode." << RESET << std::endl << std::endl;
				return;
			}
			// Send the MODE message to the users
			sendAllUsersInChan(destinataire, user->getNickname() + " MODE " + destinataire + " " + mode);
			
		}
	}
}

/* ===== ERRORS ===== */

const char* Server::SrvError::what() const throw(){
	return "\033[1;31mServer error.\033[0m";
}

const char* Server::SrvErrorClient::what() const throw(){
	// Return the error message in bold red
	return "\033[1;31mError: user not found.\033[0m";
}
