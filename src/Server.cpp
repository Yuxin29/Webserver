#include "Server.hpp"

Server::Server(const Configuration::ServerBlock& serverBlock)
	: _listenFd(-1), _servConfig(serverBlock), _addr(){}

Server::~Server(){
	shutdown();
}

Server::Server(const Server& other)
	: _listenFd(other._listenFd), _servConfig(other._servConfig), _addr(other._addr){}

void Server::shutdown(){}