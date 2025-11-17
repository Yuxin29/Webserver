#pragma once
#include <iostream>

namespace utils {

	// General exit codes
	constexpr int SUCCESS = 0;
	constexpr int FAILURE = 1;

	// Error messages
	enum error_macros{
			WRONG_ARGUMENTS = 1,
			FAILED_TO_CREATE_SERVERS,
			ERROR_RUNNING_SERVERS,
			PARSE_ERROR
	};
	
	int returnErrorMessage(int errorCode);
}