//
//  ExpressParser.cpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/22.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#include "ProgramParser.hpp"
#include "ast.hpp"
#include "lexer_parser.hpp"
#include <sstream>
#include <string>
#include <map>

using std::ostringstream;
using std::map;
using std::string;

ASTTreeType get_ast_type(TokenType token) {
    switch (token) {
        case NUMBER:
            return TYPE_NumberLiteral;
        case IDENTIFIER:
            return TYPE_IdentifierLiteral;
        case TOKEN_STRING:
            return TYPE_StringLiteral;
        default:
            // other
            return TYPE_ASTLeaf;
    }
}

void ProgramParser::check(std::shared_ptr<ASTTree> ast_tree) const
{
    switch (ast_tree->getType()) {
        case TYPE_BREAK_STMT:
            {
                if (!ast_tree->has_parent(TYPE_WHILE_STMT, TYPE_FUNC_DEF)) {
                    throw std::string("break only be in loop stmt.");
                }
            }
            break;
        case TYPE_FUNC_DEF:
            {
                ASTTree *root_pragram = ast_tree->getParent();
                if (!root_pragram) {
                    throw std::string("bad func define locate.");
                }
                if(root_pragram->getParent() != NULL) {
                     throw std::string("func define only be at root.");
                }
            }
            break;
        default:
            break;
    }
    for (int i = 0; i < ast_tree->get_child_length(); ++i) {
        std::shared_ptr<ASTTree> node = ast_tree->get_child(i);
        check(node);
    }
}

std::shared_ptr<ASTProgram> ProgramParser::root_program()
{
    std::shared_ptr<ASTProgram> ast_program = program();
    check(ast_program);
    return ast_program;
}

std::shared_ptr<ASTProgram> ProgramParser::program()
{
    std::shared_ptr<ASTProgram> ast_program(new ASTProgram);
    for (;;) {
        bool end = false;
        while (is_token(TOKEN_EOL)) {
            read_token_as_leaf();
        }
        if (is_token("}")) {
            // end of block
            break;
        }
        std::shared_ptr<ASTTree> st = stmt(end);
        ast_program->add_stmt(st);
        if (end) {
            break;
        }
        while (is_token(TOKEN_EOL)) {
            read_token_as_leaf();
        }
        if (is_token(TOKEN_EOF)) {
            // end of file
            break;
        }
        if (is_token("}")) {
            // end of block
            break;
        }
    }    
    return ast_program;
}

bool ProgramParser::end_of_stmt()
{
    if (is_token(TOKEN_EOL)) {
        read_token_as_leaf();
        return true;
    }
    if (is_token(TOKEN_EOF)) {
        read_token_as_leaf();
        return false;
    }
    throw_exception(std::string("need a new line or end of file."));
    return false;
}

std::shared_ptr<ASTTree> ProgramParser::stmt(bool& end)
{
    while (is_token(TOKEN_EOL)) {
        read_token_as_leaf();
    }
   
    if(is_token("if")) {
        // if stmt
        std::shared_ptr<ASTLeaf> if_leaf(read_token_as_leaf());
        get_token_as_leaf("(");
        std::shared_ptr<ASTTree> expr(express());
        get_token_as_leaf(")");
        get_token_as_leaf("{");
        std::shared_ptr<ASTProgram> ast_program(program());
        get_token_as_leaf("}");
        while (is_token(TOKEN_EOL)) {
            read_token_as_leaf();
        }
        shared_ptr<ASTIfStmt> if_stmt(new ASTIfStmt(expr, ast_program));
        
        while (is_token("elseif")) {
            std::shared_ptr<ASTLeaf> elseif_leaf(read_token_as_leaf());
            get_token_as_leaf("(");
            std::shared_ptr<ASTTree> expr0(express());
            get_token_as_leaf(")");
            
            get_token_as_leaf("{");
            std::shared_ptr<ASTProgram> ast_program_elseif(program());
            get_token_as_leaf("}");
            
            ElseIfNode elseIfNode;
            elseIfNode.expr = expr0;
            elseIfNode.program = ast_program_elseif;
            if_stmt->add_elseif(elseIfNode);
        }
        if (is_token("else")) {
            std::shared_ptr<ASTLeaf> else_leaf(read_token_as_leaf());
            while (is_token(TOKEN_EOL)) {
                read_token_as_leaf();
            }
            get_token_as_leaf("{");
            std::shared_ptr<ASTProgram> else_ast_program(program());
            get_token_as_leaf("}");
            bool end_of_line = end_of_stmt();
            if (!end_of_line) {
                end = true;
            }
            if_stmt->set_else(else_ast_program);
        } else {
            // no else
        }
        return if_stmt;
    } else if(is_token("while")) {
        // while stmt
        std::shared_ptr<ASTLeaf> while_leaf(read_token_as_leaf());
        std::shared_ptr<ASTLeaf> lparen(get_token_as_leaf("("));
        std::shared_ptr<ASTTree> expr(express());
        std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(")"));
        get_token_as_leaf("{");
        std::shared_ptr<ASTProgram> ast_program(program());
        get_token_as_leaf("}");
        bool end_of_line = end_of_stmt();
        if (!end_of_line) {
            end = true;
        }
        return shared_ptr<ASTWhileStmt>(new ASTWhileStmt(expr, ast_program));
    } else if (is_token("break")) {
        std::shared_ptr<ASTLeaf> break_leaf(read_token_as_leaf());
        return shared_ptr<ASTBreakStmt>(new ASTBreakStmt());
    } else if (is_token("func")) {
        // function define
        std::shared_ptr<ASTLeaf> func_leaf(read_token_as_leaf());
        std::shared_ptr<ASTLeaf> name_leaf(get_token_as_leaf(IDENTIFIER));
        std::shared_ptr<ASTLeaf> lparen(get_token_as_leaf(LPAREN));
        std::vector<std::string> names;
        if (is_token(IDENTIFIER)) {
            std::shared_ptr<ASTLeaf> arg(get_token_as_leaf(IDENTIFIER));
            names.push_back(arg->get_value());
            while(is_token(COMMA)) {
                std::shared_ptr<ASTLeaf> comma(read_token_as_leaf());
                arg = get_token_as_leaf(IDENTIFIER);
                names.push_back(arg->get_value());
            }
        }
        std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPAREN));
        get_token_as_leaf("{");
        std::shared_ptr<ASTProgram> ast_program(program());
        get_token_as_leaf("}");
        bool end_of_line = end_of_stmt();
        if (!end_of_line) {
            end = true;
        }
        return std::shared_ptr<ASTFuncDefStmt> (new ASTFuncDefStmt(name_leaf->get_token(), names, ast_program));
    } else if (is_token("return")) {
        std::shared_ptr<ASTLeaf> return_leaf(read_token_as_leaf());
        if (!is_token("}")) {
            return shared_ptr<ASTReturnStmt>(new ASTReturnStmt(express()));
        } else {
            return shared_ptr<ASTReturnStmt>(new ASTReturnStmt(NULL));
        }
    } else if (is_token("for")) {
        std::shared_ptr<ASTLeaf> for_leaf(read_token_as_leaf());
        std::shared_ptr<ASTLeaf> var_name;
        std::shared_ptr<ASTTree> iter;
        if (is_token(LPAREN)) {
            // for (a in arr) { ... }
            std::shared_ptr<ASTLeaf> lparen(get_token_as_leaf(LPAREN));
            var_name = get_token_as_leaf(IDENTIFIER);
            std::shared_ptr<ASTLeaf> word_in(get_token_as_leaf("in"));
            iter = express();
            std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPAREN));
        } else {
            // for a in arr { ... }
            var_name = get_token_as_leaf(IDENTIFIER);
            std::shared_ptr<ASTLeaf> word_in(get_token_as_leaf("in"));
            iter = express();
        }
        // for {..} body
        get_token_as_leaf("{");
        std::shared_ptr<ASTProgram> ast_program(program());
        get_token_as_leaf("}");
        bool end_of_line = end_of_stmt();
        if (!end_of_line) {
            end = true;
        }
        return std::shared_ptr<ASTForStmt>(new ASTForStmt(var_name->get_token(), iter, ast_program));
    } else if (is_token("import")) {
        std::shared_ptr<ASTLeaf> import_leaf(read_token_as_leaf());
        std::shared_ptr<ASTLeaf> word_leaf = get_token_as_leaf(IDENTIFIER);
        std::shared_ptr<IdentifierLiteral> word = std::dynamic_pointer_cast<IdentifierLiteral>(word_leaf);
        bool end_of_line = end_of_stmt();
        if (!end_of_line) {
            end = true;
        }
        return std::shared_ptr<ASTImportStmt>(new ASTImportStmt(word->get_token()));
    } else {
        // express
        std::shared_ptr<ASTTree> expr = express();
        bool end_of_line = end_of_stmt();
        if (!end_of_line) {
            end = true;
        }
        return expr;
    }
}


std::shared_ptr<ASTTree> ProgramParser::express()
{
    if (is_token(IDENTIFIER) && is_token(ASSIGNER, 2)) {
        // for assigner
        std::shared_ptr<IdentifierLiteral> name = std::dynamic_pointer_cast<IdentifierLiteral>(read_token_as_leaf());
        std::shared_ptr<ASTAssignExpr> expr(new ASTAssignExpr(name, NULL));
        while (is_token("=")) {
            std::shared_ptr<ASTLeaf> assigner(read_token_as_leaf());
            std::shared_ptr<ASTTree> right(express());
            expr = std::shared_ptr<ASTAssignExpr>(new ASTAssignExpr(name, right));
        }
        return expr;
    } else {
        return assign_expr();
    }
}

std::shared_ptr<ASTTree> ProgramParser::assign_expr()
{
    std::shared_ptr<ASTTree> left = compare_expr();
    while (is_token("and") || is_token("or")) {
        std::shared_ptr<ASTLeaf> logic_op(read_token_as_leaf());
        std::shared_ptr<ASTTree> right(compare_expr());
        left = std::shared_ptr<LogicBinaryExpr>(new LogicBinaryExpr(left, logic_op, right));
    }
    return left;
}

std::shared_ptr<ASTTree> ProgramParser::compare_expr()
{
    std::shared_ptr<ASTTree> left = add_or_sub();
    while (is_token(TOKEN_COMPARE)) {
        std::shared_ptr<ASTLeaf> op(read_token_as_leaf());
        std::shared_ptr<ASTTree> right(add_or_sub());
        left = std::shared_ptr<ASTTree>(new BinaryExpr(left, op, right));
    }
    return left;
}


std::shared_ptr<ASTTree> ProgramParser::add_or_sub()
{
    std::shared_ptr<ASTTree> left = term();
    while (is_token("+") || is_token("-")) {
        std::shared_ptr<ASTLeaf> op(read_token_as_leaf());
        std::shared_ptr<ASTTree> right(term());
        left = std::shared_ptr<ASTTree>(new BinaryExpr(left, op, right));
    }
    return left;
}

std::shared_ptr<ASTLeaf> ProgramParser::read_token_as_leaf()
{
    std::shared_ptr<Token> outToken(new Token);
    if (!m_lexer->readNext(outToken)) {
        throw_exception(std::string("except term."));
    }
    switch (get_ast_type(outToken->getType())){
        case TYPE_NumberLiteral:
            {
                return std::shared_ptr<NumberLiteral>(new NumberLiteral(outToken));
            }
        case TYPE_IdentifierLiteral:
            {
                return std::shared_ptr<IdentifierLiteral>(new IdentifierLiteral(outToken));
            }
        case TYPE_StringLiteral:
            {
                return std::shared_ptr<StringLiteral>(new StringLiteral(outToken));
            }
        default:
            std::shared_ptr<ASTLeaf> leaf(new ASTLeaf(outToken, get_ast_type(outToken->getType())));
            return leaf;
    }
}

std::shared_ptr<ASTTree> ProgramParser::term()
{
    std::shared_ptr<ASTTree> left = parentheses_expr(dot_oper());
    while (is_token("*") || is_token("/") || is_token("%")) {
        std::shared_ptr<ASTLeaf> op(read_token_as_leaf());
        std::shared_ptr<ASTTree> right(parentheses_expr(dot_oper()));
        left = std::shared_ptr<ASTTree>(new BinaryExpr(left, op, right));
    }
    return left;
}

std::shared_ptr<ASTTree> ProgramParser::parentheses_expr(std::shared_ptr<ASTTree> left) {
    while (true) {
        if (is_token(LPARENTHESES)) {
            // [] visit
            std::shared_ptr<ASTLeaf> lparentheses(get_token_as_leaf(LPARENTHESES));
            std::shared_ptr<ASTTree> a;
            std::shared_ptr<ASTTree> b;
            bool slice = false;
            if (is_token(COLON)) {
                std::shared_ptr<ASTLeaf> colon(get_token_as_leaf(COLON));
                if (is_token(RPARENTHESES)) {
                    // x[:]
                    slice = true;
                } else {
                    // x[:b]
                    b = express();
                    slice = true;
                }
            } else {
                a = express();
                if (is_token(COLON)) {
                    std::shared_ptr<ASTLeaf> colon(get_token_as_leaf(COLON));
                    if (is_token(RPARENTHESES)) {
                        // x[a:]
                        slice = true;
                    } else {
                        // x[a:b]
                        b = express();
                        slice = true;
                    }
                } else {
                    // x[a]
                    slice = false;
                }
            }
            std::shared_ptr<ASTLeaf> rparentheses(get_token_as_leaf(RPARENTHESES));
            if (is_token(ASSIGNER)) {
                std::shared_ptr<ASTLeaf> assigner(get_token_as_leaf(ASSIGNER));
                std::shared_ptr<ASTTree> value_expr = express();
                left = std::shared_ptr<ASTParenthesesAssignerExpr>(new ASTParenthesesAssignerExpr(left, a, value_expr));
            } else {
                left = std::shared_ptr<ASTParenthesesExpr>(new ASTParenthesesExpr(left, a, b, slice));
            }
        } else if (is_token(LPAREN)) {
            // func call
            std::vector<std::shared_ptr<ASTTree>> arg_list(call_args());
            left = std::shared_ptr<ASTCallFuncExpr>(new ASTCallFuncExpr(left, arg_list));
        } else {
            break;
        }
    }
    return left;
}

std::shared_ptr<ASTTree> ProgramParser::dot_oper()
{
    std::shared_ptr<ASTTree> left = factor();
    while (is_token(".")) {
        std::shared_ptr<ASTLeaf> op(read_token_as_leaf());
        std::shared_ptr<ASTLeaf> name = get_token_as_leaf(IDENTIFIER);
        if (is_token(LPAREN)) {
            std::shared_ptr<ASTLeaf> lparen(get_token_as_leaf(LPAREN));
            std::vector<std::shared_ptr<ASTTree>> arg_list;
            while (!is_token(RPAREN)) {
                std::shared_ptr<ASTTree> arg = express();
                arg_list.push_back(arg);
                if (!is_token(COMMA)) {
                    break;
                }
                get_token_as_leaf(COMMA);
            }
            std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPAREN));

            left = std::shared_ptr<ASTDotCallFuncExpr>(new ASTDotCallFuncExpr(left, name->get_token(), arg_list));
        } else {
            left = std::shared_ptr<ASTDotPropertyExpr>(new ASTDotPropertyExpr(left, name->get_token()));
        }
    }
    return left;
}

std::vector<std::shared_ptr<ASTTree>> ProgramParser::call_args() {
    std::shared_ptr<ASTLeaf> lparen(get_token_as_leaf(LPAREN));
    std::vector<std::shared_ptr<ASTTree>> arg_list;
    while (!is_token(RPAREN)) {
        std::shared_ptr<ASTTree> arg = express();
        arg_list.push_back(arg);
        if (!is_token(COMMA)) {
            break;
        }
        get_token_as_leaf(COMMA);
    }
    std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPAREN));
    return arg_list;
}

std::shared_ptr<ASTTree> ProgramParser::factor()
{
    if (is_token("(")) {
        std::shared_ptr<ASTLeaf> lparen(read_token_as_leaf()); // for (
        std::shared_ptr<ASTTree> expr = express();
        std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(")"));
        return expr;
    } else if (is_token("-")) {
        std::shared_ptr<ASTLeaf> negative(read_token_as_leaf()); // for -
        std::shared_ptr<ASTTree> expr = express();
        return std::shared_ptr<MinusExpr>(new MinusExpr(expr));
    }  else if (is_token(TOKEN_STRING)) {
        std::shared_ptr<ASTLeaf> string_ast(get_token_as_leaf(TOKEN_STRING));
        return string_ast;
    } else if (is_token(IDENTIFIER) && is_token(LPAREN, 2)) {
        // func call
        std::shared_ptr<ASTLeaf> name_leaf(get_token_as_leaf(IDENTIFIER));
        std::vector<std::shared_ptr<ASTTree>> arg_list(call_args());
        std::shared_ptr<ASTTree> left = std::shared_ptr<ASTCallFuncExpr>(new ASTCallFuncExpr(name_leaf, arg_list));
        while (true) {
            if (is_token(LPAREN)) {
                arg_list = call_args();
                left = std::shared_ptr<ASTCallFuncExpr>(new ASTCallFuncExpr(left, arg_list));
            } else if (is_token(LPARENTHESES)) {
                left = parentheses_expr(left);
            } else {
                break;
            }
        }
        return left;
    } else if (is_token(LPARENTHESES)) {
        std::shared_ptr<ASTLeaf> lparen(get_token_as_leaf(LPARENTHESES));
        std::vector<std::shared_ptr<ASTTree>> init_express;
        if (!is_token(RPARENTHESES)) {
            std::shared_ptr<ASTTree> expr = express();
            if (is_token(COMMA)) {
                // for list init
                init_express.push_back(expr);
                while(is_token(COMMA)) {
                    std::shared_ptr<ASTLeaf> comma(read_token_as_leaf());
                    expr = express();
                    init_express.push_back(expr);
                }
                std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPARENTHESES));
                return shared_ptr<ASTCreateListExpr>(new ASTCreateListExpr(init_express));
            } else if (is_token(DOUBLE_DOT)) {
                // for int iterator init
                std::shared_ptr<ASTLeaf> ddot1(get_token_as_leaf(DOUBLE_DOT));
                std::shared_ptr<ASTTree> to_expr = express();
                std::shared_ptr<ASTTree> step_expr;
                if (is_token(DOUBLE_DOT)) {
                    std::shared_ptr<ASTLeaf> ddot2(get_token_as_leaf(DOUBLE_DOT));
                    step_expr = express();
                }
                std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPARENTHESES));
                return shared_ptr<ASTCreateIntIteratorExpr>(new ASTCreateIntIteratorExpr(expr, to_expr, step_expr));
            } else {
                // [ expr ]
                std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPARENTHESES));
                init_express.push_back(expr);
                return shared_ptr<ASTCreateListExpr>(new ASTCreateListExpr(init_express));
            }
        } else {
            // empty array
            std::shared_ptr<ASTLeaf> rparen(get_token_as_leaf(RPARENTHESES));
            return shared_ptr<ASTCreateListExpr>(new ASTCreateListExpr(init_express));
        }
    } else if (is_token(BLOCK_BEGIN)) {
        // init map
        std::shared_ptr<ASTLeaf> block_begin(get_token_as_leaf(BLOCK_BEGIN));
        std::vector<std::pair<std::shared_ptr<ASTTree>, std::shared_ptr<ASTTree>>> init_express;
        bool first = true;
        while (!is_token(BLOCK_END)) {
            if (first) {
                first = false;
            } else {
                std::shared_ptr<ASTLeaf> comma(get_token_as_leaf(COMMA));
            }
            std::shared_ptr<ASTTree> name;
            if (is_token(TOKEN_STRING)) {
                name = get_token_as_leaf(TOKEN_STRING);
            } else {
                name = express();
            }
            std::shared_ptr<ASTLeaf> colon(get_token_as_leaf(COLON));
            std::shared_ptr<ASTTree> value;
            if (is_token(TOKEN_STRING)) {
                value = get_token_as_leaf(TOKEN_STRING);
            } else {
                value = express();
            }
            init_express.push_back(std::make_pair(name, value));
        }
        std::shared_ptr<ASTLeaf> block_end(get_token_as_leaf(BLOCK_END));
        return std::shared_ptr<ASTCreateMapExpr>(new ASTCreateMapExpr(init_express));
    } else {
        std::shared_ptr<Token> outToken(new Token);
        if (!m_lexer->readNext(outToken)) {
            throw_exception(std::string("except number or indentifier!"));
        }
        if (outToken->getType() == NUMBER) {
            return std::shared_ptr<ASTLeaf>(new NumberLiteral(outToken));
        } else if (outToken->getType() == IDENTIFIER) {
            return std::shared_ptr<ASTLeaf>(new IdentifierLiteral(outToken));
        } else if (outToken->getType() == RESERVED_WORD) {
            if (SPECIAL_WORD_True == outToken->getString() ||
                SPEICAL_WORD_False == outToken->getString() ||
                SPECAIL_WORD_Null == outToken->getString()) {
                return std::shared_ptr<ASTLeaf>(new ASTSpecialValueLiteral(outToken));
            }
        }
    }
    throw_exception(std::string("except number or indentifier !!"));
    return 0;
}

std::shared_ptr<Token> ProgramParser::peek_token(int n)
{
    std::shared_ptr<Token> outToken(new Token);
    if (!m_lexer->peek(outToken, n)) {
        return NULL;
    }
    return outToken;
}

bool ProgramParser::is_token(const char* name, int n)
{
    std::shared_ptr<Token> outToken(peek_token(n));
    if (!outToken) {
        return false;
    }
    std::string the_name(name);
    return the_name == outToken->getString();
}

bool ProgramParser::is_token(TokenType tokenType, int n) {
    std::shared_ptr<Token> outToken(peek_token(n));
    if (!outToken) {
        return false;
    }
    return tokenType == outToken->getType();
}

bool ProgramParser::is_token(TokenType tokenType, const char* name, int n)
{
    std::shared_ptr<Token> outToken(peek_token(n));
    if (!outToken) {
        return false;
    }
    std::string name_string(name);
    return tokenType == outToken->getType() && name_string == outToken->getString();
}

void ProgramParser::throw_exception(const std::string& msg) const {
    std::ostringstream oss;
    oss << msg;
    oss << " at line: " <<  this->m_lexer->m_line;
    throw oss.str();
}

std::shared_ptr<ASTLeaf> ProgramParser::get_token_as_leaf(const char* name)
{
    if (!is_token(name)) {
        throw_exception(std::string("except ") + name);
        return 0;
    } else {
        return read_token_as_leaf();
    }
}

std::shared_ptr<ASTLeaf> ProgramParser::get_token_as_leaf(TokenType tokenType)
{
    if (!is_token(tokenType)) {
        std::ostringstream oss;
        oss << "except " << tokenType;
        throw_exception(oss.str());
        return 0;
    } else {
        return read_token_as_leaf();
    }
}
