//
//  time.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/15.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef lib_time_hpp
#define lib_time_hpp

#include "runtime_types.hpp"
#include "meta_types.hpp"

using std::vector;
using std::map;
using std::pair;


class GlobalRuntimeContext;
class RuntimeContext;

#define TIME_MODULE_NAME "time"

class XBoot;
void lib_register_time_module(XBoot &boot);


class XTimeClass: public XClass {
public:
    static RtObject newObject(LONG64 ts);
    static RtObject newObject();
    virtual void on_destroying(XObject& instance) const;
    static XTimeClass* instance() {
        static XTimeClass instance;
        return &instance;
    }
private:
    static RtObject __str__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __add__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __sub__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __eq__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __lt__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __gt__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __hashcode__(XObject& instance, const std::vector<RtObject>& args);

private:
    XTimeClass() : XClass("time") {
        register_method(FN_STR, __str__);
        register_method(FN_SUB, __sub__);
        register_method(FN_ADD, __add__);
        register_method(FN_EQ, __eq__);
        register_method(FN_LT, __lt__);
        register_method(FN_GT, __gt__);
        register_method(FN_HASHCODE, __hashcode__);
    }
};
#endif /* lib_time_hpp */
