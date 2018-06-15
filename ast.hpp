//
//  ast.hpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/21.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#ifndef ast_hpp
#define ast_hpp

#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include "lexer_parser.hpp"


using std::string;
using std::ostringstream;
using std::shared_ptr;
using std::vector;
using std::pair;

class Token;

enum ASTTreeType {
    TYPE_ASTTree = 0,
    TYPE_BinaryExpr = 1,
    TYPE_MINUS_EXPR = 2,
    TYPE_LOGIC_EXPR = 3,
    TYPE_ASSIGN_EXPR = 4,
    TYPE_CALL_FUNC_EXPR = 10,
    TYPE_DOT_CALL_FUNC_EXPR = 11,
    TYPE_DOT_PROPERTY_EXPR = 12,
    TYPE_PARENTHESES_EXPR = 13,
    TYPE_PARENTHESES_ASSIGNER_EXPR = 14,
    TYPE_CREATE_LIST_EXPR = 15,
    TYPE_CREATE_MAP_EXPR = 16,
    TYPE_CREATE_INT_ITERATOR_EXPR = 17,
    TYPE_ASTLeaf = 100,
    TYPE_NumberLiteral = 101,
    TYPE_IdentifierLiteral = 102,
    TYPE_SpecialValueLiteral = 103,
    TYPE_StringLiteral = 104,
    TYPE_STMT = 200,
    TYPE_PRINT_STMT = 201,
    TYPE_IF_STMT = 203,
    TYPE_WHILE_STMT = 204,
    TYPE_FOR_STMT = 205,
    TYPE_BREAK_STMT = 210,
    TYPE_RETURN_STMT = 212,
    TYPE_IMPORT_STMT = 214,
    TYPE_FUNC_DEF = 220,
    TYPE_PROGRAM = 300,
    TYPE_NATIVE_FUNC_DEF = 1000,
};

class ASTTree {
private:
    std::vector<std::shared_ptr<ASTTree>>* m_childrenNodes;
    ASTTree(const ASTTree&);
    ASTTree& operator=(const ASTTree&);
protected:
    void add_child(std::shared_ptr<ASTTree> child) {
        if (!m_childrenNodes) {
            m_childrenNodes = new std::vector<std::shared_ptr<ASTTree>>;
        }
        child->m_parent = this;
        m_childrenNodes->push_back(child);
    }
    ASTTreeType m_type;
    ASTTree *m_parent;
    ASTTree(ASTTreeType type) :m_childrenNodes(NULL), m_type(type), m_parent(0) {}
public:
    ASTTreeType getType() const {
        return m_type;
    }
    ASTTree * getParent() const {
        return m_parent;
    }
    unsigned long get_child_length() const {
        if (!m_childrenNodes) {
            return 0;
        }
        return m_childrenNodes->size();
    }
    std::shared_ptr<ASTTree> get_child(int index) const {
        if (!m_childrenNodes) {
            return NULL;
        }
        return m_childrenNodes->at(index);
    }
    bool has_child() const {
        return m_childrenNodes->size() > 0;
    }
    bool has_parent(ASTTreeType treeType, ASTTreeType stopType) const {
        ASTTree *parent = m_parent;
        while (true) {
            if (!parent) {
                return false;
            }
            if (parent->m_type == stopType) {
                return false;
            }
            if (parent->m_type == treeType) {
                return true;
            }
            parent = parent->m_parent;
        }
        return false;
    }
 
    virtual ~ASTTree() {
        if (m_childrenNodes) {
            delete m_childrenNodes;
            m_childrenNodes = NULL;
        }
    }
};

class ASTLeaf: public ASTTree {
public:
    ASTLeaf(std::shared_ptr<Token> token, ASTTreeType type) : ASTTree(type), m_token(token) {}
protected:
    std::shared_ptr<Token> m_token;
public:
    const char* get_value() const {
        return m_token->getString();
    }
    std::shared_ptr<Token> get_token() const {
        return m_token;
    }
};


class BinaryExpr: public ASTTree {
public:
    BinaryExpr(std::shared_ptr<ASTTree> left, std::shared_ptr<ASTLeaf> op, std::shared_ptr<ASTTree> right)
    : ASTTree(TYPE_BinaryExpr)
    {
        add_child(left);
        add_child(op);
        add_child(right);
    }    
};

class LogicBinaryExpr: public ASTTree {
public:
    LogicBinaryExpr(std::shared_ptr<ASTTree> left, std::shared_ptr<ASTLeaf> op, std::shared_ptr<ASTTree> right)
    : ASTTree(TYPE_LOGIC_EXPR)
    {
        add_child(left);
        add_child(op);
        add_child(right);
    }
};

class MinusExpr: public ASTTree {
public:
    MinusExpr(std::shared_ptr<ASTTree> expr)
    : ASTTree(TYPE_MINUS_EXPR)
    {
        add_child(expr);
    }
};


class NumberLiteral: public ASTLeaf {
public:
    NumberLiteral(std::shared_ptr<Token> token):ASTLeaf(token, TYPE_NumberLiteral){}
    bool is_float() const {
        const char* value = this->get_value();
        while (*value != '\0') {
            if (*value == '.') {
                return true;
            }
            value++;
        }
        return false;
    }
};

class IdentifierLiteral: public ASTLeaf {
public:
    IdentifierLiteral(std::shared_ptr<Token> token):ASTLeaf(token, TYPE_IdentifierLiteral){}
};

class StringLiteral: public ASTLeaf {
public:
    StringLiteral(std::shared_ptr<Token> token):ASTLeaf(token, TYPE_StringLiteral){}
};

class ASTSpecialValueLiteral: public ASTLeaf {
public:
    ASTSpecialValueLiteral(std::shared_ptr<Token> token):ASTLeaf(token, TYPE_SpecialValueLiteral){}
};

class ASTStmt: public ASTTree {
protected:
    ASTStmt(ASTTreeType type) : ASTTree(type) {}
};

class ASTPrintStmt : public ASTStmt {
public:
    ASTPrintStmt(std::shared_ptr<ASTTree> expr) : ASTStmt(TYPE_PRINT_STMT) {
        add_child(expr);
    }
};

class ASTAssignExpr : public ASTTree {
public:
    ASTAssignExpr(std::shared_ptr<IdentifierLiteral> identifer, std::shared_ptr<ASTTree> expr)
    : ASTTree(TYPE_ASSIGN_EXPR) {
        add_child(identifer);
        if (expr && expr.get()) {
            add_child(expr);
            m_has_right = true;
        } else {
            m_has_right = false;
        }
    }
    bool has_right() const {
        return m_has_right;
    }
private:
    bool m_has_right;
};

class ASTProgram: public ASTTree {
public:
    ASTProgram() : ASTTree(TYPE_PROGRAM) {}
    void add_stmt(std::shared_ptr<ASTTree> stmt) {
        add_child(stmt);
    }
};

class ElseIfNode{
public:
    std::shared_ptr<ASTTree> expr;
    std::shared_ptr<ASTProgram> program;
};

class ASTIfStmt: public ASTStmt {
public:
    ASTIfStmt(std::shared_ptr<ASTTree> expr, std::shared_ptr<ASTProgram> program) :
    ASTStmt(TYPE_IF_STMT), m_has_else(false) {
        add_child(expr);
        add_child(program);
    }
    void set_else(std::shared_ptr<ASTProgram> else_program) {
        add_child(else_program);
        m_has_else = true;
    }
    void add_elseif(ElseIfNode ast_elseif) {
        m_ast_elseif_list.push_back(ast_elseif);
    }
    bool has_elseif() const {
        return m_ast_elseif_list.size() > 0;
    }
    bool has_else() const {
        return m_has_else;
    }
    const std::vector<ElseIfNode>& get_elseif_list() const {
        return m_ast_elseif_list;
    }
private:
    bool m_has_else;
    std::vector<ElseIfNode> m_ast_elseif_list;
};

class ASTWhileStmt: public ASTStmt {
public:
    ASTWhileStmt(std::shared_ptr<ASTTree> expr, std::shared_ptr<ASTProgram> program) :
    ASTStmt(TYPE_WHILE_STMT) {
        add_child(expr);
        add_child(program);
    }
};

class ASTForStmt: public ASTStmt {
public:
    ASTForStmt(std::shared_ptr<Token> var_name, std::shared_ptr<ASTTree> iter, std::shared_ptr<ASTProgram> program) :
    ASTStmt(TYPE_FOR_STMT), m_var_name(var_name) {
        add_child(iter);
        add_child(program);
    }
    const char* get_var_name() const {
        return m_var_name->getString();
    }
private:
    std::shared_ptr<Token> m_var_name;
};

class ASTBreakStmt: public ASTStmt {
public:
    ASTBreakStmt() :
    ASTStmt(TYPE_BREAK_STMT) {}
};

class ASTReturnStmt: public ASTStmt {
public:
    ASTReturnStmt(std::shared_ptr<ASTTree> expres) : ASTStmt(TYPE_RETURN_STMT) {
        if (expres && expres.get()) {
             add_child(expres);
        }
    }
};

class ASTImportStmt: public ASTStmt {
public:
    ASTImportStmt(std::shared_ptr<Token> name) : ASTStmt(TYPE_IMPORT_STMT), m_name(name) {}
    const char* get_name() const {
        return m_name->getString();
    }
private:
    std::shared_ptr<Token> m_name;
};

class ASTFuncDefStmt: public ASTStmt {
public:
    ASTFuncDefStmt(std::shared_ptr<Token> name, const std::vector<std::string>& arg_names, std::shared_ptr<ASTProgram> program)
    : ASTStmt(TYPE_FUNC_DEF), m_name(name),m_arg_names(arg_names)
    {
        add_child(program);
    }
    const char* get_name() const {
        return m_name->getString();
    }
    const std::vector<std::string>& get_arg_names() {
        return m_arg_names;
    }
    std::shared_ptr<ASTProgram> get_body() {
        return std::dynamic_pointer_cast<ASTProgram>(get_child(0));
    }
private:
    std::shared_ptr<Token> m_name;
    std::vector<std::string> m_arg_names;
};

class ASTCallFuncExpr: public ASTTree {
public:
    ASTCallFuncExpr(std::shared_ptr<ASTTree> func, const std::vector<std::shared_ptr<ASTTree>>& arg_expr_list) :
    ASTTree(TYPE_CALL_FUNC_EXPR), m_func(func), m_arg_expr_list(arg_expr_list) {
    }
    std::shared_ptr<ASTTree> get_func() const {
        return m_func;
    }
    const std::vector<std::shared_ptr<ASTTree>>& get_arg_expr_list() {
        return m_arg_expr_list;
    }
private:
    std::shared_ptr<ASTTree> m_func;
    std::vector<std::shared_ptr<ASTTree>> m_arg_expr_list;
};

class ASTDotCallFuncExpr: public ASTTree {
public:
    ASTDotCallFuncExpr(std::shared_ptr<ASTTree> owner_express, std::shared_ptr<Token> func_name, const std::vector<std::shared_ptr<ASTTree>>& arg_expr_list) :
    ASTTree(TYPE_DOT_CALL_FUNC_EXPR), m_owner_express(owner_express), m_func_name(func_name), m_arg_expr_list(arg_expr_list) {
    }
    const char* get_func_name() const {
        return m_func_name->getString();
    }
    const std::vector<std::shared_ptr<ASTTree>>& get_arg_expr_list() const {
        return m_arg_expr_list;
    }
    std::shared_ptr<ASTTree> get_owner() const {
        return m_owner_express;
    }
private:
    std::shared_ptr<ASTTree> m_owner_express;
    std::shared_ptr<Token> m_func_name;
    std::vector<std::shared_ptr<ASTTree>> m_arg_expr_list;
};

class ASTDotPropertyExpr: public ASTTree {
public:
    ASTDotPropertyExpr(std::shared_ptr<ASTTree> owner_express, std::shared_ptr<Token> property_name) :
    ASTTree(TYPE_DOT_PROPERTY_EXPR), m_owner_express(owner_express), m_property_name(property_name) {}
    const char* get_property_name() const {
        return m_property_name->getString();
    }
    std::shared_ptr<ASTTree> get_owner() const {
        return m_owner_express;
    }
private:
    std::shared_ptr<ASTTree> m_owner_express;
    std::shared_ptr<Token> m_property_name;
};

class ASTParenthesesExpr: public ASTTree {
public:
    ASTParenthesesExpr(std::shared_ptr<ASTTree> owner, std::shared_ptr<ASTTree> a, std::shared_ptr<ASTTree> b, bool slice) :
    ASTTree(TYPE_PARENTHESES_EXPR), m_owner(owner), m_a(a), m_b(b), m_slice(slice) {}
    std::shared_ptr<ASTTree> get_owner() const {
        return m_owner;
    }
    std::shared_ptr<ASTTree> get_a() const {
        return m_a;
    }
    std::shared_ptr<ASTTree> get_b() const {
        return m_b;
    }
    bool is_slice() const {
        return m_slice;
    }
private:
    std::shared_ptr<ASTTree> m_owner;
    std::shared_ptr<ASTTree> m_a;
    std::shared_ptr<ASTTree> m_b;
    bool m_slice;
};

class ASTParenthesesAssignerExpr: public ASTTree {
public:
    ASTParenthesesAssignerExpr(std::shared_ptr<ASTTree> owner, std::shared_ptr<ASTTree> index, std::shared_ptr<ASTTree> value) :
    ASTTree(TYPE_PARENTHESES_ASSIGNER_EXPR) {
        add_child(owner);
        add_child(index);
        add_child(value);
    }
};

class ASTCreateListExpr: public ASTTree {
public:
    ASTCreateListExpr(const std::vector<std::shared_ptr<ASTTree>>& init_express) : ASTTree(TYPE_CREATE_LIST_EXPR), m_init_express(init_express) {}
    const std::vector<std::shared_ptr<ASTTree>>& get_init_express() const {
        return m_init_express;
    }
private:
    std::vector<std::shared_ptr<ASTTree>> m_init_express;
};


class ASTCreateMapExpr: public ASTTree {
public:
    ASTCreateMapExpr(const std::vector<std::pair<std::shared_ptr<ASTTree>, std::shared_ptr<ASTTree>>>& init_express) : ASTTree(TYPE_CREATE_MAP_EXPR), m_init_express(init_express) {}
    const std::vector<std::pair<std::shared_ptr<ASTTree>, std::shared_ptr<ASTTree>>>& get_init_express() const {
        return m_init_express;
    }
private:
    std::vector<std::pair<std::shared_ptr<ASTTree>, std::shared_ptr<ASTTree>>> m_init_express;
};

class ASTCreateIntIteratorExpr: public ASTTree {
public:
    ASTCreateIntIteratorExpr(std::shared_ptr<ASTTree> from_expr,
                             std::shared_ptr<ASTTree> to_expr,
                             std::shared_ptr<ASTTree> step_expr) :
    ASTTree(TYPE_CREATE_INT_ITERATOR_EXPR){
        add_child(from_expr);
        add_child(to_expr);
        if (step_expr.get()) {
            add_child(step_expr);
        }
    }
};




#endif /* ast_hpp */
