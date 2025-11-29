#include "Server.hpp"

using namespace config;

Server::Server(const std::string& host, int port, 
	const std::vector<ServerConfig>& serverBlocks)
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
		 _virtualHosts(other._virtualHosts), _addr(other._addr),
		 	_requestCount(other._requestCount), _parsers(other._parsers){}

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
		_parsers.erase(clientFd);
		return CLIENT_ERROR;
	}
	if (nBytes == 0){
		cleanMaps(clientFd);
		return CLIENT_ERROR;
	}
	if (_requestCount[clientFd] >= 100){
		cleanMaps(clientFd);
		return CLIENT_COMPLETE;
	}
	_requestCount[clientFd]++;
	HttpParser& parser = _parsers[clientFd];
	std::string chunk(buffer, nBytes);
	HttpRequest request = parser.parseHttpRequest(chunk);
	if (parser._state != DONE){
		return CLIENT_INCOMPLETE;
	}
	std::map<std::string, std::string> headers = request.getrequestHeaders();
	auto it = headers.find("Host");
	if (it == headers.end()){
		cleanMaps(clientFd);
		return CLIENT_ERROR;
	}
	std::string hostHeader = it->second;
	const ServerConfig* virtualHost = matchVirtualHost(hostHeader);
	if (!virtualHost){
		_parsers.erase(clientFd);
		return CLIENT_ERROR;
	}
	HttpResponse response = _httpHandler.handleRequest(request, virtualHost);
	std::string responseString = response.buildResponseString();
	ssize_t sent = send(clientFd, responseString.c_str(), responseString.size(), 0);
	if (sent < 0){
		_parsers.erase(clientFd);
		return CLIENT_ERROR;
	}
	bool keepAlive = response._keepConnectionAlive; //do i need to access it directly? 
	if (keepAlive){
		_parsers[clientFd] = HttpParser();
		return CLIENT_KEEP_ALIVE;
	} else {
		cleanMaps(clientFd);
		return CLIENT_COMPLETE;
	}
}

int Server::getListenFd() const {
	return _listenFd;
}

int Server::getPort() const {
	return _port;
}

const ServerConfig* Server::matchVirtualHost(const std::string& hostHeader){
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

void  Server::cleanMaps(int clientFd){
	_parsers.erase(clientFd);
	_requestCount.erase(clientFd);
}
