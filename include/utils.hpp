#pragma once

#include <iostream>
#include <sstream>

namespace utils {
	// General exit codes
	constexpr int SUCCESS = 0;
	constexpr int FAILURE = 1;

	// Default path to configuration if none is passed to the program
	inline constexpr const char* DEFAULT_CONFIG_PATH
		= "configuration/simple.conf";

	// Error messages
	enum errorMacros {
			WRONG_ARGUMENTS = 1,
			FAILED_TO_CREATE_SERVERS,
			ERROR_RUNNING_SERVERS,
	};

	int returnErrorMessage(int errorCode);
}

namespace config {

	inline std::string makeError(const std::string& msg, int line, int col)
	{
		std::ostringstream oss;
		oss << msg << "at line " << line << ", col " << col;
		throw std::runtime_error(oss.str());
	}
}