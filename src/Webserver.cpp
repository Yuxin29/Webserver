#include "Webserver.hpp"

Webserver::Webserver(){}

/* Destructor uses RAII method to stop and clean all webserver object created
   and this calls Server cleanup in their destructors
*/
Webserver::~Webserver(){
	stopWebserver();
}

/* Signal handling of SIGINT and more
   moreeeee
*/
void Webserver::handleSignal(){

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
	return utils::SUCCESS;
}

int Webserver::runWebserver(){
	
	return utils::SUCCESS;
}

// Stop all the servers at error or when the program ends
void Webserver::stopWebserver(){
	for (size_t i = 0; i < _servers.size(); i++){
		_servers[i].shutdown();
	}
}