// Definitions for Parser
#ifndef __PARSER_HH__
#define __PARSER_HH__

#include "lexer/lexer.hh"

#include <cassert>
#include <iostream>
#include <memory>
#include <variant>

namespace Frontend
{
// This class helps us track variable/function types
class ValueType
{
  public:
    enum class Type : int
    {
        VOID,
        VOID_PTR,

        INT,
        INT_ARRAY,      // Array of integer values
        INT_PTR,        // Pointer to integer type

        FLOAT,
        FLOAT_ARRAY,    // Array of floating-point values 
        FLOAT_PTR,      // Pointer to floating-point type 

        MAX
    };

    ValueType() {}

    // Return type of variable 
    static Type typeTokenToValueType(Token _tok, bool is_array = false,
                                                 bool is_ptr = false)
    {
        if (_tok.isTokenDesVoid()) {
            if (is_ptr) 
                return Type::VOID_PTR;
	    else 
                return Type::VOID;
        }
        else if (_tok.isTokenDesInt()) {
            if (is_array)
                return Type::INT_ARRAY;
            else if (is_ptr)
                return Type::INT_PTR;
            else
                return Type::INT;
        }
        else if (_tok.isTokenDesFloat()) {
            if (is_array)
                return Type::FLOAT_ARRAY;
            else if (is_ptr)
                return Type::FLOAT_PTR;
            else
                return Type::FLOAT;
        }
        else {
            return Type::MAX;
        }
    }

    // Return type corresponding to the string descriptor 
    static Type strToValueType(std::string _type)
    {
        if (_type == "void")
            return ValueType::Type::VOID;
        else if (_type == "int") 
            return ValueType::Type::INT;
        else if (_type == "float") 
            return ValueType::Type::FLOAT;
        else
            return ValueType::Type::MAX;
    }
};

// Class definition for identifier 
class Identifier
{
  protected:
    Token tok;

  public:
    Identifier(const Identifier &_iden) : tok(_iden.tok) {}

    Identifier(Token &_tok) : tok(_tok) {}

    virtual std::string print()
    {
        return tok.getLiteral();
    }

    auto &getLiteral() { return tok.getLiteral(); }
    auto getType() { return tok.prinTokenType(); }
};

// Class definition for the base class for expression 
class Expression
{
  public:
    enum class ExpressionType : int
    {
        LITERAL,    // i.e., 1
        ARRAY,
        INDEX,

        PLUS,       // i.e., 1 + 2
        MINUS,      // i.e., 1 - 2
        ASTERISK,   // i.e., 1 * 2
        SLASH,      // i.e., 1 / 2

        CALL,

        ILLEGAL
    };

  protected:
    ExpressionType type = ExpressionType::ILLEGAL;
    
  public:
    Expression() {}

    auto getType() { return type; }

    virtual std::string print(unsigned level) { return "[Error] No implementation"; }

    bool isExprLiteral() { return type == ExpressionType::LITERAL; }
    bool isExprArray() { return type == ExpressionType::ARRAY; }
    bool isExprIndex() { return type == ExpressionType::INDEX; }
    bool isExprCall() { return type == ExpressionType::CALL; }
    bool isExprArith()
    {
        return (type == ExpressionType::PLUS || 
                type == ExpressionType::MINUS ||
                type == ExpressionType::ASTERISK ||
                type == ExpressionType::SLASH);
    }
};

// Class definition for literal 
class LiteralExpression : public Expression
{
  protected:
    Token tok;
    
  public:
    LiteralExpression(const LiteralExpression &_expr) 
    {
        tok = _expr.tok;
        type = ExpressionType::LITERAL;
    }

    LiteralExpression(Token &_tok) : tok(_tok) 
    {
        type = ExpressionType::LITERAL;
    }

    std::string& getLiteral() { return tok.getLiteral(); }

    bool isLiteralInt() { return tok.isTokenInt(); }
    bool isLiteralFloat() { return tok.isTokenFloat(); }

    // Debug print associated with the print in ArithExpression
    std::string print(unsigned level) override
    {
        return (tok.getLiteral() + "\n");
    }
};

// Class definition for arithmetic expression 
class ArithExpression : public Expression
{
  protected:
    // Unique pointer is also fine, but will incur errors when invoking copy constructors
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;

  public:

    // Constructor 
    // Unique pointers can be easily converted to shared pointers 
    ArithExpression(std::unique_ptr<Expression> &_left,
                    std::unique_ptr<Expression> &_right,
                    ExpressionType _type)
    {
        left = std::move(_left);
        right = std::move(_right);
        type = _type;
    }

    // Copy constructor 
    ArithExpression(const ArithExpression &_expr)
    {
        left = std::move(_expr.left);
        right = std::move(_expr.right);
        type = _expr.type;
    }

    auto getLeft() { return left.get(); }
    auto getRight() { return right.get(); }

    // Returns symbol for operator
    char getOperator()
    {
        switch(type) {
            case ExpressionType::PLUS:
                return '+';
            case ExpressionType::MINUS:
                return '-';
            case ExpressionType::ASTERISK:
                return '*';
            case ExpressionType::SLASH:
                return '/';
            default:
                assert(false && "unsupported operator");
        }
    }

    // Navigate the abstract syntax tree (AST) and print contents
    std::string print(unsigned level) override
    {
        std::string prefix(level * 2, ' ');

        std::string ret = "";
        if (left != nullptr) {
            if (left->getType() == ExpressionType::LITERAL) {
                ret += prefix;
            }

            if (left->getType() == ExpressionType::CALL)
                ret += left->print(level);
            else
                ret += left->print(level + 1);
        }
        
        if (right != nullptr) {
            ret += prefix;
            if (type == ExpressionType::PLUS) {
                ret += "+";
            }
            else if (type == ExpressionType::MINUS) {
                ret += "-";
            }
            else if (type == ExpressionType::ASTERISK) {
                ret += "*";
            }
            else if (type == ExpressionType::SLASH) {
                ret += "/";
            }
            ret += "\n";
            
            if (right->getType() == ExpressionType::LITERAL) {
                ret += prefix;
            }

            if (right->getType() == ExpressionType::CALL)
                ret += right->print(level);
            else
                ret += right->print(level + 1);
        }
         
        return ret;
    }
};

// Class definition for array 
class ArrayExpression : public Expression
{
  protected:
    std::shared_ptr<Expression> num_ele;            // Number of elements in array 
    std::vector<std::shared_ptr<Expression>> eles;  // Start of array

  public:
    
    // Constructors 
    ArrayExpression(std::unique_ptr<Expression> &_num_ele,
                    std::vector<std::shared_ptr<Expression>> &_eles)
    {
        num_ele = std::move(_num_ele);
        eles = std::move(_eles);
        type = ExpressionType::ARRAY;
    }

    ArrayExpression(const ArrayExpression &_expr)
    {
        num_ele = std::move(_expr.num_ele);
        eles = std::move(_expr.eles);
        type = _expr.type;
    }
   
    auto getNumElements() { return num_ele.get(); }
    auto &getElements() { return eles; }

    std::string print(unsigned level) override
    {
        std::string prefix(level * 2, ' ');

        std::string ret = prefix + "{\n";
        ret += (prefix + "  [ARRAY] \n");
        ret += (prefix + "  [NUM ELEMENTS]\n");
        ret += (prefix + "  {\n");
        if (num_ele->isExprLiteral())
            ret += (prefix + "    ");
        ret += num_ele->print(level + 2);
        ret += (prefix + "  }\n");

        ret += (prefix + "  [ELEMENTS]\n");
        ret += (prefix + "  {\n");
        for (auto &ele : eles)
        {
            ret += (prefix + "    {\n");
            if (ele->isExprLiteral())
                ret += (prefix + "      ");
            ret += ele->print(level + 3);
            ret += (prefix + "    }\n");
        }
        ret += (prefix + "  }\n");
	ret += (prefix + "}\n");
        return ret;
    }

};

// Class definition for array index 
class IndexExpression : public Expression
{
  protected:
    std::shared_ptr<Identifier> iden;
    std::shared_ptr<Expression> idx;

  public:
    IndexExpression(std::unique_ptr<Identifier> &_iden,
                    std::unique_ptr<Expression> &_idx)
    {
        type = ExpressionType::INDEX;

        iden = std::move(_iden);
        idx = std::move(_idx);
    }

    auto &getIden() { return iden->getLiteral(); }
    auto getIndex() { return idx.get(); }

    IndexExpression(const IndexExpression& _expr)
    {
        type = ExpressionType::INDEX;

        iden = std::move(_expr.iden);
        idx = std::move(_expr.idx);
    }
    
    std::string print(unsigned level) override
    {
        std::string prefix(level * 2, ' ');

        std::string ret = prefix + "{\n";
        ret += (prefix + "  [ARRAY] " + iden->getLiteral() + "\n");
        ret += (prefix + "  [INDEX]\n");
        ret += (prefix + "  {\n");
        if (idx->isExprLiteral())
            ret += (prefix + "      ");
        ret += idx->print(level + 3);
        ret += (prefix + "  }\n");
	ret += (prefix + "}\n");
        return ret;

    }
    
};

// Class definition for a function call
class CallExpression : public Expression
{
  protected:
    std::shared_ptr<Identifier> def;
    std::vector<std::shared_ptr<Expression>> args;

  public:

    // Constructors
    CallExpression(const CallExpression &_expr) 
    {
        def = std::move(_expr.def);
        args = std::move(_expr.args);
        type = ExpressionType::CALL;
    }

    CallExpression(std::unique_ptr<Identifier> &_tok, 
        std::vector<std::shared_ptr<Expression>>_args) 
        : def(std::move(_tok))
        , args(std::move(_args))
    {
        type = ExpressionType::CALL;
    }

    // Print AST 
    std::string print(unsigned level) override
    {
        std::string prefix(level * 2, ' ');

        std::string ret = prefix + "{\n";
        ret += (prefix + "  [CALL] " + def->getLiteral() + "\n");
        unsigned idx = 0;
        for (auto &arg : args) {
            ret += (prefix + "  [ARG " + std::to_string(idx++) + "]\n");
            ret += (prefix + "  {\n");
            if (arg->getType() == Expression::ExpressionType::LITERAL)
                ret += (prefix + "    ");
            
            ret += arg->print(level + 2);
            ret += (prefix + "  }\n");
        }

        ret += (prefix + "}\n");
        return ret;
    }

    auto &getCallFunc() { return def->getLiteral(); }
    auto &getArgs() { return args; }
};

// Class definition for generic statement 
class Statement
{
  public:
    enum class StatementType : int
    {
        ASSN_STATEMENT,
        FUNC_STATEMENT,
        RET_STATEMENT,
        BUILT_IN_CALL_STATEMENT,
        NORMAL_CALL_STATEMENT,
        IF_STATEMENT,
        FOR_STATEMENT,
        ILLEGAL
    };

  protected:
    StatementType type = StatementType::ILLEGAL;

  public:
    Statement() {}

    virtual void printStatement() {}

    bool isStatementFunc() { return type == StatementType::FUNC_STATEMENT; }
    bool isStatementAssn() { return type == StatementType::ASSN_STATEMENT; }
    bool isStatementRet() { return type == StatementType::RET_STATEMENT; }

    bool isStatementBuiltinCall() 
    {
        return type == StatementType::BUILT_IN_CALL_STATEMENT; 
    }
    
    bool isStatementNormalCall()
    {
        return type == StatementType::NORMAL_CALL_STATEMENT;
    }
    
    bool isStatementIf() { return type == StatementType::IF_STATEMENT; }
    bool isStatementFor() { return type == StatementType::FOR_STATEMENT; }
};

// Class definition for assignment statement
class AssnStatement : public Statement
{
  protected:
	  std::shared_ptr<Expression> iden;
	  std::shared_ptr<Expression> expr;

  public:
    
    // Constructors 
    AssnStatement(std::unique_ptr<Expression> &_iden,
                 std::unique_ptr<Expression> &_expr)
    {
        type = StatementType::ASSN_STATEMENT;

        iden = std::move(_iden);
        expr = std::move(_expr);
    }
    
    AssnStatement(const AssnStatement &_statement)
    {
        iden = std::move(_statement.iden);
        expr = std::move(_statement.expr);
        type = _statement.type;
    }

    auto getIden() { return iden.get(); }
    auto getExpr() { return expr.get(); }

    void printStatement() override;
};

// Class definition for function definition 
class FuncStatement : public Statement
{
  public:
    // Class definition for a function argument 
    class Argument
    {
      protected:
        ValueType::Type type = ValueType::Type::MAX;
        std::shared_ptr<Identifier> iden;

      public:
        Argument(std::string &_type, std::unique_ptr<Identifier> &_iden)
        {
            type = ValueType::strToValueType(_type);

            assert(type != ValueType::Type::MAX);

            iden = std::move(_iden);
        }

        Argument(const Argument &arg)
        {
            type = arg.type;
            iden = std::move(arg.iden);
        }

        std::string print()
        {
            std::string ret = "";
            if (type == ValueType::Type::INT) ret += "int : ";
            else if (type == ValueType::Type::FLOAT) ret += "float : ";

            ret += iden->getLiteral();

            return ret;
        }

        std::string &getLiteral() { return iden->getLiteral(); }
        auto getArgType() { return type; }
    };

  protected:
    ValueType::Type func_type;                                      // Return type 
    std::shared_ptr<Identifier> iden;                               // Function name 
    std::vector<Argument> args;                                     // Argument list 
    std::vector<std::shared_ptr<Statement>> codes;                  // Function body 
    std::unordered_map<std::string, ValueType::Type> local_vars;    // Symbol table to store local variables 

  public:
    FuncStatement(ValueType::Type _type,
                  std::unique_ptr<Identifier> &_iden,
                  std::vector<Argument> &_args,
                  std::vector<std::shared_ptr<Statement>> &_codes,
                  std::unordered_map<std::string,ValueType::Type> &_local_vars)
    {
        type = StatementType::FUNC_STATEMENT;

        func_type = _type;
        iden = std::move(_iden);
        args = _args;
        codes = std::move(_codes);
        local_vars = _local_vars;
    }
    
    FuncStatement(const FuncStatement &_statement)
    {
        type = _statement.type;

        func_type = _statement.func_type;
        iden = std::move(_statement.iden);
        args = _statement.args;
        codes = std::move(_statement.codes);
        local_vars = _statement.local_vars;
    }
  
    auto getLocalVars() {return &local_vars; }

    auto getRetType() { return func_type; }

    auto &getFuncName() { return iden->getLiteral(); }
    auto &getFuncArgs() { return args; }
    auto &getFuncCodes() { return codes; }

    void printStatement() override;
};

// Class definition for function call
class CallStatement : public Statement
{
  protected:
    std::shared_ptr<Expression> expr;

  public:
    CallStatement(std::unique_ptr<Expression> &_expr,
                  StatementType _type)
    {
        type = _type;

        expr = std::move(_expr);
    }
    
    CallStatement(const CallStatement &_statement)
    {
        type = _statement.type;

        expr = std::move(_statement.expr);
    }
    
    void printStatement() override
    {
        std::cout << expr->print(2);
    }

    CallExpression* getCallExpr()
    {
        CallExpression *call = 
            static_cast<CallExpression*>(expr.get());
        return call;
    }
};

// Class definition for return statement
class RetStatement : public Statement
{
  protected:
    std::shared_ptr<Expression> ret;

  public:
    RetStatement(std::unique_ptr<Expression> &_ret) : ret(std::move(_ret))
    {
        type = StatementType::RET_STATEMENT;
    }

    RetStatement(const RetStatement &_statement)
    {
        type = _statement.type;
        ret = std::move(_statement.ret);
    }

    auto getRetVal() { return ret.get(); }

    void printStatement() override;
};

// Class definition for condition, for selection statements and loops
// TODO: condition may need a vector to accommodate situations like cond_0 && cond_1
class Condition
{
  protected:
    ValueType::Type comp_type;

    enum class OperatorType : int
    {
        EQ, NE, GT, GE, LT, LE, MAX
    };
    OperatorType opr_type = OperatorType::MAX;
    std::string opr_type_str;

    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;

  public:
    Condition(std::unique_ptr<Expression> &_left,
              std::unique_ptr<Expression> &_right,
              std::string &_opr_type_str,
              ValueType::Type _comp_type)
    {
        left = std::move(_left);
        right = std::move(_right);

        if (_opr_type_str == "==")
            opr_type = OperatorType::EQ;
        else if (_opr_type_str == "!=")
            opr_type = OperatorType::NE;
        else if (_opr_type_str == ">")
            opr_type = OperatorType::GT;
        else if (_opr_type_str == ">=")
            opr_type = OperatorType::GE;
        else if (_opr_type_str == "<")
            opr_type = OperatorType::LT;
        else if (_opr_type_str == "<=")
            opr_type = OperatorType::LE;
        else
            assert(false);

        opr_type_str = _opr_type_str;

        comp_type = _comp_type; 
    }

    Condition(const Condition& _cond)
        : left(std::move(_cond.left))
        , right(std::move(_cond.right))
        , opr_type_str(_cond.opr_type_str)
        , comp_type(_cond.comp_type)
    {}

    auto getType() { return comp_type; }
    auto &getOpr() { return opr_type_str; }
    auto getLeft() { return left.get(); }
    auto getRight() { return right.get(); }

    void printStatement();
};

// Class definition for selection statement 
class IfStatement : public Statement
{    
  protected:
    std::shared_ptr<Condition> cond;
    std::vector<std::shared_ptr<Statement>> taken_block;
    std::vector<std::shared_ptr<Statement>> not_taken_block;

    std::unordered_map<std::string, ValueType::Type> taken_local_vars;
    std::unordered_map<std::string, ValueType::Type> not_taken_local_vars;

  public:

    IfStatement(std::unique_ptr<Condition> &_cond,
                std::vector<std::shared_ptr<Statement>> &_taken_block,
                std::vector<std::shared_ptr<Statement>> &_not_taken_block,
                std::unordered_map<std::string, 
                                   ValueType::Type> &_taken_local_vars,
                std::unordered_map<std::string, 
                                   ValueType::Type> &_not_taken_local_vars)
    {
        type = StatementType::IF_STATEMENT;

        cond = std::move(_cond);
        taken_block = std::move(_taken_block);
        not_taken_block = std::move(_not_taken_block);

        taken_local_vars = _taken_local_vars;
        not_taken_local_vars = _not_taken_local_vars;
    }

    IfStatement(const IfStatement &_if)
        : cond(std::move(_if.cond))
        , taken_block(std::move(_if.taken_block))
        , not_taken_block(std::move(_if.not_taken_block))
        , taken_local_vars(_if.taken_local_vars)
        , not_taken_local_vars(_if.not_taken_local_vars)
    {}

    auto getCond() { return cond.get(); }
    auto &getTakenBlock() { return taken_block; }
    auto &getNotTakenBlock() { return not_taken_block; }
    auto getTakenBlockVars() { return &taken_local_vars; }
    auto getNotTakenBlockVars() { return &not_taken_local_vars; }

    void printStatement() override;
};

// Class definition for loop 
class ForStatement : public Statement
{    
  protected:
    std::shared_ptr<Statement> start;
    std::shared_ptr<Condition> end;
    std::shared_ptr<Statement> step;
    std::vector<std::shared_ptr<Statement>> block;

    std::unordered_map<std::string, ValueType::Type> block_local_vars;

  public:

    ForStatement(std::unique_ptr<Statement> &_start,
                 std::unique_ptr<Condition> &_end,
                 std::unique_ptr<Statement> &_step,
                 std::vector<std::shared_ptr<Statement>> &_block,
                 std::unordered_map<std::string, 
                                    ValueType::Type> &_block_local_vars)
    {
        type = StatementType::FOR_STATEMENT;
        
        start = std::move(_start);
        end = std::move(_end);
        step = std::move(_step);
        block = std::move(_block);

        block_local_vars = _block_local_vars;
    }

    ForStatement(const ForStatement &_for)
        : start(std::move(_for.start))
        , end(std::move(_for.end))
        , step(std::move(_for.step))
        , block(std::move(_for.block))
        , block_local_vars(_for.block_local_vars)
    {}

    auto getStart() { return start.get(); }
    auto getEnd() { return end.get(); }
    auto getStep() { return step.get(); }
    auto &getBlock() { return block; }
    auto getBlockVars() { return &block_local_vars; }

    void printStatement() override;
};

// Class definition for program 
class Program
{
  protected:
    std::vector<std::unique_ptr<Statement>> statements;

  public:
    Program() {}

    void addStatement(std::unique_ptr<Statement> &_statement)
    {
        statements.push_back(std::move(_statement));
    }

    void printStatements()
    {
        for (auto &statement : statements) { statement->printStatement(); }
    }

    auto& getStatements() { return statements; }
};

// Class definition for parser
class Parser
{
  protected:
    Program program;

  protected:
    Token cur_token;
    Token next_token;    
    
  /************* Section one - record local variable types ***************/
  protected:
    
    bool isTokenTypeKeyword(Token &_tok)
    {
        return ValueType::typeTokenToValueType(_tok) !=
               ValueType::Type::MAX;
    }

    // Track each local variable's type
    // vector is needed because we need a way to distinguish vars inside
    // if/else, for.
    int entering_sub_block = 0;
    std::vector<std::unordered_map<std::string,
                                   ValueType::Type>*> local_vars_tracker;
    
    // recordLocalVars v1 - record the arguments
    void recordLocalVars(FuncStatement::Argument &arg,
                         bool is_array = false,
                         bool is_ptr = false)
    {
        auto arg_name = arg.getLiteral();
        auto arg_type = arg.getArgType();
        assert(arg_type != ValueType::Type::MAX);

        auto &tracker = local_vars_tracker.back();

        if (auto iter = tracker->find(arg_name);
                iter != tracker->end()) {
            std::cerr << "[Error] recordLocalVars: "
                      << "duplicated variable definition."
                      << std::endl;
            exit(0);
        }
        else {
            tracker->insert({arg_name, arg_type});
        }
    }
    
    // recordLocalVars v2 - record local variables
    void recordLocalVars(Token &_tok, Token &_type_tok,
                         bool is_array = false,
                         bool is_ptr = false)
    {
        // determine the token type
        auto var_type = 
            ValueType::typeTokenToValueType(_type_tok, is_array, is_ptr);
        assert(var_type != ValueType::Type::MAX);
        cur_expr_type = var_type;

        if (cur_expr_type == ValueType::Type::INT_ARRAY ||
            cur_expr_type == ValueType::Type::INT_PTR) {
            cur_expr_type = ValueType::Type::INT;
        }
        else if (cur_expr_type == ValueType::Type::FLOAT_ARRAY ||
                 cur_expr_type == ValueType::Type::FLOAT_PTR) {
            cur_expr_type = ValueType::Type::FLOAT;
        }
        
        // We should always allocate new variables to the most inner block
        auto &tracker = local_vars_tracker.back();
        tracker->insert({_tok.getLiteral(), var_type});
    }

    std::pair<bool,ValueType::Type> isVarAlreadyDefined(Token &_tok)
    {
        for (int i = local_vars_tracker.size() - 1; i >= 0; i--) {
            auto &tracker = local_vars_tracker[i];
            if (auto iter = tracker->find(_tok.getLiteral());
                    iter != tracker->end()) {
                return std::make_pair(true, iter->second);
            }
        }

        return std::make_pair(false, ValueType::Type::MAX);
    }

    // Record function information 
    struct FuncRecord
    {
        ValueType::Type ret_type;
        std::vector<ValueType::Type> arg_types;

        bool is_built_in = false;

        FuncRecord() {}

        FuncRecord(const FuncRecord& _record)
            : ret_type(_record.ret_type)
            , arg_types(_record.arg_types)
            , is_built_in(_record.is_built_in)
        {}
    };

    std::unordered_map<std::string,FuncRecord> func_def_tracker;
    
    void recordDefs(std::string &_def,
                    ValueType::Type _type,
                    std::vector<FuncStatement::Argument> &_args)
    {
        auto iter = func_def_tracker.find(_def);
        assert(iter == func_def_tracker.end() && "duplicated def");

        FuncRecord record;
        record.ret_type = _type;

        auto &arg_types = record.arg_types;
        for (auto &arg : _args) {
            arg_types.push_back(arg.getArgType());
        }
        
        func_def_tracker[_def] = record;
    }
    
    std::pair<bool,bool> isFuncDef(std::string &_def)
    {
        if (auto iter = func_def_tracker.find(_def);
                iter != func_def_tracker.end()) {
            return std::make_pair(true, iter->second.is_built_in);
        }
        else {
            return std::make_pair(false,false);
        }
    }

  public:
    auto& getFuncArgTypes(std::string &func_name)
    {
        auto iter = func_def_tracker.find(func_name);
        assert(iter != func_def_tracker.end());
        return iter->second.arg_types;
    }

    auto &getFuncRetType(std::string &_def)
    {
        auto iter = func_def_tracker.find(_def);
        assert(iter != func_def_tracker.end());

        return iter->second.ret_type;
    }

  protected:

    // Enforce strict type checking
    // Force all elements inside an expression to be of same type
    ValueType::Type cur_expr_type = ValueType::Type::MAX;
    
    // strictTypeCheck v1 - check the token has the same type as the cur_expr_type
    void strictTypeCheck(Token &_tok, bool is_index_or_deref = false)
    {
        if (cur_expr_type == ValueType::Type::MAX) return;

        ValueType::Type tok_type = getTokenType(_tok, is_index_or_deref);
        
        if (tok_type == cur_expr_type)
            return;

        std::cerr << "[Error] Token type of <" << _tok.getLiteral()
                  << "> inconsistent within expression" << std::endl;
        std::cerr << "[Line] " << cur_token.getLine() << "\n";
        exit(0);
    }

    ValueType::Type getTokenType(Token &_tok, bool is_index_or_deref = false)
    {
        ValueType::Type tok_type;
        if (_tok.isTokenInt()) tok_type = ValueType::Type::INT;
        else if (_tok.isTokenFloat()) tok_type = ValueType::Type::FLOAT;
        else tok_type = ValueType::Type::MAX;

        // If token is a variable, extract its recorded type
        for (int i = local_vars_tracker.size() - 1; i >= 0; i--) {
            auto &tracker = local_vars_tracker[i];
            if (auto iter = tracker->find(_tok.getLiteral());
                    iter != tracker->end()) {
                tok_type = iter->second;
                break;
            }
        }
        
        // If token is a function name, extract its recorded type
        if (auto iter = func_def_tracker.find(_tok.getLiteral());
                iter != func_def_tracker.end()) {
            tok_type = iter->second.ret_type;
        }
       
        // If dealing with array, extract its type
        if (is_index_or_deref) {
            if (tok_type == ValueType::Type::INT_ARRAY)
                tok_type = ValueType::Type::INT;
            else if (tok_type == ValueType::Type::FLOAT_ARRAY)
                tok_type = ValueType::Type::FLOAT;
        }

        return tok_type;
    }

  protected:
    std::unique_ptr<Lexer> lexer;

  public:
    Parser(const char* fn); 

    void printStatements() { program.printStatements(); }

    auto &getProgram() { return program; }

  protected:
    void parseProgram();
    void advanceTokens();

    void parseStatement(std::string&,
                        std::vector<std::shared_ptr<Statement>>&);
    std::unique_ptr<Statement> parseAssnStatement();

    std::unique_ptr<Condition> parseCondition();
    std::unique_ptr<Statement> parseIfStatement(std::string&);
    std::unique_ptr<Statement> parseForStatement(std::string&);

    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseTerm(
        std::unique_ptr<Expression> pending_left = nullptr);
    std::unique_ptr<Expression> parseFactor();

    std::unique_ptr<Expression> parseArrayExpr();
    std::unique_ptr<Expression> parseIndex();
    std::unique_ptr<Expression> parseCall();
};
}
#endif
