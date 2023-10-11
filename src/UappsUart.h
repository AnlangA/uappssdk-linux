/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : UappsUart.h
 * Author      :
 * Date        : 2023-09-12
 * Version     : 1.0
 * Description :
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/

#ifndef _UAPPS_UART_H
#define _UAPPS_UART_H

#ifdef __cplusplus
extern "C"{
#endif

#define UAPPS_MSG_MIN_LEN ESC_LEN + 4
#define LME_COAP_UART_BUF_MAX_SIZE      500


#define lme_uapps_msg_start_check(remainLen, pData) do { \
    while ((remainLen >= 1) && ((pData[0] != ESC_CHAR))) { \
        LOGD("丢弃%02X\n",pData[0]); \
        pData++;\
        remainLen--;\
        continue;\
    }\
} while(0)


#define LME_UAPPS_MSG_SPLIT_CHECK_STATE_NEED_MORE       -1          //need more data input
#define LME_UAPPS_MSG_SPLIT_CHECK_STATE_NOT_FOUND       -2          //SYNC HEAD NOT FOUND.



#define lme_uapps_msg_split_check(c) { \
    if (i > remainLen) {\
        return LME_UAPPS_MSG_SPLIT_CHECK_STATE_NEED_MORE;\
    }\
    if (pData[i++] != c) {\
        continue;\
    }\
}
/*--------------------------------------------------------------
函数名称：UappsSendMsg
函数功能：通过串口发送uapps消息数据。
输入参数：msg-uapps消息结构体；
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
LmeError UappsSendMsg(UappsMessage *msg);
/*--------------------------------------------------------------
函数名称：UappsSendResponse
函数功能：根据请求req创建Uapps响应报文结构体，并通过串口发送出去。
输入参数：type-响应消息类型，一般为ACK；
         hdrCode-Uapps报文头的code码
         fmt-数据格式L；
         payload_buf-数据载荷内容；
         len-数据载荷长度，字节为单位；
         opt-uapps选项数组，需要按选项码从小到大排列；
         opsNum-选项数量；
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
LmeError UappsSendResponse(UappsMessage *req, u8 type, u8 hdrCode, u16 fmt, u8 *payload_buf, U32 len, UappsOption *opt, U32 opsNum);

#ifdef __cplusplus
}
#endif

#endif /* _UAPPS_UART_H */


