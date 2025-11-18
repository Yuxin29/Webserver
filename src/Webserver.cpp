#include "Webserver.hpp"

static volatile sig_atomic_t signalRunning = 1;

static void signalHandler(int sig)
{
	(void)sig;
	signalRunning = 0;
}

/* Constructor initializing signals and setting up webserver 
   to false when object is created.
*/
Webserver::Webserver() : _running(false)
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
}

/* Destructor uses RAII method to stop and clean all webserver object created
   and this calls Server cleanup in their destructors
*/
Webserver::~Webserver(){
	stopWebserver();
}

/* CreateServers groups servers based on port coincidence similarly as nginx
   or create new object Server if it is on a different port
*/
int Webserver::createServers(const Configuration& config)
{
	std::map<std::string, std::vector<Configuration::ServerBlock>> bindGroups;
	for (size_t i = 0; i < config.getNumberOfServers(); i++){
		const auto& block = config.getServerBlock(i);

		std::string host = block.host;
		if (host.empty() || host == "*"){
			host = "0.0.0.0";
		}
		std::string bindKey = host + ":" + std::to_string(block.port);
		bindGroups[bindKey].push_back(block);
	}

	for (const auto& [bindKey, blocks] : bindGroups){
		size_t colonPos = bindKey.find(":");
		std::string host = bindKey.substr(0, colonPos);
		int port = stoi(bindKey.substr(colonPos + 1));
		_servers.emplace_back(host, port, blocks);
	}

	for (size_t i = 0; i < _servers.size(); i++){
		if (_servers[i].start() != Server::START_SUCCESS){
			return utils::FAILURE;
		}
		int listenFd = _servers[i].getListenFd();
		_pollFds.push_back({listenFd, POLLIN, 0});
		_listenFdToServerIndex[listenFd] = i;
	}
	return utils::SUCCESS;
}

/* Main loop to run in the program, this loop forwards requests to 
   corresponding server that matches the port, poll on ready requests and
   close fds.
*/
int Webserver::runWebserver()
{	
	_running = true;
	while(_running && signalRunning){

	}
	return utils::SUCCESS;
}

/* Stop the main webserver on error or when the program ends,
   it will call stop to all the servers in the vector automatically.
*/
void Webserver::stopWebserver()
{
	_running = false;
	for (size_t i = 0; i < _servers.size(); i++){
		_servers[i].shutdown();
	}
}

bool Webserver::isListeningSocket(int fd) const
{	
	return _listenFdToServerIndex.find(fd) != _listenFdToServerIndex.end();
}

void Webserver::handleNewConnection(int listenFd)
{	
	auto it = _listenFdToServerIndex.find(listenFd);
	if (it == _listenFdToServerIndex.end()){
		return;
	}
	uint8_t serverIndex = it->second;
	if (_servers[serverIndex].acceptConnection() < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EMFILE){
			std::cerr << "Server not read\n";
			return;
		}
	}
}

void Webserver::handleClientRequest(int clientFd)
{
	auto it = _clientFdToServerIndex.find(clientFd);
	if (it == _clientFdToServerIndex.end()){
		return;
	}
	uint8_t serverIndex = it->second;
	_servers[serverIndex].handleClient(clientFd);
}

void Webserver::addClientToPoll(int clientFd, size_t serverIndex)
{
	_clientFdToServerIndex[clientFd] = serverIndex;
	_pollFds.push_back({clientFd, POLLIN, 0});
}

void Webserver::removeFdFromPoll(int fd)
{
	for (auto it = _pollFds.begin(); it != _pollFds.end(); it++){
		if (it->fd == fd){
			_pollFds.erase(it);
			break;
		}
	}
}

void Webserver::removeClientFd(int clientFd)
{
	const auto& it = _clientFdToServerIndex.find(clientFd);
	if (it != _clientFdToServerIndex.end()){
		close (clientFd);
		_clientFdToServerIndex.erase(it);
	}
}

void Webserver::closeAllClients(void)
{
	for (const auto& [fd, serverIndex] : _clientFdToServerIndex){
		(void)serverIndex;
		close (fd);
	}
	_clientFdToServerIndex.clear();
}

bool Webserver::hasError(const pollfd& pollFd) const
{
	return (pollFd.revents & POLL_HUP) || (pollFd.revents & POLL_ERR) || (pollFd.revents & POLLNVAL);
}