/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 22:50:13 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/15 00:33:56 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/User.hpp"

User::User() {}
User::~User() {}

User::User(pollfd Client, int fd) {
    this->_fd = fd;
    this->isVerified = false;
    this->userIsVerified = false;
    this->nickIsVerified = false;
    this->isInChannel = false;
    this->noChannels = 0;
    this->_client = Client;
}

User User::USER(std::string username) {
    this->setUsername(username);
    this->setUserVerification(1);
    
    send(this->getFd(), "\nUsername set to ", 18, 0);
    send(this->getFd(), this->getUsername().data(), username.size(), 0);
    send(this->getFd(), "\n", 2, 0);
    return (*this);
}

User    User::NICK(std::string nickname) {
    this->setNickname(nickname.substr(0, nickname.size()));
    this->setNickVerification(1);
    
    send(this->getFd(), "\nNickname set to ", 17, 0);
    send(this->getFd(), this->getNickname().data(), nickname.size(), 0);
    send(this->getFd(), "\n", 2, 0);
    return (*this);
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

int     User::getFd() const {
    return this->_fd;
}
bool    User::getVerification() {
    return this->isVerified;
}
bool    User::getUserVerification() {
    return this->userIsVerified;
}
bool    User::getNickVerification() {
    return this->nickIsVerified;
}
bool    User::getChannelVerification() {
    return this->isInChannel;
}

pollfd  User::getPollFd() {
    return this->_client;
}
int     User::getisInChannel() {
    return this->isInChannel;
}
std::string User::getWhatChannel() const {
    return this->whatChannel;
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
void    User::setPort(int fd) {
    this->_fd = fd;
}
void    User::setVerification(bool type) {
    this->isVerified = type;
}
void    User::setUserVerification(bool type) {
    this->userIsVerified = type;
}
void    User::setNickVerification(bool type) {
    this->nickIsVerified = type;
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

// Commands
bool    User::channelLimit() {
    if (this->noChannels >= 10)
        return false;
    return true;
}

void    User::increaseChannelNo() {
    this->noChannels++;
}
void    User::decreaseChannelNo() {
    this->noChannels--;
}
