//
//  xobject.hpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/29.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#ifndef xobject_hpp
#define xobject_hpp
#include <string>
#include "runtime_types.hpp"

using std::string;
class XMethodCallParam;

#define FN_ADD          "__add__"
#define FN_RADD         "__radd__"
#define FN_SUB          "__sub__"
#define FN_MULTI        "__multi__"
#define FN_DIV          "__div__"
#define FN_MOD          "__mod__"
#define FN_EQ           "__eq__"
#define FN_LEN          "__len__"
#define FN_NEQ          "__neq__"
#define FN_LT           "__lt__"
#define FN_LTEQ         "__lteq__"
#define FN_GT           "__gt__"
#define FN_GTEQ         "__gteq__"
#define FN_CALL         "__call__"
#define FN_STR          "__str__"
#define FN_GETITEM      "__getitem__"
#define FN_SETITEM      "__setitem__"
#define FN_SLICE        "__slice__"
#define FN_ITER         "__iter__"
#define FN_NEXT         "__next__"
#define FN_HASNEXT      "__hasnext__"
#define FN_HASHCODE     "__hashcode__"
#define FN_DOT          "__dot__"

class XClass;

class XObject {
public:
    void _add_ref() {
        m_ref_count++;
    }
    void _release() {
        m_ref_count--;
        if (m_ref_count <= 0) {
            delete this;
        }
    }
    const XClass* getClass() const {
        return m_clazz;
    }
    void* getData() const {
        return m_data;
    }
    RtObject invoke(const std::string& method_name);
    RtObject invoke(const std::string& method_name, const RtObject& arg1);
    RtObject invoke(const std::string& method_name, const RtObject& arg1, const RtObject& arg2);
    RtObject invoke(const std::string& method_name, const std::vector<RtObject>& args);
    RtObject get(const std::string& method_name) const;
    void set(const std::string& method_name, const RtObject& value);
    bool callable() const;
    XObject(const XClass* clazz, void* data = NULL);
    ~XObject();
private:
    XObject& operator=(const XObject&);
    XObject(const XObject&);
    size_t m_ref_count;
    const XClass* m_clazz;
public:
    void *m_data;
};


#endif /* xobject_hpp */
