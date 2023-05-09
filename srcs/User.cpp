/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:50:13 by dvergobb          #+#    #+#             */
/*   Updated: 2023/05/09 15:18:37 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/User.hpp"
#include "../inc/Channel.hpp"

User::User() {}
User::~User() {}

User::User(pollfd Client, int fd, int socket) {
    this->_fd = fd;
    this->_socket = socket;
    this->isVerified = false;
    this->userIsVerified = false;
    this->isInChannel = false;
    this->_client = Client;
    this->passOk = false;
}

// Getters
std::string User::getUsername() const {
    return this->_username;
}
std::string User::getNickname() const {
    return this->_nickname;
}
std::string User::getPassword() const {
    return this->_password;
}
std::string User::getHostname() const {
    return this->_hostname;
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
bool    User::getChannelVerification() {
    return this->isInChannel;
}
bool    User::getPassOk() {
    return this->passOk;
}
// bool    User::getIsOperator() const {
//     return this->_isOperator;
// }
// bool    User::getIsVoiced() const {
//     return this->_isVoiced;
// }

pollfd  User::getPollFd() {
    return this->_client;
}
bool     User::getisInChannel() {
    return this->isInChannel;
}
std::string User::getWhatChannel() const {
    return this->whatChannel;
}
// std::string User::getMode() const {
//     // Get the mode of the user with _isOperator and _isVoiced
//     std::string mode = "";
//     if (this->_isOperator)
//         mode += "@";
//     else if (this->_isVoiced)
//         mode += "+";

//     return mode;
// }

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
void    User::setPort(int fd) {
    this->_fd = fd;
}
void    User::setVerification(bool type) {
    this->isVerified = type;
}
void    User::setUserVerification(bool type) {
    this->userIsVerified = type;
}
void    User::setChannelVerification(bool type) {
    this->isInChannel = type;
}
void    User::setisInChannel(int n) {
    this->isInChannel = n;
}
void    User::setWhatChannel(std::string channelName) {
    this->whatChannel = channelName;
}
void    User::setPassOk(int ok) {
    this->passOk = ok;
}
void    User::setHostname(std::string hostname) {
    this->_hostname = hostname;
}

// void    User::setIsOperator(bool isOperator) {
//     this->_isOperator = isOperator;
// }

// void    User::setIsVoiced(bool isVoiced) {
//     this->_isVoiced = isVoiced;
// }
