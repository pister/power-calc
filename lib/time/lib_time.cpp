//
//  time.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/15.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "lib_time.hpp"
#include "xboot.hpp"
#include "unisys.hpp"


static RtObject time_now(RuntimeContext& runtimeContext) {
    return XTimeClass::newObject();
}

static RtObject time_sleep(RuntimeContext& runtimeContext) {
    RtObject ms =  runtimeContext.get_variable("ms");
    LONG64 intValue = ms.getIntValue();
    unisys_sleep(intValue);
    return RtObject::Null;
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
    
    {
        NativeMethodInfo m;
        m.method = time_sleep;
        std::vector<std::string> arg_names;
        arg_names.push_back("ms");
        m.arg_names = arg_names;
        creator.add_method("Sleep", m);
    }
    // tobe add more methods
    
    boot.register_modules(creator);
}


// for XTimeClass
RtObject XTimeClass::newObject() {
    struct timeval tv;
    if (unisys_gettimeofday(&tv, NULL) != 0) {
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
    int ms;
} TimeDelta;

typedef enum {
    TDS_INIT,
    TDS_NUMBER,
    TDS_UNIT
} TimeDeltaStatus;

#define MAX_TIME_NUMBER_SIZE 10

static
bool parse_from_delta_expr(TimeDelta &out, const char* str) {
    TimeDelta temp = {0};
    TimeDeltaStatus st = TDS_INIT;
    char number_buf[MAX_TIME_NUMBER_SIZE] = {0};
    char* number_ptr = number_buf;
    char unit_c = 0;
    int val;
    while (1) {
        char c = *str;
        if (c == '\0') break;
        if ( c >= '0' && c <= '9') {
            st = TDS_NUMBER;
            *number_ptr++ = c;
        } else {
            unit_c = c;
            val = atoi(number_buf);
            switch (unit_c) {
                    // not support this version
                    /*
                case 'y':
                    temp.year = val;
                    break;
                case 'M':
                    temp.month = val;
                    break;
                case 'd':
                    temp.day = val;
                    break;
                     */
                case 'h':
                    temp.hour = val;
                    break;
                case 'm':
                    temp.minute = val;
                    break;
                case 's':
                    temp.second = val;
                    break;
                case 't':
                    temp.ms = val;
                    break;
                default:
                    break;
            }
            memset(number_buf, 0, sizeof(number_buf));
            number_ptr = number_buf;
        }
        str++;
    }
    memcpy(&out, &temp, sizeof(TimeDelta));
    return true;
}

// suport for
// 40h522m6122s14t
// h-hour, m-minute, s-second, t-ms
// 12y23M34d40h522m6122s14t
// not support y-year, M-month, d-day this version, will be support in future
RtObject XTimeClass::__add__(XObject& instance, const std::vector<RtObject>& args) {
    LONG64* timeData = (LONG64*)instance.m_data;
    const RtObject& right = args.at(0);
    const XObject* other_object = dynamic_cast<const XObject*>(right.getObject());
    if (!other_object) {
        return XTimeClass::newObject(*timeData);
    }
    std::string* other = (std::string*)other_object->m_data;
    TimeDelta timeDelta;
    if (!parse_from_delta_expr(timeDelta, other->c_str())) {
        return XTimeClass::newObject(*timeData);
    }
    LONG64 newTime = *timeData;
    newTime += timeDelta.ms;
    newTime += timeDelta.second * 1000;
    newTime += timeDelta.minute * 60 * 1000;
    newTime += timeDelta.hour * 60 * 60 * 1000;
    // TODO to be support for day, month and year.

    return XTimeClass::newObject(newTime);
}

RtObject XTimeClass::__sub__(XObject& instance, const std::vector<RtObject>& args) {
    LONG64* timeData = (LONG64*)instance.m_data;
    const RtObject& right = args.at(0);
    const XObject* other_object = dynamic_cast<const XObject*>(right.getObject());
    if (!other_object) {
        return XTimeClass::newObject(*timeData);
    }
    std::string* other = (std::string*)other_object->m_data;
    TimeDelta timeDelta;
    if (!parse_from_delta_expr(timeDelta, other->c_str())) {
        return XTimeClass::newObject(*timeData);
    }
    LONG64 newTime = *timeData;
    newTime -= timeDelta.ms;
    newTime -= timeDelta.second * 1000;
    newTime -= timeDelta.minute * 60 * 1000;
    newTime -= timeDelta.hour * 60 * 60 * 1000;
    // TODO to be support for day, month and year.
    
    return XTimeClass::newObject(newTime);

}
RtObject XTimeClass::__eq__(XObject& instance, const std::vector<RtObject>& args) {
    const XObject* other_object = dynamic_cast<const XObject*>(args.at(0).getObject());
    if (!other_object) {
        return false;
    }
    LONG64* my = (LONG64*)instance.m_data;
    LONG64* other = (LONG64*)other_object->m_data;
    if (my == other) {
        return true;
    }
    return *my == *other;
}

RtObject XTimeClass::__lt__(XObject& instance, const std::vector<RtObject>& args) {
    const XObject* other_object = dynamic_cast<const XObject*>(args.at(0).getObject());
    if (!other_object) {
        return false;
    }
    LONG64* my = (LONG64*)instance.m_data;
    LONG64* other = (LONG64*)other_object->m_data;
    return *my < *other;
}

RtObject XTimeClass::__gt__(XObject& instance, const std::vector<RtObject>& args) {
    const XObject* other_object = dynamic_cast<const XObject*>(args.at(0).getObject());
    if (!other_object) {
        return false;
    }
    LONG64* my = (LONG64*)instance.m_data;
    LONG64* other = (LONG64*)other_object->m_data;
    return *my > *other;
}

RtObject XTimeClass::__hashcode__(XObject& instance, const std::vector<RtObject>& args) {
    LONG64* timeData = (LONG64*)instance.m_data;
    return *timeData;

}
