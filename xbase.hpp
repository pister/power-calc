//
//  xbase.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/1/6.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef xbase_hpp
#define xbase_hpp

#if defined(_WIN32) || defined( __CYGWIN__)
#define FILE_PATH_SEP '\\'
#else
#define FILE_PATH_SEP '/'
#endif

#define MODULE_EXT ".ybc"


#ifdef WIN32
typedef __int64 LONG64;
#else
typedef long long LONG64;
#endif


#endif /* xbase_hpp */
