//
//  os.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/19.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "lib_os.hpp"
#include "xboot.hpp"
#include "unisys.hpp"

#include "lib_io.hpp"

static
RtObject os_open(RuntimeContext& runtimeContext) {
    RtObject filename =  runtimeContext.get_variable("filename");
    RtObject mode =  runtimeContext.get_variable("mode");
    return XFileClass::newObject(filename.getStringValue(), mode.getStringValue());
}

void lib_register_os_module(XBoot &boot) {
    NativeMethodModuleCreator creator(boot, OS_MODULE_NAME);
    {
        NativeMethodInfo m;
        m.method = os_open;
        std::vector<std::string> arg_names;
        arg_names.push_back("filename");
        arg_names.push_back("mode");
        m.arg_names = arg_names;
        creator.add_method("Open", m);
    }
    
    // tobe add more methods
    
    boot.register_modules(creator);
}

