#include "Server.hpp"

Server::Server(const std::string& host, int port, 
	const std::vector<Configuration::ServerBlock>& serverBlocks){}

Server::~Server(){
	shutdown();
}

Server::StartResult Server::start(){
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0){
		return Server::START_SOCKET_ERROR;
	}
	int opt = 1;
	if(setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		close (_listenFd);
		return START_SOCKET_ERROR;
	}
	if (bind(_listenFd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0){
		close(_listenFd);
		return START_BIND_ERROR;
	}
	if (listen(_listenFd, SOMAXCONN) < 0){
		close (_listenFd);
		return START_LISTEN_ERROR;
	}
	int flags = fcntl(_listenFd, F_GETFL, 0);
	if (flags < 0 || fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK) < 0){
		close (_listenFd);
		return START_SOCKET_ERROR;
	}
	return Server::START_SUCCESS;
}

void Server::shutdown(){
	for (size_t i = 0; i < _virtualHosts.size(); i++){
		std::cout << "Stopping servers listening on port: " << _virtualHosts[i].port << std::endl;
	}
}

int  Server::acceptConnection(void){

}

Server::ClientStatus Server::handleClient(int clientFd){

}

int Server::getListenFd() const {
	return _listenFd;
}

int Server::getPort() const {
	return _port;
}

const Configuration::ServerBlock* matchVirtualHost(const std::string& host){

}

std::string Server::extractHostHeader(const std::string& rawRequest) const {

}

const Configuration::ServerBlock::LocationBlock* 
	Server::findLocation(const Configuration::ServerBlock& server,
		const std::string& path) const {

}

bool Server::shouldKeepAlive(const std::string& rawRequest) const {

}