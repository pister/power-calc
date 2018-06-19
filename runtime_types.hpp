//
//  runtime_types.hpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/29.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#ifndef runtime_types_hpp
#define runtime_types_hpp

#include <string>
#include <vector>
#include "ast.hpp"
#include "xbase.hpp"

using std::string;
using std::vector;



class XObject;

class RtObject {
public:
    static const RtObject Null;
    static const RtObject True;
    static const RtObject False;
    
    enum RtObjectType {
        RT_TYPE_Boolean,
        RT_TYPE_Int,
        RT_TYPE_Float,
        RT_TYPE_Null,
        RT_TYPE_OBJECT
    };
    union RtValue {
        bool    boolean_value;
        LONG64  int_value;
        double  float_value;
        XObject * object_value;
    };
    
    RtObject(int i):RtObject(RT_TYPE_Int) {
        m_value.int_value = i;
    }
    RtObject(long i):RtObject(RT_TYPE_Int) {
        m_value.int_value = i;
    }
    RtObject(LONG64 i):RtObject(RT_TYPE_Int) {
        m_value.int_value = i;
    }
    RtObject(float v):RtObject(RT_TYPE_Float) {
        m_value.float_value = v;
    }
    RtObject(double v):RtObject(RT_TYPE_Float) {
        m_value.float_value = v;
    }
    RtObject(bool b):RtObject(RT_TYPE_Boolean) {
        m_value.boolean_value = b;
    }
    RtObject(XObject* obj);
    RtObject():m_type(RT_TYPE_Null) {
        m_value.int_value = 0;
    }
    RtObjectType getType() const {
        return m_type;
    }
    RtValue getValue() const {
        return m_value;
    }
    
    RtObject(const RtObject& copy) : m_type(copy.m_type), m_value(copy.m_value) {
        add_object_ref();
    }
    
    RtObject& operator=(const RtObject& copy);
    
    ~RtObject() {
        release_object_ref();
    }
    
    bool getBooleanValue() const;
    LONG64 getIntValue() const;
    double getFloatValue() const;
    const std::string getStringValue() const;
    XObject* getObject() const;
    bool callable() const;
    bool isNull() const {
        return (RT_TYPE_Null == m_type);
    }
    bool rt_equals(const RtObject& v) const;
    bool rt_less_than(const RtObject& v) const;
    RtObject rt_logic_and(const RtObject& v) const;
    RtObject rt_logic_or(const RtObject& v) const;
    RtObject rt_multi(const RtObject& v) const;
    RtObject rt_add(const RtObject& v) const;
    RtObject rt_sub(const RtObject& v) const;
    RtObject rt_div(const RtObject& v) const;
    RtObject rt_mod(const RtObject& v) const;
    RtObject rt_dot(const RtObject& v) const;
    bool rt_not_equals(const RtObject& v) const {
        return !rt_equals(v);
    }
    bool rt_less_equals_than(const RtObject& v) const {
        return rt_less_than(v) || rt_equals(v);
    }
    bool rt_greater_than(const RtObject& v) const {
        return !rt_less_than(v) && rt_not_equals(v);
    }
    bool rt_greater_equals_than(const RtObject& v) const {
        return !rt_less_than(v);
    }
    operator bool() const;
    bool operator==(const RtObject& v) const {
        return rt_equals(v);
    }
private:
    void add_object_ref();
    void release_object_ref();
    RtObject(RtObjectType type): m_type(type) {}
    RtObjectType m_type;
    RtValue m_value;
};
#endif /* runtime_types_hpp */
