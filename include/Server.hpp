#pragma once
#include <vector>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include "ConfigBuilder.hpp"

#include "HttpRequestParser.hpp"
#include "HttpResponseHandler.hpp"

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
		static constexpr int	MAX_REQUESTS = 20;

		std::string 				_host;
		int							_listenFd;
		int  						_port;
		std::vector<config::ServerConfig>	_virtualHosts;
		sockaddr_in					_addr;
		std::map<int, int>			_requestCount;
		std::map<int, HttpParser>	_parsers;
		HttpResponseHandler			_httpHandler;

		const config::ServerConfig* matchVirtualHost(const std::string& hostHeader);
		void  cleanMaps(int clientFd);

	public:
		Server() = delete;
		explicit Server(const std::string& host, int port, const std::vector<config::ServerConfig>& serverBlocks);
		Server(Server&& other) noexcept;
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();

		StartResult start(void);
		void shutdown(void);
		int  acceptConnection(void);
		ClientStatus handleClient(int clientFd);
		int  getListenFd(void) const;
		int  getPort(void) const;
};
