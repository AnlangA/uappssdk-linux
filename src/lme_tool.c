/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : lme_tool.c
 * Author      :
 * Date        : 2023-09-12
 * Verslon     : 1.0
 * Description : APS层的Uapps报文处理的源文件
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/

#include <ctype.h>
#include <dirent.h>
#include <stdarg.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <stdio.h>	 /* Standard input/output definitions */
#include <unistd.h>	 /* UNIX standard function definitions */
#include <fcntl.h>	 /* File control definitions */
#include <errno.h>	 /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <pthread.h>
#include <linux/types.h>
#include <linux/mman.h>
#include <linux/watchdog.h>
#include "lme_tool.h"
#include <string.h>
#include "Uapps.h"
#include "lme_timer.h"

/*****************************************************************************
 Prototype    : delSpecialChar
 Description  : 删除一个字符串中不要的特殊字符
 Input        : unsigned char s 
                char c              
 Output       : None
 return value : 
 calls        : 
 Called By    : 
 Date         : 2020/05/20
 Author       : yaoj
 Modification : Created function
*****************************************************************************/
void delSpecialChar(char s[], char c)
{
  int i,j;
  for(i=0;s[i]!='\0';i++)
  {
       if(s[i]==c)
       {
           for(j=i;s[j]!='\0';j++)
            s[j]=s[j+1];              
       }
  }
}

/*****************************************************************************
 Prototype    : StrToHex
 Description  : ×Ö·û´®×ªHEX
 Input        : unsigned char *pbDest  
                unsigned char *pbSrc   
                int nLen               
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 Date         : 2020/05/20
 Author       : yaoj
 Modification : Created function
*****************************************************************************/
void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
	char h1, h2;
	unsigned char s1, s2;
	int i;

	if (nLen == 0)
		return;

	for (i = 0; i < nLen; i++)
	{
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];

		if (IS_DIGIT(h1))
		{
			s1 = h1 - '0';
		}
		else
		{
			s1 = UPCASE(h1) - '0';
			s1 -= 7;
		}

		if (IS_DIGIT(h2))
		{
			s2 = h2 - '0';
		}
		else
		{
			s2 = UPCASE(h2) - '0';
			s2 -= 7;
		}

		pbDest[i] = s1 * 16 + s2;
	}
}
/*****************************************************************************
 Prototype    : HexToStr
 Description  : HEX×ª×Ö·û
 Input        : unsigned char *pbDest  
                unsigned char *pbSrc   
                int nLen               
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 History      :
 Date         : 2020/05/20
 Author       : yaoj
 Modification : Created function
*****************************************************************************/
void Hex_To_Str(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
	char ddl, ddh;
	int i;

	for (i = 0; i < nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;

		if (ddh > 57)
			ddh = ddh + 7;
		if (ddl > 57)
			ddl = ddl + 7;

		pbDest[i * 2] = ddh;
		pbDest[i * 2 + 1] = ddl;
	}

	pbDest[nLen * 2] = '\0';
}

/*****************************************************************************
 Prototype    : macStrToMacNum
 Description  : convert string mac to hex mac 
 Input        : unsigned char *macstr  
                unsigned char *macAddrNum 
                int len   
 Output       : 
 Return Value : 
 Calls        : 
 Called By    : 
 Date         : 2021/01/12
 Author       : yaoj
 Modification : Created function
*****************************************************************************/
void macStrToMacNum(unsigned char *macstr, int len, unsigned char *macAddrNum)
{
	int i = 0, value = 0;
	for (i = 0; i < len; i++)
	{
		sscanf((char *)macstr + 2 * i, "%2x", &value);
		macAddrNum[i] = (unsigned char)value;
	}
}
void PrintStr(unsigned char *pbuf, unsigned short len)
{
	unsigned short i;

	for (i = 0; i < len; i++)
	{
		printf("%c", pbuf[i]);
	}
	printf("\r\n");
}

void PrintHexBytes(const char *tag, unsigned char *pbuf, unsigned short len)
{
	unsigned short i;
	printf("%s %d bytes: ", tag, len);
	for (i = 0; i < len; i++)
	{
		printf("%02x,", pbuf[i]);
	}
	printf("\r\n");
}

#define PRINT_SEGMENT_SIZE      80
void PrintHexBytesInline(const char *tag, unsigned char *pbuf, unsigned int len, unsigned int singleLine)
{
    unsigned short i;
    unsigned int olen = 0, tlen = 0;
    char * out =NULL;
    unsigned int seg = 0;
    char segBuf[PRINT_SEGMENT_SIZE + 1];
    
    if (tag != NULL)
    {
        tlen = strlen(tag);
        out = (char *)Lme_malloc(len * 2 + tlen + 1 +16);
        olen = sprintf(out, "[%s]:%d:", tag, len); 
    }
    else
    {
        out = (char *)Lme_malloc(len * 2 + 1 +16);
        olen = sprintf(out, "%d:", len); 
    }   
    olen = olen + hexTostr((u8 *)pbuf, len, NULL, out + olen);
    if (singleLine != TRUE)
    {
        segBuf[PRINT_SEGMENT_SIZE] = '\0';
        for (i = 0; i < olen / PRINT_SEGMENT_SIZE + 1; i++)
        {
            seg = i * PRINT_SEGMENT_SIZE;
            if (i != olen / PRINT_SEGMENT_SIZE)
            {
                memcpy(segBuf, out + seg, PRINT_SEGMENT_SIZE);
                LOGD("CONTINUED:%s\n", segBuf);
            }
            else
            {
                memcpy(segBuf, out + seg, olen % PRINT_SEGMENT_SIZE);
                segBuf[olen % PRINT_SEGMENT_SIZE] = '\0';
                LOGD("END,olen:%d:%s\n", olen, out + seg);
            }
        }
    }
    else
    {
        LOGD("%s\n", out);
    }
    Lme_free(out);
    return;
}

unsigned long GetFileSize(const char *filepath)
{
	unsigned long filesize = -1;
	struct stat statbuff;

	if (stat(filepath, &statbuff) < 0)
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	return filesize;
}

unsigned char GetCheckSum(unsigned char *pMem, unsigned short len)
{
	unsigned char crc;

	crc = 0;
	while (len--)
	{
		crc += *pMem++;
	}

	return crc;
}

int OTl_get_recvdatalen(int fd)
{
	int clen = 0;
	if (ioctl(fd, FIONREAD, &clen) != 0)
		return 0;
	return clen;
}
int OTl_pipe_create(char *pathname)
{
	unlink(pathname);
	return mkfifo(pathname, O_RDWR | O_NONBLOCK | 0666);
}
int OTl_pipe_open(char *pathname)
{
	int fd = -1;
	fd = open(pathname, O_RDWR | O_NONBLOCK);
	return fd;
}
int OTl_pipe_read(int fd, void *ptr, size_t size, unsigned long nsecs) //ºÁÃë
{
	int ret = -1;

	ret = OTl_pipe_select(fd, size, nsecs);
	if (ret > 0)
	{
		ret = read(fd, ptr, size);
	}
	else if (ret == 0)
	{
		ret = -2;
	}
	else
	{
		ret = -1;
	}
	//*nsecs=time_out.tv_sec*1000+time_out.tv_usec/1000;
	return ret;
}

int OTl_pipe_write(int fd, void *ptr, size_t size)
{
	return write(fd, ptr, size);
}

int OTl_pipe_close(int fd)
{
	return close(fd);
}

int OTl_pipe_clear(int fd)
{
	int len = 0;
	int size = 0;
	unsigned char buf[512];
	len = OTl_get_recvdatalen(fd);
	while (len > 0)
	{
		if (len > sizeof(buf))
			size = sizeof(buf);
		else
			size = len;
		if (read(fd, buf, size) != size)
			break;
		len -= size;
	}
	return len;
}
int OTl_pipe_select(int fd, size_t size, unsigned long msec) //ms
{
	int ret = 0;
	fd_set fdset = {0};
	struct timeval time_out = {0};

	ret = OTl_get_recvdatalen(fd);
	if (ret >= size)
		return ret;

	time_out.tv_sec = (msec / 1000);
	time_out.tv_usec = ((msec % 1000) * 1000);

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	ret = select(fd + 1, &fdset, NULL, NULL, &time_out);
	if (ret == 0) //select timeout
	{
		return 0;
	}
	else if (ret > 0 && FD_ISSET(fd, &fdset)) //recv data
	{
		ret = OTl_get_recvdatalen(fd);
		return ret;
	}
	else //error
	{
		return -1;
	}
}
int OTl_create_thread(pthread_t *pthreadid, void *(*proc)(void *), void *arg)
{
	int ret = 0;
	pthread_t threadid;
	pthread_attr_t attr = {0};
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&threadid, &attr, proc, arg);
	if (ret == 0 && pthreadid)
		*pthreadid = threadid;
	return ret;
}

//判断是否是十六进制的基数
int isHexNum(unsigned char _hexNum)
{
	if ('0' <= _hexNum && _hexNum <= '9')
	{
		return 1;
	}
	else if ('A' <= _hexNum && _hexNum <= 'F')
	{
		return 2;
	}
	else if ('a' <= _hexNum && _hexNum <= 'f')
	{
		return 3;
	}
	return -1;
}

void numToHexStr(unsigned char _hexNum, unsigned char *_hexStr)
{
	unsigned char tmp;

	if (NULL == _hexStr)
	{
		return;
	}

	//低4bit
	tmp = (_hexNum >> 4) & 0x0f;
	if (tmp <= 9)
		*_hexStr = tmp + '0';
	else
		*_hexStr = tmp - 0x0a + 'A';

	_hexStr++;

	//高4bit
	tmp = _hexNum & 0x0f;
	if (tmp <= 9)
		*_hexStr = tmp + '0';
	else
		*_hexStr = tmp - 0x0a + 'A';
}

//十六进制的字符转数字
unsigned char charToHexNum(unsigned char hexChar)
{
	unsigned char tmp;
	if (1 > isHexNum(hexChar))
	{
		return 0xFF;
	}

	if (hexChar <= '9')
	{
		tmp = hexChar - '0';
	}
	else if (hexChar <= 'F')
	{
		tmp = hexChar - '7';
	}
	else
	{
		tmp = hexChar - 'W';
	}
	return tmp;
}

//将字符串转为16进制形式，以查看不可见字符 "01" ==> "3031"
int toHexStr(const unsigned char *_str, unsigned char *_hexStr)
{
	int i;
	int len;
	unsigned char *resultPtr;
	if (NULL == _str || NULL == _hexStr)
	{
		return -1;
	}

	len = strlen((char *)_str);
	resultPtr = _hexStr;
	for (i = 0; i < len; i++)
	{
		numToHexStr(_str[i], resultPtr);
		resultPtr += 2;
	}
	return strlen((char *)_hexStr);
}

//将16进制形式的字符串转为文本形式 "3031" ==> "01"
int hexToStr(const unsigned char *_hexStr, unsigned char *_str)
{
	int i;
	int len;
	unsigned char *ptr;
	if (NULL == _str || NULL == _hexStr)
	{
		return -1;
	}

	len = strlen((char *)_hexStr);
	ptr = _str;

	//要是单数个字符，则最后一个会被丢弃
	for (i = 0; i < len - 1; i++)
	{
		//是十六进制的基数才转换
		//if(0<isHexNum(_hexStr[i]))
		{
			*ptr = charToHexNum(_hexStr[i]) * 16;
			i++;
			*ptr += charToHexNum(_hexStr[i]);
			ptr++;
		}
	}

	return strlen((char *)_str);
}

int convert_hex_to_str(unsigned char *src, unsigned int len, unsigned char *dest)
{
	unsigned int i = 0;
	unsigned char ch1, ch2;
	if (len % 2 != 0)
	{
		return -1;
	}

	for (i = 0; i < len / 2; i++)
	{
		ch1 = src[i * 2];
		ch2 = src[i * 2 + 1];
		ch1 -= ((ch1 > (9 + 0x30)) ? 0x57 : 0x30);
		ch2 -= ((ch2 > (9 + 0x30)) ? 0x57 : 0x30);
		dest[i] = (ch1 << 4) | ch2;
	}

	return 0;
}

int convert_str_to_hex(unsigned char *str, unsigned int str_len, unsigned char *out_str, unsigned int *out_len)
{
	unsigned int i = 0;
	unsigned char ch1 = 0, ch2 = 0;
	if (str_len * 2 + 2 > *out_len)
	{
		return -1;
	}
	sprintf((char *)out_str, "0x");

	for (i = 0; i < str_len; i++)
	{
		ch1 = (str[i] & 0xf0) >> 4;
		ch2 = (str[i] & 0x0f);

		ch1 += ((ch1 > 9) ? 0x57 : 0x30);
		ch2 += ((ch2 > 9) ? 0x57 : 0x30);

		out_str[2 * i + 2] = ch1;
		out_str[2 * i + 3] = ch2;
	}

	out_str[str_len * 2 + 2] = 0;
	*out_len = str_len * 2 + 2;

	return 0;
}

//将十六进制的字符串转换为十六进制数组
int StringHextoHex(char *str, unsigned char *out, int outlen)
{
	if (str == NULL || out == NULL)
		return -1;

//	int i = 0, ret = 0;
//
//	ret = (strlen(str) / (2 * sizeof(char))) + strlen(str) % (2 * sizeof(char));
//	for (i = 0; i < ret && i < outlen; i++)
//	{
//		sscanf(str + 2 * i, "%02X", (unsigned int *)(out + i));
//	}
	int i=0, j=0;
    for (i = 0, j = 0; i < outlen; i += 2, j++) {
        sscanf(&str[i], "%02hhx", &out[j]);
    }

	return 0;
}


//将十进制字符串转化为十进制数组
int StringToCom(char *str, unsigned char *out, int *outlen)
{
	char *p = str;
	char high = 0, low = 0;
	int tmplen = strlen(p), cnt = 0;
	tmplen = strlen(p);
	if (tmplen % 2 != 0)
		return -1;
	while (cnt < tmplen / 2) //1213141516171819
	{
		out[cnt] = (*p - 0x30) * 10 + (*(++p) - 0x30);
		p++;
		cnt++;
	}
	*outlen = tmplen / 2;
	return tmplen / 2;
}

//将数组转换为十六进制同值的字符串,读取数组中的数字，打印成字符串的时候以2位大写的格式。
int arrayToStr(unsigned char *buf, unsigned int buflen, char *out, int outlen)
{
	char pbuf[2+1] = {0};
	int i;
	memset(out,0x00,outlen);
	for( i = 0; i < buflen && i < (outlen / 2); i++ )
	{
		sprintf(pbuf, "%02X", buf[i]);
		strncat(out, pbuf, 2);
	}
	printf("out = %s\n", out);
	return i * 2;
}

//字节流转换为十六进制字符串
void Hex2Str( const char *sSrc,  char *sDest, int nSrcLen )
{
    int  i;
    char szTmp[3];
 
    for( i = 0; i < nSrcLen; i++ )
    {
        sprintf( szTmp, "%02X", (unsigned char) sSrc[i] );
        memcpy( &sDest[i * 2], szTmp, 2 );
    }
    return ;
}

/*--------------------------------------------------------------
函数名称：memfind  
函数功能：在目标内存中查找相同的数据
输入参数： 
        unsigned char *desBuffer,目标内存数据指针
        int desLen,目标数据长度
        unsigned char *srcBuffer, 待查找数据指针
        int srcLen，查找数据长度
输出参数：
返回值：  -1, 错误参数
         0,无对应数据
		 len,数据第一次出现偏移长度
备 注：
---------------------------------------------------------------*/
int memfind(unsigned char *desBuffer,int desLen,unsigned char *srcBuffer,int srcLen)
{   
   int count = 0,len = 0;
   unsigned char *temp = srcBuffer;
   if (!srcLen)
      return -1;
   while (desLen--)
   {
      if(*desBuffer == *temp){
      	count++;
         temp++;
      }else{
      	temp = srcBuffer;
      	count = 0;
      }
      desBuffer++;
      len++;
      if ((count == srcLen))
      {
        return len-count;
      }
   }
   return 0;
}


int lme_gateway_log(lme_gw_event_E e, const char * file, const char * func, unsigned int line, const char * fmt, ...)
{
    int n = 0;
    char buf[1024] = {0};
    va_list arg;

    n = snprintf(buf, sizeof(buf), "[%s E %04x]%s:%s:%d:", lme_get_time_str(), e, file, func, line); 
    va_start(arg, fmt);
    vsnprintf(buf + n, sizeof(buf) - n, fmt, arg); 
    va_end(arg); 

    printf("%s", buf);
    return 0;
}


