//
//  ExpressParser.hpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/22.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#ifndef ExpressParser_hpp
#define ExpressParser_hpp
#include <memory>
#include <vector>
#include "lexer_parser.hpp"

class Lexer;
class ASTTree;
class ASTLeaf;
class ASTStmt;
class ASTProgram;

using std::shared_ptr;
using std::vector;

const std::string SPECIAL_WORD_True = "True";
const std::string SPEICAL_WORD_False = "False";
const std::string SPECAIL_WORD_Null = "Null";

class ProgramParser {
public:
    ProgramParser(Lexer *lexer):m_lexer(lexer){}
    std::shared_ptr<ASTTree> express();
    std::shared_ptr<ASTProgram> root_program();
private:
    void check(std::shared_ptr<ASTTree> ast_tree) const;
    std::shared_ptr<ASTTree> stmt(bool& end);
    std::shared_ptr<ASTProgram> program();
    std::shared_ptr<ASTTree> parentheses_expr(std::shared_ptr<ASTTree> left);
    std::shared_ptr<ASTTree> assign_expr();
    std::shared_ptr<ASTTree> compare_expr();
    std::shared_ptr<ASTTree> add_or_sub();
    std::shared_ptr<ASTTree> term();
    std::shared_ptr<ASTTree> dot_oper();
    std::shared_ptr<ASTTree> factor();
    std::vector<std::shared_ptr<ASTTree>> call_args();
private:
    bool is_token(const char* name, int n = 1);
    bool is_token(TokenType tokenType, int n = 1);
    bool is_token(TokenType tokenType, const char* name, int n = 1);
    std::shared_ptr<Token> peek_token(int n = 1);
    std::shared_ptr<ASTLeaf> read_token_as_leaf();
    std::shared_ptr<ASTLeaf> get_token_as_leaf(const char* name);
    std::shared_ptr<ASTLeaf> get_token_as_leaf(TokenType tokenType);
    bool end_of_stmt();
    
    void throw_exception(const std::string& msg) const;
    
private:
    Lexer *m_lexer;
};

#endif /* ExpressParser_hpp */
