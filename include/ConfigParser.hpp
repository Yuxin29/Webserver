#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include <map>
#include "ConfigTokenizer.hpp"

struct ServerNode
{
	std::vector<std::string> server_names;
	std::pair<std::string, int> listen;
	std::map<int, std::string> error_pages; //??
	int client_max_body_size;
	std::string root;
	std::vector<LocationNode> locations;
};

struct LocationNode
{
	std::string path;
	std::string root;
	std::string redirect;
	std::string index;
	std::string cgi_path;
	std::string upload_dir;
	bool autoindex;
	std::vector<std::string> methods;
};

class Parser
{
public:
	Parser(const std::string& filename);
	std::vector<ServerNode> parse();
	//LocationNode parseLocationBlock();

private:
	std::vector<Token> _tokens;
	std::size_t _pos;

	Token peek() const;
	Token get();
	bool match(TokenType type);
	void expect(TokenType type, const std::string& msg);
	ServerNode parseServerBlock();
	LocationNode parseLocationBlock();
	std::string parseSimpleDirective(const std::string& name);
};

#endif
