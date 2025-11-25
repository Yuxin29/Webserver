#pragma once
#include <vector>
#include <map>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "Config.hpp"
#include "HttpRequest.hpp"
#include "Http.hpp"

class Server {
	public:
		enum ClientStatus {
			CLIENT_INCOMPLETE,
			CLIENT_KEEP_ALIVE,
			CLIENT_COMPLETE,
			CLIENT_ERROR
		};

		enum StartResult {
			START_SUCCESS,
			START_SOCKET_ERROR,
			START_BIND_ERROR,
			START_LISTEN_ERROR
		};

	private:
		std::string 	_host;
		int				_listenFd;
		int  			_port;
		std::vector<Configuration::ServerBlock>	_virtualHosts;
		sockaddr_in		_addr;
		Http			_httpHandler;
		std::map<int, std::string> _partialRequests;

		const Configuration::ServerBlock* matchVirtualHost(const std::string& hostHeader);
		std::string extractHostHeader(const std::string& rawRequest) const;
		const Configuration::ServerBlock::LocationBlock* findLocation(const Configuration::ServerBlock& server,
						const std::string& path) const;

	public:
		Server() = delete;
		explicit Server(const std::string& host, int port, const std::vector<Configuration::ServerBlock>& serverBlocks);
		Server(const Server& other);
		Server& operator=(const Server& other) = delete;
		~Server();

		StartResult start(void);
		void shutdown(void);
		int  acceptConnection(void);
		ClientStatus handleClient(int clientFd);
		int  getListenFd(void) const;
		int  getPort(void) const;
};