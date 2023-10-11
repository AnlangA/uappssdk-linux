/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : UappsUart.c
 * Author      :
 * Date        : 2023-09-12
 * Verslon     : 1.0
 * Description : APS层的Uapps报文处理的源文件
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/
#include "Common.h"
#include "lme_tool.h"
#include "lme_timer.h"
#include "lme_uart.h"
#include "Uapps.h"
#include "UappsUart.h"

#ifdef __cplusplus
extern "C"{
#endif

int lme_uapps_create_uart_buf(UappsMessage * msg, u8 **out, U32 max_len)
{
    u8 *uartbuff = NULL;
    U32 len = 0;
    
    LOGD("enter, msg:%p\n", msg);

    if (max_len > LME_COAP_UART_BUF_MAX_SIZE)
    {
        return -1;
    }
    uartbuff =(u8 *) Lme_malloc(max_len);
    if (uartbuff == NULL)
    {
        return -1;
    }
    memset(uartbuff, 0, max_len);
    memcpy(uartbuff, ESC_STRING, ESC_LEN);
    LOGD("lme_coap_create_uart_buf here, max_len:%d\n", max_len);
    len = Uapps2Bytes(msg, uartbuff + ESC_LEN);
    LOGD("lme_coap_create_uart_buf here\n");
    len = ESC_LEN + len;
    *out = uartbuff;
    LOGD("exit\n");
    return len;
}


LmeError UappsSendMsg(UappsMessage *msg)
{
    BYTE *buf = NULL;
    int len = 0;
    
    len = lme_uapps_create_uart_buf(msg, &buf, LME_COAP_UART_BUF_MAX_SIZE);
    if (len < 0)
    {
        LOGE("lme_uapps_create_uart_buf fail\n");
        return LME_ERROR;
    }
    
    lme_uart_send(buf, len);
    Lme_free(buf);
}

LmeError UappsSendResponse(UappsMessage *req, u8 type, u8 hdrCode, u16 fmt, u8 *payload_buf, U32 len, UappsOption *opt, U32 opsNum)
{
    UappsMessage respMsg = {0};
    LmeError lerr = LME_OK;
    int i = 0;
   
    UappsDataResponse(req, &respMsg, type, hdrCode, payload_buf, len, fmt, 0);
    if (payload_buf != NULL)
    {
        UappsPutData(&respMsg, payload_buf, len, fmt, 0);
    }

    if (opt != NULL && opsNum > 0)
    {
        for (i = 0; i < opsNum; i++)
        {
            UappsPutOption(&respMsg, opt[i].opt_code, opt[i].opt_val, opt[i].opt_len);
        }
    }
    
    lerr = UappsSendMsg(&respMsg);
    return lerr;
}



#ifdef __cplusplus
}
#endif


