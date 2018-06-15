//
//  runtime_types.cpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/29.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#include "runtime_types.hpp"
#include "xobject.hpp"
#include "meta_types.hpp"

#include <sstream>
using std::ostringstream;


const RtObject RtObject::True = RtObject(true);
const RtObject RtObject::False = RtObject(false);
const RtObject RtObject::Null = RtObject();

bool RtObject::getBooleanValue() const {
    switch (m_type) {
        case RT_TYPE_Boolean:
            return m_value.boolean_value;
        case RT_TYPE_Int:
            return m_value.int_value != 0;
        case RT_TYPE_Float:
            return m_value.float_value != 0;
        case RT_TYPE_Null:
            return false;
        default:
            return false;
    }
}
LONG64 RtObject::getIntValue() const {
    switch (m_type) {
        case RT_TYPE_Boolean:
            return m_value.boolean_value ? 1: 0;
        case RT_TYPE_Int:
            return m_value.int_value;
        case RT_TYPE_Float:
            return (LONG64)(m_value.float_value);
        case RT_TYPE_Null:
            return 0;
        default:
            return 0;
    }
}

double RtObject::getFloatValue() const {
    switch (m_type) {
        case RT_TYPE_Boolean:
            return m_value.boolean_value ? 1: 0;
        case RT_TYPE_Int:
            return m_value.int_value;
        case RT_TYPE_Float:
            return m_value.float_value;
        case RT_TYPE_Null:
            return 0;
        default:
            return 0;
    }
}

const std::string RtObject::getStringValue() const {
    switch (m_type) {
        case RT_TYPE_Boolean:
            return m_value.boolean_value ? "True": "False";
        case RT_TYPE_Int:
        {
            std::ostringstream oss;
            oss << m_value.int_value;
            return oss.str();
        }
        case RT_TYPE_Float:
        {
            std::ostringstream oss;
            oss.precision(12);
            oss << m_value.float_value;
            return oss.str();
        }
        case RT_TYPE_Null:
            return "Null";
        case RT_TYPE_OBJECT:
        {
            XObject *obj = getObject();
            return obj->getClass()->str(*obj);
        }
        default:
            return "";
    }
}

bool RtObject::rt_equals(const RtObject& v) const {
    const RtObjectType other_type = v.getType();
    switch (m_type) {
        case RT_TYPE_Boolean:
            return getBooleanValue() == v.getBooleanValue();
        case RT_TYPE_Int:
            if (other_type == RT_TYPE_Int || other_type == RT_TYPE_Boolean || other_type == RT_TYPE_Null) {
                return getIntValue() == v.getIntValue();
            }
            if (other_type == RT_TYPE_Float) {
                return getFloatValue() == v.getFloatValue();
            }
            return false;
        case RT_TYPE_Float:
            if (other_type == RT_TYPE_Int || other_type == RT_TYPE_Float || other_type == RT_TYPE_Boolean || other_type == RT_TYPE_Null) {
                return getFloatValue() == v.getFloatValue();
            } else {
                return false;
            }
        case RT_TYPE_OBJECT:
        {
            if (other_type != RT_TYPE_OBJECT) {
                return false;
            }
            return getObject()->invoke(FN_EQ, v).getBooleanValue();
        }
        case RT_TYPE_Null:
            if (v.isNull()) {
                return true;
            }
            if (other_type == RT_TYPE_Int || other_type == RT_TYPE_Boolean || other_type == RT_TYPE_Null) {
                return getIntValue() == 0;
            }
            if (other_type == RT_TYPE_Float) {
                return getFloatValue() == 0;
            }
            if (other_type == RT_TYPE_OBJECT) {
                return false;
            }
            return false;
        default:
            break;
    }
    return false;
}

RtObject::operator bool() const {
    const RtObjectType my_type = getType();
    switch(my_type) {
        case RT_TYPE_Null:
            return false;
        case RT_TYPE_Int:
            return 0 != m_value.int_value;
        case RT_TYPE_Float:
            return 0 != m_value.float_value;
        case RT_TYPE_OBJECT:
            return true;
        case RT_TYPE_Boolean:
            return m_value.boolean_value;
        default:
            return false;
    }
}
bool RtObject::rt_less_than(const RtObject& v) const {
    const RtObjectType my_type = getType();
    const RtObjectType other_type = v.getType();
    if (my_type == RT_TYPE_Float || other_type == RT_TYPE_Float) {
        return getFloatValue() < v.getFloatValue();
    }
    if (my_type == RT_TYPE_Int || other_type == RT_TYPE_Int) {
        return getIntValue() < v.getIntValue();
    }
    if (my_type == RT_TYPE_OBJECT && other_type == RT_TYPE_OBJECT) {
        return getObject()->invoke(FN_LT, v.getObject()).getBooleanValue();
    }
    return getIntValue() < v.getIntValue();
}

RtObject RtObject::rt_logic_and(const RtObject& v) const {
    bool v1 = getBooleanValue();
    bool v2 = v.getBooleanValue();
    return v1 && v2;
}
RtObject RtObject::rt_logic_or(const RtObject& v) const {
    bool v1 = getBooleanValue();
    bool v2 = v.getBooleanValue();
    return v1 || v2;
}

RtObject RtObject::rt_add(const RtObject& v) const {
    const RtObjectType my_type = getType();
    const RtObjectType other_type = v.getType();
    if (my_type == RT_TYPE_OBJECT) {
        XObject* self = getObject();
        return self->invoke(FN_ADD, v);
    }
    if (other_type == RT_TYPE_OBJECT) {
        XObject* other = v.getObject();
        return other->invoke(FN_RADD, *this);
    }
    if (my_type == RT_TYPE_Float || other_type == RT_TYPE_Float) {
        return getFloatValue() + v.getFloatValue();
    }
    return getIntValue() + v.getIntValue();
}

RtObject RtObject::rt_multi(const RtObject& v) const {
    const RtObjectType my_type = getType();
    const RtObjectType other_type = v.getType();
    if (my_type == RT_TYPE_OBJECT) {
        XObject* self = getObject();
        return self->invoke(FN_MULTI, v);
    }
    if (my_type == RT_TYPE_Float || other_type == RT_TYPE_Float) {
        return getFloatValue() * v.getFloatValue();
    }
    return getIntValue() * v.getIntValue();
}
RtObject RtObject::rt_sub(const RtObject& v) const {
    const RtObjectType my_type = getType();
    const RtObjectType other_type = v.getType();
    if (my_type == RT_TYPE_Float || other_type == RT_TYPE_Float) {
        return getFloatValue() - v.getFloatValue();
    }
    return getIntValue() - v.getIntValue();
}
RtObject RtObject::rt_div(const RtObject& v) const {
    const RtObjectType my_type = getType();
    const RtObjectType other_type = v.getType();
    if (my_type == RT_TYPE_Float || other_type == RT_TYPE_Float) {
        return getFloatValue() / v.getFloatValue();
    }
    return getIntValue() / v.getIntValue();
}

RtObject RtObject::rt_mod(const RtObject& v) const {
    return getIntValue() % v.getIntValue();
}

RtObject RtObject::rt_dot(const RtObject& v) const {
    const RtObjectType my_type = getType();
    if (my_type == RT_TYPE_OBJECT) {
        XObject* self = getObject();
        return self->invoke(FN_DOT, v);
    }
    throw new std::string("unsupport operator dot(.)");
}

RtObject::RtObject(XObject* obj) {
    if (obj == NULL) {
        m_type = RT_TYPE_Null;
        m_value.int_value = 0;
    } else {
        m_type = RT_TYPE_OBJECT;
        m_value.object_value = obj;
        obj->_add_ref();
    }
}

XObject* RtObject::getObject() const {
    if (m_type != RT_TYPE_OBJECT) {
        return NULL;
    }
    return dynamic_cast<XObject*>(m_value.object_value);
}

void RtObject::add_object_ref() {
    switch (m_type) {
        case RT_TYPE_OBJECT:
            m_value.object_value->_add_ref();
            break;
        default:
            break;
    }
}
void RtObject::release_object_ref() {
    switch (m_type) {
        case RT_TYPE_OBJECT:
            m_value.object_value->_release();
            break;
        default:
            break;
    }
}

bool RtObject::callable() const {
    switch (m_type) {
        case RT_TYPE_OBJECT:
            return m_value.object_value->callable();
        default:
            return false;
    }
}


RtObject& RtObject::operator=(const RtObject& copy) {
    // for old
    release_object_ref();
    // for new
    m_type = copy.m_type;
    m_value = copy.m_value;
    add_object_ref();
    return *this;
}

