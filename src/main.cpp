#include "Webserver.hpp"
#include "Config.hpp"
#include "utils.hpp"

int main(int argc, char **argv){
	try {
		Configuration configFile;
		if (argc == 1){
			if (!configFile.load(utils::DEFAULT_CONFIG_PATH)){
				return utils::returnErrorMessage(utils::FAILED_TO_LOAD_DEFAULT_PATH);
			}
		} 
		else if (argc == 2){
			(void)argv;
			/* Maybe a:
				validateFile()
				parseFile()
				validateDataInFile()
			*/
		}
		else {
			return utils::returnErrorMessage(utils::WRONG_ARGUMENTS);
		}
		Webserver miniNginx;
		if (miniNginx.createServers(configFile) == utils::FAILURE){
			return utils::returnErrorMessage(utils::FAILED_TO_CREATE_SERVERS);
		}
		if (miniNginx.runWebserver() == utils::FAILURE){
			return utils::returnErrorMessage(utils::ERROR_RUNNING_SERVERS);
		}
		return utils::SUCCESS;
	} catch (const std::exception& e){
		std::cerr << "Error: " << e.what() << std::endl;
		return utils::FAILURE;
	}
}