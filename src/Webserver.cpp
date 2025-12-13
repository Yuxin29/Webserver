#include "Webserver.hpp"
#include <fstream>
#include <sstream>

using namespace config;
using namespace utils;

static volatile sig_atomic_t signalRunning = 1;

static void signalHandler(int sig){
	(void)sig;
	signalRunning = 0;
}

Webserver::Webserver() : _running(false){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, SIG_IGN);
	_epollFd = epoll_create1(0);
	if (_epollFd < 0){
		throw std::runtime_error("Failed to create epoll instance");
	}
}

Webserver::~Webserver(){
	stopWebserver();
	if (_epollFd >= 0){
		close(_epollFd);
	}
}

/* This creates the necessary servers, grouping them by host:port. Call Server::Start to bind ips and create listening sockets.*/
int Webserver::createServers(const std::vector<ServerConfig>& config){
	std::map<std::string, std::vector<config::ServerConfig>> bindGroups;
	for (size_t i = 0; i < config.size(); i++){
		const auto& block = config[i];
		std::string host = block.host;
		if (host.empty() || host == "*"){
			host = "0.0.0.0";
		}
		if (host == "255.255.255.255"){
			std::cout << "Binding to special 255.255.255.255, No connection expected" << std::endl;
		}
		std::string bindKey = host + ":" + std::to_string(block.port);
		bindGroups[bindKey].push_back(block);
	}

	for (const auto& [bindKey, blocks] : bindGroups){
		size_t colonPos = bindKey.find(":");
		std::string host = bindKey.substr(0, colonPos);
		int port = stoi(bindKey.substr(colonPos + 1));
		_servers.emplace_back(host, port, blocks);
	}

	for (size_t i = 0; i < _servers.size(); i++){
		if (_servers[i].start() != Server::START_SUCCESS){
			return FAILURE;
		}
		int listenFd = _servers[i].getListenFd();
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = listenFd;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, listenFd, &ev) < 0){
			return FAILURE;
		}
		_listenFdToServerIndex[listenFd] = i;
	}
	
	for (size_t i = 0; i < _servers.size(); i++){
		std::cout << "Server successfully listening on port: " 
				<< _servers[i].getPort() << std::endl;
	}
	return SUCCESS;
}

/* Main loop running during server execution. It handles Epoll events, manages server and client connections.*/
int Webserver::runWebserver(){	
	_running = true;
	const int MAX_EVENTS = 64;
	struct epoll_event events[MAX_EVENTS];
	while(_running && signalRunning){
		int nfds = epoll_wait(_epollFd, events, MAX_EVENTS, 1000);
		if (nfds < 0){
			if (errno == EINTR){
				continue;
			}
			return utils::FAILURE;
		}
		if (nfds == 0){
			checkIdleConnections();
			continue;
		}
		for (int i = 0; i < nfds; i++){
			int fd = events[i].data.fd;
			if (events[i].events & EPOLLIN){
				if (isListeningSocket(fd)){
					handleNewConnection(fd);
				} else if (!isFdWriting(fd)) {
					handleClientRequest(fd);
				}
			}
			if (events[i].events & EPOLLOUT){
				handleClientWrite(fd);
			}
			if (hasError(events[i])){
				removeClientFd(fd);
			}
		}
	}
	return SUCCESS;
}

void Webserver::stopWebserver(){
	_running = false;
	closeAllClients();
	for (size_t i = 0; i < _servers.size(); i++){
		_servers[i].shutdown();
	}
}

bool Webserver::isListeningSocket(int fd) const {	
	return _listenFdToServerIndex.find(fd) != _listenFdToServerIndex.end();
}

void Webserver::handleNewConnection(int listenFd){	
	const auto& it = _listenFdToServerIndex.find(listenFd);
	if (it == _listenFdToServerIndex.end()){
		return;
	}
	size_t serverIndex = it->second;
	int clientFd = _servers[serverIndex].acceptConnection();
	if (clientFd < 0){
		return;
	}
	addClientToPoll(clientFd, serverIndex);
}

void Webserver::handleClientRequest(int clientFd){
	auto it = _clientFdToServerIndex.find(clientFd);
	if (it == _clientFdToServerIndex.end()){
		return;
	}
	time_t now = time(NULL);
	size_t serverIndex = it->second;
	Server::ClientStatus status = _servers[serverIndex].handleClient(clientFd);

	switch (status){
	case Server::CLIENT_INCOMPLETE:
		if (now - _lastActivity[clientFd] > CONNECTION_TIMEOUT){ 
			std::cerr << "Connection timedout on Fd: " << clientFd << std::endl;
			sendTimeoutResponse(clientFd);
			removeClientFd(clientFd);
			return;
		}
		_lastActivity[clientFd] = now;
		break;
	case Server::CLIENT_WRITING:
		modifyClientEvents(clientFd, EPOLLOUT);
		_lastActivity[clientFd] = now;
		break;
	case Server::CLIENT_KEEP_ALIVE:
		_lastActivity[clientFd] = now;
		break;
	case Server::CLIENT_COMPLETE:
	case Server::CLIENT_ERROR:
		removeClientFd(clientFd);
		break;
	}
}

void Webserver::handleClientWrite(int clientFd){
	auto it = _clientFdToServerIndex.find(clientFd);
	if (it == _clientFdToServerIndex.end()){
		return ;
	}
	size_t serverIndex = it->second;
	Server::ClientStatus status = _servers[serverIndex].handleClientWrite(clientFd);

	switch (status){
	case Server::CLIENT_WRITING:
		_lastActivity[clientFd] = time(NULL);
		break;
	case Server::CLIENT_KEEP_ALIVE:
		modifyClientEvents(clientFd, EPOLLIN);
		_lastActivity[clientFd] = time(NULL);
		break;
	case Server::CLIENT_COMPLETE:
	case Server::CLIENT_ERROR:
		removeClientFd(clientFd);
		break;
	case Server::CLIENT_INCOMPLETE:
		break;
	}
}

void Webserver::modifyClientEvents(int clientFd, uint32_t events){
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = clientFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientFd, &ev) < 0){
		std::cerr << "epoll_ctl MOD failed: " << strerror(errno) << std::endl;
		removeClientFd(clientFd);
	}
}

void Webserver::addClientToPoll(int clientFd, size_t serverIndex){
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = clientFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &ev) < 0){
		close (clientFd);
		std::cerr << strerror(errno) << std::endl;
		return;
	}
	_clientFdToServerIndex[clientFd] = serverIndex;
	_lastActivity[clientFd] = time(NULL);
}

void Webserver::removeFdFromPoll(int fd){
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) < 0){
		if (errno != ENOENT && errno != EBADF){
			std::cerr << "epoll_ctl_del failed on fd " << fd << ": "
				 << strerror(errno) << std::endl;
		}
	}
}

void Webserver::removeClientFd(int clientFd){
	_lastActivity.erase(clientFd);
	const auto& it = _clientFdToServerIndex.find(clientFd);
	if (it != _clientFdToServerIndex.end()){
		_clientFdToServerIndex.erase(it);
	}
	removeFdFromPoll(clientFd);
	close (clientFd);
}

void Webserver::closeAllClients(void){
	for (const auto& [fd, serverIndex] : _clientFdToServerIndex){
		(void)serverIndex;
		close (fd);
	}
	_clientFdToServerIndex.clear();
}

void Webserver::checkIdleConnections(){
	time_t now = time(NULL);
	std::vector<int> toRemove;
	for (const auto& [fd, lastTime] : _lastActivity){
		if (now - lastTime > CONNECTION_TIMEOUT){
			toRemove.push_back(fd);
		}
	}
	for (int fd : toRemove){
		std::cerr << "Idle connection timeout on fd: " << fd << std::endl;
		if (!isFdWriting(fd)){
			sendTimeoutResponse(fd);
		}
		removeClientFd(fd);
	}
}

bool Webserver::isFdWriting(int clientFd) const {
	auto it = _clientFdToServerIndex.find(clientFd);
	if (it == _clientFdToServerIndex.end()){
		return false;
	}
	return _servers[it->second].hasWriteBuffer(clientFd);
}

bool Webserver::hasError(const epoll_event& event) const {
	return (event.events & EPOLLHUP) ||
		   (event.events & EPOLLERR) ||
		   (event.events & EPOLLRDHUP);
}

void Webserver::sendTimeoutResponse(int clientFd){
	std::string body;
	std::ifstream file("sites/static/errors/408.html");
		
	if (file.is_open()){
		std::stringstream buffer;
		buffer << file.rdbuf();
		body = buffer.str();
		file.close();
	} else {
		body = "<h1>408 Request Timeout</h1>";
	}
	std::string response = "HTTP/1.1 408 Request Timeout\r\n";
	response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;
	
	send(clientFd, response.c_str(), response.size(), 0);
}
