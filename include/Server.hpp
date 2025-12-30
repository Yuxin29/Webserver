#pragma once

#include "ConfigBuilder.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponseHandler.hpp"

#include <arpa/inet.h>
#include <fcntl.h>

using namespace config;

class Server {
	public:
		/// Possible statuses after handling a client request.
		enum ClientStatus {
			CLIENT_INCOMPLETE,
			CLIENT_KEEP_ALIVE,
			CLIENT_COMPLETE,
			CLIENT_ERROR,
			CLIENT_WRITING
		};

		/// Possible results when starting the server.
		enum StartResult {
			START_SUCCESS,
			START_SOCKET_ERROR,
			START_BIND_ERROR,
			START_LISTEN_ERROR
		};

		/// Structure to hold data pending to be written to a client.
		struct WriteBuffer {
			std::string data;
			size_t sent = 0;
			bool keepAlive = false;

			bool isComplete() const;
			size_t remainingToSend() const;
		};

	private:
		static constexpr int MAX_REQUESTS = 20;
		static constexpr int NOT_VALID_FD = -1;

		//related to listening socket
		std::string 						_host;				///< IP address to bind
		int									_listenFd;			///< Listening socket file descriptor
		int  								_port;				///< Port number to bind
		std::vector<config::ServerConfig>	_virtualHosts;		///< Configured virtual hosts
		sockaddr_in							_addr;				///< Socket address structure

		//related to clients
		std::map<int, int>					_requestCount;		///< Count of requests per client
		std::map<int, HttpParser>			_parsers;			///< HTTP parsers per client/
		std::map<int, WriteBuffer>			_writeBuffers;		///< Pending write buffers per client

		//HttpRequest + ServerConfig → HttpResponse
		HttpResponseHandler					_httpHandler;		///< HTTP response handler

		//private helpers
		const config::ServerConfig* matchVirtualHost(const std::string& hostHeader);
		const config::ServerConfig* getDefaultVhost() const;
		
	public:
		// lifecycle management of the server
		Server() = delete;
		explicit Server(const std::string& host, int port, const std::vector<config::ServerConfig>& serverBlocks);
		Server(Server&& other) noexcept;
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();

		// startup and shutdown of the server
		StartResult start(void);
		void shutdown(void);

		// client connection handling
		int  acceptConnection(void);
		ClientStatus handleClient(int clientFd);
		ClientStatus handleClientWrite(int clientFd);

		// status checkers and cleaners
		bool hasWriteBuffer(int clientFd) const;
		void cleanMaps(int clientFd);

		//client info getters
		int  getListenFd(void) const	{return _listenFd;};
		int  getPort(void) const		{return _port;};
};
