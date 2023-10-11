/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : lme_uart.h
 * Author      :
 * Date        : 2020-05-09
 * Version     : 1.0
 * Description :
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/

#ifndef JNI_APP_SRC_LME_OS_ADPTER_LME_UART_H_
#define JNI_APP_SRC_LME_OS_ADPTER_LME_UART_H_

#ifdef __cplusplus
extern "C"{
#endif

#define LME_UART_FILE_NAME "/dev/ttyS1"

/**
 * 功能：解析协议
 * 参数：pData 协议数据，len 数据长度
 * 返回值：实际解析协议的长度
 */
typedef int (*lme_on_message)(const u8 *pData, U32 len);


LmeError lme_uart_send(const u8 *pData, U32 len);

LmeError lme_uart_open(const char *pFileName, U32 baudRate, lme_on_message callback);
LmeError lme_uart_close();

int lme_uart_is_open(void);

LmeError lme_uart_wait_exit();

#ifdef __cplusplus
}
#endif

#endif /* JNI_APP_SRC_LME_OS_ADPTER_LME_UART_H_ */
