//
//  xboot.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/1/6.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "xboot.hpp"
#include "ProgramParser.hpp"
#include "ast_runtime.hpp"
#include "lexer_parser.hpp"
#include "meta_types.hpp"

RtObject XBoot::eval(InputStream* source) {
    Lexer lexer(source);
    ProgramParser programParser(&lexer);
    std::shared_ptr<ASTTree> ast = programParser.root_program();
    return eval_ast(ast, &m_globalRuntimeContext);
}

void XBoot::register_modules(const NativeMethodModuleCreator& nativeMethodModuleCreator) {
    auto moduleDef = new XMapModuleDef(nativeMethodModuleCreator.getModuleDef());
    RtObject modules = XModuleClass::newObject(m_globalRuntimeContext, moduleDef);
    m_context.register_module(moduleDef->m_name, modules);
}

NativeMethodModuleCreator::NativeMethodModuleCreator(XBoot& boot, const std::string& name) {
    std::map<std::string, NativeMethodInfo> native_methods;
    m_mapModuleDef = new XMapModuleDef(boot.getGlobalRuntimeContext(), name, native_methods);
}

void NativeMethodModuleCreator::add_method(const std::string& name, const NativeMethodInfo& native_method) {
    m_mapModuleDef->add_native_method(name, native_method);
}

NativeMethodModuleCreator::~NativeMethodModuleCreator(){
    if (m_mapModuleDef) {
        delete m_mapModuleDef;
        m_mapModuleDef = NULL;
    }
}


