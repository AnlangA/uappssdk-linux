/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * FileName    : lme_timer.h
 * Author      :
 * Date        : 2022年8月24日
 * Version     : 1.0
 * Description : 
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/

#ifndef JNI_APP_SRC_LME_OS_ADPTER_LME_TIMER_H_
#define JNI_APP_SRC_LME_OS_ADPTER_LME_TIMER_H_
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>


#ifdef __cplusplus
extern "C"{
#endif

int lme_time_diff(struct timeval *start, struct timeval *end);

char * lme_get_time_str();

#ifdef __cplusplus
}
#endif

#endif /* JNI_APP_SRC_LME_OS_ADPTER_LME_TIMER_H_ */
