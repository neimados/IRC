/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvergobb <dvergobb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/14 17:53:09 by dvergobb          #+#    #+#             */
/*   Updated: 2023/04/14 17:56:00 by dvergobb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include "poll.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

class Server {
	public:
		Server();
		~Server();

		void startSrv();

		class SrvError : public std::exception {
			public:
				virtual const char* what() const throw();
		};


	private:
		Server(Server const &srv);
		Server &operator= (Server const &srv);

		struct pollfd			*_fds;
		int						_fdSrv;
};