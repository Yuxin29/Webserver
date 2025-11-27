#include "utils.hpp"

namespace utils {

	int returnErrorMessage(int errorCode){
		std::string errorMessage;
		switch (errorCode){
			case WRONG_ARGUMENTS:
				errorMessage = "Invalid number of arguments";
				break;
			case FAILED_TO_CREATE_SERVERS:
				errorMessage = "Failed to create one or more servers";
				break;
			case ERROR_RUNNING_SERVERS:
				errorMessage = "One or more servers could not be started";
				break;
			case PARSE_ERROR:
				errorMessage = "Some parsing error for Lin to check";
				break;
			case FAILED_TO_LOAD_DEFAULT_PATH:
				errorMessage = "Could not find the file in default path";
				break;
		}
		std::cerr << "Error: " << errorMessage << std::endl;
		return FAILURE;
	}
}