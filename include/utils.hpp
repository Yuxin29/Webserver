#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>

//if Declaration and definition in .hpp, we should use inline avoiding multiple definition
namespace config {
	inline std::string makeError(const std::string& msg, int line, int col)
	{
		std::ostringstream oss;
		oss << msg << "at line " << line << ", col " << col;
		throw std::runtime_error(oss.str());
	}
}


#endif
