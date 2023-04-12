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