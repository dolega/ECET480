/* Implementation of recursive-descent parser.
 * Author: Shihao Song
 * Modified by: Naga Kandasamy
 * Date modified: September 15, 2021
 *
 * Student name(s): Darius Olega
 * Date: October 21, 2021
 */

#include "parser/parser.hh"

namespace Frontend
{
Parser::Parser(const char* fn) : lexer(new Lexer(fn))
{
    // Load tokens from Lexer
    lexer->getToken(cur_token);
    lexer->getToken(next_token);

    // Fill in the pre-built functions: printVarInt and printVarFloat 
    std::vector<ValueType::Type> arg_types;
    ValueType::Type ret_type = ValueType::Type::VOID;
    FuncRecord record;

    // printVarInt prints an integer value to stdout
    arg_types.push_back(ValueType::Type::INT);
    record.ret_type = ret_type;
    record.arg_types = arg_types;
    record.is_built_in = true;
    func_def_tracker.insert({"printVarInt", record});

    // printVarFloat prints a floating-point value to stdout
    arg_types.clear();
    arg_types.push_back(ValueType::Type::FLOAT);
    record.ret_type = ret_type;
    record.arg_types = arg_types;
    record.is_built_in = true;
    func_def_tracker.insert({"printVarFloat", record});

    parseProgram();
}

// Advance current and lookahead tokens
void Parser::advanceTokens()
{
    cur_token = next_token;
    lexer->getToken(next_token);
}

// Parse program
void Parser::parseProgram()
{
    // We assume the program starts with a function 
    // Current implementation does not support global variables or structures
    while (!cur_token.isTokenEOF()) {
        ValueType::Type ret_type;
        std::unique_ptr<Identifier> iden;
        std::vector<FuncStatement::Argument> args;
        std::vector<std::shared_ptr<Statement>> codes;

        // Parse return type from function
        ret_type = ValueType::typeTokenToValueType(cur_token);
        if (ret_type == ValueType::Type::MAX) {
            std::cerr << "[Error] parseProgram: unsupported return type\n"
                      << "[Line] " << cur_token.getLine() << "\n";
            exit(0);
        }
                
        // Parse function name
        advanceTokens();
        iden = std::make_unique<Identifier>(cur_token);
        if (!next_token.isTokenLP()) {
            std::cerr << "[Error] Incorrect function defition.\n "
                      << "[Line] " << cur_token.getLine() << "\n";
            exit(0);
        }

        advanceTokens();
        assert(cur_token.isTokenLP());

        // Track local variables
        std::unordered_map<std::string,ValueType::Type> local_vars;
        local_vars_tracker.push_back(&local_vars);

        // Extract arguments
        while (!cur_token.isTokenRP()) {
            advanceTokens();
            if (cur_token.isTokenRP()) break; // Function accepts no arguments

            std::string arg_type = cur_token.getLiteral();

            advanceTokens();
            std::unique_ptr<Identifier> iden(new Identifier(cur_token));
            FuncStatement::Argument arg(arg_type, iden);
            args.push_back(arg);

            recordLocalVars(arg);

            advanceTokens();
        }
        assert(cur_token.isTokenRP()); // Check if right parenthesis terminates function definition

        // Record the function definition
        recordDefs(iden->getLiteral(), ret_type, args);

        // Parse function body
        advanceTokens();
        assert(cur_token.isTokenLBrace());

        while (true) {
            advanceTokens();
            if (cur_token.isTokenRBrace())
                    break; // End of function body

            parseStatement(iden->getLiteral(), codes);
        }
        
        std::unique_ptr<Statement> func_proto
            (new FuncStatement(ret_type, 
                               iden, 
                               args, 
                               codes,
                               local_vars));
        
        local_vars_tracker.pop_back();

        program.addStatement(func_proto);
        
        advanceTokens();
    }
}

// Parse statement as per the grammar 
void Parser::parseStatement(std::string &cur_func_name, 
                            std::vector<std::shared_ptr<Statement>> &codes)
{
    // Parse function-call statement
    if (auto [is_def, is_built_in] = 
            isFuncDef(cur_token.getLiteral());
        is_def) {
        Statement::StatementType call_type = is_built_in ?
            Statement::StatementType::BUILT_IN_CALL_STATEMENT :
            Statement::StatementType::NORMAL_CALL_STATEMENT;

        auto code = parseCall();
        std::unique_ptr<CallStatement> call = 
            std::make_unique<CallStatement>(code, call_type); 

        codes.push_back(std::move(call));

        return;
    }

    // Parse return statement
    if (cur_token.isTokenReturn()) {
        advanceTokens();
        cur_expr_type = getFuncRetType(cur_func_name);

        auto ret = parseExpression();
        std::unique_ptr<RetStatement> ret_statement = 
            std::make_unique<RetStatement>(ret);

        codes.push_back(std::move(ret_statement));

        return;
    }

    // Parse variable-assignment statement
    if (isTokenTypeKeyword(cur_token) || cur_token.isTokenIden()) {
        auto code = parseAssnStatement();

        codes.push_back(std::move(code));

        return;
    }
}

// Parse assignment statement 
std::unique_ptr<Statement> Parser::parseAssnStatement()
{

    // FIXME: Add code to accommodate options: 
    // int variableName;
    // float variableName;

    // Allocating new variables
    // Example: int variableName = value;
    // Example: float variableName = value;
    if (isTokenTypeKeyword(cur_token)) {
        Token type_token = cur_token;

        advanceTokens();
        // Check if variable has been previously defined
        if (auto [already_defined, type] = isVarAlreadyDefined(cur_token);
            already_defined) {
            std::cerr << "[Error] Re-definition of "
                      << cur_token.getLiteral() << "\n";
            std::cerr << "[Line] " << cur_token.getLine() << "\n";
            exit(0);
        }

        // Check if variable is of type array
        // NOTE: we do not support array data type yet
        bool is_array = (next_token.isTokenLBracket()) ? true : false;

        recordLocalVars(cur_token, type_token, is_array);

        // Build node for identifier on left-hand side of assignment statement
        std::unique_ptr<Expression> iden =
            std::make_unique<LiteralExpression>(cur_token);

        // Build node for expression on right-hand side of assignment statement
        std::unique_ptr<Expression> expr;
        
        if (!is_array) {
            advanceTokens();
            assert(cur_token.isTokenEqual());

            advanceTokens();
            expr = parseExpression();
        }
        else {
            // parseArrayExpr() method will be added later
        }

        // Build node for assignment statement
        std::unique_ptr<Statement> statement = 
            std::make_unique<AssnStatement>(iden, expr);

        return statement;
    }
    else {  // Variable must have been previously defined 
        auto [already_defined, type] = isVarAlreadyDefined(cur_token);
        if (!already_defined) {
            std::cerr << "[Error] Undefined variable of "
                      << cur_token.getLiteral() << "\n";
            std::cerr << "[Line] " << cur_token.getLine() << "\n";
            exit(0);
        }

        cur_expr_type = ValueType::Type::MAX;

        // Build node on left-hand side of assignment statement 
        auto iden = parseExpression();

        assert(cur_token.isTokenEqual());
        advanceTokens();

        // Build node on right-hand side of assignment statement
        std::unique_ptr<Expression> expr;
        if (type == ValueType::Type::INT_ARRAY || type == ValueType::Type::FLOAT_ARRAY) {
            cur_expr_type = (type == ValueType::Type::INT_ARRAY)
                            ? type = ValueType::Type::INT
                            : type = ValueType::Type::FLOAT;
        }
        else {
            cur_expr_type = type;
        }

        expr = parseExpression();
        
        // Build node for assignment statment
        std::unique_ptr<Statement> statement = 
            std::make_unique<AssnStatement>(iden, expr);

        return statement;
    }
}

// Parse function-call statement
std::unique_ptr<Expression> Parser::parseCall()
{
    // Parse function name 
    std::unique_ptr<Identifier> def(new Identifier(cur_token));

    advanceTokens();
    assert(cur_token.isTokenLP());

    advanceTokens();

    // Parse arguments passed to function 
    std::vector<std::shared_ptr<Expression>> args;

    auto &arg_types = getFuncArgTypes(def->getLiteral());
    unsigned idx = 0;
    while (!cur_token.isTokenRP()) {
        if (cur_token.isTokenRP())
            break;

        auto swap = cur_expr_type;
        cur_expr_type = arg_types[idx++];
        args.push_back(parseExpression());
        cur_expr_type = swap;

        if (cur_token.isTokenRP())
            break;

        advanceTokens();
    }

    // Build node for the function call
    std::unique_ptr<Expression> ret = 
        std::make_unique<CallExpression>(def, args);

    return ret;
}

// Parse expression 
std::unique_ptr<Expression> Parser::parseExpression()
{
    // Apply production rule to generate term 
    std::unique_ptr<Expression> left = parseTerm(); 

    while (true) {
        if (cur_token.isTokenPlus() || cur_token.isTokenMinus()) {
            Expression::ExpressionType expr_type;
            if (cur_token.isTokenPlus()) {
                expr_type = Expression::ExpressionType::PLUS;
            }
            else {
                expr_type = Expression::ExpressionType::MINUS;
            }

            advanceTokens();

            std::unique_ptr<Expression> right;

            // Priority one: handle sub-expression in paranthesis ()
            if (cur_token.isTokenLP()) {
                right = parseTerm();
                left = std::make_unique<ArithExpression>(left, 
                       right, 
                       expr_type);
                continue;
            }
            
            // Priority two: check if next token is a function call
            std::unique_ptr<Expression> pending_expr = nullptr;
            if (auto [is_def, is_built_in] = 
                    isFuncDef(cur_token.getLiteral());
                    is_def) {
                strictTypeCheck(cur_token);
                pending_expr = parseCall();
            }

            if (next_token.isTokenAsterisk() || next_token.isTokenSlash()) {
                if (pending_expr != nullptr) {
                    advanceTokens();
                    right = parseTerm(std::move(pending_expr));
                }
                else {
                    right = parseTerm();
                }
            }

            else if (cur_token.isTokenPlus() || cur_token.isTokenMinus()) {
                right = parseTerm();
            }

            else {
                if (pending_expr != nullptr) {
                    right = std::move(pending_expr);
                }
                else {
                    strictTypeCheck(cur_token);
                    right = std::make_unique<LiteralExpression>(cur_token);
                }
                advanceTokens();
            }

            left = std::make_unique<ArithExpression>(left, 
                       right, 
                       expr_type);
        }
        else {
            return left;
        }
    }
}

// Parse term 
std::unique_ptr<Expression> Parser::parseTerm(std::unique_ptr<Expression> pending_left)
{   
    std::unique_ptr<Expression> left = 
        (pending_left != nullptr) ? std::move(pending_left) : parseFactor(); // Parse factor 

    while (true) {
        if (cur_token.isTokenAsterisk() || cur_token.isTokenSlash()) {
            Expression::ExpressionType expr_type;
            if (cur_token.isTokenAsterisk()) {
                expr_type = Expression::ExpressionType::ASTERISK;
            }
            else {
                expr_type = Expression::ExpressionType::SLASH;
            }

            advanceTokens();

            std::unique_ptr<Expression> right;

            // We are trying to mul/div something with higher priority
            if (cur_token.isTokenLP()) {
                right = parseTerm();
            }
            else {
                if (auto [is_def, is_built_in] = 
                            isFuncDef(cur_token.getLiteral());
                            is_def)
                    right = parseCall();
                else
                    right = std::make_unique<LiteralExpression>(cur_token);

                advanceTokens();
            }

            left = std::make_unique<ArithExpression>(left, 
                       right, 
                       expr_type);

        }
        else {
            break;
        }

    }

    return left;
}

// Parse factor 
std::unique_ptr<Expression> Parser::parseFactor()
{
    std::unique_ptr<Expression> left;

    // FIXME: Add code here to handle unary plus and unary minus operators 
    if (cur_token.isTokenMinus()) {
        
    }
    
    // Recursively parse sub-expression 
    if (cur_token.isTokenLP()) {
        advanceTokens();
        left = parseExpression();
        assert(cur_token.isTokenRP()); // Error checking
        advanceTokens();
        return left;
    }
    
    if (auto [is_def, is_built_in] = 
                 isFuncDef(cur_token.getLiteral());
                 is_def)
        left = parseCall();
    else
        left = std::make_unique<LiteralExpression>(cur_token);

    advanceTokens();

    return left;
}

// Print functions for the various nodes in the AST
void RetStatement::printStatement()
{
    std::cout << "    {\n";
    std::cout << "      [Return]\n";
    if (ret->getType() == Expression::ExpressionType::LITERAL) 
        std::cout << "      " << ret->print(4);
    else 
        std::cout << ret->print(4);

    std::cout << "    }\n";
}

void AssnStatement::printStatement()
{
    std::cout << "    {\n";
    if (iden->getType() == Expression::ExpressionType::LITERAL) 
        std::cout << "      " << iden->print(4);
    else 
        std::cout << iden->print(4);

    std::cout << "      =\n";
    
    if (expr->getType() == Expression::ExpressionType::LITERAL) 
        std::cout << "      " << expr->print(4);
    else 
        std::cout << expr->print(4);
    
    std::cout << "    }\n";
}

void FuncStatement::printStatement()
{
    std::cout << "{\n";
    std::cout << "  Function Name: " << iden->print() << "\n";
    std::cout << "  Return Type: ";
    if (func_type == ValueType::Type::VOID) 
        std::cout << "void\n";
    else if (func_type == ValueType::Type::INT) 
        std::cout << "int\n";
    else if (func_type == ValueType::Type::FLOAT) 
        std::cout << "float\n";

    std::cout << "  Arguments\n";
    for (auto &arg : args) 
        std::cout << "    " << arg.print() << "\n";
    
    if (!args.size()) std::cout << "    NONE\n";

    std::cout << "  Codes\n";
    std::cout << "  {\n";
    for (auto &code : codes) 
        code->printStatement();
    
    std::cout << "  }\n";
    std::cout << "}\n";
}
}
