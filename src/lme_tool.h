/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : lme_tool.h
 * Author      :
 * Date        : 2023-09-12
 * Verslon     : 1.0
 * Description : APS层的Uapps报文处理的源文件
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/

#ifndef __LME_TOOL_H
#define __LME_TOOL_H

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned char BOOL; //boolean
#define FALSE 0
#define TRUE 1
#define mPACKET __attribute__((packed))

typedef enum
{
    DEF_LONG_ADDR = 0,
    SHORT_ADDR
} addr_type_e;

#define LONG_ADDR_LEN (6)
#define SHORT_ADDR_LEN (2)

#define UPCASE(c) (((c) >= 'a' && (c) <= 'z') ? (c + 20) : (c))

#define DECCHK(c) ((c) >= '0' && (c) <= '9')

#define HEXCHK(c) (((c) >= '0' && (c) <= '9') || \
                   ((c) >= 'A' && (c) <= 'F') || \
                   ((c) >= 'a' && (c) <= 'f'))

#define IS_DIGIT(_c) ((_c) >= '0' && (_c) <= '9')

void delSpecialChar(char s[], char c);
void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen);

BOOL IsBuffZero(unsigned char *pbuf, unsigned short len);
void PrintStr(unsigned char *pbuf, unsigned short len);
void PrintHexBytes(const char *tag, unsigned char *pbuf, unsigned short len);
void PrintHexBytesInline(const char *tag, unsigned char *pbuf, unsigned int len, unsigned int singleLine);

void DebugDataToFile(unsigned char flag, char *filepath_1, char *filepath_2, unsigned long filemaxK, char *msg, void *data, unsigned short len, unsigned char dataformat);
void DebugLogToFile(unsigned char flag, char *filepath_1, char *filepath_2, unsigned long filemaxK, char *format, ...);
unsigned char GetCheckSum(unsigned char *pMem, unsigned short len);

int StrToMac(unsigned char *pbDest, unsigned char *pbSrc);
void Hex_To_Str(unsigned char *pbDest, unsigned char *pbSrc, int nLen);

int OTl_get_recvdatalen(int fd);
int OTl_pipe_create(char *pathname);
int OTl_pipe_open(char *pathname);
int OTl_pipe_read(int fd, void *ptr, size_t size, unsigned long nsecs);
int OTl_pipe_write(int fd, void *ptr, size_t size);
int OTl_pipe_close(int fd);
int OTl_pipe_clear(int fd);
int OTl_pipe_select(int fd, size_t size, unsigned long msec); //ms;
int OTl_create_thread(pthread_t *pthreadid, void *(*proc)(void *), void *arg);

int isHexNum(unsigned char _hexNum);
void numToHexStr(unsigned char _hexNum, unsigned char *_hexStr);
unsigned char charToHexNum(unsigned char hexChar);
int toHexStr(const unsigned char *_str, unsigned char *_hexStr);
int hexToStr(const unsigned char *_hexStr, unsigned char *_str);
int convert_hex_to_str(unsigned char *src, unsigned int len, unsigned char *dest);
int convert_str_to_hex(unsigned char *str, unsigned int str_len, unsigned char *out_str, unsigned int *out_len);
int StringHextoHex(char *str, unsigned char *out, int outlen);
int arrayToStr(unsigned char *buf, unsigned int buflen, char *out, int outlen);
void Hex2Str( const char *sSrc,  char *sDest, int nSrcLen );
int generate_snid(unsigned char *buf);
int memfind(unsigned char *desBuffer,int desLen,unsigned char *srcBuffer,int srcLen);

#define LME_COPY_STR_VAL_TO_STRUCT(dest, name, src) do { \
    if (src->name != NULL) {\
        char *temp = new char[strlen(src->name) + 1]; \
        if (temp != NULL) { \
            strcpy(temp, src->name); \
            dest->name = temp; \
        } \
    }\
} while(0)

#ifdef __cplusplus
}
#endif

#endif
