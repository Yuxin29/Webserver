#include "ConfigTokenizer.hpp"
#include "utils.hpp"

/**
 * @file ConfigTokenizer.hpp
 * @brief Lexical analyzer for webserver configuration files.
 *
 * This module converts a raw configuration file into 
 * a sequence of typed tokens that can be consumed
 * by the configuration parser.
 */
namespace config{
    // Construct a new Tokenizer object.
	Tokenizer::Tokenizer(std::string& source) :_source(source),_pos(0),_line(1),_col(1){
	}

	//Peek at the current character without advancing the position.
	char Tokenizer::peek() const{
		if(eof())
			return '\0';
		return _source[_pos];
	}

	/**
     * @brief Get the current character and advance the position.
     * @return The current character, or '\0' if at EOF.
     */
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

	/**
     * @brief Check if the end of the source has been reached.
     * @return True if at EOF, false otherwise.
     */
	bool Tokenizer::eof() const {
		return _pos >= _source.size();
	}

	/**
     * @brief Skip whitespace and comments.
     *
     * Comments start with '#' and continue to the end of the line.
     */
	void Tokenizer::skipWhitespaceAndComments() {
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

	/**
     * @brief Get the next token from the source.
     * @return The next Token object.
     * @throws std::runtime_error if an unexpected character is found.
     */
	Token Tokenizer::nextToken(){
		int start_line = _line;
		int start_col = _col;
		if (eof())
			return Token{TK_EOF, "", start_line, start_col};
		char c = peek();
		if (isalnum(c) || c == '/'|| c == '.' || c == '_'|| c == '-' || c == ':' || c == '*')
			return tokenizeIdentifier();
		if (c == '{' || c == '}' || c == ';')
			return tokenizeSymbol();
		if (c == '"')
			return tokenizeString();
		throw std::runtime_error(makeError("Unexpected character ", start_line, start_col));
	}

	/**
     * @brief Tokenize an identifier or number.
     * @return A Token of type TK_IDENTIFIER or TK_NUMBER.
     */
	Token Tokenizer::tokenizeIdentifier(){
		int start_pos = _pos;
		int start_line = _line;
		int start_col = _col;

		while (!eof())
		{
			char c = peek();
			if (isalnum(c) || c == '/'|| c == '.' || c == '_'|| c == '-'|| c == ':' || c == '*')
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

	/**
     * @brief Tokenize a quoted string.
     * @return A Token of type TK_STRING.
     * @throws std::runtime_error if the string is unterminated.
     */
	Token Tokenizer::tokenizeString()
	{
		int start_line = _line;
		int start_col = _col;
		std::string value;
		get();
		while(!eof())
		{
			char c = get();
			if (c == '"')
				break;
			if (c == '\n')
				throw std::runtime_error(makeError("Unterminated string starting at ", start_line, start_col));
			value += c;
		}
		return Token{TK_STRING, value, _line, _col};
	}

	/**
     * @brief Tokenize a symbol: '{', '}', or ';'.
     * @return A Token of type TK_LBRACE, TK_RBRACE, or TK_SEMICOLON.
     * @throws std::runtime_error if an invalid symbol is found.
     */
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

	/**
     * @brief Tokenize the entire source into a vector of tokens.
     * @return A std::vector<Token> containing all tokens.
     */
	std::vector<Token> Tokenizer::tokenize(){
		std::vector<Token> tokens;
		while(!eof()){
			skipWhitespaceAndComments();
			Token token = nextToken();
			tokens.push_back(token);
			if (token.type == TK_EOF)
				break;
		}
		return tokens;
	}
}
