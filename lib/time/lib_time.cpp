//
//  time.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/15.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "lib_time.hpp"
#include "xboot.hpp"


#ifdef _WIN32
#include "sys_win.hpp"
#else
#include <sys/time.h>
#endif


static RtObject time_now(RuntimeContext& runtimeContext) {
    return XTimeClass::newObject();
}

void lib_register_time_module(XBoot &boot) {
    NativeMethodModuleCreator creator(boot, "time");
    {
        NativeMethodInfo m;
        m.method = time_now;
        std::vector<std::string> arg_names;
        m.arg_names = arg_names;
        creator.add_method("Now", m);
    }
    // tobe add more methods
    
    boot.register_modules(creator);
}


// for XTimeClass
RtObject XTimeClass::newObject() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        return RtObject::Null;
    }
    LONG64 ts = (LONG64)tv.tv_sec*1000 + tv.tv_usec/1000;
    return XTimeClass::newObject(ts);
}

RtObject XTimeClass::newObject(LONG64 ts) {
    LONG64* timeData = new LONG64;
    *timeData = ts;
    XObject* obj = new XObject(instance(), timeData);
    return obj;
}

void XTimeClass::on_destroying(XObject& instance) const {
    LONG64* timeData = (LONG64*)instance.m_data;
    delete timeData;
}

RtObject XTimeClass::__str__(XObject& instance, const std::vector<RtObject>& args) {
    LONG64* timeData = (LONG64*)instance.m_data;
    std::ostringstream oss;
    oss << *timeData;
    return XStringClass::newObject(oss.str());
}

typedef struct{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    LONG64 ms;
} TimeDelta;

typedef enum {
    TDS_INIT,
    TDS_NUMBER,
    TDS_UNIT
} TimeDeltaStatus;

static
bool parse_from_delta_expr(TimeDelta &out, const char* str) {
    TimeDelta temp = {0};
    TimeDeltaStatus st = TDS_INIT;
    while (1) {
        char c = *str;
        if (c == '\0') break;
        
        
        str++;
    }
    
    
    memcpy(&out, &temp, sizeof(TimeDelta));
    return true;
}

// suport for
// 12y23M34d40h522m6122s14ms
RtObject XTimeClass::__add__(XObject& instance, const std::vector<RtObject>& args) {
    return 0;
}

RtObject XTimeClass::__sub__(XObject& instance, const std::vector<RtObject>& args) {
    return 0;

}
RtObject XTimeClass::__eq__(XObject& instance, const std::vector<RtObject>& args) {
    return 0;

}
RtObject XTimeClass::__lt__(XObject& instance, const std::vector<RtObject>& args) {
    return 0;

}
RtObject XTimeClass::__hashcode__(XObject& instance, const std::vector<RtObject>& args) {
    return 0;

}
