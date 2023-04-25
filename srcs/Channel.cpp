/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guyar <guyar@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 16:04:04 by guyar             #+#    #+#             */
/*   Updated: 2023/04/25 19:13:37 by guyar            ###   ########.fr       */
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