/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * FileName    : SendBufNode.h
 * Author      : Fu Baicheng
 * Date        : 2023年6月12日
 * Version     : 1.0
 * Description : 
 *             :
 * Others      :
 * ModifyRecord:
 *
 ********************************************************************************/

#ifndef JNI_APP_SRC_SENDBUFNODE_H_
#define JNI_APP_SRC_SENDBUFNODE_H_

#define SEND_BUF_NODE_BUF_SIZE       640     

class SendBufNode {
public:
    SendBufNode(unsigned char  *buf, unsigned int len, unsigned int interval = 0);
    virtual ~SendBufNode();
    unsigned int m_len;
    struct timeval m_time;
    unsigned int m_interval;
    unsigned char  m_buf[SEND_BUF_NODE_BUF_SIZE];
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* JNI_APP_SRC_SENDBUFNODE_H_ */
