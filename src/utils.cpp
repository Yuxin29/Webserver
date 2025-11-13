#include "Webserver.hpp"

int exitWithError(int errorCode){
	std::string errorMessage;
	switch (errorCode){
		case 1:
			errorMessage = "Invalid number of arguments";
			break;
		case 2:
			errorMessage = "One or more servers failed to start";
			break;
	}
	std::cout << "Error: " << errorMessage << std::endl;
	return FAILURE;
}