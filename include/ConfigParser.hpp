#pragma once

#include "ConfigTokenizer.hpp"

#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace config{
	// Represents a location block in the configuration file.
	struct LocationNode
	{
		std::string 				path;				///< Location path
		std::string 				root;				///< Root directory for this location	
		std::string 				redirect;			///< Redirect URL for this location
		std::vector<std::string> 	index;				///< Default pages for this location
		std::string 				cgiPass;			///< CGI executable path
		std::string 				cgiExt;				///< CGI file extension
		std::string 				uploadDir;			///< Upload directory for this location
		std::string 				clientMaxBodySize;	///< Max body size for this location
		bool 						autoindex;			///< Directory listing  enabled/disabled
		std::vector<std::string> 	methods;			///< Allowed HTTP methods for this location
	};

	// Represents a server block in the configuration file.
	struct ServerNode
	{
		std::vector<std::string> 	serverNames;		///< Server names
		std::pair<std::string, int> listen;				///< IP and port to listen on
		std::map<int, std::string> 	errorPages;			///< Custom error pages
		std::string 				clientMaxBodySize;	///< Max body size for this server
		std::string 				 root;				///< Root directory for this server
		std::vector<std::string> 	index;				///< Default pages for this server
		std::vector<LocationNode> 	locations;			///< Location blocks within this server
	};

	class Parser
	{
	private:
		std::vector<Token>	_tokens;
		std::size_t 		_pos;
		
		Token	peek() const;
		Token	get();
		bool 	eof();
		bool	match(TokenType type);
		void	expect(TokenType type, const std::string& msg);
		bool 	isKeyword(std::string& s);

		ServerNode					parseServerBlock();
		LocationNode				parseLocationBlock();
		std::string					parseSimpleDirective(const std::string& str);
		std::vector<std::string> 	parseVectorStringDirective(const std::string& str);

	public:
		Parser(const std::string& filename);
		std::vector<ServerNode> parse();
	};
}
