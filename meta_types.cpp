//
//  meta_types.cpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/29.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#include "meta_types.hpp"
#include "xobject.hpp"
#include "ast.hpp"
#include "runtime_types.hpp"
#include "ast_runtime.hpp"
#include "xboot.hpp"
#include "xbase.cpp"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>


#include "spp/spp.h"
using spp::sparse_hash_map;
using std::vector;
using std::ostringstream;

#define MAX_CALL_STACK_SIZE 100000

// ==========================
static LONG64 X_get_pos(const RtObject& input_pos, size_t data_size, LONG64 default_value) {
    if (!input_pos) {
        return default_value;
    }
    LONG64 pos = input_pos.getIntValue();
    if (pos < 0) {
        return data_size + pos;
    }
    return pos;
}

/// ============ XStringClass begin ============
RtObject XStringClass::newObject(const std::string& str) {
    return newObject(str.c_str());
}

RtObject XStringClass::newObject(const char* str) {
    size_t len = strlen(str) + 1;
    char *pt = new char[len];
    char *buf = pt;
    memset(buf, 0, len);
    while (*str != '\0') {
        if (*str == '\\' && *(str+1) != '\0') {
            str++;
            switch (*str) {
                case 'n':
                    *buf = '\n';
                    buf++;
                    break;
                case 't':
                    *buf = '\t';
                    buf++;
                    break;
                case 'r':
                    *buf = '\r';
                    buf++;
                    break;
                case 'b':
                    *buf = '\b';
                    buf++;
                    break;
                case '\\':
                    *buf = '\\';
                    buf++;
                    break;
                case '"':
                    *buf = '\"';
                    buf++;
                    break;
                default:
                    break;
            }
        } else {
            *buf = *str;
            buf++;
        }
        str++;
        
    }
    XObject* obj = new XObject(instance(), new std::string(pt));
    delete[] pt;
    return obj;
}

void XStringClass::on_destroying(XObject& instance) const {
    std::string* data = (std::string*)instance.m_data;
    delete data;
}

const std::string XStringClass::str(XObject& instance) const {
    std::string* data = (std::string*)instance.m_data;
    return *data;
}

RtObject XStringClass::__str__(XObject& instance, const std::vector<RtObject>& args) {
    return RtObject(&instance);
}

RtObject XStringClass::__add__(XObject& instance, const std::vector<RtObject>& args) {
    std::string* data = (std::string*)instance.m_data;
    const RtObject& right = args.at(0);
    std::ostringstream oss;
    oss << *data;
    oss << right.getStringValue();
    return newObject(oss.str());
}

RtObject XStringClass::__radd__(XObject& instance, const std::vector<RtObject>& args) {
    std::string* data = (std::string*)instance.m_data;
    const RtObject& right = args.at(0);
    std::ostringstream oss;
    oss << right.getStringValue();
    oss << *data;
    return newObject(oss.str());
}

RtObject XStringClass::__multi__(XObject& instance, const std::vector<RtObject>& args) {
    std::string* data = (std::string*)instance.m_data;
    const RtObject& right = args.at(0);
    std::ostringstream oss;
    LONG64 n = right.getIntValue();
    for (LONG64 i = 0; i < n; ++i) {
        oss << *data;
    }
    return newObject(oss.str());
}

RtObject XStringClass::__eq__(XObject& instance, const std::vector<RtObject>& args) {
    const RtObject& right = args.at(0);
    const XObject* other_object = dynamic_cast<const XObject*>(right.getObject());
    if (!other_object) {
        return false;
    }
    std::string* my = (std::string*)instance.m_data;
    std::string* other = (std::string*)other_object->m_data;
    if (my == other) {
        return true;
    }
    return *my == *other;
}

RtObject XStringClass::__lt__(XObject& instance, const std::vector<RtObject>& args) {
    const RtObject& right = args.at(0);
    const XObject* other_object = dynamic_cast<const XObject*>(right.getObject());
    if (!other_object) {
        return false;
    }
    std::string* my = (std::string*)instance.m_data;
    std::string* other = (std::string*)other_object->m_data;
    if (my == other) {
        return false;
    }
    return *my < *other;
}

RtObject XStringClass::__len__(XObject& instance, const std::vector<RtObject>& args) {
    std::string* string_data = (std::string*)instance.m_data;
    return (LONG64)string_data->size();
}

RtObject XStringClass::__getitem__(XObject& instance, const std::vector<RtObject>& args) {
    std::string* string_data = (std::string*)instance.m_data;
    size_t data_size = string_data->size();
    if (data_size <= 0) {
        return XStringClass::newObject("");
    }
    LONG64 pos = X_get_pos(args.at(0), data_size, 0);
    char buf[2] = {0};
    buf[0] = string_data->operator[](pos);
    return XStringClass::newObject(buf);
}

RtObject XStringClass::__slice__(XObject& instance, const std::vector<RtObject>& args) {
    std::string* string_data = (std::string*)instance.m_data;
    size_t data_size = string_data->size();
    if (data_size <= 0) {
        return XStringClass::newObject("");
    }
    LONG64 from = X_get_pos(args.at(0), data_size, 0);
    LONG64 to = X_get_pos(args.at(1), data_size, data_size);
    if (from >= to) {
        return XStringClass::newObject("");
    }
    char *buf = new char[to - from + 1];
    char *buf_ptr = buf;
    memset(buf, 0, to - from + 1);
    LONG64 pos = 0;
    for (LONG64 i = 0; i < data_size; ++i, ++pos) {
        if (pos >= to) {
            break;
        }
        if (pos >= from) {
            *buf_ptr = string_data->operator[](i);
            buf_ptr++;
        }
    }
    RtObject ret = XStringClass::newObject(buf);
    delete[] buf;
    return ret;
}

RtObject XStringClass::__hashcode__(XObject& instance, const std::vector<RtObject>& args) {
    std::string* string_data = (std::string*)instance.m_data;
    std::size_t seed = 0;
    spp::hash_combine(seed, *string_data);
    return (LONG64)seed;
}

/// ============ XStringClass end ============


/// ============ XCallableClass begin ============


RtObject XFuncDef::call(const XfuncCallParam& param) {
    const std::vector<std::shared_ptr<ASTTree>>& arg_list = param.m_arg_expr_list;
    if (param.m_globalRuntimeContext.get_current_stack_size() > MAX_CALL_STACK_SIZE) {
        std::ostringstream oss;
        oss << " out of stack size: " << param.m_globalRuntimeContext.get_current_stack_size() << std::endl;
        param.m_globalRuntimeContext.fill_stack_message(oss, param.m_currentRuntimeContext);
        throw oss.str();
    }
    const std::vector<std::string>& arg_names = get_arg_names();
    if (arg_names.size() != arg_list.size()) {
        std::ostringstream oss;
        oss << "can not call func, because of arg size is not same." << std::endl;
        param.m_globalRuntimeContext.fill_stack_message(oss, param.m_currentRuntimeContext);
        throw oss.str();
    }
    LocalRuntimeContext localRuntimeContext(m_name);
    // prepare arguments
    int index = 0;
    for (std::vector<std::string>::const_iterator it = arg_names.begin(); it != arg_names.end(); ++it) {
        const std::string& name = *it;
        std::shared_ptr<ASTTree> real_arg_expr = arg_list.at(index);
        localRuntimeContext.set_variable(name, param.m_eval_ast_call(real_arg_expr, param.m_globalRuntimeContext, param.m_currentRuntimeContext));
        ++index;
    }
    param.m_globalRuntimeContext.push_runtime(&param.m_currentRuntimeContext);
    invoke(param.m_eval_ast_call, param.m_globalRuntimeContext, localRuntimeContext);
    RtObject return_value = localRuntimeContext.get_return_value();
    param.m_globalRuntimeContext.pop_runtime();
    return return_value;
}

class XASTFuncDef : public XFuncDef {
public:
    XASTFuncDef(const std::string& name, std::shared_ptr<ASTFuncDefStmt> ast_func_def) : XFuncDef(name, "ast"), m_ast_func_def(ast_func_def) {}
    virtual void invoke(XfuncCallParam::EVAL_AST_FN eval_ast_call, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& localRuntimeContext) const {
        std::shared_ptr<ASTProgram> func_body = m_ast_func_def->get_body();
        eval_ast_call(func_body, globalRuntimeContext, localRuntimeContext);
    }
    virtual const std::vector<std::string>& get_arg_names() const {
        return m_ast_func_def->get_arg_names();
    }
private:
    std::shared_ptr<ASTFuncDefStmt> m_ast_func_def;
};

class XNativeFuncDef : public XFuncDef {
public:
    XNativeFuncDef(const std::string& name, std::shared_ptr<NativeFunDef> native_func_def) : XFuncDef(name, "native"), m_native_func_def(native_func_def) {}
    virtual void invoke(XfuncCallParam::EVAL_AST_FN eval_ast_call, GlobalRuntimeContext& globalRuntimeContext, RuntimeContext& localRuntimeContext) const {
        RtObject return_value = m_native_func_def->call_native(localRuntimeContext);
        localRuntimeContext.set_return_value(return_value);
    }
    virtual const std::vector<std::string>& get_arg_names() const {
        return m_native_func_def->get_arg_names();
    }
private:
    std::shared_ptr<NativeFunDef> m_native_func_def;
};

RtObject XCallableClass::newObject(const std::string& name, std::shared_ptr<ASTFuncDefStmt> func) {
    return newObject(new XASTFuncDef(name, func));
}

RtObject XCallableClass::newObject(const std::string& name, std::shared_ptr<NativeFunDef> func) {
    return newObject(new XNativeFuncDef(name, func));
}

void XCallableClass::on_destroying(XObject& instance) const {
    XFuncDef* xFuncDef = (XFuncDef*)instance.m_data;
    delete xFuncDef;
}

RtObject XCallableClass::newObject(XFuncDef* xFuncDef) {
    return new XObject(instance(), xFuncDef);
}

RtObject XCallableClass::call(XObject &instance, const XfuncCallParam& param) const {
    XFuncDef* xFuncDef = (XFuncDef*)instance.m_data;
    return xFuncDef->call(param);
}

/// ============ XCallableClass end ============


/// ============ XListClass begin ============

RtObject XListClass::newObject() {
    std::vector<RtObject>* data = new std::vector<RtObject>;
    return new XObject(instance(), data);
}

RtObject XListClass::newObject(const std::vector<RtObject>& init_objects) {
    std::vector<RtObject>* data = new std::vector<RtObject>(init_objects);
    return new XObject(instance(), data);
}

void XListClass::on_destroying(XObject& instance) const {
    std::vector<RtObject>* data = (std::vector<RtObject>*)instance.m_data;
    delete data;
}

RtObject XListClass::__str__(XObject& instance, const std::vector<RtObject>& args) {
    std::ostringstream oss;
    oss << "[";
    std::vector<RtObject> input_args;
    input_args.push_back(XStringClass::newObject(", "));
    RtObject data = XListClass::join(instance, input_args);
    oss << data.getStringValue();
    oss << "]";
    return XStringClass::newObject(oss.str());
}

void appendObjectAsString(std::ostream& os, const RtObject& obj) {
    if (obj.getType() == RtObject::RT_TYPE_OBJECT && obj.getObject()->getClass() == XStringClass::instance()) {
        os << '"';
        os << obj.getStringValue();
        os << '"';
    } else {
        os << obj.getStringValue();
    }
}

RtObject XListClass::join(XObject& instance, const std::vector<RtObject>& args) {
    std::string token;
    if (args.size() == 0) {
        token = ",";
    } else {
        token = args.at(0).getStringValue();
    }
    std::ostringstream oss;
    bool first = true;
    const std::vector<RtObject>* data = (std::vector<RtObject>*)instance.m_data;
    for (std::vector<RtObject>::const_iterator it = data->begin(); it != data->end(); ++it) {
        RtObject obj = *it;
        if (first) {
            first = false;
        } else {
            oss << token;
        }
        appendObjectAsString(oss, obj);
    }
    return XStringClass::newObject(oss.str());
}



RtObject XListClass::__getitem__(XObject& instance, const std::vector<RtObject>& args) {
    RtObject index = args.at(0);
    LONG64 pos = index.getIntValue();
    const std::vector<RtObject>* data = (std::vector<RtObject>*)instance.m_data;
    size_t data_size = data->size();
    if (pos < 0) {
        pos = data_size + pos;
    }
    if (pos >= data_size) {
        throw std::string("out of list range.");
    } else {
        return data->operator[](pos);
    }
}

RtObject XListClass::__slice__(XObject& instance, const std::vector<RtObject>& args) {
    const std::vector<RtObject>* data = (std::vector<RtObject>*)instance.m_data;
    size_t data_size = data->size();
    if (data_size <= 0) {
        return XListClass::newObject();
    }
    LONG64 from = X_get_pos(args.at(0), data_size, 0);
    LONG64 to = X_get_pos(args.at(1), data_size, data_size);
    if (from >= to) {
        return XListClass::newObject();
    }
    std::vector<RtObject> init_objects;
    LONG64 pos = 0;
    for (std::vector<RtObject>::const_iterator it = data->begin(); it != data->end(); ++it, ++pos) {
        if (pos >= to) {
            break;
        }
        if (pos >= from) {
            init_objects.push_back(*it);
        }
    }
    return XListClass::newObject(init_objects);
}

RtObject XListClass::__setitem__(XObject& instance, const std::vector<RtObject>& args) {
    RtObject index = args.at(0);
    RtObject value = args.at(1);
    LONG64 pos = index.getIntValue();
    std::vector<RtObject>* data = (std::vector<RtObject>*)instance.m_data;
    size_t data_size = data->size();
    if (pos < 0) {
        pos = data_size + pos;
    }
    if (pos >= data_size) {
        throw std::string("out of list range.");
    } else {
        (*data)[pos] = value;
        return value;
    }
}

RtObject XListClass::__add__(XObject& instance, const std::vector<RtObject>& args) {
    RtObject other = args.at(0);
    if (other.getType() == RtObject::RT_TYPE_OBJECT) {
        XObject *other_obj = other.getObject();
        const XClass *other_class = other_obj->getClass();
        if (other_class == XListClass::instance()) {
            std::vector<RtObject>* my_data = (std::vector<RtObject>*)instance.m_data;
            std::vector<RtObject>* other_data = (std::vector<RtObject>*)other_obj->m_data;
            std::vector<RtObject> result(*my_data);
            for (std::vector<RtObject>::const_iterator it = other_data->begin(); it != other_data->end(); ++it) {
                result.push_back(*it);
            }
            return XListClass::newObject(result);
        }
    }
    throw std::string("list only can be extends or add with list type.");
}

RtObject XListClass::append(XObject& instance, const std::vector<RtObject>& args) {
    RtObject obj = args.at(0);
    std::vector<RtObject>* my_data = (std::vector<RtObject>*)instance.m_data;
    my_data->push_back(obj);
    return RtObject::Null;
}

RtObject XListClass::__multi__(XObject& instance, const std::vector<RtObject>& args) {
    RtObject other = args.at(0);
    if (other.getType() == RtObject::RT_TYPE_Int) {
        LONG64 value = other.getIntValue();
        std::vector<RtObject>* my_data = (std::vector<RtObject>*)instance.m_data;
        std::vector<RtObject> result;
        for (LONG64 i = 0; i < value; ++i) {
            for (std::vector<RtObject>::const_iterator it = my_data->begin(); it != my_data->end(); ++it) {
                result.push_back(*it);
            }
        }
        return XListClass::newObject(result);
    }
    throw std::string("list only can be multi by an integer value.");
}

RtObject XListClass::__eq__(XObject& instance, const std::vector<RtObject>& args) {
    RtObject other = args.at(0);
    if (other.getType() == RtObject::RT_TYPE_OBJECT) {
        XObject *other_obj = other.getObject();
        const XClass *other_class = other_obj->getClass();
        if (other_class == XListClass::instance()) {
            std::vector<RtObject>* my_data = (std::vector<RtObject>*)instance.m_data;
            std::vector<RtObject>* other_data = (std::vector<RtObject>*)other_obj->m_data;
            if (my_data->size() != other_data->size()) {
                return false;
            }
            for (size_t pos = 0; pos < my_data->size(); ++pos) {
                RtObject o1 = my_data->operator[](pos);
                RtObject o2 = other_data->operator[](pos);
                if (!o1.rt_equals(o2)) {
                    return RtObject::False;
                }
            }
            return RtObject::True;
        }
    }
    return RtObject::False;

}
RtObject XListClass::__len__(XObject& instance, const std::vector<RtObject>& args) {
    const std::vector<RtObject>* data = (std::vector<RtObject>*)instance.m_data;
    return (LONG64)data->size();
}

class XListIterator: public BaseIterator{
public:
    XListIterator(std::vector<RtObject>::const_iterator begin, std::vector<RtObject>::const_iterator end): m_it(begin), m_end(end) { }
    virtual bool has_next() const {
        return m_it != m_end;
    }
    virtual RtObject next()  {
        RtObject ret = *m_it;
        m_it++;
        return ret;
    }
    virtual ~XListIterator() {}
private:
    std::vector<RtObject>::const_iterator m_end;
    std::vector<RtObject>::const_iterator m_it;
};

class XVectorListIterator: public BaseIterator {
public:
    XVectorListIterator(std::vector<RtObject>* vec):m_vector(vec), m_it(vec->begin()), m_end(vec->end()) { }
    virtual bool has_next() const {
        return m_it != m_end;
    }
    virtual RtObject next()  {
        RtObject ret = *m_it;
        m_it++;
        return ret;
    }
    virtual ~XVectorListIterator() {
        if (m_vector) {
            delete m_vector;
            m_vector = NULL;
        }
    }
private:
    std::vector<RtObject>* m_vector;
    std::vector<RtObject>::const_iterator m_end;
    std::vector<RtObject>::const_iterator m_it;
};

RtObject XListClass::__iter__(XObject& instance, const std::vector<RtObject>& args) {
    std::vector<RtObject>* data = (std::vector<RtObject>*)instance.m_data;
    return XIteratorClass::newObject(new XListIterator(data->begin(), data->end()));
}

/// ============ XListClass end ============


/// ============ XIteratorClass begin ============

RtObject XIteratorClass::newObject(BaseIterator* iterator) {
    return new XObject(instance(), iterator);
}

void XIteratorClass::on_destroying(XObject& instance) const {
    BaseIterator* iterator = reinterpret_cast<BaseIterator*>(instance.m_data);
    delete iterator;
}

RtObject XIteratorClass::__hasnext__(XObject& instance, const std::vector<RtObject>& args) {
    BaseIterator* iterator = reinterpret_cast<BaseIterator*>(instance.m_data);
    return iterator->has_next();
}

RtObject XIteratorClass::__next__(XObject& instance, const std::vector<RtObject>& args) {
    BaseIterator* iterator = reinterpret_cast<BaseIterator*>(instance.m_data);
    return iterator->next();
}

/// ============ XIteratorClass end ============

/// ============ XIntIteratorFactoryClass begin ============

class IntIterator: public BaseIterator {
public:
    IntIterator(LONG64 from, LONG64 to, LONG64 step):
    m_from(from),
    m_to(to),
    m_step(step),
    m_value(from),
    m_order_greater(to >= from){}
    virtual bool has_next() const {
        if (m_order_greater) {
            return m_value < m_to;
        } else {
            return m_value > m_to;
        }
    }
    virtual RtObject next() {
        LONG64 ret = m_value;
        m_value += m_step;
        return ret;
    }
private:
    LONG64 m_from;
    LONG64 m_to;
    LONG64 m_step;
    LONG64 m_value;
    bool m_order_greater;
};

class IntIteratorData {
public:
    LONG64 m_from;
    LONG64 m_to;
    LONG64 m_step;
};

RtObject XIntIteratorFactoryClass::newObject(LONG64 from, LONG64 to, LONG64 step) {
    IntIteratorData* holder = new IntIteratorData();
    holder->m_from = from;
    holder->m_to = to;
    holder->m_step = step;
    return RtObject(new XObject(instance(), holder));
}

void XIntIteratorFactoryClass::on_destroying(XObject& instance) const {
    IntIteratorData* holder = (IntIteratorData*)instance.m_data;
    delete holder;
}

RtObject XIntIteratorFactoryClass::__iter__(XObject& instance, const std::vector<RtObject>& args) {
    IntIteratorData* holder = (IntIteratorData*)instance.m_data;
    return XIteratorClass::newObject(new IntIterator(holder->m_from, holder->m_to, holder->m_step));
}


/// ============ XIntIteratorFactoryClass end ============


/// ============ XMapClass begin ============

namespace std
{
    // inject specialization of std::hash for RtObject into namespace std
    // ----------------------------------------------------------------
    template<>
    struct hash<RtObject>
    {
        std::size_t operator()(RtObject const &rt_object) const
        {
            if (rt_object.getType() == RtObject::RT_TYPE_OBJECT) {
                XObject* obj = rt_object.getObject();
                return obj->invoke(FN_HASHCODE).getIntValue();
            } else {
                std::size_t seed = 0;
                spp::hash_combine(seed, rt_object.getIntValue());
                return seed;
            }
        }
    };
}

RtObject XMapClass::newObject() {
    auto data = new spp::sparse_hash_map<RtObject, RtObject>;
    return new XObject(XMapClass::instance(), data);
}
RtObject XMapClass::newObject(const std::vector<std::pair<RtObject, RtObject>>& init_objects) {
    auto data = new spp::sparse_hash_map<RtObject, RtObject>;
    for (const auto& it : init_objects) {
        (*data)[it.first] = it.second;
    }
    return new XObject(XMapClass::instance(), data);
}

void XMapClass::on_destroying(XObject& instance) const {
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    delete map;
}

RtObject XMapClass::__str__(XObject& instance, const std::vector<RtObject>& args) {
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& it : (*map)) {
        if (first) {
            first = false;
        } else {
            oss << ", ";
        }
        const RtObject& key = it.first;
        const RtObject& value = it.second;
        appendObjectAsString(oss, key);
        oss << ":";
        appendObjectAsString(oss, value);
    }
    oss << "}";
    return XStringClass::newObject(oss.str());
}

RtObject XMapClass::__eq__(XObject& instance, const std::vector<RtObject>& args) {
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    const RtObject& other = args.at(0);
    if (other.getType() != RtObject::RT_TYPE_OBJECT) {
        return RtObject::False;
    }
    XObject* other_obj = other.getObject();
    if (other_obj == NULL) {
        return RtObject::False;
    }
    if (other_obj->getClass() != XMapClass::instance()) {
        return RtObject::False;
    }
    auto other_map = (spp::sparse_hash_map<RtObject, RtObject>*)other_obj->m_data;
    if (map->size() != other_map->size()) {
        return RtObject::False;
    }
    for (const auto& it : (*map)) {
        const RtObject& key = it.first;
        const RtObject& value = it.second;
        auto other_it = other_map->find(key);
        if (other_it == other_map->end()) {
            return RtObject::False;
        }
        const RtObject& other_value = other_it->second;
        if (value.rt_equals(other_value)) {
            return RtObject::False;
        }
    }
    return RtObject::True;
}
RtObject XMapClass::__len__(XObject& instance, const std::vector<RtObject>& args) {
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    return (LONG64)map->size();
}

RtObject XMapClass::__getitem__(XObject& instance, const std::vector<RtObject>& args) {
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    const RtObject& key = args.at(0);
    auto it = map->find(key);
    if (it == map->end()) {
        return RtObject::Null;
    }
    return it->second;
}
RtObject XMapClass::__setitem__(XObject& instance, const std::vector<RtObject>& args) {
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    const RtObject& key = args.at(0);
    const RtObject& value = args.at(1);
    return (*map)[key] = value;
}

RtObject XMapClass::__add__(XObject& instance, const std::vector<RtObject>& args) {
    const RtObject& other = args.at(0);
    if (other.getType() != RtObject::RT_TYPE_OBJECT) {
        throw std::string("add only support for map type.");
    }
    XObject* other_obj = other.getObject();
    if (other_obj == NULL) {
        throw std::string("add only support for map type.");
    }
    if (other_obj->getClass() != XMapClass::instance()) {
        throw std::string("add only support for map type.");
    }
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    auto other_map = (spp::sparse_hash_map<RtObject, RtObject>*)other_obj->m_data;
    auto data = new spp::sparse_hash_map<RtObject, RtObject>;
    for (const auto& it : *map) {
        (*data)[it.first] = it.second;
    }
    for (const auto& it : *other_map) {
        (*data)[it.first] = it.second;
    }
    return new XObject(XMapClass::instance(), data);
}

class XMapIterator: public BaseIterator{
public:
    XMapIterator(spp::sparse_hash_map<RtObject, RtObject>::const_iterator begin, spp::sparse_hash_map<RtObject, RtObject>::const_iterator end): m_it(begin), m_end(end) { }
    virtual bool has_next() const {
        return m_it != m_end;
    }
    virtual RtObject next()  {
        std::pair<RtObject, RtObject> ret = *m_it;
        std::vector<RtObject> init_objects;
        init_objects.push_back(m_it->first);
        init_objects.push_back(m_it->second);
        m_it++;
        return XListClass::newObject(init_objects);
    }
private:
    spp::sparse_hash_map<RtObject, RtObject>::const_iterator m_end;
    spp::sparse_hash_map<RtObject, RtObject>::const_iterator m_it;
};

RtObject XMapClass::__iter__(XObject& instance, const std::vector<RtObject>& args) {
    auto map = (spp::sparse_hash_map<RtObject, RtObject>*)instance.m_data;
    return XIteratorClass::newObject(new XMapIterator(map->begin(), map->end()));
}


/// ============ XMapClass end ============

/// ============ XModuleClass start ============


RtObject XModuleClass::newObject(GlobalRuntimeContext& globalRuntimeContext, XModuleDef *moduleDef) {
    XBootContext& parentContext = *globalRuntimeContext.getBootContext();
    // make moduleDef be delete when return in if
    RtObject ret(new XObject(XModuleClass::instance(), moduleDef));
    if (parentContext.has_module(moduleDef->m_name)) {
        return parentContext.get_module(moduleDef->m_name);
    }
    return ret;
}

static void x_normalize_module_name(const std::string& name, std::string& out) {
    // normalize name abcd.efg.xx =>  abcd/efg/xx
    const char* name_str = name.c_str();
    size_t len = strlen(name_str);
    char* buf = new char[len+1];
    buf[len] = '\0';
    for (int i = 0; i < len; ++i) {
        if (name_str[i] == '.') {
            buf[i] = FILE_PATH_SEP;
        } else {
            buf[i] = name_str[i];
        }
    }
    out = buf;
    delete[] buf;
}

static FILE * x_find_file_module(const std::vector<string>& paths, const std::string& name) {
    std::string new_name;
    x_normalize_module_name(name, new_name);
    
    for (auto it = paths.begin(); it != paths.end(); ++it) {
        std::string module_path = *it + FILE_PATH_SEP + new_name + MODULE_EXT;
        FILE *fp = fopen(module_path.c_str(), "r");
        if (fp != NULL) {
            return fp;
        }
    }
    return NULL;
}

class XFileModuleDef: public XModuleDef {
public:
    XFileModuleDef(const std::string& name, XBoot *subBoot):
    XModuleDef(name), m_subBoot(subBoot) {}
    
    virtual void get_export_names(std::vector<std::string>& out) const {
        auto variables = m_subBoot->getGlobalRuntimeContext().get_variables();
        for (auto it = variables.begin(); it != variables.end(); ++it) {
            const std::string& name = it->first;
            if (is_export(name)) {
                out.push_back(name);
            }
        }
    }
    virtual RtObject get_export(const std::string& name) const {
        if (!is_export(name)) {
            return RtObject::Null;
        }
        return m_subBoot->getGlobalRuntimeContext().get_variable(name);
    }
    
    GlobalRuntimeContext& getGlobalRuntimeContext() const {
        return m_subBoot->getGlobalRuntimeContext();
    }
    virtual ~XFileModuleDef() {
        if (m_subBoot) {
            delete m_subBoot;
            m_subBoot = NULL;
        }
    }
private:
    XBoot *m_subBoot;
};

class NativeMethodFuncDef: public NativeFunDef {
public:
    NativeMethodFuncDef(const NativeMethodInfo& nativeMethodInfo) : m_nativeMethodInfo(nativeMethodInfo) {}
    virtual RtObject call_native(RuntimeContext& runtimeContext) {
        return m_nativeMethodInfo.method(runtimeContext);
    }
    virtual const std::vector<std::string>& get_arg_names() {
        return m_nativeMethodInfo.arg_names;
    }
private:
    NativeMethodInfo m_nativeMethodInfo;
};

GlobalRuntimeContext& XMapModuleDef::getGlobalRuntimeContext() const {
    return m_globalRuntimeContext;    
}

XMapModuleDef::XMapModuleDef(GlobalRuntimeContext &globalRuntimeContext, const std::string& name, const std::map<std::string, NativeMethodInfo>& native_methods) : m_globalRuntimeContext(globalRuntimeContext),XModuleDef(name) {
    for (auto it = native_methods.begin(); it != native_methods.end(); ++it) {
        std::shared_ptr<NativeFunDef> func(new NativeMethodFuncDef(it->second));
        m_properties[it->first] = XCallableClass::newObject(it->first, func);
    }
}

void XMapModuleDef::add_native_method(const std::string& name, const NativeMethodInfo& native_method) {
    std::shared_ptr<NativeFunDef> func(new NativeMethodFuncDef(native_method));
    m_properties[name] = XCallableClass::newObject(name, func);
}

void XMapModuleDef::get_export_names(std::vector<std::string>& out) const {
    const auto& variables = m_properties;
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        const std::string& name = it->first;
        if (is_export(name)) {
            out.push_back(name);
        }
    }
}

RtObject XMapModuleDef::get_export(const std::string& name) const {
    if (!is_export(name)) {
        return RtObject::Null;
    }
    auto it = m_properties.find(name);
    if (it == m_properties.end()) {
        return RtObject::Null;
    }
    return it->second;
}

RtObject XModuleClass::newObject(GlobalRuntimeContext& globalRuntimeContext , const std::string& name, const std::map<std::string, NativeMethodInfo>& native_methods) {
    XBootContext& parentContext = *globalRuntimeContext.getBootContext();
    if (parentContext.has_module(name)) {
        return parentContext.get_module(name);
    }
    XMapModuleDef *mapModuleDef = new XMapModuleDef(globalRuntimeContext, name, native_methods);
    return XModuleClass::newObject(globalRuntimeContext, mapModuleDef);
}

RtObject XModuleClass::newObject(GlobalRuntimeContext& globalRuntimeContext, const std::string& name) {
    XBootContext& parentContext = *globalRuntimeContext.getBootContext();
    if (parentContext.has_module(name)) {
        return parentContext.get_module(name);
    }
    RtObject fromRegistered = parentContext.get_register_module(name);
    if (!RtObject::Null.rt_equals(fromRegistered)) {
        return fromRegistered;
    }
    const std::vector<string>& paths = parentContext.get_paths();
    FILE * fp = x_find_file_module(paths, name);
    if (fp == NULL) {
        throw std::string("can not find module:") + name;
    }
    FileInputStream is(fp);
    XBoot *subBoot = new XBoot;
    subBoot->eval(&is);
    return XModuleClass::newObject(globalRuntimeContext, new XFileModuleDef(name, subBoot));
}

void XModuleClass::on_destroying(XObject& instance) const {
    XModuleDef *moduleDef = (XModuleDef *)instance.m_data;
    delete moduleDef;
}

RtObject XModuleClass::__str__(XObject& instance, const std::vector<RtObject>& args) {
    RtObject it = XModuleClass::__iter__(instance, args);
    std::string token(", ");
    XModuleDef *moduleDef = (XModuleDef *)instance.m_data;
    std::vector<string> out_names;
    moduleDef->get_export_names(out_names);
    std::ostringstream oss;
    oss << "{<Module:";
    oss << moduleDef->m_name;
    oss << ">:";
    bool first = true;
    for (auto it = out_names.begin(); it != out_names.end(); ++it) {
        const std::string& name = *it;
        if (first) {
            first = false;
        } else {
            oss << token;
        }
        oss << name;
    }
    oss << "}";
    return XStringClass::newObject(oss.str());
}

RtObject XModuleClass::__iter__(XObject& instance, const std::vector<RtObject>& args) {
    XModuleDef *moduleDef = (XModuleDef *)instance.m_data;
    std::vector<string> out_names;
    moduleDef->get_export_names(out_names);
    auto out = new std::vector<RtObject>;
    for (auto it = out_names.begin(); it != out_names.end(); ++it) {
        out->push_back(XStringClass::newObject(*it));
    }
    return XIteratorClass::newObject(new XVectorListIterator(out));
}

RtObject XModuleClass::get_item(XObject& instance, const std::string& name) const {
    XModuleDef *moduleDef = (XModuleDef *)instance.m_data;
    return moduleDef->get_export(name);
}

RtObject XModuleClass::__getitem__(XObject& instance, const std::vector<RtObject>& args) {
    XModuleDef *moduleDef = (XModuleDef *)instance.m_data;
    const RtObject& key = args.at(0);
    return moduleDef->get_export(key.getStringValue());
}

GlobalRuntimeContext& XModuleClass::getModuleGlobalRuntimeContext(XObject& instance) {
    XModuleDef *moduleDef = (XModuleDef *)instance.m_data;
    return moduleDef->getGlobalRuntimeContext();
}

/// ============ XModuleClass end ============

