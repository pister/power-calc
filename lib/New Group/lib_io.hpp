//
//  io.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/19.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef lib_io_hpp
#define lib_io_hpp

#include "runtime_types.hpp"
#include "meta_types.hpp"


class XFileClass: public XClass {
public:
    static RtObject newObject(const char* filename, const char* mode);
    static RtObject newObject(const std::string& filename, const std::string& mode);
    virtual void on_destroying(XObject& instance) const;
    static XFileClass* instance() {
        static XFileClass instance;
        return &instance;
    }
private:
    static RtObject __str__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __close__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __read__(XObject& instance, const std::vector<RtObject>& args);
    static RtObject __read_byte__(XObject& instance, const std::vector<RtObject>& args);
private:
    XFileClass() : XClass("file") {
        register_method(FN_STR, __str__);
        register_method("close", __close__);
        register_method("read", __read__);
        register_method("readByte", __read_byte__);

    }
};

#endif /* io_hpp */
