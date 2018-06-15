//
//  ast_runtime.hpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/26.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#ifndef ast_runtime_hpp
#define ast_runtime_hpp

#include "ProgramParser.hpp"
#include "ast.hpp"
#include <sstream>
#include <string>
#include <map>
#include <list>
#include <string>
#include "ast.hpp"
#include "runtime_types.hpp"

using std::ostringstream;
using std::map;
using std::string;
using std::list;
using std::string;

class RuntimeContext;

class NativeFunDef : public ASTTree {
public:
    NativeFunDef() : ASTTree(TYPE_NATIVE_FUNC_DEF) {}
    virtual RtObject call_native(RuntimeContext& runtimeContext) = 0;
    virtual const std::vector<std::string>& get_arg_names() = 0;
};

class RuntimeContext {
private:
    std::map<std::string, RtObject>* m_variables;
    RuntimeContext(const RuntimeContext&);
    RuntimeContext& operator=(const RuntimeContext&);
public:
    RtObject get_variable(const std::string& name) const {
        return (*m_variables)[name];
    }
    void set_variable(const std::string& name, RtObject value) {
        (*m_variables)[name] = value;
    }
    bool has_variable(const std::string& name) const {
        auto it = m_variables->find(name);
        return (it != m_variables->end());
    }
    const std::map<std::string, RtObject>& get_variables() const {
        return *m_variables;
    }
    virtual void set_return_value(RtObject v) {
        throw std::string("return only be allowd in func!");
    }
    virtual RtObject get_return_value() const  {
        throw std::string("return only be allowd in func!");
    }
    virtual void make_returned() {
        throw std::string("return only be allowd in func!");
    }
    virtual bool is_returned() const {
        return false;
    }
    virtual const std::string get_call_name() const {
        return std::string("<main>");
    }
    bool breaked;
    bool continued;
    RuntimeContext() : m_variables(new std::map<std::string, RtObject>), breaked(false), continued(false) {}
    ~RuntimeContext() {
        if (m_variables) {
            delete m_variables;
            m_variables = NULL;
        }
    }
};

class XBootContext;

class GlobalRuntimeContext: public RuntimeContext {
private:
    std::map<std::string, RtObject> m_importedModules;
    std::list<RuntimeContext*> m_runtimeContextStack;
    XBootContext* m_bootContext;
public:
    XBootContext* getBootContext() {
        return m_bootContext;
    }
    GlobalRuntimeContext(XBootContext* bootContext) :m_bootContext(bootContext) {}
    unsigned long get_current_stack_size() const {
        return m_runtimeContextStack.size();
    }
    
    void def_func(const std::string& name, RtObject func) {
        set_variable(name, func);
    }
    void def_module(const std::string& name, RtObject module) {
        m_importedModules[name] = module;
    }
    
    RtObject get_module(const std::string& name) const {
        auto it = m_importedModules.find(name);
        if (it == m_importedModules.end()) {
            return RtObject::Null;
        }
        return it->second;
    }
    
    bool has_func(const std::string& name) const {
        return has_variable(name);
    }
    RtObject get_func(const std::string& name) const {
        return get_variable(name);
    }
    
    void fill_stack_message(std::ostream& os, RuntimeContext& currentRuntimeContext) {
        const std::list<RuntimeContext*>& the_stack = get_stack();
        os << "call stack: " << std::endl;
        os << currentRuntimeContext.get_call_name() << std::endl;
        for (std::list<RuntimeContext*>::const_reverse_iterator it = the_stack.rbegin(); it != the_stack.rend(); it++) {
            RuntimeContext* context = *it;
            os << context->get_call_name() << std::endl;
        }
    }
    void push_runtime(RuntimeContext* rt) {
        m_runtimeContextStack.push_back(rt);
    }
    RuntimeContext* pop_runtime() {
        if (m_runtimeContextStack.empty()) {
            return NULL;
        }
        RuntimeContext* ret = m_runtimeContextStack.back();
        m_runtimeContextStack.pop_back();
        return ret;
    }
    const std::list<RuntimeContext*>& get_stack() const {
        return m_runtimeContextStack;
    }
};

class LocalRuntimeContext: public RuntimeContext {
public:
    LocalRuntimeContext(const std::string& call_name): m_call_name(call_name), m_has_returned(false) {
    }
    virtual void make_returned() {
        m_has_returned = true;
    }
    virtual bool is_returned() const {
        return m_has_returned;
    }
    virtual void set_return_value(RtObject v) {
        m_return_value = v;
    }
    virtual RtObject get_return_value() const {
        return m_return_value;
    }
    virtual const std::string get_call_name() const {
        return m_call_name;
    }
private:
    std::string m_call_name;
    bool m_has_returned;
    RtObject m_return_value;
};

RtObject eval_ast(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext* globalRuntimeContext);


#endif /* ast_runtime_hpp */
