#pragma once
#include <iostream>

namespace utils {

	// General exit codes
	constexpr int SUCCESS = 0;
	constexpr int FAILURE = 1;

	// Default path to configuration if none is passed to the program
	inline constexpr const char* DEFAULT_CONFIG_PATH 
		= "/configuration/simple.conf";

	// Error messages
	enum errorMacros{
			WRONG_ARGUMENTS = 1,
			FAILED_TO_CREATE_SERVERS,
			ERROR_RUNNING_SERVERS,
			PARSE_ERROR,
			FAILED_TO_LOAD_DEFAULT_PATH
	};
	
	int returnErrorMessage(int errorCode);
}