#include "utils.hpp"

namespace utils {
	//Return an specific error message based on the error code passed to
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
		}
		std::cerr << "Error: " << errorMessage << std::endl;
		return FAILURE;
	}
}
