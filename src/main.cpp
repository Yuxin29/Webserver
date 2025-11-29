#include "Webserver.hpp"
#include "ConfigBuilder.hpp"
#include "utils.hpp"
#include "Server.hpp"

using namespace config;
using namespace utils;

int main(int argc, char **argv){
	try {
		std::vector<ServerConfig> configs;
		if (argc == 1){
			Parser parser(DEFAULT_CONFIG_PATH);
			std::vector<ServerNode> servers = parser.parse();
			configs = ConfigBuilder::build(servers);
		} 
		else if (argc == 2){
			Parser parser(argv[1]);
			std::vector<ServerNode> servers = parser.parse();
			configs = ConfigBuilder::build(servers);
		}
		else {
			return returnErrorMessage(WRONG_ARGUMENTS);
		}
		Webserver miniNginx;
		if (miniNginx.createServers(configs) == FAILURE){
			return returnErrorMessage(FAILED_TO_CREATE_SERVERS);
		}
		if (miniNginx.runWebserver() == FAILURE){
			return returnErrorMessage(ERROR_RUNNING_SERVERS);
		}
		return SUCCESS;
	} catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << std::endl;
		return FAILURE;
	}
}
