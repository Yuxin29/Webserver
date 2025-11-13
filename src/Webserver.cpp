#include "Webserver.hpp"

Webserver::Webserver(){}

// Destructor calls for servers to stop before destroying class
Webserver::~Webserver(){
	stopServers();
}

void Webserver::handleSignal(){}

// Create servers based on the number retrieved from the configuration file
int Webserver::createServers(const Configuration& file){

	return utils::SUCCESS;
}

int Webserver::runServers(){
	
	return utils::SUCCESS;
}

// Stop all the servers at error or when the program ends
void Webserver::stopServers(){
	for (size_t i = 0; i < _servers.size(); i++){
		_servers[i].shutdown();
	}
}