/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guyar <guyar@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/25 19:16:00 by guyar            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Server::Server(int port, std::string password) {
	_fds = new struct pollfd[10];
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
	delete[] _fds;
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
		
		pollTest = poll(_fds, _nbUsers, -1);

		if (pollTest == -1)
			throw SrvError();

		for (int i = 0; i < _nbUsers; i++) {
			if (_fds[i].revents & POLLIN) {
				if (_fds[i].fd == _fdSrv)
					addUser(); // Add user to the server
				else
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

	User* user = new User(client, client.fd, fd);

	std::string nickname = "User_";
	ss << fd;
	nickname += ss.str();
		
	user->setNickname(nickname);

	std::cout << std::endl << BLUE << BOLD << "New connection" << RESET << " of " << ITALIC << CYAN << user->getNickname() << RESET;
	std::cout << " from " << UNDERLINE << inet_ntop(AF_INET, &(s->sin_addr), ip_str, INET_ADDRSTRLEN) << RESET;
	std::cout << " on socket " << CYAN << BOLD << fd << RESET << " at " << this->getTime() << std::endl;

	user->setVerification(true);
	_usrs.push_back(*user);

	// Update pollfd
	_nbUsers++;

	delete user;
}

void	Server::parseCmd(int fd){
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
		// Connection closed by client
		disconnectUser(user);
	} else {
		// Command received from client
		std::string cmd(buf);

		if (cmd.find("\n") != std::string::npos) {
			std::string delimiter = "\n";
			size_t pos = 0;
			std::string token;
			
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

void	Server::disconnectUser(User *user) {
	std::vector<User>::iterator it;
	
	// A loop to find the user in the vector and delete it
	for (it = _usrs.begin(); it != _usrs.end(); it++) {
		if (it->getFd() == user->getFd()) {
			close(user->getFd());
			_usrs.erase(it);
			break;
		}
	}
	
	// Update the pollfd array
	_nbUsers--;
	
	// Display the disconnection
	std::cout << std::endl << YELLOW << "Client " << BOLD << user->getNickname() << NORMAL;
	std::cout << " on socket " << ITALIC << user->getSocket() << RESET << YELLOW << " disconnected at " << this->getTime() << RESET << std::endl;
}

void Server::execCmd(User *user, std::string cmd) {
	std::cout << BOLD << user->getNickname() << ": " << RESET << cmd <<std::endl;

	if (user->getPassOk() == false && cmd.substr(0, 4) != "PASS") {
		sendToUser(user, "User not connected yet. Please use PASS command first.");
		std::cout << RED << BOLD << "User " << user->getNickname() << " not connected yet ; PASS command not used." << RESET << std::endl;
		disconnectUser(user);
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
		this->cmdCap(user, cmd);
	} else if (cmd.substr(0, 4) == "PING") {
		this->cmdPing(user, cmd);
	} else if (cmd.substr(0, 4) == "LIST") {
		this->cmdList(user, cmd);
	} else if (cmd.substr(0, 4) == "QUIT") {
		this->cmdQuit(user, cmd);
	} else if (cmd.substr(0, 4) == "JOIN") {
		this->cmdJoin(user, cmd);
	} else if (cmd.substr(0, 5) == "NAMES") {
		this->cmdNames(user, cmd);
	} else {
		std::cout << RED << BOLD << "Command not found." << RESET << std::endl << std::endl;
		sendToUser(user, "Command not found.");
	}
}

void Server::sendToUser(User *user, std::string msg) {
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


/* ===== COMMANDS ===== */

void Server::cmdQuit(User *user, std::string cmd) {
	std::string msg = "";
	
	if (cmd.size() > 6)
		msg = cmd.substr(6);
	
	std::cout << "Command QUIT received from " << BOLD << user->getNickname() << RESET << " with message: " << ITALIC << msg << RESET << std::endl;
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
				return;
			}
			it++;
		}

		std::cout << ORANGE << ITALIC << user->getNickname() << RESET << " is now " << CYAN << BOLD << new_nickname << RESET << std::endl << std::endl;
		user->setNickname(new_nickname);
		sendToUser(user, "Nickname updated.");
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
		disconnectUser(user);

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

		// Update the user's information
		user->setNickname(username);

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
	std::string usrs = "Users: ";
	std::string chans = "Channels: ";

	std::vector<User>::iterator it = _usrs.begin();
	std::vector<User>::iterator it2 = _chans.begin();

	std::cout << CYAN << ITALIC << "Showing users and channels." << RESET << std::endl << std::endl;
	
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

	sendToUser(user, usrs);
	sendToUser(user, chans);
}

void Server::cmdJoin(User *user, std::string cmd) {
	// ! REPLACE USER BY CHANNEL
	std::vector<Channel>::iterator it = _channels.begin();
	int tmp;

	std::string channel = cmd.substr(5);
	
	std::cout << "Command JOIN received from " << BOLD << user->getNickname() << RESET << " with channel: " << ITALIC << channel << RESET << std::endl;

	if (channel.size() == 0) {
		std::cout << RED << BOLD << "Missing channel name." << RESET << std::endl << std::endl;
		sendToUser(user, "Error: missing channel name.");
		return;
	}
	else if ((_channels.size() == 0) || (findChan(_channels, channel) == -1)) {
		_channels.push_back(channel);
		std::cout << GREEN << BOLD << "Channel " << channel << " created." << RESET << std::endl << std::endl;
		sendToUser(user, "Channel " + channel + " created.");
		it = _channels.end() - 1;
		it->addUsr(user);
		std::cout << GREEN << BOLD << user->getNickname() << " joined " << channel << RESET << std::endl << std::endl;
		it = _channels.begin();
		std::cout << "_channels size = " << _channels.size() << std::endl;
		// while((tmp <= _channels.size()) && (_channels.size() != 0))
		// {
		// 	while (i <= _channels[tmp]._usrs.size() && _channels[tmp]._usrs.size() != 0)
		// 	{
		// 		_channels[tmp]._usrs[i].getNickname();
		// 		i++;
		// 	}
		// 	i = 0;
		// 	tmp++;
		// }
	}
	else if (findChan(_channels, channel) >= 0)
	{
		tmp = findChan(_channels, channel);
		_channels[tmp].addUsr(user);
		std::cout << GREEN << BOLD << user->getNickname() << " joined " << channel << RESET << std::endl << std::endl;
	}
	// test if user is already in channel;
	// send to others that USER joined the channel;
	// 
	// 	}
	// 	it++;
	// }


	// while (it != _channels.end()) {
	// 	if (it->getNickname() == channel) {
	// 		std::cout << RED << BOLD << "Channel already exists." << RESET << std::endl << std::endl;
	// 		sendToUser(user, "Error: channel already exists.");
	// 		return;);
	// Create the channel
	// User chan;
	// chan.setTopic(channel);
	// _channels.push_back(chan);

}


// i wish i could do it other wise, without an int, but return a Channel;
int Server::findChan(std::vector<Channel> const _channels, std::string const name) const {
	size_t i = 0;
	if (_channels.size() == 0)
		return (-1);
	while (i < _channels.size())
	{
		if (_channels[i].getName() == name)
			return i;
		else
			i++;
	}
	return (-1);

	// Create the channel
	// User chan;
	// chan.setTopic(channel);
	// _channels.push_back(chan);

	
}

void Server::cmdNames(User *user, std::string cmd) {
	// Clear command
	cmd.erase(0, 6);
	
	// Parse the command to get the channel name(s)
	std::vector<std::string> channels;
	std::string::size_type channel_start = cmd.find_first_of("#");
	
	while (channel_start != std::string::npos) {
			std::string::size_type channel_end = cmd.find_first_not_of("#", channel_start);
			channels.push_back(cmd.substr(channel_start, channel_end - channel_start));
			channel_start = cmd.find_first_of("#", channel_end);
	}

	// Iterate over the channels and get the list of users
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
		/*
		
		UNCOMMENT THIS WHEN CHANNELS ARE IMPLEMENTED

		Channel* channel = getChannelByName(*it);
		if (channel != NULL) {
				// Get the list of users from the channel (getUsers() for instance)
				// On met tout dans une string et on envoie
				std::string userNames;

				userNames = "Elle, lui, nous";
				sendToUser(user, "NAMES " + *it + " :" + userNames);
		} else {
				sendToUser(user, "403 " + user->getNickname() + " " + *it + " :No such channel");
		}

		*/

		// Return a dummy list of users
		sendToUser(user, "NAMES " + *it + " :Elle, lui, nous");
		std::cout << CYAN << ITALIC << "NAMES " + *it + " : Elle, lui, nous" << RESET << NORMAL << std::endl;
	}

	if (channels.size() == 0) {
		// Send help message: use as #channel or #channel1,#channel2,#channel3
		sendToUser(user, "Usage: /names #channel1 #channel2 #channel3");
		std::cout << RED << ITALIC << "Wrong use of /names." << RESET << NORMAL << std::endl;
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
