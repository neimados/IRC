/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:06 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/26 13:06:04 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Server::Server(int port, std::string password) {
	for (int i = 0; i < 10; i++){
		_fds.push_back(pollfd());
	}

	SERVER_NAME = "ft_irc";

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
		send(user->getFd(), cmsg, msg.size(), 0);
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
		if (it->isInChan(chan)) {
			sendUserInChan(&(*it), msg);
		}
	}
}

void Server::sendPrivMsgInChan(std::string chan, std::string msg, std::string user) {
	// Sending a message to all users in a channel
	std::vector<User>::iterator it;
	
	for (it = _usrs.begin(); it != _usrs.end(); it++) {
		if (it->isInChan(chan) && it->getNickname() != user) {
			sendUserInChan(&(*it), msg);
		}
	}
}

void Server::sendToUserInChan(User *user, std::string code, std::string chan, std::string msg) {
	// Sending a message to a user while he is in a channel with a custom code
	std::string start = ":IRC ";
	start += code;
	start += " ";
	start += user->getNickname();
	start += " ";
	start += chan;
	start += " :";
	
	msg = start + msg + "\n\r";
	
	const void *cmsg = msg.c_str();

	if (user->getVerification()) {
		send(user->getFd(), cmsg, msg.size(), 0);
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

Channel* Server::getChannel(std::string const name) {
	Channel *tmp = NULL;
	if (name[0] != '#')
		return tmp;
	
	// Search and return a channel by its name
	for (size_t i = 0; i < _channels.size(); i++) {
		if (_channels[i].getName() == name)
			return &_channels[i];
	}
	return tmp;
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

std::string Server::getPassword() const {
	return (this->_password);
}


/* ===== COMMANDS ===== */

void Server::cmdPing(User *user, std::string cmd) {
	std::string address = "";
	if (cmd.size() > 5)
		address = cmd.substr(5);
		
	std::cout << CYAN << "Sending PONG to address : " << ORANGE << BOLD << address << RESET << NORMAL << std::endl;
	sendToUser(user, "PONG");
}

// void Server::cmdPrivmsg(User *user, std::string cmd) {
// 	std::string msg;
// 	std::string::size_type msg_start;
	
// 	cmd.erase(0, 8);
// 	msg_start = cmd.find_first_of(":");
	
// 	if (msg_start == std::string::npos) {
// 		std::cout << RED << BOLD << "Invalid PRIVMSG message." << RESET << std::endl << std::endl;
// 		return;
// 	}

// 	msg = cmd.substr(msg_start + 1, cmd.size());
	
// 	if (cmd.find_first_of(" ") == std::string::npos){
// 		std::cout << RED << BOLD << "Invalid PRIVMSG message." << RESET << std::endl << std::endl;
// 		return;
// 	}

// 	//looking for the " " before the ":", cutting to have juste the list of destinaters separeted with ","
// 	cmd.erase(cmd.find_first_of(" "), cmd.size());;
// 	std::vector<std::string> destinataires;
// 	std::string delimiter = ",";
// 	size_t pos = 0;
// 	std::string token;
// 	//split cmd (there is only the list of destinataires at this point) string with the delimiter "," and putting it in the vector destinataires
// 	while ((pos = cmd.find(delimiter)) != std::string::npos) {
// 	    token = cmd.substr(0, pos);
// 	    destinataires.push_back(token);
// 	    cmd.erase(0, pos + delimiter.length());
// 	};
// 	destinataires.push_back(cmd);
// 	std::cout << CYAN << "Command PRIVMSG received from " << BOLD << user->getNickname() << RESET << " with message: " << ITALIC << msg << RESET << std::endl;
		

// 	// ****************************************usless to Gok ***************************** ///
// 	// Get the channel name or username
// 	// std::string destinataire;
// 	// std::string::size_type destinataire_start = cmd.find_first_of("#");
	
// 	// if (destinataire_start != std::string::npos) {
// 	// 	// destinataire is a channel
// 	// 	destinataire = clearString(cmd.substr(destinataire_start, msg_start - destinataire_start - 1));
// 	// } else {
// 	// 	// destinataire is a username
// 	// 	destinataire = clearString(cmd.substr(0, msg_start));
// 	// }
// 	// Check if the destinataire is a channel or a username
// 	// ****************************************end************************************** ///
	
// 	size_t i = 0;

// 	// checking all the strings in the vector destinataires if it's a channel or a user
// 	while (i < destinataires.size())
// 	{
// 		if (destinataires[i][0] == '#')
// 		{
// 			// destinataires[i] is a channel
// 			int chan_index = findChan(destinataires[i]);

// 			// Check if the channel is moderated and if the user is not an operator or a voiced user
			
// 			if (chan_index == -1) {
// 				// Send the error message to the user
// 				sendToUser(user, ERR_NOSUCHCHANNEL(user->getNickname(), destinataires[i]));
// 				std::cout << RED << BOLD << "Channel " << destinataires[i] << " doesn't exist." << RESET << std::endl << std::endl;
// 				continue;
// 			}
// 			if (
// 				_channels[chan_index].getIsModerated() == true
// 				&& _channels[chan_index].isOperator(user) == false
// 				&& _channels[chan_index].isVoiced(user) == false
// 			) {
// 				// Send the error message to the user
// 				sendToUserInChan(user, "404", destinataires[i], "Channel is moderated.");
// 				std::cout << RED << BOLD << "Channel " << destinataires[i] << " is moderated." << RESET << std::endl << std::endl;
// 				return;
// 			}

// 			// Check if the user is in the channel or if the user is an operator or a voiced user
// 			if (_channels[chan_index].getExternalMessage() == false && _channels[chan_index].isInChannel(user) == false)
// 			{
// 				// Send the error message to the user
// 				if (!user->isInChan(destinataires[i]))
// 					sendToUserInChan(user, "404", destinataires[i], "You're not in the channel " + destinataires[i] + ".");
// 				else
// 					sendToUser(user, "Error: you're not in the channel " + destinataires[i] + ".");
// 				std::cout << RED << BOLD << "User " << user->getNickname() << " is not in the channel " << destinataires[i] << " without external messages." << RESET << std::endl << std::endl;
// 				return;
// 			}
// 			// Send the message to the channel
// 			sendPrivMsgInChan(destinataires[i], user->getNickname() + " PRIVMSG " + destinataires[i] + " :" + msg, user->getNickname());
// 			std::cout << GREEN << BOLD << user->getNickname() << " sent a message to " << destinataires[i] << RESET << std::endl << std::endl;
// 		}
// 		else
// 		{
// 			// destinataires[i] is a username
// 			int user_index = findUser(destinataires[i]);

// 			if (user_index == -1) {
// 				// Send the error message to the user
// 				sendToUser(user, ERR_NOSUCHNICK(user->getNickname(), destinataires[i]));
// 				std::cout << RED << BOLD << "User " << destinataires[i] << " doesn't exist." << RESET << std::endl << std::endl;
// 			} else {
// 				// Send the message to the user
// 				sendToUser(&(_usrs[user_index]), user->getNickname() + " PRIVMSG " + destinataires[i] + " :" + msg);
// 				std::cout << GREEN << BOLD << user->getNickname() << " sent a message to " << destinataires[i] << RESET << std::endl << std::endl;
// 			}
// 		}
// 		i++;
// 	}
// }

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
	if (!user->isInChan(channel)) {
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

	if (!user->isInChan(channel)) {
		// Send the error message to the user
		if (user->getisInChannel() == false)
			sendToUser(user, "Error: you are not in channel " + channel + ".");
		else
			sendToUserInChan(user, "404", channel, " you are not allowed to kick " + username + " from " + channel + ".");
		
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
			sendToUserInChan(user, "404", channel, " channel " + channel + " doesn't exist.");

		std::cout << RED << BOLD << "Channel doesn't exist." << RESET << std::endl << std::endl;
		return;
	}

	// Check if the user exists with a loop
	for (size_t i = 0; i < _usrs.size(); i++) {
		if (_usrs[i].getNickname() == username) {
			// Send the KICK message to the user
			if (_usrs[i].getisInChannel() == false || !_usrs[i].isInChan(channel)) {
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
			
				_usrs[i].delChannel(_channels[chan_index].getName());
				
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

// void Server::cmdNotice(User *user, std::string cmd) {
// 	// Format of the command : "NOTICE #no :er" or "NOTICE User_5 :er"
// 	std::string msg;
// 	std::string::size_type msg_start;
	
// 	cmd.erase(0, 7);

// 	msg_start = cmd.find_first_of(":");
	
// 	if (msg_start == std::string::npos) {
// 		std::cout << RED << BOLD << "Invalid NOTICE message." << RESET << std::endl << std::endl;
// 		return;
// 	}

// 	msg = cmd.substr(msg_start + 1, cmd.size());
	
// 	std::cout << CYAN << "Command NOTICE received from " << BOLD << user->getNickname() << RESET << " with message: " << ITALIC << msg << RESET << std::endl;

// 	// Get the channel name or username
// 	std::string destinataire;
// 	std::string::size_type destinataire_start = cmd.find_first_of("#");
	
// 	if (destinataire_start != std::string::npos) {
// 		// destinataire is a channel
// 		destinataire = clearString(cmd.substr(destinataire_start, msg_start - destinataire_start - 1));
// 	} else {
// 		// destinataire is a username
// 		destinataire = clearString(cmd.substr(0, msg_start));
// 	}

// 	// Check if the destinataire is a channel or a username
// 	if (destinataire[0] == '#') {
// 		// destinataire is a channel
// 		int chan_index = findChan(destinataire);


// 		// Check if the channel is moderated and if the user is not an operator or a voiced user
// 		if (
// 			_channels[chan_index].getIsModerated() == true
// 			&& _channels[chan_index].isOperator(user) == false
// 			&& _channels[chan_index].isVoiced(user) == false
// 		) {
// 			// Send the error message to the user
// 			sendToUserInChan(user, "404", destinataire, "Channel is moderated.");
// 			std::cout << RED << BOLD << "Channel " << destinataire << " is moderated." << RESET << std::endl << std::endl;
// 			return;
// 		}
		
// 		if (chan_index == -1) {
// 			// Send the error message to the user
// 			sendToUser(user, "Error: channel " + destinataire + " doesn't exist.");

// 			std::cout << RED << BOLD << "Channel " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
// 		} else {

// 			// Check if the user is in the channel or if the user is an operator or a voiced user
// 			if (_channels[chan_index].getExternalMessage() == false && _channels[chan_index].isInChannel(user) == false) {
// 				// Send the error message to the user
// 				if (user->getisInChannel() == false)
// 					sendToUserInChan(user, "404", destinataire, "You're not in the channel " + destinataire + ".");
// 				else
// 					sendToUser(user, "Error: you're not in the channel " + destinataire + ".");
// 				std::cout << RED << BOLD << "User " << user->getNickname() << " is not in the channel " << destinataire << " without external messages." << RESET << std::endl << std::endl;
// 				return;
// 			}
			
// 			// Send the NOTICE message to the users
// 			sendAllUsersInChan(destinataire, user->getNickname() + " NOTICE " + destinataire + " :" + msg);
// 			std::cout << GREEN << BOLD << user->getNickname() << " sent a notice to " << destinataire << RESET << std::endl << std::endl;
// 		}
// 	} else {
// 		// destinataire is a username
// 		for (size_t i = 0; i < _usrs.size(); i++) {
// 			if (_usrs[i].getNickname() == destinataire) {
// 				// Send the NOTICE message to the user
// 				sendToUser(&_usrs[i], user->getNickname() + " NOTICE " + destinataire + " :" + msg);
// 				std::cout << GREEN << BOLD << user->getNickname() << " sent a notice to " << destinataire << RESET << std::endl << std::endl;
// 				return;
// 			}
// 		}

// 		// Send the error message to the user
// 		sendToUser(user, "Error: user " + destinataire + " doesn't exist.");
// 		std::cout << RED << BOLD << "User " << destinataire << " doesn't exist." << RESET << std::endl << std::endl;
// 	}
// }

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
				!user->isInChan(destinataire)
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
			// ! to update with channel -> std::vector
			// int chan_index = findChan(_usrs[user_index].getWhatChannel());

			// if (chan_index == -1) {
			// 	// Send the error message to the user
			// 	sendToUser(user, "Error: channel " + _usrs[user_index].getWhatChannel() + " doesn't exist.");
			// 	std::cout << RED << BOLD << "Channel " << _usrs[user_index].getWhatChannel() << " doesn't exist." << RESET << std::endl << std::endl;
			// } else {
			// 	// Send the MODE message to the user
			// 	sendToUser(user, "IRC 221 " + destinataire + " " + _channels[chan_index].getModeUser(&_usrs[user_index]));
			// 	std::cout << GREEN << BOLD << "Mode of " << destinataire << ": " << _channels[chan_index].getModeUser(&_usrs[user_index]) << RESET << std::endl << std::endl;
			// }
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
			// ! to update with channels std::vector
			// int chan_index = findChan(_usrs[user_index].getWhatChannel());

			// if (chan_index == -1) {
			// 	// Send the error message to the user
			// 	sendToUser(user, "Error: channel " + _usrs[user_index].getWhatChannel() + " doesn't exist.");
			// 	std::cout << RED << BOLD << "Channel " << _usrs[user_index].getWhatChannel() << " doesn't exist." << RESET << std::endl << std::endl;
			// 	return;
			// }

			// destinataire = _channels[chan_index].getName();
			// sender = user;
			// user = &_usrs[user_index];
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
				!user->isInChan(destinataire)
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
