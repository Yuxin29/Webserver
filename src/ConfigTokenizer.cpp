#include "ConfigTokenizer.hpp"
#include "utils.hpp"
#include <cctype> //std::isspace, std::isdigit
#include <stdexcept>
#include <iostream> //debug

namespace config{
	Tokenizer::Tokenizer(std::string& source)
	:_source(source),_pos(0),_line(1),_col(1){}

	char Tokenizer::peek() const
	{
		if(eof())
			return '\0';
		return _source[_pos];
	}

	char Tokenizer::get()
	{
		if(eof())
			return '\0';
		char c = _source[_pos++];
		if (c =='\n'){
			_line++;
			_col = 1;
		}
		else
			_col++;
		return c;
	}

	bool Tokenizer::eof() const
	{
		return _pos >= _source.size();
	}

	void Tokenizer::skipWhitespaceAndComments()
	{
		while(!eof())
		{
			char c = peek();
			if (std::isspace(c))
				get();
			else if (c == '#'){
				while (peek() != '\n' && !eof())
					get();
			}
			else
				break;
		}
	}

	Token Tokenizer::nextToken()
	{
		int start_line = _line;
		int start_col = _col;
		if (eof())
			return Token{TK_EOF, "", start_line, start_col};
		char c = peek(); //should put it after peek aviding go over the boundary
		if (isalnum(c) || c == '/'|| c == '.' || c == '_'|| c == '-') //identifier or path
			return tokenizeIdentifier();
		if (c == '{' || c == '}' || c == ';')
			return tokenizeSymbol();
		throw std::runtime_error(makeError("Unexpected character ", start_line, start_col));
	}

	Token Tokenizer::tokenizeIdentifier()
	{
		int start_pos = _pos;
		int start_line = _line;
		int start_col = _col;

		while (!eof())
		{
			char c = peek();
			if (isalnum(c) || c == '/'|| c == '.' || c == '_'|| c == '-')
				get();
			else
				break;
		}
		std::string keyword = _source.substr(start_pos, _pos-start_pos);
		bool is_number = true;
		for(std::size_t i = 0; i < keyword.length(); i++)
		{
			if(!isdigit(keyword[i])){
				is_number = false;
				break;
			}
		}
		TokenType type = is_number ? TK_NUMBER: TK_IDENTIFIER;
		return Token{type, keyword, start_line, start_col};
	}

	Token Tokenizer::tokenizeSymbol()
	{
		TokenType type;
		char c = get();
		if (c == '{')
			type = TK_LBRACE;
		else if(c == '}')
			type = TK_RBRACE;
		else if (c == ';')
			type = TK_SEMICOLON;
		else
			throw std::runtime_error("Invalid symbol");
		return Token{type, std::string(1,c), _line, _col};
	}

	std::vector<Token> Tokenizer::tokenize()
	{
		std::vector<Token> tokens;

		while(!eof())
		{
			skipWhitespaceAndComments();
			Token token = nextToken();
			tokens.push_back(token);
			if (token.type == TK_EOF)
				break;
		}
		return tokens;
	}
}
// Token Tokenizer::tokenizeNumber();
// Token Tokenizer::tokenizerString();
