#include "Webserver.hpp"

Webserver::Webserver(){}

// Destructor calls for servers to stop before destroying class
Webserver::~Webserver(){
	stopWebserver();
}

void Webserver::handleSignal(){}

// Create servers based on the number retrieved from the configuration file
int Webserver::createServers(const Configuration& config){
	for (size_t i = 0; i < config.getNumberOfServers(); i++){
		const auto& serverBlock = config.getServerBlock(i);
		_servers.emplace_back(serverBlock);
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