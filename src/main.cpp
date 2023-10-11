/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : main.cpp
 * Author      :
 * Date        : 2020-05-09
 * Version     : 1.0
 * Description :
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <memory.h>
#include <termio.h>
#include <sys/ioctl.h>
#include "Common.h"
#include "lme_tool.h"
#include "lme_timer.h"
#include "lme_uart.h"
#include "debug_printf.h"
#include "Uapps.h"
#include "UappsUart.h"

typedef struct {
    char did[25];
}SubDevice;


unsigned int sendCnt = 0;
unsigned int sendFailCnt = 0;

unsigned int recvCnt = 0;
unsigned int recvFailCnt = 0;  

UappsMessage msg = {0};
UappsMessage *pMsg = NULL;


typedef struct {
    UappsMessage msg;
    u8 state;
    char rsl[96];
    u8 type;
    u8 code;
    u8 fmt;
    u8 hasPayload;
    u8 payload[256];
}testData;

testData testMap[] = 
{
    {{0}, 0, "k1@200.4c5a000e8d05/_close", UAPPS_TYPE_CON, UAPPS_REQ_POST, 0, 0, {0}},
    {{0}, 0, "k1@200.4c5a000e8d05/_open", UAPPS_TYPE_CON, UAPPS_REQ_POST, 0, 0, {0}},
    {{0}, 0, "@0./104/4", UAPPS_TYPE_CON, UAPPS_REQ_GET, 0, 0, {0}},
    {{0}, 0, "@0./_mac", UAPPS_TYPE_CON, UAPPS_REQ_GET, 0, 0, {0}},
    {{0}, 0, "@0./_ver", UAPPS_TYPE_CON, UAPPS_REQ_GET, 0, 0, {0}},
};

LmeError lme_uapps_msg_response_process(UappsMessage *msg)
{
    u8 match = 0;
    int i = 0;
    
    if (pMsg != NULL)
    {
     match = UappsMatchMessage(pMsg, msg); 
    if (match == 1)
    {
        LOGD("response match!\n");
        Lme_free(pMsg);
        pMsg = NULL;
        return LME_OK; 
    }
    }
    

    else
    {
        for (i  = 0; i < (sizeof(testMap)/ sizeof(testMap[0])); i++)
        {
            match = UappsMatchMessage(&testMap[i].msg, msg);
            if (match == 1)
            {
                LOGD("response match, rsl:%s!\n", testMap[i].rsl);
                testMap[i].state = 1;
                break;
            }
        }
    }
    if (msg->pl_ptr)
    {
        PrintHexBytesInline("response data", msg->pl_ptr, msg->pl_len, 1);
    }
    return LME_OK;
}


SubDevice * lme_find_sub_dev(const char *did)
{
    return NULL;
}


LmeError lme_uapps_msg_process(const u8 *pData, U32 len)
{
    UappsMessage *msg = NULL;
    UappsMessage respMsg = {0};
    UappsError uerr = UAPPS_OK;
    SubDevice * sub_dev = NULL;
    RSL_t rsl = {0};
    LmeError lerr = LME_OK;

    LOGD("recv msg, len:%d\n", len);
    msg = (UappsMessage *)Lme_malloc(sizeof(UappsMessage));
    if (msg == NULL)
    {
        LOGE("UappsMessage maloc fail\n");
        return LME_ERROR;
    }
    
    memset(msg, 0, sizeof(UappsMessage));
    uerr = UappsFromBytes((u8 *)pData, len, msg);
    if (uerr != UAPPS_OK)
    {
        LOGE("UappsFromBytes fail\n");
        Lme_free(msg);
        return LME_ERROR;
    }
    if (UAPPS_MSG_IS_REQUEST(msg))
    {
        uerr = UappsGetRsl(msg, &rsl);
        if (uerr != UAPPS_OK)
        {
            LOGE("UappsGetRsl err\n");
            Lme_free(msg);
            return LME_ERROR;
        }
        if (UAPPS_OK == UappsGetNodaFromOptionFROM(msg, rsl.noda))
        {
            LOGD("using noda from from option [mac:%s]\n", rsl.noda);
        }

        LOGD("real device [mac:%s]\n", rsl.noda);
        sub_dev = lme_find_sub_dev(rsl.noda);
        //process request.
        if (msg->pl_ptr)
        {
            PrintHexBytesInline("request data", msg->pl_ptr, msg->pl_len, 1);
        }
        #if 0
        UappsSimpleResponse(msg, &respMsg, UAPPS_TYPE_ACK, UAPPS_ACK_CREATED);
        lerr = UappsSendMsg(&respMsg);
        #else
        char payload[] = "Created.";
        lerr = UappsSendResponse(msg, UAPPS_TYPE_ACK, UAPPS_ACK_CREATED, UAPPS_FMT_TEXT, (u8 *)payload, strlen(payload), NULL, 0);
        #endif
        if (lerr == LME_OK)
        {
            sendCnt++;
            LOGD("UappsSendMsg send count:%d\n", sendCnt);
        }
        else
        {
            sendFailCnt++;
            LOGD("UappsSendMsg send count:%d\n", sendFailCnt);
        }

    }
    else if (UAPPS_MSG_IS_RESPONSE(msg))
    {
        lerr = lme_uapps_msg_response_process(msg);
    }
    else
    {
        LOGD("unknown code:%d\n", UAPPS_MSG_CODE(msg));
    }
        if(msg != NULL) {
            LOGD("dd before delete\n");
        	Lme_free(msg);
        	msg = NULL;
        }
    return lerr;
}


int  lme_uapps_msg_split(const u8 *pData, U32 remainLen)
{
    for (U32 i = 0; i < remainLen; i++)
    {
        if (pData[i] == ESC_CHAR)
        {
            i++;
            lme_uapps_msg_split_check(ESC_CHAR);
            lme_uapps_msg_split_check(ESC_CHAR);
            lme_uapps_msg_split_check(ESC_CHAR);
            lme_uapps_msg_split_check(ESC_CHAR);
            lme_uapps_msg_split_check('\n');
            lme_uapps_msg_split_check(ESC_CHAR);
            lme_uapps_msg_split_check('\n');
            return i - ESC_LEN;
        }
    }
    return LME_UAPPS_MSG_SPLIT_CHECK_STATE_NOT_FOUND;
}

static inline void lme_uapps_recv_success_count()
{
    recvCnt++;
    LOGD("lme_uapps_msg_process success recvCnt:%d\n", recvCnt);
}

static inline void lme_uapps_recv_fail_count()
{
    recvFailCnt++;
    LOGD("lme_uapps_msg_process fail recvCnt:%d\n", recvFailCnt);
}


int lme_uapps_decoder(const u8 *pData, U32 len)
{
    LmeError lerr = LME_OK;
    U32 remainLen = len;   // 剩余数据长度
    U32 frameLen;  // 帧长度
    int splitPos = 0;

    /**
     * 以下部分需要根据协议格式进行相应的修改，解析出每一帧的数据
     */
    while (remainLen >= UAPPS_MSG_MIN_LEN) {
        // 找到一帧数据的数据头
        lme_uapps_msg_start_check(remainLen, pData);

        if (remainLen < UAPPS_MSG_MIN_LEN) {
            break;
        }
        //remove sync head
        pData += ESC_LEN;
        remainLen -= ESC_LEN;
        splitPos = lme_uapps_msg_split(pData, remainLen);
        if (splitPos > 0)
        {
            LOGD("2+ msg found, split at:%d, remain len:%d\n", splitPos, remainLen - splitPos);
            frameLen = splitPos;
        }
        else
        {
            if (splitPos == LME_UAPPS_MSG_SPLIT_CHECK_STATE_NEED_MORE)
            {
                LOGD("more data needed, remain len:%d\n", remainLen);
                return remainLen;
            }
            else if (splitPos == LME_UAPPS_MSG_SPLIT_CHECK_STATE_NOT_FOUND)
            {
                frameLen = remainLen;
                LOGD("no split found, remain len:%d\n", remainLen);
            }
            else
            {
                //TBD: check right return value.
                return len;
            }
        }
        // 打印一帧数据，需要时在CommDef.h文件中打开DEBUG_PRO_DATA宏
#if 1//def DEBUG_PRO_DATA
        PrintHexBytesInline("UAPPS msg:", (unsigned char *)pData, frameLen, 1);
#endif

        lerr = lme_uapps_msg_process(pData, frameLen);
        if (lerr == LME_OK)
        {
            lme_uapps_recv_success_count();
        }
        else
        {
            lme_uapps_recv_fail_count();
        }
        pData += frameLen;
        remainLen -= frameLen;
    }

    return len - remainLen;
}




int main()
{
    LmeError lerr = LME_OK;
    char rslStr[] = "k1@200.4c5a000e8d05/_close";
    char rslStr_104_4[] = "@0./104/4";
    const char* switch_op[] {"k1@200.4c5a000e8d05/_close","k1@200.4c5a000e8d05/_open"};
    char payload[] = "hello Uapps!";
    int i = 0;
    testData *testCase = NULL;
    
    if (lme_uart_open(LME_UART_FILE_NAME, B115200, lme_uapps_decoder)!= LME_OK)
    {
        return LME_ERROR;
    }

    for (i  = 0; ; i = (++i) % (sizeof(testMap)/ sizeof(testMap[0])))
    {
        testCase = &testMap[i];
        LOGD("testCase rsl:%s state:%d\n", testCase->rsl, testCase->state);
        //switch device interface test, plz change the rslStr for other interface test. responses go to lme_uapps_msg_response_process.
        memset(&testCase->msg, 0, sizeof(UappsMessage));
        UappsCreateMessage(&testCase->msg, testCase->type, testCase->code, testCase->rsl);
        lerr = UappsSendMsg(&testCase->msg);
        if (lerr == LME_OK)
        {
            sendCnt++;
            LOGD("UappsSendMsg send count:%d\n", sendCnt);
        }
        else
        {
            sendFailCnt++;
            LOGD("UappsSendMsg send count:%d\n", sendFailCnt);
        }
        usleep(4000000);
    
    #if 0
        //switch device interface test, plz change the rslStr for other interface test. responses go to lme_uapps_msg_response_process.
        memset(&msg, 0, sizeof(msg));
        UappsCreateMessage(&msg, UAPPS_TYPE_CON, UAPPS_REQ_GET, (char *)switch_op[sendCnt % 2]);
        lerr = UappsSendMsg(&msg);
        if (lerr == LME_OK)
        {
            sendCnt++;
            LOGD("UappsSendMsg send count:%d\n", sendCnt);
        }
        else
        {
            sendFailCnt++;
            LOGD("UappsSendMsg send count:%d\n", sendFailCnt);
        }
        usleep(4000000);
    #endif
    
    #if 0
        //loop test
        if (pMsg == NULL)
        {
            pMsg = UappsBuildReq(rslStr_104_4, UAPPS_TYPE_CON, UAPPS_REQ_POST, payload, strlen(payload));
            if (pMsg != NULL)
            {
                lerr = UappsSendMsg(pMsg);
                if (lerr == LME_OK)
                {
                    sendCnt++;
                    LOGD("UappsSendMsg send count:%d\n", sendCnt);
                }
                else
                {
                    sendFailCnt++;
                    LOGD("UappsSendMsg send count:%d\n", sendFailCnt);
                }
            }
            else
            {
                LOGD("UappsBuildReq fail:\n");
            }
        }

        usleep(4000000);
        #endif
    }

    lme_uart_wait_exit();
}
