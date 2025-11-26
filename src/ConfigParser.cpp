#include "ConfigParser.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream> //std::stringstream
#include <iostream>

namespace config{
	Parser::Parser(const std::string& filename)
	:_pos(0)
	{
		std::ifstream infile(filename);
		if (!infile)
			throw std::runtime_error("Failed to open config file " + filename);
		std::stringstream buffer;
		buffer << infile.rdbuf();//.rdbuf() gives access to its underlying stream buffer.
		std::string content = buffer.str();
		Tokenizer tokenizer(content);
		_tokens = tokenizer.tokenize();

		//debug
		for (auto &tk : _tokens) {
		std::cout << "[" << tk.value << "] "
				<< tk.type
				<< " line " << tk.line
				<< " col " << tk.col << "\n";
			} //for print the token list, debug
	}

	Token Parser::peek() const
	{
		if(_pos >= _tokens.size())
			return Token{TK_EOF, "", 0, 0};
		return _tokens[_pos];
	}

	Token Parser::get()
	{
		if (_pos >= _tokens.size())
			return Token{TK_EOF, "", 0, 0};
		return _tokens[_pos++];
	}

	bool Parser::eof()
	{
		return _pos >= _tokens.size();
	}

	bool Parser::match(TokenType type)
	{
		if (peek().type == type)
		{
			get();
			return true;
		}
		return false;
	}

	void Parser::expect(TokenType type, const std::string& msg)
	{
		Token cur_token = get();
		if (cur_token.type != type)
			throw std::runtime_error(makeError(msg, cur_token.line, cur_token.col));
	}

	std::string Parser::parseSimpleDirective()
	{
		get();
		Token valuetoken = get();
		if (valuetoken.type != TK_IDENTIFIER)
			throw std::runtime_error(makeError("Expect value ", valuetoken.line, valuetoken.col));
		expect(TK_SEMICOLON, "Expected ';'");
		return valuetoken.value;
	}

	ServerNode Parser::parseServerBlock()
	{
		Token token = get();
		//std::cout<< token.value << std::endl;
		if (token.value != "server")
			throw std::runtime_error(makeError("Expect server ", token.line, token.col));
		expect(TK_LBRACE, "Expected '{' after location path");

		ServerNode server;
		while(true)
		{
			Token token = peek();
			if (token.type == TK_RBRACE){
				get();
				break;
			}
			if (token.type == TK_IDENTIFIER && token.value == "listen")
			{
				get();
				Token listenToken = get();
				if(listenToken.type != TK_IDENTIFIER && listenToken.type != TK_NUMBER)
					throw std::runtime_error(makeError("Expect port ", listenToken.line, listenToken.col));
				std::string host;
				int port;
				std::string listenValue = listenToken.value;
				size_t colon = listenValue.find(':');
				if(colon == std::string::npos){
					host = "0.0.0.0";
					port = std::stoi(listenValue);
				}
				else{
					host = listenValue.substr(0, colon);
					port = std::stoi(listenValue.substr(colon+1));
				}
				server.listen = std::make_pair(host, port);
				expect(TK_SEMICOLON, "Expected ';' after port ");
			}
			else if(token.type == TK_IDENTIFIER && token.value == "server_name")
			{
				get();//eat server_name
				while(true){
					if (eof()) {
						throw std::runtime_error("Unexpected EOF while parsing server_name");
					}
					Token sernameToken = get();
					if(sernameToken.type != TK_IDENTIFIER)
						throw std::runtime_error(makeError("Expect server_name ", sernameToken.line, sernameToken.col));
					server.server_names.push_back(sernameToken.value);
					if(peek().type == TK_SEMICOLON)
					{
						get(); //eat ;
						break;
					}
				}
			}
			else if(token.type == TK_IDENTIFIER && token.value == "error_page")
			{
				get();//eat error_page
				Token codeTok = get();
				if (codeTok.type != TK_NUMBER)
					throw std::runtime_error(makeError("Expect error code ", codeTok.line, codeTok.col));
				Token pathTok = get();
				if (pathTok.type != TK_IDENTIFIER)
					throw std::runtime_error(makeError("Expect path ", codeTok.line, codeTok.col));
				server.error_pages[std::stoi(codeTok.value)] = pathTok.value;
				expect(TK_SEMICOLON, "Expected ';' after error path");
			}
			else if(token.type == TK_IDENTIFIER && token.value == "client_max_body_size")
				server.client_max_body_size = parseSimpleDirective();
			// else if(token.type == TK_IDENTIFIER && token.value == "client_max_body_size")
			// {
			// 	get(); // eat client max body size
			// 	Token sizeToken = get();
			// 	if(sizeToken.type != TK_NUMBER)
			// 		throw std::runtime_error(makeError("Expect size ", sizeToken.line, sizeToken.col));
			// 	server.client_max_body_size = std::stoi(sizeToken.value);
			// 	expect(TK_SEMICOLON, "Expected ';' after error path");
			// }
			else if (token.type == TK_IDENTIFIER && token.value == "root")
				server.root = parseSimpleDirective();
			else if(token.type==TK_IDENTIFIER && token.value == "index")
			{
				get(); //eat index
				while(true)
				{
					if (eof()) {
						throw std::runtime_error("Unexpected EOF while parsing index value");
					}
					Token indexValue = get();
					if(indexValue.type != TK_IDENTIFIER)
						throw std::runtime_error(makeError("Expect index URI ", indexValue.line, indexValue.col));
					server.index.push_back(indexValue.value);
					if(peek().type == TK_SEMICOLON){
						get(); //eat ;
						break;
					}
				}
			}
			else if (token.type == TK_IDENTIFIER && token.value == "location")
				server.locations.push_back(parseLocationBlock());
			else
				throw std::runtime_error(makeError("Unknown keyword in server block ", token.line, token.col));
		}
		return server;
	}

	LocationNode Parser::parseLocationBlock()
	{

		Token token = get();
		if (token.value != "location")
			throw std::runtime_error(makeError("Expect location ", token.line, token.col));
		Token path = get();
		if (path.type != TK_IDENTIFIER)
			throw std::runtime_error(makeError("Expected path after 'location '", path.line, path.col));
		expect(TK_LBRACE, "Expected '{' after location path");

		LocationNode location;
		location.path = path.value;
		location.autoindex = false;
		while(true)
		{
			Token token = peek();
			if (token.type == TK_RBRACE){
				get();
				break;
			}
			if (token.type == TK_IDENTIFIER && token.value == "root")
				location.root = parseSimpleDirective();
			else if(token.type == TK_IDENTIFIER && token.value == "redirect")
				location.redirect = parseSimpleDirective();
			else if(token.type==TK_IDENTIFIER && token.value == "cgi_pass")
				location.cgi_pass = parseSimpleDirective();
			else if(token.type==TK_IDENTIFIER && token.value == "cgi_ext")
				location.cgi_ext = parseSimpleDirective();
			else if(token.type==TK_IDENTIFIER && token.value == "upload_dir")
				location.upload_dir = parseSimpleDirective();
			else if(token.type == TK_IDENTIFIER && token.value == "client_max_body_size")
				location.client_max_body_size = parseSimpleDirective();
			else if(token.type==TK_IDENTIFIER && token.value == "index")
			{
				get(); //eat index
				while(true)
				{
					if (eof()) {
						throw std::runtime_error("Unexpected EOF while parsing index value");
					}
					Token indexValue = get();
					if(indexValue.type != TK_IDENTIFIER)
						throw std::runtime_error(makeError("Expect index URI ", indexValue.line, indexValue.col));
					location.index.push_back(indexValue.value);
					if(peek().type == TK_SEMICOLON){
						get(); //eat ;
						break;
					}
				}
			}
			else if(token.type==TK_IDENTIFIER && token.value == "autoindex")
			{
				get();
				Token autoindex = get();
				if (autoindex.value == "on")
					location.autoindex = true;
				else if(autoindex.value == "off")
					location.autoindex = false;
				else
					throw std::runtime_error(makeError("Expect on/off after autoindex ", autoindex.line, autoindex.col));
				expect(TK_SEMICOLON, "Expected ';' after location path");
			}
			else if(token.type==TK_IDENTIFIER && (token.value == "methods" || token.value == "allowed_methods"))
			{
				get();
				while(true){
					if (eof()) {
						throw std::runtime_error("Unexpected EOF while parsing server_name");
					}
					Token methodValue = get();
					if(methodValue.value == "GET" || methodValue.value == "POST" || methodValue.value == "DELETE")
						location.methods.push_back(methodValue.value);
					else
						throw std::runtime_error(makeError("Expect GET/POST/DELETE after methods ", methodValue.line, methodValue.col));
					if(peek().type == TK_SEMICOLON){
						get();
						break;
					}
				}
			}
			else
				throw std::runtime_error(makeError("Unknown keyword in location block ", token.line, token.col));
		}
		return location;
	}

	std::vector<ServerNode> Parser::parse()
	{
		std::vector<ServerNode> servers;
		while(peek().type != TK_EOF)
		{
			Token t = peek();
			if (t.type == TK_IDENTIFIER && t.value == "server")
				servers.push_back(parseServerBlock());
			else
				throw std::runtime_error(makeError("Expected 'server' block ", t.line, t.col));
		}
		return servers;
	}
}
