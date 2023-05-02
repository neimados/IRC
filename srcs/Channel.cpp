/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guyar <guyar@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:04:04 by guyar             #+#    #+#             */
/*   Updated: 2023/05/01 17:43:59 by guyar            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/User.hpp"

Channel::Channel(std::string name): _name(name)
{
    std::cout << "Channel default constructor called" << std::endl;
}
Channel::~Channel()
{
    std::cout << "Channel default destructor called" << std::endl;
    
}

std::string Channel::getName() const {
    return this->_name;
}

void Channel::addUsr(User * user) {
    _usrs.push_back(*user);
}

std::vector<std::string> Channel::getChanUsrs() const {
    
    std::vector<std::string> list;
    int tmp = 0;

    if (_usrs.size() == 0)
        return (list);
    while (tmp < static_cast<int>(_usrs.size()))
    {
        list.push_back(_usrs[tmp].getNickname());
        tmp++;
    }
    return (list);
}

// need to corecte the condition to avoid segfault;
User * Channel::getChanUsr(int i) {
    if ((_usrs.size() != 0)) //&& (_usrs->empty()))
        return &_usrs[i];
    return &_usrs[i];
}