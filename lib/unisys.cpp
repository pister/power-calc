//
//  unisys.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/19.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "unisys.hpp"
#include <time.h>

#ifdef _WIN32
#include <windows.h>

int unisys_gettimeofday(struct timeval *tp, void *tzp) {
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year   = wtm.wYear - 1900;
    tm.tm_mon   = wtm.wMonth - 1;
    tm.tm_mday   = wtm.wDay;
    tm.tm_hour   = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm. tm_isdst  = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return 0;
}

void unisys_sleep(LONG64 ms) {
    Sleep(ms);
}

#else

#include <sys/time.h>
#include <unistd.h>

int unisys_gettimeofday(struct timeval *tp, void *tzp) {
    return gettimeofday(tp, tzp);
}

void unisys_sleep(LONG64 ms) {
    usleep((unsigned int)ms * 1000);
}

#endif