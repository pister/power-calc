//
//  ast_runtime.cpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/26.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#include "ast_runtime.hpp"
#include "lexer_parser.hpp"
#include "xobject.hpp"
#include "meta_types.hpp"
#include <iostream>
#include <sstream>

using std::istringstream;
using std::ostringstream;

using std::cout;
using std::endl;

static RtObject eval_ast_impl(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext);

static inline RtObject handle_TYPE_CREATE_INT_ITERATOR_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_int_iterator_expr = std::dynamic_pointer_cast<ASTCreateIntIteratorExpr>(ast);
    auto ast_from = ast_int_iterator_expr->get_child(0);
    auto ast_to = ast_int_iterator_expr->get_child(1);
    auto from = eval_ast_impl(ast_from, globalRuntimeContext, currentRuntimeContext);
    auto to = eval_ast_impl(ast_to, globalRuntimeContext, currentRuntimeContext);
    LONG64 step = 0;
    if (ast_int_iterator_expr->get_child_length() > 2) {
        std::shared_ptr<ASTTree> ast_step = ast_int_iterator_expr->get_child(2);
        if (ast_step) {
            auto user_step = eval_ast_impl(ast_step, globalRuntimeContext, currentRuntimeContext);
            LONG64 user_step_val = user_step.getIntValue();
            if (user_step_val != 0) {
                step = user_step_val;
            }
        }
    }
    LONG64 from_val = from.getIntValue();
    LONG64 to_val = to.getIntValue();
    if (step == 0) {
        step = to_val > from_val ? 1 : -1;
    }
    return XIntIteratorFactoryClass::newObject(from_val, to_val, step);
}

static inline RtObject handle_TYPE_PARENTHESES_ASSIGNER_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto parentheses_expr = std::dynamic_pointer_cast<ASTParenthesesAssignerExpr>(ast);
    auto ast_owner = parentheses_expr->get_child(0);
    auto ast_index = parentheses_expr->get_child(1);
    auto ast_value = parentheses_expr->get_child(2);
    RtObject owner = eval_ast_impl(ast_owner, globalRuntimeContext, currentRuntimeContext);
    if (owner.getType() != RtObject::RT_TYPE_OBJECT) {
        std::ostringstream oss;
        oss << "owner is not an object" << std::endl;
        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
        throw oss.str();
    }
    XObject *obj = owner.getObject();
    RtObject index = eval_ast_impl(ast_index, globalRuntimeContext, currentRuntimeContext);
    RtObject value = eval_ast_impl(ast_value, globalRuntimeContext, currentRuntimeContext);
    return obj->invoke(FN_SETITEM, index, value);
}

static inline RtObject handle_TYPE_CREATE_LIST_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto create_list_expr = std::dynamic_pointer_cast<ASTCreateListExpr>(ast);
    auto init_express = create_list_expr->get_init_express();
    std::vector<RtObject> init_objects;
    for (auto it = init_express.begin(); it != init_express.end(); ++it) {
        std::shared_ptr<ASTTree> express = *it;
        init_objects.push_back(eval_ast_impl(express, globalRuntimeContext, currentRuntimeContext));
    }
    return XListClass::newObject(init_objects);
}

static inline RtObject handle_TYPE_PARENTHESES_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto parentheses_expr = std::dynamic_pointer_cast<ASTParenthesesExpr>(ast);
    auto ast_owner = parentheses_expr->get_owner();
    RtObject owner = eval_ast_impl(ast_owner, globalRuntimeContext, currentRuntimeContext);
    if (owner.getType() != RtObject::RT_TYPE_OBJECT) {
        std::ostringstream oss;
        oss << "owner is not an object" << std::endl;
        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
        throw oss.str();
    }
    XObject *obj = owner.getObject();
    if (!parentheses_expr->is_slice()) {
        RtObject index = eval_ast_impl(parentheses_expr->get_a(), globalRuntimeContext, currentRuntimeContext);
        return obj->invoke(FN_GETITEM, index);
    } else {
        RtObject a, b;
        if (!parentheses_expr->get_a()) {
            a = RtObject::Null;
        } else {
            a = eval_ast_impl(parentheses_expr->get_a(), globalRuntimeContext, currentRuntimeContext);
        }
        if (!parentheses_expr->get_b()) {
            b = RtObject::Null;
        } else {
            b = eval_ast_impl(parentheses_expr->get_b(), globalRuntimeContext, currentRuntimeContext);
        }
        return obj->invoke(FN_SLICE, a, b);
    }
}

static inline RtObject handle_TYPE_DOT_CALL_FUNC_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto dot_call_func_expr = std::dynamic_pointer_cast<ASTDotCallFuncExpr>(ast);
    auto onwer_express = dot_call_func_expr->get_owner();
    RtObject left = eval_ast_impl(onwer_express, globalRuntimeContext, currentRuntimeContext);
    if (left.getType() != RtObject::RT_TYPE_OBJECT) {
        std::ostringstream oss;
        oss << "owner is not an object" << std::endl;
        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
        throw oss.str();
    }
    XObject *owner = left.getObject();
    std::string method_name = dot_call_func_expr->get_func_name();
    auto arg_list = dot_call_func_expr->get_arg_expr_list();
    std::vector<RtObject> arg_values;
    for (auto it = arg_list.begin(); it != arg_list.end(); it++) {
        RtObject value = eval_ast_impl(*it, globalRuntimeContext, currentRuntimeContext);
        arg_values.push_back(value);
    }
    if (owner->getClass() == XModuleClass::instance()) {
        XModuleClass* moduleClass = (XModuleClass*)owner->getClass();
        RtObject target_func = moduleClass->get_item(*owner, method_name);
        if (target_func == RtObject::Null) {
            std::ostringstream oss;
            oss << "can not find func or method " << method_name << " in " << left.getStringValue() << std::endl;
            globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
            throw oss.str();
        }
        if (target_func.callable()) {
            XObject* obj = target_func.getObject();
            const XCallableClass* callableClass = dynamic_cast<const XCallableClass*>(obj->getClass());
            GlobalRuntimeContext& moduleGlobalRuntimeContext = XModuleClass::getModuleGlobalRuntimeContext(*owner);
            XfuncCallParam callParam = XfuncCallParam(dot_call_func_expr->get_arg_expr_list(), moduleGlobalRuntimeContext, currentRuntimeContext, eval_ast_impl);
            return callableClass->call(*obj, callParam);
        } else {
            std::ostringstream oss;
            oss << target_func << " is not an func." << std::endl;
            globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
            throw oss.str();
        }
    } else {
        return owner->getClass()->invoke(*owner, method_name, arg_values);
    }
}

static inline RtObject handle_TYPE_RETURN_STMT(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    currentRuntimeContext.make_returned();
    auto ast_return = std::dynamic_pointer_cast<ASTReturnStmt>(ast);
    auto returnExpr = ast_return->get_child(0);
    if (returnExpr) {
        RtObject o = eval_ast_impl(returnExpr, globalRuntimeContext, currentRuntimeContext);
        currentRuntimeContext.set_return_value(o);
    } else {
        currentRuntimeContext.set_return_value(RtObject::Null);
    }
    return RtObject::Null;
}

static RtObject handle_TYPE_CALL_FUNC_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) ;

static inline RtObject eval_func_owner(std::shared_ptr<ASTTree> ast_onwer, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
        switch(ast_onwer->getType()) {
            case TYPE_IdentifierLiteral:
            {
                auto ast_onwer_name = std::dynamic_pointer_cast<IdentifierLiteral>(ast_onwer);
                const char* name = ast_onwer_name->get_value();
                if (globalRuntimeContext.has_func(name)) {
                    return globalRuntimeContext.get_func(name);
                }
                if (currentRuntimeContext.has_variable(name)) {
                    RtObject v = currentRuntimeContext.get_variable(name);
                    if (!v.callable()) {
                        std::ostringstream oss;
                        oss << v.getStringValue() << " is not a func." << std::endl;
                        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
                        throw oss.str();
                    }
                    return v;
                }
                std::ostringstream oss;
                oss << " can not func:" << name << std::endl;
                globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
                throw oss.str();
            }
            case TYPE_CALL_FUNC_EXPR:
            {
                return handle_TYPE_CALL_FUNC_EXPR(ast_onwer, globalRuntimeContext, currentRuntimeContext);
            }
            case TYPE_PARENTHESES_EXPR:
            {
                return handle_TYPE_PARENTHESES_EXPR(ast_onwer, globalRuntimeContext, currentRuntimeContext);
            }
            default:
            {
                std::ostringstream oss;
                oss << " except a func." << std::endl;
                globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
                throw oss.str();
            }
        }
}

static RtObject handle_TYPE_CALL_FUNC_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_call_func = std::dynamic_pointer_cast<ASTCallFuncExpr>(ast);
    RtObject target_func = eval_func_owner(ast_call_func->get_func(), globalRuntimeContext, currentRuntimeContext);
    if (!target_func.callable()) {
        std::ostringstream oss;
        oss << target_func.getStringValue() << " is not a func." << std::endl;
        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
        throw oss.str();
    }
    XObject* obj = target_func.getObject();
    const XCallableClass* callableClass = dynamic_cast<const XCallableClass*>(obj->getClass());
    XfuncCallParam callParam = XfuncCallParam(ast_call_func->get_arg_expr_list(), globalRuntimeContext, currentRuntimeContext, eval_ast_impl);
    return callableClass->call(*obj, callParam);
}

static inline RtObject handle_TYPE_FUNC_DEF(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_func_def = std::dynamic_pointer_cast<ASTFuncDefStmt>(ast);
    const char* name = ast_func_def->get_name();
    globalRuntimeContext.def_func(name, XCallableClass::newObject(name, ast_func_def));
    return RtObject::Null;
}

static inline RtObject handle_TYPE_WHILE_STMT(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_while_stmt = std::dynamic_pointer_cast<ASTWhileStmt>(ast);
    auto expr = ast_while_stmt->get_child(0);
    auto program = std::dynamic_pointer_cast<ASTProgram>(ast_while_stmt->get_child(1));
    RtObject last_value;
    while(true) {
        RtObject expr_result = eval_ast_impl(expr, globalRuntimeContext, currentRuntimeContext);
        if (RtObject::True.rt_not_equals(expr_result)) {
            break;
        }
        last_value = eval_ast_impl(program, globalRuntimeContext, currentRuntimeContext);
        if (currentRuntimeContext.is_returned()) {
            return last_value;
        }
        if (globalRuntimeContext.breaked) {
            break;
        }
    }
    // reset for outer loop!
    globalRuntimeContext.breaked = false;
    return last_value;
}

static inline RtObject handle_TYPE_FOR_STMT(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_for_stmt = std::dynamic_pointer_cast<ASTForStmt>(ast);
    const char* name = ast_for_stmt->get_var_name();
    auto ast_iter = ast_for_stmt->get_child(0);
    auto program = std::dynamic_pointer_cast<ASTProgram>(ast_for_stmt->get_child(1));
    RtObject iter_owner = eval_ast_impl(ast_iter, globalRuntimeContext, currentRuntimeContext);
    if (iter_owner.getType() != RtObject::RT_TYPE_OBJECT) {
        std::ostringstream oss;
        oss << "owner is not an object" << std::endl;
        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
        throw oss.str();
    }
    XObject* obj = iter_owner.getObject();
    RtObject it(obj->invoke(FN_ITER)); // hold the iterable object force not to be released!
    XObject* iterable = it.getObject();
    RtObject last_value;
    while (true) {
        RtObject has_next = iterable->invoke(FN_HASNEXT);
        if (RtObject::True.rt_not_equals(has_next)) {
            break;
        }
        RtObject next_value = iterable->invoke(FN_NEXT);
        currentRuntimeContext.set_variable(name, next_value);
        last_value = eval_ast_impl(program, globalRuntimeContext, currentRuntimeContext);
        if (currentRuntimeContext.is_returned()) {
            return last_value;
        }
        if (globalRuntimeContext.breaked) {
            break;
        }
    }
    // reset for outer loop!
    globalRuntimeContext.breaked = false;
    return last_value;
}

static inline RtObject handle_TYPE_IF_STMT(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_if_stmt = std::dynamic_pointer_cast<ASTIfStmt>(ast);
    auto expr = ast_if_stmt->get_child(0);
    auto program = std::dynamic_pointer_cast<ASTProgram>(ast_if_stmt->get_child(1));
    RtObject value = eval_ast_impl(expr, globalRuntimeContext, currentRuntimeContext);
    if (RtObject::True.rt_equals(value)) {
        return eval_ast_impl(program, globalRuntimeContext, currentRuntimeContext);
    } else {
        const std::vector<ElseIfNode>& elseif_list =  ast_if_stmt->get_elseif_list();
        if (!elseif_list.empty()) {
            for (auto it = elseif_list.begin(); it != elseif_list.end(); ++it) {
                auto elseif_node = *it;
                auto elsif_expr = elseif_node.expr;
                auto elseif_program = elseif_node.program;
                auto v = eval_ast_impl(elsif_expr, globalRuntimeContext, currentRuntimeContext);
                if (RtObject::True.rt_equals(v)) {
                    return eval_ast_impl(elseif_program, globalRuntimeContext, currentRuntimeContext);
                }
            }
        }
        if (ast_if_stmt->has_else()) {
            std::shared_ptr<ASTProgram> else_program = std::dynamic_pointer_cast<ASTProgram>(ast_if_stmt->get_child(2));
            return eval_ast_impl(else_program, globalRuntimeContext, currentRuntimeContext);
        }
    }
    return RtObject::Null;
}

static inline RtObject handle_TYPE_PROGRAM(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_program = std::dynamic_pointer_cast<ASTProgram>(ast);
    RtObject last_value;
    for (int i = 0; i < ast_program->get_child_length(); ++i) {
        if (currentRuntimeContext.is_returned()) {
            return last_value;
        }
        std::shared_ptr<ASTTree> stmt = std::dynamic_pointer_cast<ASTTree>(ast_program->get_child(i));
        last_value = eval_ast_impl(stmt, globalRuntimeContext, currentRuntimeContext);
    }
    return last_value;
}

static inline RtObject handle_TYPE_ASSIGN_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto assignStmt = std::dynamic_pointer_cast<ASTAssignExpr>(ast);
    auto identifier = std::dynamic_pointer_cast<IdentifierLiteral>(assignStmt->get_child(0));
    if (assignStmt->has_right()) {
        std::shared_ptr<ASTTree> expr = assignStmt->get_child(1);
        RtObject value = eval_ast_impl(expr, globalRuntimeContext, currentRuntimeContext);
        currentRuntimeContext.set_variable(identifier->get_value(), value);
        return value;
    } else {
        return eval_ast_impl(identifier, globalRuntimeContext, currentRuntimeContext);
    }
}

static inline RtObject handle_TYPE_NumberLiteral(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext){
    auto leaf = std::dynamic_pointer_cast<NumberLiteral>(ast);
    std::istringstream iss(leaf->get_value());
    if (leaf->is_float()) {
        double v;
        iss >> v;
        return v;
    } else {
        LONG64 v;
        iss >> v;
        return v;
    }
}

static inline RtObject handle_TYPE_SpecialValueLiteral(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto leaf = std::dynamic_pointer_cast<ASTLeaf>(ast);
    const std::string token_value = leaf->get_value();
    if (token_value == SPECIAL_WORD_True) {
        return RtObject::True;
    }
    if (token_value == SPEICAL_WORD_False) {
        return RtObject::False;
    }
    if (token_value == SPECAIL_WORD_Null) {
        return RtObject::Null;
    }
    return RtObject::Null;
}

static inline RtObject handle_TYPE_IdentifierLiteral(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto leaf = std::dynamic_pointer_cast<ASTLeaf>(ast);
    std::string key = leaf->get_value();
    if (currentRuntimeContext.has_variable(key)) {
        return currentRuntimeContext.get_variable(key);
    }
    if (globalRuntimeContext.has_variable(key)) {
        return globalRuntimeContext.get_variable(key);
    }
    RtObject func = globalRuntimeContext.get_func(key);
    if (func) {
        return func;
    }
    std::ostringstream oss;
    oss << "undefined variable " << key << std::endl;
    globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
    throw oss.str();
}

static inline RtObject handle_TYPE_LOGIC_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto logicExpr = std::dynamic_pointer_cast<LogicBinaryExpr>(ast);
    auto left = logicExpr->get_child(0);
    auto right = logicExpr->get_child(2);
    auto logic_op = std::dynamic_pointer_cast<ASTLeaf>(logicExpr->get_child(1));
    std::string value(logic_op->get_value());
    const RtObject left_value = eval_ast_impl(left, globalRuntimeContext, currentRuntimeContext);
    const RtObject rigth_value = eval_ast_impl(right, globalRuntimeContext, currentRuntimeContext);
    if (value == "and") {
        return left_value.rt_logic_and(rigth_value);
    } else if (value == "or") {
        return left_value.rt_logic_or(rigth_value);
    } else {
        std::ostringstream oss;
        oss << "unsupport logic: " << value;
        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
        throw oss.str();
    }
    return RtObject::Null;
}

static inline RtObject handle_Binary_OPERATOR( const char* op_c, const RtObject& left_value, const RtObject& right_value, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    switch (op_c[0]) {
        case '+':
            return left_value.rt_add(right_value);
        case '-':
            return left_value.rt_sub(right_value);
        case '*':
            return left_value.rt_multi(right_value);
        case '/':
            return left_value.rt_div(right_value);
        case '%':
            return left_value.rt_mod(right_value);
        default:
        {
            std::ostringstream oss;
            oss << "unknown operator!" << std::endl;
            globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
            throw oss.str();
        }
    }
}

static inline RtObject handle_Binary_COMPARE( const char* op_c, const RtObject& left_value, const RtObject& right_value, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    std::string op_string(op_c);
    if (op_string == "==") {
        return left_value.rt_equals(right_value);
    } else if (op_string == "!=") {
        return left_value.rt_not_equals(right_value);
    } else if (op_string == "<=") {
        return left_value.rt_less_equals_than(right_value);
    } else if (op_string == ">=") {
        return left_value.rt_greater_equals_than(right_value);
    } else if (op_string == ">") {
        return left_value.rt_greater_than(right_value);
    } else if (op_string == "<") {
        return left_value.rt_less_than(right_value);
    } else {
        std::ostringstream oss;
        oss << "unknown compare!" << std::endl;
        globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
        throw oss.str();
    }
}

static inline RtObject handle_TYPE_BinaryExpr(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto left = ast->get_child(0);
    auto op = ast->get_child(1);
    auto right = ast->get_child(2);
    RtObject left_value = eval_ast_impl(left, globalRuntimeContext, currentRuntimeContext);
    RtObject right_value = eval_ast_impl(right, globalRuntimeContext, currentRuntimeContext);
    auto op_leaf = std::dynamic_pointer_cast<ASTLeaf>(op);
    auto token = op_leaf->get_token();
    const char* op_c = op_leaf->get_value();
    switch (token->getType()) {
        case OPERATOR:
            return handle_Binary_OPERATOR(op_c, left_value, right_value, globalRuntimeContext, currentRuntimeContext);
        case TOKEN_COMPARE:
            return handle_Binary_COMPARE(op_c, left_value, right_value, globalRuntimeContext, currentRuntimeContext);
        default:
            break;
    }
    std::ostringstream oss;
    oss << "unknown operator:" << op_c << std::endl;
    globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
    throw oss.str();
}

static inline RtObject handle_TYPE_MINUS_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto minusExpr = std::dynamic_pointer_cast<MinusExpr>(ast);
    auto expr = minusExpr->get_child(0);
    RtObject value = eval_ast_impl(expr, globalRuntimeContext, currentRuntimeContext);
    return value.rt_multi(-1);
}

static inline RtObject handle_TYPE_StringLiteral(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext){
    auto leaf = std::dynamic_pointer_cast<StringLiteral>(ast);
    const char* value = leaf->get_value();
    return RtObject(XStringClass::newObject(value));
}

static inline RtObject handle_TYPE_CREATE_MAP_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_create_map_expr = std::dynamic_pointer_cast<ASTCreateMapExpr>(ast);
    auto init_expr = ast_create_map_expr->get_init_express();
    std::vector<std::pair<RtObject, RtObject>> init_objects;
    for(const auto& it : init_expr) {
        auto key_ast = it.first;
        auto value_ast = it.second;
        RtObject key = eval_ast_impl(key_ast, globalRuntimeContext, currentRuntimeContext);
        RtObject value = eval_ast_impl(value_ast, globalRuntimeContext, currentRuntimeContext);
        init_objects.push_back(std::make_pair(key, value));
    }
    return XMapClass::newObject(init_objects);
}

static RtObject handle_TYPE_IMPORT_STMT(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_import_stmt = std::dynamic_pointer_cast<ASTImportStmt>(ast);
    const char* name = ast_import_stmt->get_name();
    RtObject imported_modules = XModuleClass::newObject(globalRuntimeContext, name);
    globalRuntimeContext.set_variable(name, imported_modules);
    return RtObject::Null;
}

static RtObject handle_TYPE_DOT_PROPERTY_EXPR(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext) {
    auto ast_dot_expr = std::dynamic_pointer_cast<ASTDotPropertyExpr>(ast);
    RtObject owner = eval_ast_impl(ast_dot_expr->get_owner(), globalRuntimeContext, currentRuntimeContext);
    return owner.rt_dot(XStringClass::newObject(ast_dot_expr->get_property_name()));
}

static RtObject eval_ast_impl(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext)
{
    switch (ast->getType()) {
        case TYPE_IMPORT_STMT:
            return handle_TYPE_IMPORT_STMT(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_CREATE_MAP_EXPR:
            return handle_TYPE_CREATE_MAP_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_CREATE_INT_ITERATOR_EXPR:
            return handle_TYPE_CREATE_INT_ITERATOR_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_PARENTHESES_ASSIGNER_EXPR:
            return handle_TYPE_PARENTHESES_ASSIGNER_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_CREATE_LIST_EXPR:
            return handle_TYPE_CREATE_LIST_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_PARENTHESES_EXPR:
            return handle_TYPE_PARENTHESES_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_DOT_CALL_FUNC_EXPR:
            return handle_TYPE_DOT_CALL_FUNC_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_DOT_PROPERTY_EXPR:
            return handle_TYPE_DOT_PROPERTY_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_RETURN_STMT:
            return handle_TYPE_RETURN_STMT(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_CALL_FUNC_EXPR:
            return handle_TYPE_CALL_FUNC_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_FUNC_DEF:
            return handle_TYPE_FUNC_DEF(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_WHILE_STMT:
            return handle_TYPE_WHILE_STMT(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_FOR_STMT:
            return handle_TYPE_FOR_STMT(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_BREAK_STMT:
        {
            globalRuntimeContext.breaked = true;
            return RtObject::Null;
        }
        case TYPE_IF_STMT:
            return handle_TYPE_IF_STMT(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_PROGRAM:
            return handle_TYPE_PROGRAM(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_ASSIGN_EXPR:
            return handle_TYPE_ASSIGN_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_NumberLiteral:
            return handle_TYPE_NumberLiteral(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_SpecialValueLiteral:
            return handle_TYPE_SpecialValueLiteral(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_IdentifierLiteral:
            return handle_TYPE_IdentifierLiteral(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_StringLiteral:
            return handle_TYPE_StringLiteral(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_LOGIC_EXPR:
            return handle_TYPE_LOGIC_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_MINUS_EXPR:
            return handle_TYPE_MINUS_EXPR(ast, globalRuntimeContext, currentRuntimeContext);
        case TYPE_BinaryExpr:
            return handle_TYPE_BinaryExpr(ast, globalRuntimeContext, currentRuntimeContext);
        default:
            break;
    }
    std::ostringstream oss;
    oss << "unknown ast type!" << std::endl;
    globalRuntimeContext.fill_stack_message(oss, currentRuntimeContext);
    throw oss.str();
}

class PrintNativeCallStmt : public NativeFunDef {
public:
    PrintNativeCallStmt() {
        m_arg_names.push_back("value");
    }
    virtual RtObject call_native(RuntimeContext& runtimeContext) {
        RtObject v = runtimeContext.get_variable("value");
        std::cout << v.getStringValue() << std::endl;
        return RtObject::Null;
    }
    virtual const std::vector<std::string>& get_arg_names() {
        return m_arg_names;
    }
private:
    std::vector<std::string> m_arg_names;
};

static void register_native_functions(GlobalRuntimeContext& globalRuntimeContext)
{
    {
        std::shared_ptr<NativeFunDef> printFunc(new PrintNativeCallStmt);
        globalRuntimeContext.def_func("print", XCallableClass::newObject("print", printFunc));
    }
    // to be add other inner func here ...
}

RtObject eval_ast(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext* input_globalRuntimeContext) {
    register_native_functions(*input_globalRuntimeContext);
    return eval_ast_impl(ast, *input_globalRuntimeContext, *input_globalRuntimeContext);
}
