//
//  unisys.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/19.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef unisys_hpp
#define unisys_hpp

#include "xbase.hpp"

int unisys_gettimeofday(struct timeval *tp, void *tzp);
void unisys_sleep(LONG64 ms);

#endif /* unisys_hpp */
