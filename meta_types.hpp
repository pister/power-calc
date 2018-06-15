//
//  meta_types.hpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/29.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#ifndef meta_types_hpp
#define meta_types_hpp
#include "runtime_types.hpp"
#include "xobject.hpp"

#include <utility>
#include <map>
#include <vector>

using std::vector;
using std::map;
using std::pair;


class GlobalRuntimeContext;
class RuntimeContext;


class XClass {
protected:
    XClass() {}
public:
    typedef RtObject (*INVOKE_FN)(XObject& instance, const std::vector<RtObject>& args);
    virtual RtObject invoke(XObject& instance, const std::string& method_name, const std::vector<RtObject>& args) const {
        std::map<std::string, INVOKE_FN>::const_iterator it = m_methods.find(method_name);
        if (it == m_methods.end()) {
            throw std::string("can not find method:") + method_name;
        }
        INVOKE_FN fn = it->second;
        return fn(instance, args);
    }
    virtual RtObject get(const XObject& instance, const std::string& property_name) const {
        return RtObject::Null;
    }
    virtual void set(XObject& instance, const std::string& property_name, const RtObject& value) const {}
    virtual void on_destroying(XObject& instance) const = 0;
    virtual bool callable(const XObject& instance) const {
        return false;
    }
    virtual const std::string str(XObject& instance) const {
        std::map<std::string, INVOKE_FN>::const_iterator it = m_methods.find(FN_STR);
        if (it == m_methods.end()) {
            return "<Object>";
        }
        INVOKE_FN fn = it->second;
        std::vector<RtObject> args;
        RtObject obj = fn(instance, args);
        return obj.getStringValue();
    }

protected:
    void register_method(const std::string& method_name, INVOKE_FN fn) {
        m_methods[method_name] = fn;
    }
private:
    XClass(const XClass&);
    XClass& operator=(const XClass&);
    std::map<std::string, INVOKE_FN> m_methods;
};


class XStringClass: public XClass {
public:
    static RtObject newObject(const char* str);
    static RtObject newObject(const std::string& str);
    virtual void on_destroying(XObject& instance) const;
    virtual const std::string str(XObject& instance) const;
    static XStringClass* instance() {
        static XStringClass instance;
        return &instance;
    }
private:
    static RtObject __str__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __add__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __radd__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __multi__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __eq__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __lt__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __len__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __getitem__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __slice__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __hashcode__(XObject& instance, const std::vector<RtObject>& args);
private:
    XStringClass() {
        register_method(FN_STR, __str__);
        register_method(FN_ADD, __add__);
        register_method(FN_RADD, __radd__);
        register_method(FN_MULTI, __multi__);
        register_method(FN_EQ, __eq__);
        register_method(FN_LT, __lt__);
        register_method(FN_LEN, __len__);
        register_method("length", __len__);
        register_method(FN_GETITEM, __getitem__);
        register_method(FN_SLICE, __slice__);
        register_method(FN_HASHCODE, __hashcode__);

    }
    bool equals(const RtObject& others) const;
};


class XfuncCallParam {
public:
    typedef RtObject (*EVAL_AST_FN)(std::shared_ptr<ASTTree> ast, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& currentRuntimeContext);
    XfuncCallParam(const std::vector<std::shared_ptr<ASTTree>>& arg_expr_list,
                   GlobalRuntimeContext& globalRuntimeContext,
                   RuntimeContext& currentRuntimeContext,
                   EVAL_AST_FN eval_ast_call):
    m_arg_expr_list(arg_expr_list),
    m_globalRuntimeContext(globalRuntimeContext),
    m_currentRuntimeContext(currentRuntimeContext),
    m_eval_ast_call(eval_ast_call) {}
public:
    std::vector<std::shared_ptr<ASTTree>> m_arg_expr_list;
    GlobalRuntimeContext& m_globalRuntimeContext;
    RuntimeContext& m_currentRuntimeContext;
    EVAL_AST_FN m_eval_ast_call;
    //std::shared_ptr<ASTCallFuncExpr> m_ast_call_func;
};

class XFuncDef: private NoncopyAble {
public:
    RtObject call(const XfuncCallParam& param);
    virtual void invoke(XfuncCallParam::EVAL_AST_FN eval_ast_call, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& localRuntimeContext) const = 0;
    virtual const std::vector<std::string>& get_arg_names() const  {
        throw std::string("XFuncDef error!!" );
    }
    virtual ~XFuncDef() {}
protected:
    XFuncDef(const std::string& name, const std::string& type) : m_name(name), m_type(type) {}
    std::string m_name;
    std::string m_type;
};

class NativeFunDef;

class XCallableClass: public XClass {
private:
    static RtObject newObject(XFuncDef* xFuncDef);
public:
    static RtObject newObject(const std::string& name, std::shared_ptr<NativeFunDef> func);
    static RtObject newObject(const std::string& name, std::shared_ptr<ASTFuncDefStmt> func);
    virtual bool callable(const XObject& instance) const {
        return true;
    }
    virtual RtObject call(XObject &instance, const XfuncCallParam& param) const;
    virtual void on_destroying(XObject& instance) const;
    virtual const std::string str(XObject& instance) const {
        return "<func>";
    }
    static XCallableClass* instance() {
        static XCallableClass instance;
        return &instance;
    }
private:
    XCallableClass() {}
};


class XListClass: public XClass {
public:
    static RtObject newObject();
    static RtObject newObject(const std::vector<RtObject>& init_objects);
    virtual void on_destroying(XObject& instance) const;
    static XListClass* instance() {
        static XListClass instance;
        return &instance;
    }
private:
    static RtObject __str__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __add__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __multi__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __eq__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __len__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __getitem__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __setitem__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __slice__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __iter__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject join(XObject& instance, const std::vector<RtObject>& args);
    static RtObject append(XObject& instance, const std::vector<RtObject>& args);
private:
    XListClass() {
        register_method(FN_STR, __str__);
        register_method(FN_ADD, __add__);
        register_method(FN_MULTI, __multi__);
        register_method(FN_EQ, __eq__);
        register_method(FN_LEN, __len__);
        register_method(FN_GETITEM, __getitem__);
        register_method(FN_SETITEM, __setitem__);
        register_method(FN_SLICE, __slice__);
        register_method(FN_ITER, __iter__);
        register_method("length", __len__);
        register_method("extends", __add__);
        register_method("join", join);
        register_method("append", append);
    }
};

class BaseIterator {
public:
    virtual bool has_next() const = 0;
    virtual RtObject next() = 0;
    virtual ~BaseIterator() {}
};

class XIteratorClass: public XClass {
public:
    static RtObject newObject(BaseIterator* iterator);
    virtual void on_destroying(XObject& instance) const;
    static XIteratorClass* instance() {
        static XIteratorClass instance;
        return &instance;
    }
private:
    static RtObject __hasnext__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __next__(XObject& instance, const std::vector<RtObject>& args);
private:
    XIteratorClass() {
        register_method(FN_HASNEXT, __hasnext__);
        register_method(FN_NEXT, __next__);
    }
};

class XIntIteratorFactoryClass: public XClass {
public:
    static RtObject newObject(LONG64 from, LONG64 to, LONG64);
    virtual void on_destroying(XObject& instance) const;
    static XIntIteratorFactoryClass* instance() {
        static XIntIteratorFactoryClass instance;
        return &instance;
    }
    virtual const std::string str(XObject& instance) const {
        return std::string("<intIterator>");
    }
private:
    static RtObject __iter__(XObject& instance, const std::vector<RtObject>& args);
private:
    XIntIteratorFactoryClass() {
        register_method(FN_ITER, __iter__);
    }
};


class XMapClass: public XClass {
public:
    static RtObject newObject();
    static RtObject newObject(const std::vector<std::pair<RtObject, RtObject>>& init_objects);
    virtual void on_destroying(XObject& instance) const;
    static XMapClass* instance() {
        static XMapClass instance;
        return &instance;
    }
private:
    static RtObject __str__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __eq__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __len__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __getitem__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __setitem__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __iter__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __add__(XObject& instance, const std::vector<RtObject>& args);
private:
    XMapClass() {
        register_method(FN_STR, __str__);
        register_method(FN_EQ, __eq__);
        register_method(FN_LEN, __len__);
        register_method(FN_GETITEM, __getitem__);
        register_method(FN_SETITEM, __setitem__);
        register_method(FN_ITER, __iter__);
        register_method(FN_ADD, __add__);
        register_method("length", __len__);
    }
};

class XModuleDef {
protected:
    XModuleDef(const std::string& name) : m_name(name) {}
    bool is_export(const std::string& name) const {
        if (name.size() == 0) {
            return false;
        }
        if (name[0] >= 'A' && name[0] <= 'Z') {
            return true;
        }
        return false;
    }
public:
    virtual void get_export_names(std::vector<std::string>& out) const = 0;
    virtual RtObject get_export(const std::string& name) const = 0;
    virtual GlobalRuntimeContext& getGlobalRuntimeContext() const = 0;
    virtual ~XModuleDef() {}
public:
    std::string m_name;
};



typedef RtObject (*X_NATIVE_METHOD)(RuntimeContext& runtimeContext);

class NativeMethodInfo {
public:
    X_NATIVE_METHOD method;
    std::vector<std::string> arg_names;
};

class XBootContext;

class XMapModuleDef: public XModuleDef {
public:
    XMapModuleDef(GlobalRuntimeContext& globalRuntimeContext, const std::string& name, const std::map<std::string, NativeMethodInfo>& native_methods);
    void add_native_method(const std::string& name, const NativeMethodInfo& native_method);
    virtual void get_export_names(std::vector<std::string>& out) const;
    virtual RtObject get_export(const std::string& name) const;
    virtual GlobalRuntimeContext& getGlobalRuntimeContext() const;
    virtual ~XMapModuleDef() {}
private:
    std::map<std::string, RtObject> m_properties;
    GlobalRuntimeContext& m_globalRuntimeContext;
};

class XModuleClass: public XClass {
public:
    static RtObject newObject(GlobalRuntimeContext& globalRuntimeContext, XModuleDef *moduleDef);
    static RtObject newObject(GlobalRuntimeContext& globalRuntimeContext, const std::string& name);
    static RtObject newObject(GlobalRuntimeContext& globalRuntimeContext, const std::string& name, const std::map<std::string, NativeMethodInfo>& native_methods);
    virtual void on_destroying(XObject& instance) const;
    static GlobalRuntimeContext& getModuleGlobalRuntimeContext(XObject& instance);
    RtObject get_item(XObject& instance, const std::string& name) const;
    static XModuleClass* instance() {
        static XModuleClass instance;
        return &instance;
    }
private:
    static RtObject __str__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __iter__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __getitem__(XObject& instance, const std::vector<RtObject>& args);
private:
    XModuleClass() {
        register_method(FN_STR, __str__);
        register_method(FN_ITER, __iter__);
        register_method(FN_GETITEM, __getitem__);
        register_method(FN_DOT, __getitem__);
    }
};



#endif /* meta_types_hpp */
