#include "Webserver.hpp"

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

int Webserver::createServers(const Configuration& config){
	std::map<std::string, std::vector<Configuration::ServerBlock>> bindGroups;
	for (size_t i = 0; i < config.getNumberOfServers(); i++){
		const auto& block = config.getServerBlock(i);

		std::string host = block.host;
		if (host.empty() || host == "*"){
			host = "0.0.0.0";
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
			return utils::FAILURE;
		}
		int listenFd = _servers[i].getListenFd();
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = listenFd;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, listenFd, &ev) < 0){
			return utils::FAILURE;
		}
		_listenFdToServerIndex[listenFd] = i;
	}
	return utils::SUCCESS;
}

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
		if (nfds == 0){ //expand here eventually to timeout idle connections
			continue;
		}
		for (int i = 0; i < nfds; i++){
			int fd = events[i].data.fd;
			if (events[i].events & EPOLLIN){
				if (isListeningSocket(fd)){
					handleNewConnection(fd);
				} else {
					handleClientRequest(fd);
				}
			}
			if (hasError(events[i])){
				removeClientFd(fd);
			}
		}
	}
	return utils::SUCCESS;
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
	const auto& it = _clientFdToServerIndex.find(clientFd);
	if (it == _clientFdToServerIndex.end()){
		return;
	}
	size_t serverIndex = it->second;
	Server::ClientStatus status = _servers[serverIndex].handleClient(clientFd);
	switch (status){
	case Server::CLIENT_INCOMPLETE:
	case Server::CLIENT_KEEP_ALIVE:
		break;
	case Server::CLIENT_COMPLETE:
	case Server::CLIENT_ERROR:
		removeClientFd(clientFd);
		break;
	}
}

void Webserver::addClientToPoll(int clientFd, size_t serverIndex){
	_clientFdToServerIndex[clientFd] = serverIndex;
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = clientFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &ev) < 0){
		std::cerr << strerror(errno) << std::endl;
	}
}

void Webserver::removeFdFromPoll(int fd){
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
}

void Webserver::removeClientFd(int clientFd){
	const auto& it = _clientFdToServerIndex.find(clientFd);
	if (it != _clientFdToServerIndex.end()){
		_clientFdToServerIndex.erase(it);
	}
	removeFdFromPoll(clientFd);
}

void Webserver::closeAllClients(void){
	for (const auto& [fd, serverIndex] : _clientFdToServerIndex){
		(void)serverIndex;
		close (fd);
	}
	_clientFdToServerIndex.clear();
}

bool Webserver::hasError(const epoll_event& event) const{
	return (event.events & EPOLLHUP) ||
		   (event.events & EPOLLERR) ||
		   (event.events & EPOLLRDHUP);
}