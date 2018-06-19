//
//  main.cpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/20.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#include <iostream>
#include "xboot.hpp"

#include "lib_time.hpp"
#include "lib_os.hpp"



static RtObject the_addr(RuntimeContext& runtimeContext) {
    RtObject a =  runtimeContext.get_variable("a");
    RtObject b =  runtimeContext.get_variable("b");
    return a.rt_add(b);
}

void x1() {
    try {
        // FileInputStream in("/Users/songlihuang/Documents/mine/temp/calc.txt");
       //FileInputStream in("/Users/songlihuang/Documents/mine/temp/compute_pi.txt");
        //FileInputStream in("/Users/songlihuang/Documents/mine/temp/test_1.txt");
       FileInputStream in("/Users/songlihuang/Documents/mine/temp/hello.txt");
        //FileInputStream in("/Users/songlihuang/Documents/mine/temp/r9_9.txt");
        XBoot boot;
        NativeMethodModuleCreator creator(boot, "test_module");
        {
            NativeMethodInfo m;
            m.method = the_addr;
            std::vector<std::string> arg_names;
            arg_names.push_back("a");
            arg_names.push_back("b");
            m.arg_names = arg_names;
            creator.add_method("Adder", m);
        }
        boot.register_modules(creator);
        
        
        lib_register_time_module(boot);
        lib_register_os_module(boot);

        
        boot.add_path("/Users/songlihuang/Documents/mine/temp");
        boot.eval(&in);
    } catch (const std::string& e) {
        printf("error: %s\n", e.c_str());
    }
}

void x2() {
    try {
        FileInputStream in("/Users/songlihuang/Documents/mine/temp/hello.txt");
        Lexer lexer(&in);
        shared_ptr<Token> token(new Token);
        while (lexer.readNext(token)) {
            printf("[%zu:%zu] %d: %s\n", token->m_line, token->m_column, token->getType(), token->getString() );
        }
        printf("~~DONE!");
    } catch (const std::string& e) {
        printf("error: %s\n", e.c_str());
    }
}

void test_peek() {
    try {
        FileInputStream in("/Users/songlihuang/Documents/mine/temp/hello.txt");
        Lexer lexer(&in);
        shared_ptr<Token> token(new Token);
        for (int i = 1; i <= 3; i++) {
            lexer.peek(token, i);
            printf("peek[%zu:%zu] %d: %s\n", token->m_line, token->m_column, token->getType(), token->getString() );
        }
        lexer.readNext(token);
        printf("[%zu:%zu] %d: %s\n", token->m_line, token->m_column, token->getType(), token->getString() );
        for (int i = 1; i <= 10; i++) {
            lexer.peek(token, i);
            printf("peek[%zu:%zu] %d: %s\n", token->m_line, token->m_column, token->getType(), token->getString() );
        }
        while (lexer.readNext(token)) {
            printf("[%zu:%zu] %d: %s\n", token->m_line, token->m_column, token->getType(), token->getString() );
        }
    } catch (const std::string& e) {
        printf("error: %s\n", e.c_str());
    }
}

static int run_main(int argc, const char * argv[]) {
    if (argc <= 1) {
        printf("need a file input.\n");
        return 1;
    }
    const char* filename;
    bool print_token_only;
    if (argc >= 3 && std::string("-t") == argv[1]) {
        print_token_only = true;
        filename = argv[2];
    } else {
        print_token_only = false;
        filename = argv[1];
    }
    FileInputStream in(filename);
    if (print_token_only) {
        Lexer lexer(&in);
        shared_ptr<Token> token(new Token);
        while (lexer.readNext(token)) {
            printf("%d: %s\n", token->getType(), token->getString() );
        }
    } else {
        try {
            XBoot boot;
            boot.eval(&in);
        } catch (const std::string& e) {
            printf("error: %s\n", e.c_str());
        }
    }
    return 0;
}

#include "UnicodeString.hpp"

void test_unicode() {
    const char* s = "hello";
    auto p = UnicodeString::decodeByUTF8(s, strlen(s));
    printf("done\n");
}

int main(int argc, const char * argv[]) {
   // setlocale(LC_ALL, "");
   // test_unicode();
    //x3();
    //x2();
 //   x1();
    //x4();
    //test_peek();
    //return run_main(argc, argv);
    x1();
}
