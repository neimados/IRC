/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:50:13 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/31 11:16:54 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/User.hpp"
#include "../inc/Channel.hpp"

User::User() {}
User::~User() {}

User::User(pollfd Client, int fd, int socket) {
    this->_fd = fd;
    this->_socket = socket;
    this->isRegistered = false;
    this->isVerified = false;
    this->userIsVerified = false;
    this->isInChannel = false;
    this->_client = Client;
    this->passOk = false;
    this->_cmdBuffer = "";
}

void    User::sendToUser(std::string message) {
	message.append("\r\n");
    
	int		n;
	size_t	total = 0;
	size_t	nbytes = message.length();

	while (total < nbytes) {
		n = send(this->_fd, &(message[total]), nbytes - total, 0);

        // Check for errors
		if (n == -1) break;

        // Update total
		total += n;
	}

	if (n == -1)
		std::cerr << RED << "Error User::sendToUser" << RESET << std::endl;
}

// Getters
std::string User::getNickname() const {
    return this->_nickname;
}
std::string User::getPassword() const {
    return this->_password;
}
std::string User::getBuffer() const {
    return this->_cmdBuffer;
}

int     User::getFd() const {
    return this->_fd;
}
int     User::getSocket() const {
    return this->_socket;
}
bool    User::getVerification() {
    return this->isVerified;
}
bool    User::getUserVerification() {
    return this->userIsVerified;
}
bool    User::getPassOk() {
    return this->passOk;
}
bool   User::getIsRegistered() {
    return this->isRegistered;
}

// Setters
void    User::setNickname(std::string nickname) {
    this->_nickname = nickname.substr(0, 10);
}
void    User::setPassword(std::string password) {
    this->_password = password;
}
void    User::setUsername(std::string username) {
    this->_username = username;
}
void    User::setVerification(bool type) {
    this->isVerified = type;
}
void    User::setUserVerification(bool type) {
    this->userIsVerified = type;
}
void    User::setPassOk(bool ok) {
    this->passOk = ok;
}
void    User::setHostname(std::string hostname) {
    this->_hostname = hostname;
}
void    User::setBuffer(std::string cmd) {
    this->_cmdBuffer = cmd;
}
void    User::setIsRegistered(bool type) {
    this->isRegistered = type;
}


// New channels manager
std::vector<std::string> User::getAllChans() {
    return this->_chans;
}

bool User::isInChan(const std::string &name) {
    if (name.size() == 0)
        return false;

    // A loop that checks channel's name
    for (std::vector<std::string>::iterator it = _chans.begin(); it != _chans.end(); it++) {
		if (*it == name)
            return true;
	}
    return false;
}

void    User::addChannel(const std::string &name) {
    if (name.size() == 0)
        return;

    // Check if it already in the list ; then add it
    if (!isInChan(name))
        _chans.push_back(name);
}

void    User::delChannel(const std::string &name) {
    if (name.size() == 0)
        return;

    // A loop that checks channel's name and erase it
    for (size_t i = 0; i < this->_chans.size(); i++) {
		if (_chans[i] == name)
           this->_chans.erase(_chans.begin() + i);
	}

    if (_chans.size() == 0) {
        this->setIsInChan(false);
    }
}

void    User::setIsInChan(bool state) {
    if (this->isInChannel != state)
        this->isInChannel = state;
}
