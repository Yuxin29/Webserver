#include "Server.hpp"

Server::Server(const std::string& host, int port, 
	const std::vector<Configuration::ServerBlock>& serverBlocks)
	: _host(host), _listenFd(-1), _port(port), _virtualHosts(serverBlocks), _addr(){
		_addr.sin_family = AF_INET;
		if (inet_pton(AF_INET, _host.c_str(), &_addr.sin_addr) <= 0){
			throw std::runtime_error("Invalid Ip address: " + _host);
		}
		_addr.sin_port = htons(_port);
}

Server::~Server(){
	shutdown();
}

Server::Server(const Server& other)
	: _host(other._host), _listenFd(other._listenFd), _port(other._port),
		 _virtualHosts(other._virtualHosts), _addr(other._addr){}

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
		close (_listenFd);
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
	if (nBytes < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return CLIENT_INCOMPLETE;
		}
		_partialRequests.erase(clientFd);
		return CLIENT_ERROR;
	}
	if (nBytes == 0){
		_partialRequests.erase(clientFd);
		return CLIENT_ERROR;
	}
	_partialRequests[clientFd].append(buffer, nBytes);
	std::string& request = _partialRequests[clientFd];
	size_t headerEnd = request.find("\r\n\r\n");
	if (headerEnd == std::string::npos){
		return CLIENT_INCOMPLETE;
	}
	std::string hostHeader = extractHostHeader(request);
	const Configuration::ServerBlock* virtualHost = matchVirtualHost(hostHeader);
	if (!virtualHost){
		_partialRequests.erase(clientFd);
		return CLIENT_ERROR;
	}
	httpResponse response = _httpHandler.processRequest(request, *virtualHost);
	ssize_t sent = send(clientFd, response.responseData.c_str(), response.responseData.size(), 0);
	if (sent < 0){
		_partialRequests.erase(clientFd);
		return CLIENT_ERROR;
	}
	_partialRequests.erase(clientFd);
	return response.keepConnectionAlive ? CLIENT_KEEP_ALIVE : CLIENT_COMPLETE;
}

int Server::getListenFd() const {
	return _listenFd;
}

int Server::getPort() const {
	return _port;
}

const Configuration::ServerBlock* Server::matchVirtualHost(const std::string& hostHeader){
	for(size_t i = 0; i < _virtualHosts.size(); i++){
		for (size_t j = 0; j < _virtualHosts[i].serverNames.size(); j++){
			if (_virtualHosts[i].serverNames[j] == hostHeader){
				return &_virtualHosts[i];
			}
		}
	}
	if (!_virtualHosts.empty()){
		return &_virtualHosts[0];
	}
	return nullptr;
}

std::string Server::extractHostHeader(const std::string& rawRequest) const {
	std::istringstream streamRequest(rawRequest);
	std::string line;
	while (std::getline(streamRequest, line)){
		if (line.compare(0, 5, "Host:") == 0){
			size_t start = line.find_first_not_of(" \t", 5);
			if (start != std::string::npos){
				size_t end = line.find_last_of("\r\n");
				std::string host = line.substr(start, end - start);

				size_t colonPos = host.find(":");
				if (colonPos != std::string::npos){
					host = host.substr(0, colonPos);
				}
				return host;
			}	
		}
	}
	return "";
}

const Configuration::ServerBlock::LocationBlock* 
	Server::findLocation(const Configuration::ServerBlock& server,
			const std::string& path) const {
	const Configuration::ServerBlock::LocationBlock* bestMatch = nullptr;
	size_t longestPrefix = 0;

	for (size_t i = 0; i < server.locations.size(); i++){
		const std::string& locPath = server.locations[i].path;
		if (path.find(locPath) == 0){
			if (locPath.length() > longestPrefix){
				longestPrefix = locPath.length();
				bestMatch = &server.locations[i];
			}
		}
	}
	return bestMatch;
}