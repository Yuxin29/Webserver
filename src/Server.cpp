#include "Server.hpp"

Server::Server(const Configuration& config)
	: _servConfig(config), _running(false){
}

Server::~Server(){
	stopServer();
}

Server::StartResult Server::startServer(){
	std::cout << "Server " << _servConfig.getName() 
			  << " listening to port: " << _servConfig.getPort() 
			  << " started succesfully" << std::endl;
}

void Server::stopServer(){}

void Server::handleRequest(){}

void Server::acceptNewRequests(){}

void Server::closeRequests(){}