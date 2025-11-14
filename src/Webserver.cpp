#include "Webserver.hpp"

static volatile sig_atomic_t g_running = 1;

static void signalHandler(int sig){
	(void)sig;
	g_running = 0;
}

/* Constructor initializing signals and setting up webserver 
   to false when object is created.
*/
Webserver::Webserver() : _running(false){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
}

/* Destructor uses RAII method to stop and clean all webserver object created
   and this calls Server cleanup in their destructors
*/
Webserver::~Webserver(){
	stopWebserver();
}

/* CreateServers groups servers based on port coincidence similarly as nginx
   or create new object Server if it is a different port
*/
int Webserver::createServers(const Configuration& config){
	std::map<int, std::vector<Configuration::ServerBlock>> portGroups;

	for (size_t i = 0; i < config.getNumberOfServers(); i++){
		const Configuration::ServerBlock& block = config.getServerBlock(i);
		portGroups[block.port].push_back(block);
	}
	for (const auto& [port, blocks] : portGroups){
		_servers.emplace_back(port, blocks);
	}
	for (size_t i = 0; i < _servers.size(); i++){
		Server::StartResult result = _servers[i].start();
		if(result != Server::START_SUCCESS){
			return utils::FAILURE;
		}
		int listenFd = _servers[i].getListenFd();
		_pollFds.push_back({listenFd, POLLIN, 0});
		_listenFdToServerIndex[listenFd] = i;
	}
	_running = true;
	return utils::SUCCESS;
}

/* Main loop to run in the program, this loop forwards requests to 
   corresponding server that matches the port, poll on ready requests and
   close fds.
*/
int Webserver::runWebserver(){
	while(_running && g_running){

	}
	return utils::SUCCESS;
}

/* Stop the main webserver on error or when the program ends,
   it will call stop to all the servers in the vector automatically.
*/
void Webserver::stopWebserver(){
	_running = false;
	for (size_t i = 0; i < _servers.size(); i++){
		_servers[i].shutdown();
	}
}

bool Webserver::isListeningSocket(int fd) const {

}

void Webserver::handleNewConnection(int listenFd){

}

void Webserver::handleClientRequest(int clientFd){

}

void Webserver::addClientToPoll(int clientFd, size_t serverIndex){

}

void Webserver::removeFdFromPoll(int fd){

}

void Webserver::removeClientFd(int clientFd){

}

void Webserver::closeAllClients(void){
	for (const auto& [fd, serverIndex] : _clientFdToServerIndex){
		(void)serverIndex;
		close(fd);
	}
	_clientFdToServerIndex.clear();
}

bool Webserver::hasError(const pollfd& pfd) const {

}