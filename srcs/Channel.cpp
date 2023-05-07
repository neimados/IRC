/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:04:04 by guyar             #+#    #+#             */
/*   Updated: 2023/05/08 00:28:51 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Channel::Channel(std::string name):
    _name(name),
    _topic(""),
    _externalMessage(false),
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

    if (this->_externalMessage)
        mode += "n";
    if (this->_isModerated)
        mode += "m";
    if (this->_isPrivate)
        mode += "p";
    if (this->_isTopicSettable)
        mode += "t";

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

std::vector<User> Channel::getOps() const {
    std::vector<User> ops;

    for (size_t i = 0; i < this->_usrs.size(); i++) {
        if (this->_usrs[i].getIsOperator())
            ops.push_back(this->_usrs[i]);
    }

    return ops;
}

std::vector<User> Channel::getVoiced() const {
    std::vector<User> voiced;

    for (size_t i = 0; i < this->_usrs.size(); i++) {
        if (this->_usrs[i].getIsVoiced())
            voiced.push_back(this->_usrs[i]);
    }

    return voiced;
}
// need to corecte the condition to avoid segfault;
User * Channel::getChanUsr(int i) {
    if ((_usrs.size() != 0)) //&& (_usrs->empty()))
        return &_usrs[i];
    return &_usrs[i];
}
