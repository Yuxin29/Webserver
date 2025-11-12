#include "Server.hpp"

Server::Server(const Configuration& config)
	: _servConfig(config), _running(false){
	std::cout << "Server " << _servConfig.getName() 
			  << " listening to port: " << _servConfig.getPort() 
			  << " started succesfully" << std::endl;
}

Server::~Server(){
	
}

Server::StartResult Server::startServer(){}
