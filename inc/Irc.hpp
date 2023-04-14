/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:13 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/14 17:53:29 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

class Irc {
	public:
		Irc(std::string port, std::string password);
		Irc(const Irc &i);
		~Irc();

	private:
		Irc();
		Irc(Irc const &i);
		Irc &operator=(Irc const &i);

		std::string	_port;
		std::string	_password;

};
