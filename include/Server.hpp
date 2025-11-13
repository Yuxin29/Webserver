#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <errno.h>
#include "Config.hpp"
#include "Request.hpp"
#include "Http.hpp"

class Server{

	public:
		enum StartResult{
			START_SUCCESS,
			START_SOCKET_ERROR,
			START_BIND_ERROR,
			START_LISTEN_ERROR
		};

	private:
		int						_listenFd;
		std::vector<Request>	_requests;
		Configuration::data		_servConfig;
		struct sockaddr_in		_addr;
		bool					_running;

	public:
		Server() = delete;
		explicit Server(const Configuration& config);
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();

		StartResult startServer();
		void runServer();
		void stopServer();
		void handleRequest();
		void acceptNewRequests();
		void closeRequests();
};