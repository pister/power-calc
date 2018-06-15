//
//  xboot.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/1/6.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef xboot_hpp
#define xboot_hpp

#include <map>
#include <string>
#include <vector>
#include "runtime_types.hpp"
#include "ast_runtime.hpp"
#include "lexer_parser.hpp"
#include "meta_types.hpp"

using std::string;
using std::map;
using std::vector;

class XBootContext {
public:
    XBootContext() {
        add_path(".");
    }
    void add_path(const std::string& path) {
        m_paths.push_back(path);
    }
    const std::vector<std::string>& get_paths() const {
        return m_paths;
    }
    const bool has_module(const std::string& name) const {
        auto it = m_loadedModules.find(name);
        if (it == m_loadedModules.end()) {
            return false;
        }
        return true;
    }
    const RtObject get_module(const std::string& name) const {
        auto it = m_loadedModules.find(name);
        if (it == m_loadedModules.end()) {
            return RtObject::Null;
        }
        return it->second;
    }
    const RtObject get_register_module(const std::string& name) const {
        auto it = m_registedModules.find(name);
        if (it == m_registedModules.end()) {
            return RtObject::Null;
        }
        return it->second;
    }
    void register_module(const std::string& name, const RtObject& obj) {
        m_registedModules[name] = obj;
    }
private:
    std::vector<std::string> m_paths;
    std::map<std::string, RtObject> m_loadedModules;
    std::map<std::string, RtObject> m_registedModules;
};

class XBoot;

class NativeMethodModuleCreator : private NoncopyAble {
public:
    NativeMethodModuleCreator(XBoot& boot, const std::string& name);
    void add_method(const std::string& name, const NativeMethodInfo& native_method);
    ~NativeMethodModuleCreator();
    XMapModuleDef& getModuleDef() const {
        return *m_mapModuleDef;
    }
private:
    XMapModuleDef *m_mapModuleDef;
};

class XBoot {
public:
    XBoot() : m_globalRuntimeContext(&m_context) {}
    RtObject eval(InputStream* source);
    const XBootContext& get_context() const {
        return m_context;
    }
    void add_path(const std::string& path) {
        m_context.add_path(path);
    }
    void register_modules(const NativeMethodModuleCreator& nativeMethodModuleCreator);
    GlobalRuntimeContext& getGlobalRuntimeContext() {
        return m_globalRuntimeContext;
    }
private:
    XBootContext m_context;
    GlobalRuntimeContext m_globalRuntimeContext;
};




#endif /* xboot_hpp */
