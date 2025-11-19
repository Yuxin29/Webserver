#include "ConfigParser.hpp"
#include <fstream>
#include <sstream> //std::stringstream
#include <format>


Parser::Parser(std::string& filename)
:_pos(0)
{
	std::ifstream infile(filename);
	if (!infile)
		throw std::runtime_error("Failed to open config file" + filename);
	std::stringstream buffer;
	buffer << infile.rdbuf();//.rdbuf() gives access to its underlying stream buffer.
	std::string content = buffer.str();
	Tokenizer tokenizer(content);
	_tokens = tokenizer.tokenize();
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

bool Parser::match(TokenType type)
{
	if (peek().type == type)
	{
		get();
		return true;
	}
	return false;
}

void Parser::expect(TokenType type, const std::string& msg) //msg?
{
	Token cur_token = get();
	if (cur_token.type != type)
		throw std::runtime_error(std::format("Error at line {}, col {}", cur_token.line, cur_token.col));
}

// ServerNode Parser::parseServerBlock()
// {


// }

LocationNode Parser::parseLocationBlock()
{
	expect(TK_IDENTIFIER, "Expect 'Location");
	Token path = get();
	if (path.type != TK_IDENTIFIER)
		throw std::runtime_error(std::format("Expected path after 'location' at line {}, col {}", path.line, path.col));
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
		{
			get();
			Token rootValue = get();
			if (rootValue.type != TK_IDENTIFIER)
				throw std::runtime_error(std::format("Expcept path after root at line {}, col {}", rootValue.line, rootValue.col));
			location.root = rootValue.value;
			expect(TK_SEMICOLON, "Expected ';' after location path");
		}
		else if(token.type == TK_IDENTIFIER && token.value == "redirect")
		{
			get();
			Token redirValue = get();
			if (redirValue.type != TK_IDENTIFIER)
				throw std::runtime_error(std::format("Expcept URL after redirect at line {}, col {}", redirValue.line, redirValue.col));
			location.redirect = redirValue.value;
			expect(TK_SEMICOLON, "Expected ';' after location path");
		}
		else if(token.type==TK_IDENTIFIER && token.value == "index")
		{
			get();
			Token indexValue = get();
			if (indexValue.type != TK_IDENTIFIER)
				throw std::runtime_error(std::format("Expcept filename after index at line {}, col {}", indexValue.line, indexValue.col));
			location.index = indexValue.value;
			expect(TK_SEMICOLON, "Expected ';' after location path");
		}
		else if(token.type==TK_IDENTIFIER && token.value == "cgi_path")
		{
			get();
			Token cgiValue = get();
			if (cgiValue.type != TK_IDENTIFIER)
				throw std::runtime_error(std::format("Expcept path after cgi_path at line {}, col {}", cgiValue.line, cgiValue.col));
			location.cgi_path = cgiValue.value;
			expect(TK_SEMICOLON, "Expected ';' after location path");
		}
		else if(token.type==TK_IDENTIFIER && token.value == "upload_dir")
		{
			get();
			Token uploadValue = get();
			if (uploadValue.type != TK_IDENTIFIER)
				throw std::runtime_error(std::format("Expcept path after cgi_path at line {}, col {}", uploadValue.line, uploadValue.col));
			location.cgi_path = uploadValue.value;
			expect(TK_SEMICOLON, "Expected ';' after location path");
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
				throw std::runtime_error(std::format("Expcept on/off after autoindex at line {}, col {}", autoindex.line, autoindex.col));
			expect(TK_SEMICOLON, "Expected ';' after location path");
		}
		else if(token.type==TK_IDENTIFIER && token.value == "method")
		{
			get();
			while(true)
			{
				Token methodValue = get();
				if(methodValue.value == "GET" || methodValue.value == "POST" || methodValue.value == "DELETE")
					location.methods.push_back(methodValue.value);
				else
					throw std::runtime_error(std::format("Expcept GET/POST/DELETE after methods at line {}, col {}", methodValue.line, methodValue.col));
				if(peek().type == TK_SEMICOLON){
					get();
					break;
				}
			}
		}
	}

}

// std::vector<ServerNode> Parser::parse()
// {

// }
