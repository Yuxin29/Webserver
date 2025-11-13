#include "Server.hpp"

Server::Server(const Configuration& config){}

Server::~Server(){
	stopServer();
}

Server::StartResult Server::startServer(){}

void Server::stopServer(){}

void Server::handleRequest(){}

void Server::acceptNewRequests(){}

void Server::closeRequests(){}