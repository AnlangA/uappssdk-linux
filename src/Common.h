/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved��
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC��
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code��
 *
 * FileName    : Common.h
 * Author      :
 * Date        : 2023-09-13
 * Version     : 1.0
 * Description : 
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/
#ifndef __COMMON_H_
#define __COMMON_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug_printf.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned char BYTE;
typedef unsigned int   UINT;
#define FALSE					0
#define TRUE 					1

#define URI_HOST_LEN	20	// Max uncoded str length
//#define URI_PATH_LEN	20	//
#define URI_QUERY_LEN	20

typedef enum {
	LME_OK			=  0,
	LME_ERROR	    = -1, // general error
    LME_INVALID     = -2,
    LME_MEM_ERROR   = -3,
    LME_EXIST_ERROR = -4,
    LME_NOT_SUPPORT = -5,
	LME_REGITERED	= -6,
} LmeError;


#define Lme_free(ptr)               	free(ptr)

#define Lme_malloc(req_size)        	malloc(req_size)
typedef void (*buf_free_cb)(void *);

#define WIN32

#endif
