//
//  xobject.cpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/29.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#include "xobject.hpp"
#include "meta_types.hpp"


RtObject XObject::invoke(const std::string& method_name) {
    std::vector<RtObject> args;
    return m_clazz->invoke(*this, method_name, args);
}
RtObject XObject::invoke(const std::string& method_name, const RtObject& arg1) {
    std::vector<RtObject> args;
    args.push_back(arg1);
    return m_clazz->invoke(*this, method_name, args);
}
RtObject XObject::invoke(const std::string& method_name, const RtObject& arg1, const RtObject& arg2) {
    std::vector<RtObject> args;
    args.push_back(arg1);
    args.push_back(arg2);
    return m_clazz->invoke(*this, method_name, args);
}

RtObject XObject::invoke(const std::string& method_name, const std::vector<RtObject>& args) {
    return m_clazz->invoke(*this, method_name, args);
}
RtObject XObject::get(const std::string& method_name) const {
    return m_clazz->get(*this, method_name);

}
void XObject::set(const std::string& method_name, const RtObject& value) {
    m_clazz->set(*this, method_name, value);
}

bool XObject::callable() const {
    return m_clazz->callable(*this);
}

//static int debug_objects_counts = 0;

XObject::XObject(const XClass* clazz, void* data) : m_ref_count(0), m_clazz(clazz), m_data(data) {
  //  debug_objects_counts++;
   // printf("objects: %d\n", debug_objects_counts);
}

XObject::~XObject() {
    m_clazz->on_destroying(*this);
   
    //debug_objects_counts--;
    //printf("objects: %d\n", debug_objects_counts);
}
