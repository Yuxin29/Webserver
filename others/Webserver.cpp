#include "Webserver.hpp"

Webserver::Webserver(){}

Webserver::~Webserver(){
	stopServers();
}

int Webserver::createServers(const Configuration& file){

	return utils::SUCCESS;
}

int Webserver::runServers(){}

void Webserver::stopServers(){
	for (size_t i = 0; i < _servers.size(); i++){
		_servers[i].stopServer();
	}
}