//
//  sys_win.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/15.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef sys_win_hpp
#define sys_win_hpp

#ifdef _WIN32

int gettimeofday(struct timeval *tp, void *tzp);

#endif

#endif /* sys_win_hpp */
