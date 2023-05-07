/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:04:04 by guyar             #+#    #+#             */
/*   Updated: 2023/05/07 22:27:03 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Channel::Channel(std::string name): _name(name), _topic("") {}
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

// need to corecte the condition to avoid segfault;
User * Channel::getChanUsr(int i) {
    if ((_usrs.size() != 0)) //&& (_usrs->empty()))
        return &_usrs[i];
    return &_usrs[i];
}
