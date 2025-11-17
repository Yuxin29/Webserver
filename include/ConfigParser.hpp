#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include "ConfigTokenizer.hpp"

struct ServerNode
{
	std::vector<std::string> server_name;
	int listen;
	std::string error_age;
	std::string root;
	//....
	std::vector<LocationNode> Locations;
};

struct LocationNode
{
	std::vector<std::string> methods;
	std::string path;
	std::string root;
	std::string redirect;
	bool autoindex;
	//...
};

class Parser
{
public:
	Parser(std::string& filename);
	std::vector<ServerNode> parse();

private:
	std::vector<Token> _tokens;
	std::size_t _pos;

	Token peek() const;
	Token get();
	bool match(TokenType type);
	void expect(TokenType type, const std::string& msg);
	ServerNode parseServerBlock();
	LocationNode parseLocationBlock();

};

#endif
