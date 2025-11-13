#pragma once
#include <iostream>

//Placeholder class to be obtained from parsed config file
class Configuration{
	
	private:
		std::string _name;
		std::string _port;

	public:
		void setName(const std::string& name);
		const std::string& getName(void);
		const std::string& getPort(void);
};
