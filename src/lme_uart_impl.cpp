/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : lme_uart_impl.cpp
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
#include <errno.h>
#include <sys/ioctl.h>
#include "Common.h"  
#include "lme_tool.h"
#include "lme_timer.h"
#include "lme_uart.h"
#include "debug_printf.h"
#include <deque>
#include "SendBufNode.h"

typedef std::deque<SendBufNode *> sendBufQ;

#define UART_DATA_BUF_LEN           16384   // 16KB


#define LME_PLC_SEND_WAIT_INTERVAL      (360)
#define LME_PLC_SEND_BUF_QUEUE_SIZE     (30)

typedef std::deque<SendBufNode *> sendBufQ;
/**
 * 功能：解析协议
 * 参数：pData 协议数据，len 数据长度
 * 返回值：实际解析协议的长度
 */
typedef int (*protocol_decoder)(const BYTE *pData, UINT len);

extern int parseProtocol(const BYTE *pData, UINT len);


lme_on_message decoder_cb = NULL;

pthread_t th;


static int lme_protocol_decoder_adapter(const BYTE *pData, UINT len);

protocol_decoder m_decoder = lme_protocol_decoder_adapter;

pthread_mutex_t lock;

struct timeval m_lastSentTime;

sendBufQ m_sendQ;

unsigned int readFailCount = 0;

unsigned int m_maxSendWait = LME_PLC_SEND_WAIT_INTERVAL;

bool mIsOpen = false;

int mUartID = 0;

BYTE mDataBufPtr[UART_DATA_BUF_LEN] = {0};

int mDataBufLen = 0;




static const char* getBaudRate(UINT baudRate) {
    struct {
        UINT baud;
        const char *pBaudStr;
    } baudInfoTab[] = {
        { B1200, "B1200" },
        { B2400, "B2400" },
        { B4800, "B4800" },
        { B9600, "B9600" },
        { B19200, "B19200" },
        { B38400, "B38400" },
        { B57600, "B57600" },
        { B115200, "B115200" },
        { B230400, "B230400" },
        { B460800, "B460800" },
        { B921600, "B921600" }
    };

    int len = sizeof(baudInfoTab) / sizeof(baudInfoTab[0]);
    for (int i = 0; i < len; ++i) {
        if (baudInfoTab[i].baud == baudRate) {
            return baudInfoTab[i].pBaudStr;
        }
    }

    return NULL;
}

inline void setMaxWaitTime(unsigned int ms)
{
    m_maxSendWait = ms;
}
inline unsigned int getMaxWaitTime()
{
    return m_maxSendWait;
}


int sendBufQueueAppend(BYTE *buf, unsigned len)
{
    SendBufNode * node = new SendBufNode(buf, len);
    if (node == NULL)
    {
        return -1;
    }

    if (m_sendQ.size() == LME_PLC_SEND_BUF_QUEUE_SIZE)
    {
        SendBufNode * head = m_sendQ[0];
        PrintHexBytesInline("queue full! drop data:", (unsigned char *)head->m_buf, head->m_len, TRUE);
        m_sendQ.pop_front();
        if (head != NULL)
        {
            LOGD("here\n");
            delete head;
        }
    }
    m_sendQ.push_back(node);
    return 0;
}

int writeData(const BYTE *pData, UINT len)
{
    if (write(mUartID, pData, len) != (int) len) {  // fail
        LOGD("uart_send Fail\n");
        return -1;
    }
    gettimeofday(&m_lastSentTime, NULL);

    PrintHexBytesInline("send data:", (unsigned char *)pData, len, TRUE);
    return 0;
}

static inline int canSend(struct timeval *start, struct timeval *end)
{
    return  (end->tv_sec - start->tv_sec) * 1000 + (end->tv_usec - start->tv_usec) /1000 - getMaxWaitTime();

}
int sendBufQueueTrySend()
{
    SendBufNode * head = NULL;
    int timeout = 0;
    int ret = 0;
    struct timeval tv;
    pthread_mutex_lock(&lock);
    if (m_sendQ.size() == 0)
    {
        pthread_mutex_unlock(&lock);
        return 0;
    }
    gettimeofday(&tv, NULL);
    timeout = canSend(&m_lastSentTime, &tv);
    if (timeout >= 0)
    {
        do {
            head = m_sendQ[0];
            if (head == NULL)
            {
                pthread_mutex_unlock(&lock);
                return -1;
            }
            if (lme_time_diff(&head->m_time, &tv) > 2000)
            {
                m_sendQ.pop_front();
                LOGD("too late [%d] ms! \n", lme_time_diff(&head->m_time, &tv));
                PrintHexBytesInline("drop data:",  (unsigned char *)head->m_buf, head->m_len, TRUE);
                delete head;
                head = NULL;
            }
            else
            {
                break;
            }
        }while(m_sendQ.size() > 0);

        if (head != NULL)
        {
            ret = writeData(head->m_buf, head->m_len);
            if (ret >= 0)
            {
                m_sendQ.pop_front();
                delete head;
                ret = 0;
            }
            else
            {
                ret = timeout;
            }
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        ret = 0;
    }
    pthread_mutex_unlock(&lock);
    return ret;
}

bool threadLoop() {
    int readNum = -1;
    int wait_flag = 0;
    unsigned int newRead = 0;
    fd_set rd;
    struct timeval tv;
    int err;
    if (mIsOpen) {
    // 可能上一次解析后有残留数据，需要拼接起来
        sendBufQueueTrySend();
        do {
            //pthread_mutex_lock(&lock);
            readNum = read(mUartID, mDataBufPtr + mDataBufLen, UART_DATA_BUF_LEN - mDataBufLen);
            //pthread_mutex_unlock(&lock);
            if (readNum > 0)
            {
                PrintHexBytesInline("recv new data:", mDataBufPtr + mDataBufLen, readNum, 1);
                mDataBufLen += readNum;
                newRead += readNum;
                usleep(500);
                if (mDataBufLen > 450)
                {
                    //data is enough to process.
                    break;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                FD_ZERO(&rd);
                FD_SET(mUartID,&rd);
                
                tv.tv_sec = 0;
                tv.tv_usec = 8000;
                err = select(mUartID + 1,&rd,NULL,NULL,&tv);
                if(err == 0) //超时
                {
                    //printf("select time out!\n");
                    break;
                }
                else if(err == -1)  //失败
                {
                    break;
                }
                else  //成功
                {
                    continue;
                }
            }
        }while (1);

        if (mDataBufLen > 0 && newRead > 0) {
            // 解析协议
            int len = m_decoder(mDataBufPtr, mDataBufLen);
            if ((len > 0) && (len <= mDataBufLen)) {
                // 将未解析的数据移到头部
                memcpy(mDataBufPtr, mDataBufPtr + len, mDataBufLen - len);
            }
            else
            {
                newRead = 0;
            }

            mDataBufLen -= len;
        } else {
            readFailCount++;
            if (readFailCount % 5000 == 1)
            {
                LOGE("read fail no data, fails:%d.\n", readFailCount);
            }
            usleep(1000);
        }

        return true;
    }
    LOGE("uart stoped.\n");
    return false;
}


/**
 * 功能：解析协议
 * 参数：pData 协议数据，len 数据长度
 * 返回值：实际解析协议的长度
 */
static int lme_protocol_decoder_adapter(const BYTE *pData, UINT len)
{
    PrintHexBytesInline("uart recv data:", (unsigned char *)pData, len, 0);
    if (decoder_cb)
    {
        return decoder_cb(pData, len);
    }
    return len;
}


int lme_uart_is_open(void){
    return mIsOpen == true;
}

   
void* lme_uart_task(void* args){
    bool again = false;
    LOGD("lme_uart_task start\n");
    
    mIsOpen = true;
    readFailCount = 0;
    mDataBufLen = 0;
    pthread_mutex_init(&lock, NULL);
    memset(&m_lastSentTime, 0,sizeof(m_lastSentTime));

    do
    {
        again = threadLoop();
    }while(again);
    LOGE("lme_uart_task cancel\n");
    return NULL;
}

LmeError lme_uart_open(const char *pFileName, UINT baudRate, lme_on_message callback)
{
    LmeError lerr = LME_OK;
    int ret  = 0;

    if (mIsOpen == true)
    {
        LOGD("lme_uart_open is already open, plz close it.\n");
        return LME_ERROR;
    }

    if (callback == NULL)
    {
        return LME_ERROR;
    }
    
    LOGD("openUart pFileName = %s, baudRate = %s\n", pFileName, getBaudRate(baudRate));
    mUartID = open(pFileName, O_RDWR|O_NOCTTY|O_NONBLOCK | FNDELAY);


    if (mUartID <= 0) {
        mIsOpen = false;
        LOGD("lme_uart_open fail, err:%s.\n", strerror(errno));
        return LME_ERROR;
    } else {
        struct termios oldtio = { 0 };
        struct termios newtio = { 0 };
        tcgetattr(mUartID, &oldtio);

        newtio.c_cflag = baudRate|CS8|CLOCAL|CREAD;
        newtio.c_iflag = 0; // IGNPAR | ICRNL
        newtio.c_oflag = 0;
        newtio.c_lflag = 0; // ICANON
        newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
        newtio.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */
        tcflush(mUartID, TCIOFLUSH);
        tcsetattr(mUartID, TCSANOW, &newtio);

        // 设置为非阻塞
        //fcntl(mUartID, F_SETFL, O_NONBLOCK | FNDELAY);
        //fcntl(mUartID, F_SETFL, 0);
        decoder_cb = callback;
        ret = pthread_create(&th, NULL, lme_uart_task, NULL);

        if (ret  == 0)
        {
            LOGD("openUart mIsOpen = %d\n", mIsOpen);
            
        }
        else
        {
            close(mUartID);
            mUartID = 0;
            return LME_ERROR;
        }
    }
    return LME_OK;
}


LmeError lme_uart_close()
{
    if (mIsOpen == true)
    {
        close(mUartID);
        mUartID = 0;
        mIsOpen = false;
        pthread_mutex_destroy(&lock);
        LOGD("lme_uart_close success mUartID=:%d\n", mUartID);
    }
    return LME_OK;
}


LmeError lme_uart_send(const BYTE *pData, UINT len) {
    int sendWait = 0;
    LmeError bret = LME_ERROR;
    unsigned int maxSendWait = getMaxWaitTime();
    if (!mIsOpen) {
        return LME_ERROR;
    }    

    LOGD("xks串口发送数据长度: %d...\n", len);
    pthread_mutex_lock(&lock);
    #if 1
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sendWait = lme_time_diff(&m_lastSentTime, &tv);
    
    if (sendWait  < (int)maxSendWait)
    {
        if (sendWait > 0)
        {
            LOGD("went sleep for %d ms\n", (maxSendWait - sendWait));
            if (sendBufQueueAppend((BYTE *)pData, len) == 0)
            {
                PrintHexBytesInline("queued data:", (unsigned char *)pData, len, TRUE);
                //gettimeofday(&m_lastSentTime, NULL);
                pthread_mutex_unlock(&lock);
                return LME_OK;
            }
        }

    }
    else
    {
        LOGD("since last sent time %d ms\n", sendWait);
    }
    #endif

    if (writeData(pData, len) < 0)
    {
        bret = LME_ERROR;
    }
    else
    {
        bret = LME_OK;
    }
    pthread_mutex_unlock(&lock);
    return bret;
}

LmeError lme_uart_wait_exit()
{
    pthread_join(th, NULL); //等待线程th结束
    lme_uart_close();
    return LME_OK;
}



