#include "utils.hpp"

namespace utils {

	int returnErrorMessage(int errorCode){
		std::string errorMessage;
		switch (errorCode){
			case 1:
				errorMessage = "Invalid number of arguments";
				break;
			case 2:
				errorMessage = "Failed to create one or more servers";
				break;
			case 3:
				errorMessage = "One or more servers could not be started";
				break;
			case 4:
				errorMessage = "Some parsing error for Lin to check";
				break;
		}
		std::cout << "Error: " << errorMessage << std::endl;
		return FAILURE;
	}
}