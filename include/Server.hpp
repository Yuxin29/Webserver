#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <errno.h>
#include "Config.hpp"
#include "Request.hpp"
#include "Http.hpp"

// Server can be called only by passing a configuration object that will be its attributes
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
		std::vector<int>		_clientFds;
		Configuration::data		_servConfig;
		struct sockaddr_in		_addr;

	public:
		Server() = delete;
		explicit Server(const Configuration& config);
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();

		// Setup and cleanup
		StartResult start();
		void shutdown();

		// Called by Webserver on demand
		int acceptConnection();
		void handleClient(int fd);

		// Getters and checking Fds
		int getListenFd() const;
		bool ownsClient(int fd) const;
};