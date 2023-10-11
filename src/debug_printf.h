/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : debug_printf.h
 * Author      :
 * Date        : 2023-09-12
 * Verslon     : 1.0
 * Description : APS层的Uapps报文处理的源文件
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/

#ifndef _DEBUG_PRINTF__
#define _DEBUG_PRINTF__

#ifdef __cplusplus
extern "C"{
#endif


typedef enum {
    LME_LOG_TRACE               = 0,
    LME_LOG_DEBUG               = 1,
    LME_LME_LOG_INFO            = 2,
    LME_LOG_WARN                = 3,
    LME_LOG_ERR                 = 4,
    LME_LOG_FATAL               = 5,
}lme_gw_event_E;
    
int lme_gateway_log(lme_gw_event_E e, const char * file, const char * func, unsigned int line, const char * fmt, ...);

#define LOGE(fmt,...)	lme_gateway_log(LME_LOG_ERR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGD(fmt,...) 	lme_gateway_log(LME_LOG_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//#define LOGE(fmt,...)	fprintf(stderr, fmt, ##__VA_ARGS__)
//#define LOGD(fmt,...) 	fprintf(stderr, fmt, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif
 
#endif
