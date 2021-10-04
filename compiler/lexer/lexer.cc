/* Lexer implementation.

 * Author: Shihao Song
 * Modified by: Naga Kandasamy
 * Date: September 8, 2021 

 * Student name(s): Darius Olega
 * Date modified: Ovtober 3, 2021

*/

#include "lexer/lexer.hh"

#include <cassert>
#include <iostream>

namespace Frontend
{

// Return token type as string
std::string Token::prinTokenType()
{
    switch (type) {
        case TokenType::TOKEN_ILLEGAL:
            return std::string("ILLEGAL");
        case TokenType::TOKEN_EOF:
            return std::string("EOF");
        case TokenType::TOKEN_IDENTIFIER:
            return std::string("IDENTIFIER");
        case TokenType::TOKEN_INT:
            return std::string("INT");
        case TokenType::TOKEN_FLOAT:
            return std::string("FLOAT");
        case TokenType::TOKEN_ASSIGN:
            return std::string("ASSIGN");
        case TokenType::TOKEN_PLUS:
            return std::string("PLUS");
        case TokenType::TOKEN_MINUS:
            return std::string("MINUS");
        case TokenType::TOKEN_BANG:
            return std::string("BANG");
        case TokenType::TOKEN_ASTERISK:
            return std::string("ASTERISK");
        case TokenType::TOKEN_SLASH:
            return std::string("SLASH");
        case TokenType::TOKEN_LT:
            return std::string("LT");
        case TokenType::TOKEN_GT:
            return std::string("GT");
        case TokenType::TOKEN_COMMA:
            return std::string("COMMA");
        case TokenType::TOKEN_SEMICOLON:
            return std::string("SEMICOLON");
        case TokenType::TOKEN_LPAREN:
            return std::string("LPAREN");
        case TokenType::TOKEN_RPAREN:
            return std::string("RPAREN");
        case TokenType::TOKEN_LBRACE:
            return std::string("LBRACE");
        case TokenType::TOKEN_RBRACE:
            return std::string("RBRACE");
        case TokenType::TOKEN_LBRACKET:
            return std::string("LBRACKET");
        case TokenType::TOKEN_RBRACKET:
            return std::string("RBRACKET");
        case TokenType::TOKEN_RETURN:
            return std::string("RETURN");
        case TokenType::TOKEN_DES_VOID:
            return std::string("DES-VOID");
        case TokenType::TOKEN_DES_INT:
            return std::string("DES-INT");
        case TokenType::TOKEN_DES_FLOAT:
            return std::string("DES-FLOAT");
        case TokenType::TOKEN_IF:
            return std::string("IF");
        case TokenType::TOKEN_ELSE:
            return std::string("ELSE");
        case TokenType::TOKEN_FOR:
            return std::string("FOR");
        default:
            std::cerr << "[Error] prinTokenType: "
                      << "unsupported token type. \n";
            exit(0);
    }
}

// Constructor 
Lexer::Lexer(const char* fn)
{
    code.open(fn);
    assert(code.good());

    // Fill map with pre-defined separators
    seps.insert({'=', Token::TokenType::TOKEN_ASSIGN});
    seps.insert({'+', Token::TokenType::TOKEN_PLUS});
    seps.insert({'-', Token::TokenType::TOKEN_MINUS});
    seps.insert({'!', Token::TokenType::TOKEN_BANG});
    seps.insert({'*', Token::TokenType::TOKEN_ASTERISK});
    seps.insert({'/', Token::TokenType::TOKEN_SLASH});
    seps.insert({'<', Token::TokenType::TOKEN_LT});
    seps.insert({'>', Token::TokenType::TOKEN_GT});
    seps.insert({',', Token::TokenType::TOKEN_COMMA});
    seps.insert({';', Token::TokenType::TOKEN_SEMICOLON});
    seps.insert({'(', Token::TokenType::TOKEN_LPAREN});
    seps.insert({')', Token::TokenType::TOKEN_RPAREN});
    seps.insert({'{', Token::TokenType::TOKEN_LBRACE});
    seps.insert({'}', Token::TokenType::TOKEN_RBRACE});
    seps.insert({'[', Token::TokenType::TOKEN_LBRACKET});
    seps.insert({']', Token::TokenType::TOKEN_RBRACKET});

    // fill pre-defined keywords
    keywords.insert({"return", Token::TokenType::TOKEN_RETURN});

    keywords.insert({"void", Token::TokenType::TOKEN_DES_VOID});
    keywords.insert({"int", Token::TokenType::TOKEN_DES_INT});
    keywords.insert({"float", Token::TokenType::TOKEN_DES_FLOAT});

    // FIXME: insert the "if", "else", "for", and "while" keywords into the map
    // String "if" must be assigned a token type of TOKEN_IF
    // String "else" must be assigned a token type of TOKEN_ELSE
    // String "for" must be assigned a token type of TOKEN_FOR 
    // String "while" must be assigned a token type of TOKEN_WHILE
}

// Return a token 
bool Lexer::getToken(Token &tok)
{
    // Check if toks_per_line has elements within it
    if (toks_per_line.size()) {
        tok = toks_per_line.front(); // Access the first element in queue
        toks_per_line.pop();         // Remove the first element from queue
        return true;
    }

    // Read a line
    std::string line;
    getline(code, line);

    // Return if EOF
    if (code.eof()) {
        tok = Token(Token::TokenType::TOKEN_EOF);
        return false;
    }

    // Parse line
    parseLine(line);

    // Skip empty lines
    while (toks_per_line.size() == 0) {
        getline(code, line);
        if (code.eof()) {
            tok = Token(Token::TokenType::TOKEN_EOF);
            return false;
        }

        parseLine(line);
    }

    assert(toks_per_line.size() != 0);
    tok = toks_per_line.front();
    toks_per_line.pop();
    return true;
}

// Tokenize line 
void Lexer::parseLine(std::string &line)
{
    std::shared_ptr<std::string> cur_line = 
        std::make_shared<std::string>(line);

    // Tokenize current line
    for (auto iter = line.begin(); iter != line.end(); iter++) {
        // Skip space, tab
        if (*iter == ' ' || *iter == '\t') continue;
        // FIXME: insert code to skip single-line comments
        // Single-line comment starts with the two characters: //
        // If the line is a single-line comment, we need not tokenize it; so break out of the for loop
        // FIXED: Inserted line below for skipping single line comments
        if (*iter == '/' && *iter++ == '/') break;

        // Start forming token
        std::string cur_token_str(1, *iter);

        // Is the current character separator?
        if (auto sep_iter = seps.find(*iter); 
            sep_iter != seps.end()) {
            std::string literal = cur_token_str;
            Token::TokenType type = sep_iter->second;
            Token _tok(type, literal, cur_line);

            toks_per_line.push(_tok);
                
            continue;
        }

        // Form the token
        auto next = iter + 1;
        while (next != line.end()) {
            auto next_sep_check = seps.find(*next);

            if ((*next == ' ') || (next_sep_check != seps.end()))
                break;
            cur_token_str.push_back(*next);
            next++;
            iter++;
        }

        // Create integer or floating-point token if integer or floating-point data type
        if (isType<int>(cur_token_str)) {
            Token::TokenType type = Token::TokenType::TOKEN_INT;
            Token _tok(type, cur_token_str, cur_line);
            toks_per_line.push(_tok);
            continue;
        } 
        else if (isType<float>(cur_token_str)) {
            Token::TokenType type = Token::TokenType::TOKEN_FLOAT;
            Token _tok(type, cur_token_str, cur_line);
            toks_per_line.push(_tok);
            continue;
        }

        // Check if token is a keyword of the language, else it is an identifier
        if (auto k_iter = keywords.find(cur_token_str);
            k_iter != keywords.end()) {
            std::string literal = cur_token_str;
            Token::TokenType type = k_iter->second;
            Token _tok(type, literal, cur_line);

            toks_per_line.push(_tok);
        }
        else {
            // FIXME: create token object 
            // Token type is TOKEN_IDENTIFIER
            // Push token object into queue 
        }
    }
}
}
