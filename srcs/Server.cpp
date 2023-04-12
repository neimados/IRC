#include "../inc/Server.hpp"

Server::Server(){
	_fds = new struct pollfd[10];
	_fdSrv = 0;
	std::string Port = "6666";
	int yes = 1;
	int status;
	struct addrinfo hint, *serverinfo, *tmp;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = getprotobyname("TCP")->p_proto;

	status = getaddrinfo("0.0.0.0", Port.c_str(), &hint, &serverinfo);

	if (status != 0)
		throw SrvError();

	for (tmp = serverinfo; tmp != NULL; tmp = tmp->ai_next)
	{
		_fdSrv= socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (_fdSrv < 0)
			continue;

		setsockopt(_fdSrv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if (bind(_fdSrv, tmp->ai_addr, tmp->ai_addrlen) < 0)
		{
			close(_fdSrv);
			continue;
		}
		break;
	}
	freeaddrinfo(serverinfo);
	if (listen(_fdSrv, 10) == -1)
	{
		throw SrvError();
	}
	_fds[0].fd = _fdSrv;
	_fds[0].events = POLLIN;
}

Server::~Server(){}

void Server::startSrv() {
	int pollTest, fdTest = 0;
	struct sockaddr_storage	remote;
	socklen_t				addrlen;

	//boucle principale
	while (1) {
		std::cout<<"Listening on port 6666. Essayez nc 127.0.0.1 6666"<<std::endl;
		pollTest = poll(_fds, 1, -1); 
		if (pollTest == -1)
			throw SrvError();
		addrlen = sizeof remote;
		fdTest = accept(_fds[0].fd, (struct sockaddr*)&remote, &addrlen);
		send(fdTest, "coucou 42", 9, 0);
	}
}

const char* Server::SrvError::what() const throw(){
	return "Server Error !";
}
