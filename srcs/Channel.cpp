/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:04:04 by guyar             #+#    #+#             */
/*   Updated: 2023/05/11 21:36:47 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Channel::Channel(std::string name):
    _name(name),
    _topic(""),
    _externalMessage(true),
    _isModerated(false),
    _isPrivate(false),
    _isTopicSettable(false)
    {}
Channel::~Channel() {}

std::string Channel::getName() const {
    return this->_name;
}

void Channel::addUsr(User * user) {
    // Add the user to the channel
    this->_usrs.push_back(*user);
}

bool Channel::delUsr(User *usr) {
    // Parcours du vecteur de User
    for (size_t i = 0; i < this->_usrs.size(); i++)
    {
        if (this->_usrs[i].getNickname() == usr->getNickname())
        {
            // Remove the user from the channel
            this->_usrs.erase(this->_usrs.begin() + i);
            return true;
        }
    }
    return false;
}

void Channel::updateUser(User *user) {
    // Parcours du vecteur de User
    for (size_t i = 0; i < this->_usrs.size(); i++)
    {
        if (this->_usrs[i].getFd() == user->getFd())
        {
            // Update the user in the channel
            this->_usrs[i] = *user;
            return ;
        }
    }
}

void Channel::setTopic(std::string topic) {
    this->_topic = topic;
}

std::string Channel::getTopic() const {
    if (this->_topic == "")
        return ("No topic set for this channel");
    return this->_topic;
}

std::string Channel::getChanUsrs() const {
    std::string list = "";

    for (size_t i = 0; i < this->_usrs.size(); i++)
    {
        list += this->_usrs[i].getNickname();
        if (i < this->_usrs.size() - 1)
            list += " ";
    }

    return list;
}

std::string Channel::getNbUsers() const {
    std::stringstream ss;
    ss << this->_usrs.size();
    return ss.str();
}

std::string Channel::getMode() const {
    std::string mode = "";

    // Add + before if there is one match
    if (this->_externalMessage == false || this->_isModerated || this->_isPrivate || this->_isTopicSettable)
        mode += "+";

    if (this->_externalMessage == false)
        mode += "n";
    if (this->_isModerated)
        mode += "m";
    if (this->_isPrivate)
        mode += "p";
    if (this->_isTopicSettable)
        mode += "t";

    return mode;
}

std::string Channel::getModeUser(User *user) const {
    std::string mode = " ";

    if (this->isOperator(user) && this->isVoiced(user))
        mode += "+ov";
    else if (this->isOperator(user))
        mode += "+o";
    else if (this->isVoiced(user))
        mode += "+v";

    if (mode == " ")
        mode = " - ";
    else
        mode += " ";

    return mode;
}

bool Channel::getExternalMessage() const {
    return this->_externalMessage;
}

bool Channel::getIsModerated() const {
    return this->_isModerated;
}

bool Channel::getIsPrivate() const {
    return this->_isPrivate;
}

bool Channel::getIsTopicSettable() const {
    return this->_isTopicSettable;
}

void Channel::setExternalMessage(bool externalMessage) {
    this->_externalMessage = externalMessage;
}

void Channel::setIsModerated(bool isModerated) {
    this->_isModerated = isModerated;
}

void Channel::setIsPrivate(bool isPrivate) {
    this->_isPrivate = isPrivate;
}

void Channel::setIsTopicSettable(bool isTopicSettable) {
    this->_isTopicSettable = isTopicSettable;
}

void Channel::addOperator(User *user, User *sender) {
    // Check if already operator
    if (this->isOperator(user))
        return ;
        
    // Add the user to the vector of operators
    this->_ops.push_back(user->getFd());
    
    // Send the message to the user
    std::string msg = ":" + sender->getNickname() + " MODE " + this->_name + " +o " + user->getNickname() + "\r\n";  
    
    for (size_t i = 0; i < this->_usrs.size(); i++)
        send(this->_usrs[i].getFd(), msg.c_str(), msg.size(), 0);
}

void Channel::addVoiced(User *user, User *sender) {
    // Check if already voiced
    if (this->isVoiced(user))
        return ;
        
    // Add the user to the vector of voiced
    this->_voiced.push_back(user->getFd());

    std::string msg = ":" + sender->getNickname() + " MODE " + this->_name + " +v " + user->getNickname() + "\r\n";  
    
    for (size_t i = 0; i < this->_usrs.size(); i++)
        send(this->_usrs[i].getFd(), msg.c_str(), msg.size(), 0);
}

void Channel::delOperator(User *user, User *sender) {
    // Check if already operator
    if (!this->isOperator(user))
        return ;
    
    // Remove the user from the vector of operators
    for (size_t i = 0; i < this->_ops.size(); i++) {
        if (this->_ops[i] == user->getFd())
            this->_ops.erase(this->_ops.begin() + i);
    }

    std::string msg = ":" + sender->getNickname() + " MODE " + this->_name + " -o " + user->getNickname() + "\r\n";  
    
    for (size_t i = 0; i < this->_usrs.size(); i++)
        send(this->_usrs[i].getFd(), msg.c_str(), msg.size(), 0);
}

void Channel::delVoiced(User *user, User *sender) {
    // Check if already voiced
    if (!this->isVoiced(user))
        return ;
        
    // Remove the user from the vector of voiced
    for (size_t i = 0; i < this->_voiced.size(); i++) {
        if (this->_voiced[i] == user->getFd())
            this->_voiced.erase(this->_voiced.begin() + i);
    }

    std::string msg = ":" + sender->getNickname() + " MODE " + this->_name + " -v " + user->getNickname() + "\r\n";  
    for (size_t i = 0; i < this->_usrs.size(); i++)
        send(this->_usrs[i].getFd(), msg.c_str(), msg.size(), 0);
}

bool Channel::isOperator(User *user) const {
    // Check if user's fd is in the vector of operators
    for (size_t i = 0; i < this->_ops.size(); i++) {
        if (this->_ops[i] == user->getFd())
            return true;
    }
    return false;
}

bool Channel::isVoiced(User *user) const {
    // Check if user's fd is in the vector of voiced
    for (size_t i = 0; i < this->_voiced.size(); i++) {
        if (this->_voiced[i] == user->getFd())
            return true;
    }
    return false;
}

bool Channel::isInChannel(User *user) const {
    // Check if user's fd is in the vector of users
    for (size_t i = 0; i < this->_usrs.size(); i++) {
        if (this->_usrs[i].getFd() == user->getFd())
            return true;
    }
    return false;
}

// need to corecte the condition to avoid segfault;
User * Channel::getChanUsr(int i) {
    if ((_usrs.size() != 0)) //&& (_usrs->empty()))
        return &_usrs[i];
    return &_usrs[i];
}
