#include "Server.hpp"

Server::Server(const std::string& host, int port, 
	const std::vector<Configuration::ServerBlock>& serverBlocks)
	: _host(host), _port(port), _virtualHosts(serverBlocks), _listenFd(-1), _addr(){
		_addr.sin_family = AF_INET;
		if (inet_pton(AF_INET, _host.c_str(), &_addr.sin_addr) <= 0){
			throw std::runtime_error("Invalid Ip address: " + _host);
		}
		_addr.sin_port = htons(_port);
}


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
	if (_listenFd != -1){
		std::cout << "Stopping servers listening on port: " << _port << std::endl;
		close (_listenFd);
		_listenFd = -1;
	}
}

int  Server::acceptConnection(void){
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFd = accept(_listenFd, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return -1;
		}
		std::cerr << "Accept error: " << strerror(errno) << std::endl;
		return -1;
	}
	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags < 0 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) < 0){
		close (clientFd);
		return -1;
	}
	return clientFd;

}

Server::ClientStatus Server::handleClient(int clientFd){
	char buffer[8192];
	ssize_t nBytes = recv(clientFd, buffer, sizeof(buffer), 0);
	if (nBytes <= 0){
		if (nBytes == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)){
			close (clientFd);
			return CLIENT_ERROR;
		}
		return CLIENT_INCOMPLETE;
	}

	//Temporary hardcoded to test server
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
	send (clientFd, response.c_str(), response.size(), 0);
	close (clientFd);
	return CLIENT_COMPLETE;
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