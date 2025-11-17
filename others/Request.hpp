#pragma once
#include <iostream>

// Placeholder for incoming request class that will be passed to the server
class Request{

	private:
		std::string _name;

	public:
		void setName(const std::string& name);

};