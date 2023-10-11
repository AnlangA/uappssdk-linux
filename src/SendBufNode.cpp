/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * FileName    : SendBufNode.cpp
 * Author      : Fu Baicheng
 * Date        : 2023年6月12日
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
#include "string.h"
#include "Common.h"
#include "lme_timer.h"
#include "SendBufNode.h"
SendBufNode::SendBufNode(unsigned char *buf, unsigned int len, unsigned int interval) {
    // TODO 自动生成的构造函数存根
    if (buf == NULL || len == 0 || len > SEND_BUF_NODE_BUF_SIZE)
    {
        LOGE("SendBufNode fail\n");
        return ;
    }
    memcpy(m_buf, buf, len);
    m_len = len;
    m_interval = interval;
    gettimeofday(&m_time, NULL);
}

SendBufNode::~SendBufNode() {
    // TODO 自动生成的析构函数存根
}

