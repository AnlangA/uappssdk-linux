/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved��
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC��
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code��
 *
 * FileName    : lme_timer.cpp
 * Author      :
 * Date        : 2022年8月24日
 * Version     : 1.0
 * Description : 
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/
#include "lme_timer.h"


int lme_time_diff(struct timeval *start, struct timeval *end)
{
    if (start == NULL || end == NULL)
    {
        return 0;
    }
    
    return  (end->tv_sec - start->tv_sec) * 1000 + (end->tv_usec - start->tv_usec) / 1000;
}


char * lme_get_time_str()
{
    size_t n = 0;
    struct timeval tv;
    struct tm* ptm;
    static char time_string[40] = {0};
    
    gettimeofday(&tv, NULL);
    ptm = localtime (&(tv.tv_sec));
    n = strftime (time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);  //输出格式为: 2022-03-30 20:38:37
    snprintf (time_string + n, sizeof(time_string) - n, ":%03ld", tv.tv_usec / 1000);            
    return time_string;
}

