/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Irc.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:33 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/14 17:53:40 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Irc.hpp"

Irc::Irc(std::string port, std::string password){
	_port = port;
	_password = password;
	
}

Irc::Irc(const Irc &i){
	*this = i;
}

Irc::~Irc(){}

