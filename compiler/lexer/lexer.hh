// Header file for lexer.cc

#ifndef __LEXER_HH__
#define __LEXER_HH__

#include <fstream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>

namespace Frontend {

// Token struct definition
struct Token {
     
     // Define token types
    enum class TokenType : int {
        // illegal - indicates any unsupported token types
        TOKEN_ILLEGAL,
        // EOF - indicates the end of all the tokens
        TOKEN_EOF,
        // identifier - indicates the token is a variable/function name
        TOKEN_IDENTIFIER,
        // int - indicates the token is an integer number
        TOKEN_INT,
        // float - indicates the token is a float number
        TOKEN_FLOAT,
        // assign - indicates the token is "="
        TOKEN_ASSIGN,
        // plus - indicates the token is "+"
        TOKEN_PLUS,
        // minus - indicates the token is "-"
        TOKEN_MINUS,
        // bang - indicates the token is "!"
        TOKEN_BANG,
        // asterisk - indicates the token is "*"
        TOKEN_ASTERISK,
        // slash - indicates the token is "/"
        TOKEN_SLASH,
        // LT - indicates the token is "<"
        TOKEN_LT,
        // GT - indicates the token is ">"
        TOKEN_GT,
        // comma - indicates the token is ","
        TOKEN_COMMA,
        // semicolon - indicates the token is ";"
        TOKEN_SEMICOLON,
        // lparen - indicates the token is "("
        TOKEN_LPAREN,
        // rparen - indicates the token is ")"
        TOKEN_RPAREN,
        // lbrace - indicates the token is "{"
        TOKEN_LBRACE,
        // rbrace - indicates the token is "}"
        TOKEN_RBRACE,
        // lbracket - indicates the token is "["
        TOKEN_LBRACKET,
        // rbracket - indicates the token is "]"
        TOKEN_RBRACKET,
        // return - indicates the token is "return"
        TOKEN_RETURN,
        // DES - description
        // des_void - indicates the token is "void"
        TOKEN_DES_VOID,
        // des_int - indicates the token is "int"
        TOKEN_DES_INT,
        // des_float - indicates the token is "float"
        TOKEN_DES_FLOAT,
        // if - indicates the token is "if"
        TOKEN_IF,
        // else - indicates the token "else"
        TOKEN_ELSE,
        // for - indicates the token is "for"
        TOKEN_FOR
    } type = TokenType::TOKEN_ILLEGAL;

    // Literal - container of the token value
    std::string literal = "";

    // Default constructor
    Token() {}

    // Alternative constructor
    Token(TokenType _type)
        : type(_type)
    {
    
    }

    // Alternative constructor
    Token(TokenType _type, std::string &_val)
        : type(_type)
        , literal(_val)
    {
    
    }

    // Alternative constructor
    Token(TokenType _type, 
          std::string &_val, 
          std::shared_ptr<std::string> &_line)
        : type(_type)
        , literal(_val)
        , line(_line)
    {
    
    }

    // Copy constructor
    Token(const Token &_tok)
        : type(_tok.type)
        , literal(_tok.literal)
        , line(_tok.line)
    {
    
    }

    // Return token type string (implemented in lexer.cc)
    std::string prinTokenType();

    auto &getLiteral() { return literal; }
    auto &getTokenType() { return type; }

    bool isTokenIden() { return type == TokenType::TOKEN_IDENTIFIER; }

    bool isTokenEOF() { return type == TokenType::TOKEN_EOF; }
    bool isTokenReturn() { return type == TokenType::TOKEN_RETURN; }
    bool isTokenDesVoid() { return type == TokenType::TOKEN_DES_VOID; }
    bool isTokenDesInt() { return type == TokenType::TOKEN_DES_INT; }
    bool isTokenDesFloat() { return type == TokenType::TOKEN_DES_FLOAT; }

    bool isTokenInt() { return type == TokenType::TOKEN_INT; }
    bool isTokenFloat() { return type == TokenType::TOKEN_FLOAT; }
    bool isTokenPlus() { return type == TokenType::TOKEN_PLUS; }
    bool isTokenMinus() { return type == TokenType::TOKEN_MINUS; }
    bool isTokenAsterisk() { return type == TokenType::TOKEN_ASTERISK; }
    bool isTokenSlash() { return type == TokenType::TOKEN_SLASH; }
    bool isTokenArithOpr() 
    {
        return (isTokenPlus() || isTokenMinus() || 
                isTokenAsterisk() || isTokenSlash());
    }
    bool isTokenEqual() { return type == TokenType::TOKEN_ASSIGN; }

    bool isTokenComma() { return type == TokenType::TOKEN_COMMA; }
    bool isTokenSemicolon() { return type == TokenType::TOKEN_SEMICOLON; }
    bool isTokenLP() { return type == TokenType::TOKEN_LPAREN; }
    bool isTokenRP() { return type == TokenType::TOKEN_RPAREN; }
    bool isTokenLBrace() { return type == TokenType::TOKEN_LBRACE; }
    bool isTokenRBrace() { return type == TokenType::TOKEN_RBRACE; }
    bool isTokenLBracket() { return type == TokenType::TOKEN_LBRACKET; }
    bool isTokenRBracket() { return type == TokenType::TOKEN_RBRACKET; }

    bool isTokenLT() { return type == TokenType::TOKEN_LT; }
    bool isTokenGT() { return type == TokenType::TOKEN_GT; }

    bool isTokenIf() { return type == TokenType::TOKEN_IF; }
    bool isTokenElse() { return type == TokenType::TOKEN_ELSE; }
    bool isTokenFor() { return type == TokenType::TOKEN_FOR; }

    std::shared_ptr<std::string> line;
    std::string& getLine() { return *line; }
};

// Lexer class
class Lexer
{
  protected:
    // Define separators
    std::unordered_map<char, Token::TokenType> seps;
    // Define keywords
    std::unordered_map<std::string, Token::TokenType> keywords;

  protected:
    std::ifstream code;

    std::queue<Token> toks_per_line;

  public:
    Lexer(const char *);
    ~Lexer() { code.close(); };

    bool getToken(Token&);
    
  protected:
    void parseLine(std::string &line);

    // Helper function to check for data type
    template<typename T>
    bool isType(std::string &cur_token_str)
    {
        std::istringstream iss(cur_token_str);
        T float_check;
        iss >> std::noskipws >> float_check;
        if (iss.eof() && !iss.fail()) return true;
        else return false;
    }
};

}

#endif
