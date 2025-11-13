#include "Webserver.hpp"
#include "Config.hpp"
#include "utils.hpp"

int main(int argc, char **argv){
	if (argc != 2){
		return utils::returnErrorMessage(utils::WRONG_ARGUMENTS);
	}
	Configuration configFile;
	/* Maybe a:
		validateFile()
		parseFile()
		validateDataInFile()
	*/
	Webserver miniNginx;
	if (miniNginx.createServers(configFile) == utils::FAILURE){
		return utils::returnErrorMessage(utils::FAILED_TO_CREATE_SERVERS);
	}
	if (miniNginx.runServers() == utils::FAILURE){
		return utils::returnErrorMessage(utils::ERROR_RUNNING_SERVERS);
	}
	miniNginx.stopServers();
	return utils::SUCCESS;
}