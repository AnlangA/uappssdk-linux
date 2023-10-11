/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : Uapps.c
 * Author      :
 * Date        : 2020-05-09
 * Version     : 1.0
 * Description : APS层的Uapps报文处理的源文件
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/
#include "Common.h"
#include "Uapps.h"

#ifdef __cplusplus
extern "C"{
#endif

#define UappsGenId() (uappsId++)
#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)
#define TKL(a) ((a==0)?a:a+1)



#define true  1
#define false 0
#define FORMAT_EXTAND_TYPE		15	//扩展类型
/**********************************宏定义 结束*************************************/


/**********************************枚举声明 开始************************************/
typedef enum
{
    UAPPS_ERR_NONE = 0,
    UAPPS_ERR_HEADER_TOO_SHORT = 1,
    UAPPS_ERR_VERSION_NOT_1 = 2,
    UAPPS_ERR_TOKEN_TOO_SHORT = 3,
    UAPPS_ERR_OPTION_TOO_SHORT_FOR_HEADER = 4,
    UAPPS_ERR_OPTION_TOO_SHORT = 5,
    UAPPS_ERR_OPTION_OVERRUNS_PACKET = 6,
    UAPPS_ERR_OPTION_TOO_BIG = 7,
    UAPPS_ERR_OPTION_LEN_INVALID = 8,
    UAPPS_ERR_BUFFER_TOO_SMALL = 9,
    UAPPS_ERR_UNSUPPORTED = 10,
    UAPPS_ERR_OPTION_DELTA_INVALID = 11,
} uapps_error_t;
/**********************************枚举声明 结束************************************/


/**********************************结构体声明 开始***********************************/

/**********************************结构体声明 结束***********************************/

/**********************************静态变量声明 开始**********************************/
static u16 uappsId = 0;		// msg id
static u16 uappsSeed = 1;  	// rand() seed

/**********************************静态变量声明 结束**********************************/

void uapps_option_nibble(U32 value, U8 *nibble)
{
	if (value<13)
	{
		*nibble = (U8)value;
	}
	else if (value<=268)
	{
		*nibble = 13;
	} 
	else if (value<=65804)
	{
		*nibble = 14;
	}
}


u16 uapps_parseHeader(uapps_header_t *hdr, u8 *buf, u16 buflen)
{
	if (buflen < 4)
		return UAPPS_ERR_HEADER_TOO_SHORT;
	hdr->ver = (buf[0] & 0xC0) >> 6;
	if (hdr->ver != 1)
		return UAPPS_ERR_VERSION_NOT_1;
	hdr->t = (buf[0] & 0x30) >> 4;
	hdr->tkl = buf[0] & 0x0F;
	hdr->codeOfUappsHead = buf[1];
	hdr->id[0] = buf[2];
	hdr->id[1] = buf[3];
	return 0;
}

u16 uapps_parseToken(uapps_buffer_t *tokbuf,  uapps_header_t *hdr,  u8 *buf, u16 buflen)
{
	if (hdr->tkl == 0)
	{
		tokbuf->p = NULL;
		tokbuf->len = 0;
		return 0;
	}
	else if (hdr->tkl <= 8)
	{
		if (4U + hdr->tkl > buflen)
		{
			return UAPPS_ERR_TOKEN_TOO_SHORT;   // tok bigger than packet
		}
		tokbuf->p = buf+4;  // past header
		tokbuf->len = hdr->tkl;
		return 0;
	}
	else
	{
		// invalid size
		return UAPPS_ERR_TOKEN_TOO_SHORT;
	}
}


u16 uapps_parseOption(uapps_option_t *option, u16 *running_delta,  u8 **buf, u16 buflen)
{
	U8 *p = *buf;
	U8 headlen = 1;
	U16 len, delta;

	if (buflen < headlen) // too small
	{
		return UAPPS_ERR_OPTION_TOO_SHORT_FOR_HEADER;
	}

	delta = (p[0] & 0xF0) >> 4;
	len = p[0] & 0x0F;

	// These are untested and may be buggy
	if (delta == 13)
	{
		headlen++;
		if (buflen < headlen)
		{
			return UAPPS_ERR_OPTION_TOO_SHORT_FOR_HEADER;
		}
		delta = p[1] + 13;
		p++;
	}
	else if (delta == 14)
	{
		headlen += 2;
		if (buflen < headlen)
		{
			return UAPPS_ERR_OPTION_TOO_SHORT_FOR_HEADER;
		}
		delta = ((U16)(p[1] << 8) | p[2]) + 269;
		p+=2;
	}
	else if (delta == 15)
	{
		return UAPPS_ERR_OPTION_DELTA_INVALID;
	}

	if (len == 13)
	{
		headlen++;
		if (buflen < headlen)
		{
			return UAPPS_ERR_OPTION_TOO_SHORT_FOR_HEADER;
		}
		len = p[1] + 13;
		p++;
	}
	else if (len == 14)
	{
		headlen += 2;
		if (buflen < headlen)
		{
			return UAPPS_ERR_OPTION_TOO_SHORT_FOR_HEADER;
		}
		len = ((U16)(p[1] << 8) | p[2]) + 269;
		p+=2;
	}
	else if (len == 15)
	{
		return UAPPS_ERR_OPTION_LEN_INVALID;
	}

	if ((p + 1 + len) > (*buf + buflen))
	{
		return UAPPS_ERR_OPTION_TOO_BIG;
	}

	//LOGINFO1(UART2, "option num=%d\n", delta + *running_delta);
	option->num = delta + *running_delta;
	option->buf.p = p+1;
	option->buf.len = len;
	//uapps_dump(p+1, len, false);

	// advance buf
	*buf = p + 1 + len;
	*running_delta += delta;

	return 0;
}


u16 uapps_parseOptionsAndPayload(uapps_option_t *options, u8 *numOptions, uapps_buffer_t *payload,  uapps_header_t *hdr,  u8 *buf, u16 buflen, u16 lenOfstandardUappsOptions)
{
	U16 optionIndex = 0;
	U16 delta = 0;
	U8 *p = buf + 4 + hdr->tkl + lenOfstandardUappsOptions;
	U8 *end = buf + buflen;
	U16 rc;
	if (p > end)
	{
		return UAPPS_ERR_OPTION_OVERRUNS_PACKET;   // out of bounds
	}
	
	//内容头选项解析
	if(*p == 0xFF)
	{
		p++;

		while((optionIndex < *numOptions) && (p < end) && (*p != 0xFF))
		{
			if (0 != (rc = uapps_parseOption(&options[optionIndex], &delta, &p, end-p)))
			{
				return rc;
			}
			optionIndex++;
#ifndef WIN32
			feed_dog(); 
#endif
		}
		*numOptions = (U8)optionIndex;
		if (p+1 < end && *p == 0xFF)  // payload marker
		{
			payload->p = p+1;
			payload->len = end-(p+1);
		}
		else
		{
			payload->p = NULL;
			payload->len = 0;
		}
	}
	else	
	{
		*numOptions = (U8)optionIndex;
		//return UAPPS_ERR_OPTION_LEN_INVALID;
	}

	return 0;
}
/*--------------------------------------------------------------
函数名称：uapps_parseStandardOption
函数功能：标准uapps选项解析
输入参数：options-uapps报文中的标准选项
         			 numOptions-uapps报文中的标准选项数量
         			 hdr-uapps报文头
         			 buf - 报文指针
         			 buflen - 报文长度
         			 optionLen - 选项长度指针
返回值：  错误码
备 注：
---------------------------------------------------------------*/
u16 uapps_parseStandardOption(uapps_option_t *options, u8 *numOptions, uapps_header_t *hdr,  u8 *buf, u16 buflen, u16 *optionLen)
{
	U16 optionIndex = 0;
	U16 delta = 0;
	U8 *p = buf + 4 + hdr->tkl;
	U8 *end = buf + buflen;
	U16 rc;
	if (p > end)
	{
		return UAPPS_ERR_OPTION_OVERRUNS_PACKET;   // out of bounds
	}
	
	

	while((optionIndex < *numOptions) && (p < end) && (*p != 0xFF))
	{
		if (0 != (rc = uapps_parseOption(&options[optionIndex], &delta, &p, end-p)))
		{
			return rc;
		}
		optionIndex++;
#ifndef WIN32
		feed_dog(); 
#endif
	}
	*numOptions = (U8)optionIndex;
	if (p+1 < end && *p == 0xFF)  // payload marker
	{
		*optionLen = p - (buf + 4 + hdr->tkl);
	}
	else
	{
		//return UAPPS_ERR_OPTION_LEN_INVALID;
	}
	return 0;
}


/*-------------------------------------------------------------------------------
函数名称：	uapps_parse
函数功能：	报文解析函数
输入参数：       buf-待解析报文字节流的指针
            buflen - 待解析报文字节流长度
输出参数：	pkt-解析后的报文结构体
返回值：		0 - 成功，非0-组帧失败
备注说明：
--------------------------------------------------------------------------------*/
u16 uapps_parse(uapps_packet_t *pkt,  u8 *buf, u16 buflen)
{
	U16 rc;
	U16 standradOptionLen = 0;

	
	if (0 != (rc = uapps_parseHeader(&pkt->hdr, buf, buflen)))
	{
		return rc;
	}

	if (0 != (rc = uapps_parseToken(&pkt->tok, &pkt->hdr, buf, buflen)))
	{
		return rc;
	}
	//标准uapps选项解析
	#if 1
	pkt->numOfOptionStandardUappsMsg = MAXOPT;
	if (0 != (rc = uapps_parseStandardOption(pkt->optionOfStandardUappsMsg, &(pkt->numOfOptionStandardUappsMsg), &pkt->hdr, buf, buflen, &standradOptionLen)))
	{
		return rc;
	}
	#endif
	//内容头选项及载荷解析
	pkt->numopts = MAXOPT;
	if (0 != (rc = uapps_parseOptionsAndPayload(pkt->opts, &(pkt->numopts), &(pkt->payload), &pkt->hdr, buf, buflen, standradOptionLen)))
	{
		return rc;
	}

	return 0;
}

// options are always stored consecutively, so can return a block with same option num
uapps_option_t *uapps_findOptions( uapps_packet_t *pkt, u8 num, u8 *count)
{
	// FIXME, options is always sorted, can find faster than this
	U16 i;
	uapps_option_t *first = NULL;
	*count = 0;
	for (i=0;i<pkt->numopts;i++)
	{
		if (pkt->opts[i].num == num)
		{
			if (NULL == first)
			{
				first = &pkt->opts[i];
			}
			(*count)++;
		}
		else
		{
			if (NULL != first)
			{
				break;
			}
		}
	}
	return first;
}

u16 uapps_buffer_to_string(char *strbuf, u16 strbuflen, uapps_buffer_t *buf)
{
	if (buf->len+1 > strbuflen)
	{
		return UAPPS_ERR_BUFFER_TOO_SMALL;
	}
	memcpy(strbuf, buf->p, buf->len);
	strbuf[buf->len] = 0;
	return 0;
}

/*-------------------------------------------------------------------------------
函数名称：	uapps_build
函数功能：	报文组帧函数
输入参数：	pkt-待组帧的报文结构体
输出参数：       buf-组帧后的字节流指针
            buflen - 组帧后的字节流长度
返回值：		0 - 成功，非0-组帧失败
备注说明：
--------------------------------------------------------------------------------*/
u16 uapps_build(u8 *buf, u16 *buflen, uapps_packet_t *pkt)
{
	U16 opts_len = 0;
	U16 i;
	U8 *p;
	U16 running_delta = 0;
	U32 optDelta = 0;
	U8 len, delta = 0;


	// build header
	if (*buflen < (4U + pkt->hdr.tkl))
	{
		return UAPPS_ERR_BUFFER_TOO_SMALL;
	}

	buf[0] = (pkt->hdr.ver & 0x03) << 6;
	buf[0] |= (pkt->hdr.t & 0x03) << 4;
	buf[0] |= (pkt->hdr.tkl & 0x0F);
	buf[1] = pkt->hdr.codeOfUappsHead;
	buf[2] = pkt->hdr.id[0];
	buf[3] = pkt->hdr.id[1];

	// inject token
	p = buf + 4;
	if ((pkt->hdr.tkl > 0) && (pkt->hdr.tkl != pkt->tok.len))
	{
		return UAPPS_ERR_UNSUPPORTED;
	}

	if (pkt->hdr.tkl > 0)
	{
		memcpy(p, pkt->tok.p, pkt->hdr.tkl);
	}


	// inject options
	p += pkt->hdr.tkl;
	//标准uapps选项
	if(pkt->numOfOptionStandardUappsMsg)
	{
		for(i = 0; i < pkt->numOfOptionStandardUappsMsg; i++)
		{
			
#ifndef WIN32
			feed_dog(); 
#endif
			if (((U16)(p-buf)) > *buflen)
			{
				return UAPPS_ERR_BUFFER_TOO_SMALL;
			}
			optDelta = pkt->optionOfStandardUappsMsg[i].num - running_delta;
			uapps_option_nibble(optDelta, &delta);
			uapps_option_nibble((U32)pkt->optionOfStandardUappsMsg[i].buf.len, &len);

			*p++ = (0xFF & ( (((U16)delta )<< 4) | len) );
			if (delta == 13)
			{
				*p++ = (U8)(optDelta - 13);
			}
			else if (delta == 14)
			{
				*p++ = (U8)((optDelta-269) >> 8);
				*p++ = (0xFF & (optDelta-269));
			}

			if (len == 13)
			{
				*p++ = (U8)(pkt->optionOfStandardUappsMsg[i].buf.len - 13);
			}
			else if (len == 14)
			{
				*p++ = (U8)((pkt->optionOfStandardUappsMsg[i].buf.len-269) >> 8);
				*p++ = (0xFF & (pkt->optionOfStandardUappsMsg[i].buf.len-269));
			}

			memcpy(p, pkt->optionOfStandardUappsMsg[i].buf.p, pkt->optionOfStandardUappsMsg[i].buf.len);
			p += pkt->optionOfStandardUappsMsg[i].buf.len;
			running_delta = pkt->optionOfStandardUappsMsg[i].num;
	}
	}
	// 内容头选项开始标志
	if(pkt->numopts)
	{
		*p++ = 0xFF;
	}
	running_delta = 0;
	len = 0;
	optDelta = 0;
	for (i=0;i<pkt->numopts;i++)
	{

		
#ifndef WIN32
		feed_dog(); 
#endif
		if (((U16)(p-buf)) > *buflen)
		{
			return UAPPS_ERR_BUFFER_TOO_SMALL;
		}
		optDelta = pkt->opts[i].num - running_delta;
		uapps_option_nibble(optDelta, &delta);
		uapps_option_nibble((U32)pkt->opts[i].buf.len, &len);

		*p++ = (0xFF & ( (((U16)delta )<< 4) | len) );
		if (delta == 13)
		{
			*p++ = (U8)(optDelta - 13);
		}
		else if (delta == 14)
		{
			*p++ = (U8)((optDelta-269) >> 8);
			*p++ = (0xFF & (optDelta-269));
		}

		if (len == 13)
		{
			*p++ = (U8)(pkt->opts[i].buf.len - 13);
		}
		else if (len == 14)
		{
			*p++ = (U8)((pkt->opts[i].buf.len-269) >> 8);
			*p++ = (0xFF & (pkt->opts[i].buf.len-269));
		}

		memcpy(p, pkt->opts[i].buf.p, pkt->opts[i].buf.len);
		p += pkt->opts[i].buf.len;
		running_delta = pkt->opts[i].num;
	}

	opts_len = (p - buf) - 4;   // number of bytes used by options

	if (pkt->payload.len > 0)
	{
		if (*buflen < 4 + 1 + pkt->payload.len + opts_len)
		{
			return UAPPS_ERR_BUFFER_TOO_SMALL;
		}
		buf[4 + opts_len] = 0xFF;  // payload marker
		memcpy(buf+5 + opts_len, pkt->payload.p, pkt->payload.len);
		*buflen = opts_len + 5 + pkt->payload.len;
	}
	else
	{
		*buflen = opts_len + 4;
	}
	return 0;
}




/**
  * Generate random token.
  * rand()会返回一随机数值，范围在0至RAND_MAX 间。在调用此函数产生随机数前，
  * 必须先利用srand()设好随机数种子，如果未设随机数种子，
  * rand()在调用时会自动设随机数种子为1。
  * RAND_MAX定义在stdlib.h，其值为32767
*/
void UappsGenToken(u8* buf, u8 tkl)
{
	u8 count,i,k;
	u16 val;

	// Update seed
	srand(uappsSeed++);

	i = 0; // buf index
	count = (u8)(tkl >> 1);
	for (k = 0; k < count; k++)
	{
		val = (u16)rand();
		buf[i++] = (u8)(val >> 8);
		buf[i++] = (u8)(val & 0xff);
	}

	count = (u8)(tkl & 0x01);
	if (count > 0)
	{
		val = (u16)rand();
		buf[i++] = (u8)(val >> 8);
	}
}


/**
 * Construct Uapps message with specified type, hdrCode and uri 
 */
/*--------------------------------------------------------------
函数名称：UappsCreateMessage
函数功能：创建Uapps报文结构体
输入参数：type-Uapps报文头的报文类型
         hdrCode-Uapps报文头的code码
         rslStr-Uapps报文中的RSL
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsCreateMessage(UappsMessage *msg, u8 type, u8 hdrCode, char *rslStr)
{
	// Message header
	msg->hdr.tkl = UAPPS_TKL;
	msg->hdr.type = type;
	msg->hdr.ver = UAPPS_VER;
	msg->hdr.hdrCode = hdrCode;
	msg->hdr.id = UappsGenId();

	// Token bytes
	if (msg->hdr.tkl > 0)
	{
		UappsGenToken(msg->token, msg->hdr.tkl);
	}

	// Options
	msg->num_opts = 0;
	if (rslStr != NULL && strlen(rslStr) > 0)
	{
		UappsPutStrOption(msg, UAPPS_OPT_URI_PATH, (u8*)rslStr);
	} 

	return UAPPS_OK;

}

/**
 * Add string option
 */
/*--------------------------------------------------------------
函数名称：UappsPutStrOption
函数功能：向Uapps报文结构体中添加字符串类型的选项
输入参数：opt_code-option选项中的选项码
         str-option选项中的字符串选项值
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsPutStrOption(UappsMessage *msg, u16 opt_code, u8 *str)
{

	if (msg->num_opts >= UAPPS_MAX_OPTIONS)
	{
		return UAPPS_TOO_MANY_OPTS;
	}


	if (str == NULL || strlen((const char*)str) == 0)
	{
		return UAPPS_INVALID;
	}

	return UappsPutOption(msg, opt_code, str, strlen((const char*)str));
}

/**
 * Add uint option
 */
/*--------------------------------------------------------------
函数名称：UappsPutStrOption
函数功能：向Uapps报文结构体中添加数值类型的选项
输入参数：opt_code-option选项中的选项码
         val-option选项中的数值选项值
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsPutIntOption(UappsMessage *msg, u16 opt_code, u16 val)
{
	u8 buf[2]={0};
	u8 len=0;

	if(val>255)
	{
		buf[0] = val & 0x00ff;
		buf[1] = (val & 0xff00) >> 8;
		//buf[0] = (u8)(val >> 8);
		//buf[1] = (u8)(val & 0xff);
		len = 2;
	}
	else
	{
		buf[0]=val;
		len=1;
	}
	
	return UappsPutOption(msg, opt_code, buf, len);
}

/**
 * Add option into message's option list. val is in byte array
 */
/*--------------------------------------------------------------
函数名称：UappsPutOption
函数功能：向Uapps报文结构体中添加数值数组类型的选项
输入参数：opt_code-option选项中的选项码
         val-option选项中的数值选项值数值指针
         len-option选项中的数值选项值数组长度
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsPutOption(UappsMessage *msg, u16 opt_code, u8 *val, u8 len)
{
	UappsOption *opt;
	

	if (msg->num_opts >= UAPPS_MAX_OPTIONS)
	{
		return UAPPS_TOO_MANY_OPTS;
	}

	opt = &msg->options[msg->num_opts++];
	opt->opt_code = opt_code;
	opt->opt_len = len;
	memcpy(opt->opt_val, val, len);


	return UAPPS_OK;
}

/**
 * Put data content. Note: data content is in seperate buffer
 */
/*--------------------------------------------------------------
函数名称：UappsPutData
函数功能：向Uapps报文结构体中添加数据载荷选项
输入参数：buf-数据载荷数组
         len-数据载荷数组大小
         fmt-数据载荷格式类型
         encoding - 数据载荷编码方式
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsPutData(UappsMessage *msg, u8 *buf, u16 len, u16 fmt, u16 encoding)
{
	u8 flag =0;
	UappsOption opt;
	memset(&opt, 0, sizeof(UappsOption));

	flag = UappsSetFormat(&opt, (U16)fmt, (U16)encoding);
	if(flag != UAPPS_OK)
	{
		return (UappsError)flag;
	}
	memcpy(&msg->options[msg->num_opts++], &opt, sizeof(UappsOption));

	msg->pl_ptr = buf;
	msg->pl_len = len;
//	LOGTRABUF(2, "S3:",msg,300);

	return (UappsError)flag;
}

/**
 * Put string data content. Note: data content is in seperate buffer
 */
/*--------------------------------------------------------------
函数名称：UappsPutData
函数功能：向Uapps报文结构体中添加字符串载荷选项
输入参数：str-数据载荷字符串
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsPutStrData(UappsMessage *msg, char *str)
{
	u8 flag =0;
	flag = UappsPutIntOption(msg,UAPPS_OPT_FORMAT,UAPPS_FMT_TEXT);

	msg->pl_ptr = (u8*)str;
	msg->pl_len = strlen((const char*)str);

	return (UappsError)flag;
}

/*--------------------------------------------------------------
函数名称：UappsPutData
函数功能：向Uapps报文结构体中添加分块数据载荷选项
输入参数：bf-数据载荷字符串
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsPutDataBlock(UappsMessage *msg,u8 bf, u8 *buf, u16 len, u16 fmt, u16 encoding)
{
	u8 flag = 0;
	UappsOption *opt=NULL;

	if(2000<len)
	{
		return UAPPS_INVALID;
	}

	flag = UappsPutData(msg, buf, len, fmt, encoding);

	if(UAPPS_OK != flag)
	{
		return (UappsError)flag;
	}

	if (msg->num_opts >= UAPPS_MAX_OPTIONS)
	{
		return UAPPS_TOO_MANY_OPTS;
	}

	opt = &msg->options[msg->num_opts++];
	opt->opt_code = UAPPS_OPT_BLOCK_SIZE2;
	opt->opt_len = 2;
	opt->opt_val[0]= ((bf&0xf)<<4) | ( (len>>8)&0x7) ;
	opt->opt_val[1]=  (len & 0xff) ;

	return UAPPS_OK;

}

/**
 * Return index of next option in ascending order.
 */
UappsOption* nextOpt(u16 prev, UappsMessage *msg)
{
	u8 i;
	u16 min;
	UappsOption *opt, *opt_next;

	min = 0xffff;
	opt_next = NULL;
	for (i = 0;i<msg->num_opts;i++)
	{
		opt = &msg->options[i];
		if (opt->opt_code >= prev && opt->opt_code < min)
		{
			min = opt->opt_code;
			opt_next = opt;
		}
	}

	return opt_next;
}

/**
 * Encode option to bytes. Return encoded length
 */
u16 Option2Bytes(u16 prev, UappsOption *opt, u8 *buf)
{
	u8 i;
	u16 delta;

	i = 0;
	delta = opt->opt_code - prev;
	// code delta
	if (delta < 13)
	{
		buf[i++] = (u8)(delta << 4);
	}
	else if (delta < 269)
	{
		buf[i++] = (u8)0xd0; // 13
		buf[i++] = (u8)(delta - 13);
	}
	else
	{
		buf[i++] = (u8)0xe0; // 14
		delta -= 269;
		buf[i++] = (u8)(delta >> 8);
		buf[i++] = (u8)(delta & 0xff);
	}

	if (opt->opt_len > UAPPS_MAX_OPTLEN)
	{
		opt->opt_len = UAPPS_MAX_OPTLEN;
	}

	if (opt->opt_len < 13)
	{
		buf[0] |= (u8)opt->opt_len;
	}
	else if (opt->opt_len < 269)
	{
		buf[0] |= (u8)13;
		buf[i++] = (u8)(opt->opt_len - 13);
	}
	else
	{
		buf[0] |= (u8)14;	
		delta = opt->opt_len - 269;
		buf[i++] = (u8)(delta >> 8);
		buf[i++] = (u8)(delta & 0xff);
	}
		
	memcpy(&buf[i],opt->opt_val,opt->opt_len);
	i += opt->opt_len;

	return i;
}


/**
 * Encode Uapps message into bytes for transmit.
 * Return encoded length
 */
/*--------------------------------------------------------------
函数名称：Uapps2Bytes
函数功能：将Uapps报文结构体转换为字节流
输入参数：msg-待转换的Uapps报文结构体指针
输出参数：buf-转换后的字节流数组指针
返回值：  转换后的字节流数组长度
备 注：
---------------------------------------------------------------*/
u16 Uapps2Bytes(UappsMessage *msg, u8 *buf)
{
#if 0

	u16 i,prev;
	u8 opt_i;
	u8 len;
	UappsOption *opt;

	i = 0;
	// Header
	len = sizeof(UappsHeader);
	memcpy(buf,&msg->hdr,len);
	i += len;

	// Token
	if (msg->hdr.tkl > 0)
	{
		memcpy(buf+i,&msg->token[0],msg->hdr.tkl);
		i += msg->hdr.tkl;
	}

	// Options
	prev = 0;
	for (opt_i = 0; opt_i < msg->num_opts; opt_i++)
	{
		opt = &(msg->options[opt_i]);//nextOpt(prev,msg);
		if (opt != NULL)
		{
			i += Option2Bytes(prev,opt,buf+i);
			prev = opt->opt_code;
		}
	}

	return i;

#endif
	U8 retValue = 0;
	U8 i = 0;
	U16 bufLen = 2048;
	uapps_packet_t pkt;
	memset(&pkt, 0 ,sizeof(uapps_packet_t));

	pkt.hdr.ver = msg->hdr.ver;
	pkt.hdr.t = msg->hdr.type;
	pkt.hdr.tkl = msg->hdr.tkl;
	pkt.hdr.codeOfUappsHead = msg->hdr.hdrCode;
	memcpy(pkt.hdr.id, &msg->hdr.id, sizeof(U16));
	pkt.tok.len = msg->hdr.tkl;
	pkt.tok.p = msg->token;
	pkt.numopts = msg->num_opts;
	for(i = 0; i < pkt.numopts; i++)
	{
		pkt.opts[i].num = msg->options[i].opt_code;
		pkt.opts[i].buf.len = msg->options[i].opt_len;
		pkt.opts[i].buf.p = msg->options[i].opt_val;
	}
	// 标准uapps选项
	if(msg->numOfStandardUappsOption)
	{
		pkt.numOfOptionStandardUappsMsg = msg->numOfStandardUappsOption;
		for(i = 0; i < pkt.numOfOptionStandardUappsMsg; i++)
		{
			pkt.optionOfStandardUappsMsg[i].num = msg->optionsOfStandardUappsMsg[i].opt_code;
			pkt.optionOfStandardUappsMsg[i].buf.len = msg->optionsOfStandardUappsMsg[i].opt_len;
			pkt.optionOfStandardUappsMsg[i].buf.p = msg->optionsOfStandardUappsMsg[i].opt_val;
		}
	}
	pkt.payload.len = msg->pl_len;
	pkt.payload.p = msg->pl_ptr;

	retValue = uapps_build(buf, &bufLen, &pkt);
	return bufLen;
}


/**
 * Construct simple response message
 */
/*--------------------------------------------------------------
函数名称：UappsSimpleResponse
函数功能：生成无载荷的Uapps回复帧报文结构体
输入参数：reqMsg-Uapps请求报文结构体
         type-回复帧报文头的报文类型
         resCode-回复帧报文头的回复码
输出参数：resMsg-生成的Uapps报文结构体
返回值：  回复帧报文结构体指针
备 注：
---------------------------------------------------------------*/
UappsMessage* UappsSimpleResponse(UappsMessage *reqMsg, UappsMessage *resMsg, u8 type, u8 resCode)
{
	// Header
	resMsg->hdr.ver = UAPPS_VER;
	resMsg->hdr.type = type;
	resMsg->hdr.tkl = reqMsg->hdr.tkl;
	resMsg->hdr.hdrCode = resCode;
	resMsg->hdr.id = reqMsg->hdr.id;
	// Token
	if (resMsg->hdr.tkl > 0)
	{
		memcpy(resMsg->token, reqMsg->token, resMsg->hdr.tkl);
	}

	// Options
	resMsg->num_opts = 0;
	// Content
	resMsg->pl_ptr = NULL;
	resMsg->pl_len = 0;

	return resMsg;
}

/**
 * Construct response message with data
 */
/*--------------------------------------------------------------
函数名称：UappsSimpleResponse
函数功能：生成有载荷的Uapps回复帧报文结构体
输入参数：reqMsg-Uapps请求报文结构体
         type-回复帧报文头的报文类型
         resCode-回复帧报文头的回复码
         buf - 载荷数据
         len - 载荷数据长度
         fmt - 载荷数据内容格式
输出参数：resMsg-生成的Uapps报文结构体
返回值：  回复帧报文结构体指针
备 注：
---------------------------------------------------------------*/
UappsMessage *UappsDataResponse(UappsMessage *reqMsg, UappsMessage *resMsg, u8 type, u8 resCode,
		u8* buf, u16 len, u16 fmt, u16 encoding)
{
	if (UappsSimpleResponse(reqMsg,resMsg,type,resCode) == NULL)
	{
		return NULL;
	}

	if (buf != NULL && len > 0)
	{
		UappsPutData(resMsg,buf,len,fmt, encoding);
	}

	return resMsg;
}


/**
 * Decode bytes into option. Return option total size
 */
u16 OptionFromBytes(u8 *buf, u16 prev, UappsOption *opt)
{
	u16 i=0;
	u8 b=0;

	i = 0;
	b = buf[i++];
	opt->opt_code = b >> 4;
	if (opt->opt_code == 13)
	{
		opt->opt_code = 13 + buf[i++];
	}
	else if (opt->opt_code == 14)
	{
		opt->opt_code = 269 + (buf[i++] << 8);
		opt->opt_code += buf[i++];
	}

	opt->opt_code += prev;

	opt->opt_len = b & 0x0f;
	if (opt->opt_len == 13)
	{
		opt->opt_len = 13 + buf[i++];
	}
	else if (opt->opt_len == 14)
	{
		opt->opt_len = 269 + (buf[i++] << 8);
		opt->opt_len += buf[i++];
	}

	memcpy(opt->opt_val, buf+i, opt->opt_len);
	i += opt->opt_len;

	return i;
}

/**
 * Decode received bytes into message struct. Return OK or error code
 */
/*--------------------------------------------------------------
函数名称：UappsFromBytes
函数功能：将Uapps报文字节流转换为报文结构体
输入参数：buf-待转换的Uapps报文字节流数组
        buf_len- 待转换的Uapps报文字节流数组长度
输出参数：msg-转换后的Uapps报文结构体
返回值：  转换后的字节流数组长度
备 注：
---------------------------------------------------------------*/
UappsError UappsFromBytes(u8 *buf, u16 buf_len, UappsMessage *msg)
{
#if 0
	u16 i, prev, result;
	UappsOption *opt;

	i = 0;
	// Header
	memcpy(&msg->hdr, buf, sizeof(UappsHeader));
	i += sizeof(UappsHeader);

	// Token
	if (msg->hdr.tkl > 0)
	{
		memcpy(msg->token, buf+i, msg->hdr.tkl);
		i += msg->hdr.tkl;
	}

	if (i >= buf_len)
	{
		return UAPPS_OK; // buffer end, no options, no data
	}

	// Options
	msg->num_opts = 0;
	prev = 0;
	while (i < buf_len && buf[i] != 0xff)
	{
		opt = &msg->options[msg->num_opts];
		result = OptionFromBytes(buf+i,prev,opt);
		prev = opt->opt_code;
		if (result > 0)
		{
			msg->num_opts++;
			i += result;
		}
	}

	if (i >= buf_len)
	{
		return UAPPS_OK; // buffer end, no data
	}

	if (buf[i] == 0xff)
	{  // has data
		i++;
		msg->pl_ptr = buf+i;
		msg->pl_len = buf_len - i;
	}

	return UAPPS_OK;
#endif
	U8 i = 0;
	U8 retValue = 0;
	uapps_packet_t pkt;
	
	memset(&pkt, 0, sizeof(uapps_packet_t));
	retValue = uapps_parse(&pkt, buf, buf_len);
	if(retValue != 0)
	{
		return UAPPS_ERROR;
	}
	msg->hdr.tkl = pkt.hdr.tkl;
	msg->hdr.type = pkt.hdr.t;
	msg->hdr.ver = pkt.hdr.ver;
	msg->hdr.hdrCode = pkt.hdr.codeOfUappsHead;
	memcpy(&msg->hdr.id, pkt.hdr.id, sizeof(U16));
	memcpy(msg->token, pkt.tok.p, pkt.hdr.tkl * sizeof(U8));
	//内容头选项复制
	msg->num_opts = pkt.numopts;
	for(i = 0; i < pkt.numopts; i++)
	{
		msg->options[i].opt_code = pkt.opts[i].num;
		msg->options[i].opt_len = pkt.opts[i].buf.len;
		memcpy(msg->options[i].opt_val, pkt.opts[i].buf.p, pkt.opts[i].buf.len * sizeof(U8));
	}
	//标准uapps选项复制
	msg->numOfStandardUappsOption = pkt.numOfOptionStandardUappsMsg;
	for(i = 0; i < pkt.numOfOptionStandardUappsMsg; i++)
	{
		msg->optionsOfStandardUappsMsg[i].opt_code = pkt.optionOfStandardUappsMsg[i].num;
		msg->optionsOfStandardUappsMsg[i].opt_len = pkt.optionOfStandardUappsMsg[i].buf.len;
		memcpy(msg->optionsOfStandardUappsMsg[i].opt_val, pkt.optionOfStandardUappsMsg[i].buf.p, pkt.optionOfStandardUappsMsg[i].buf.len * sizeof(U8));
	}
	msg->pl_len = pkt.payload.len;
	msg->pl_ptr = pkt.payload.p;
	return UAPPS_OK;
} 

/**
 * Get option ptr in message width specified opt code
 */
/*--------------------------------------------------------------
函数名称：UappsGetOption
函数功能：从Uapps报文结构中提取指定选项码的选项结构体
输入参数：msg-待处理的Uapps报文结构体
        opt_code-待提取的选项码
输出参数：无
返回值：  提取的选项结构体指针
备 注：
---------------------------------------------------------------*/
UappsOption *UappsGetOption(UappsMessage *msg, u16 opt_code)
{
	u8 i;
	UappsOption *opt;

	if (msg->num_opts == 0)
	{
		return NULL;
	}

	opt = NULL;
	for (i = 0; i < msg->num_opts; i++)
	{
		if (msg->options[i].opt_code == opt_code)
		{
			opt = &msg->options[i];
			break;
		}
	}

	return opt;
}

/*--------------------------------------------------------------
函数名称：UappsGetIntOpt
函数功能：从Uapps报文结构中提取指定选项码的数值型选项值
输入参数：msg-待处理的Uapps报文结构体
        opt_code-待提取的选项码
输出参数：val - 提取指定选项码的数值型选项值
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsGetIntOpt(UappsMessage *pMsg, u16 opt_code, u16 *val)
{
	UappsOption *opt;
	opt = UappsGetOption(pMsg,opt_code);
	if (opt == NULL)
	{
		return UAPPS_NULL;
	}
	
	if (opt->opt_val != NULL && opt->opt_len >= 2)
	{
		//*val = (u16)((opt->opt_val[0] << 8) | (opt->opt_val[1] & 0xff));
		*val = opt->opt_val[0] + ((opt->opt_val[1] & 0x00ff) << 8);
	}

	return UAPPS_OK;
}

/*--------------------------------------------------------------
函数名称：UappsGetStrOpt
函数功能：从Uapps报文结构中提取指定选项码的字符串型选项值
输入参数：msg-待处理的Uapps报文结构体
        opt_code-待提取的选项码
输出参数：s - 提取指定选项码的字符串型选项值
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsGetStrOpt(UappsMessage *pMsg, u16 opt_code, char *s)
{
	UappsOption *opt;
	opt = UappsGetOption(pMsg,opt_code);
	if (opt == NULL)
	{
		return UAPPS_NULL;
	}

	if (opt->opt_val != NULL)
	{
		opt->opt_val[opt->opt_len] = 0;
		strcpy(s,(const char*)opt->opt_val);
	}

	return UAPPS_OK;
}


UappsError UappsGetRsl(UappsMessage *pMsg, RSL_t *pRsl)
{
	UappsOption *opt;
	opt = UappsGetOption(pMsg,UAPPS_OPT_URI_PATH);
	if (opt == NULL)
	{
		return UAPPS_INVALID;
	}

	return RslFromOpt(pRsl,opt);
}

//add by Leonard for zaidai mac
UappsError UappsGetNbMac(UappsMessage *pMsg, char* mac)
{
	UappsOption *opt = NULL;
	opt = UappsGetOption(pMsg, UAPPS_OPT_NB_MAC);
	if (opt == NULL) {
		return UAPPS_INVALID;
	}

	if(opt->opt_len != 12) {
		return UAPPS_INVALID;
	}

	memcpy(mac, opt->opt_val, opt->opt_len);

	return UAPPS_OK;
}
//end add

UappsError UappsSetNbMac(UappsMessage *pMsg, char* mac)
{
    return UappsPutStrOption(pMsg, UAPPS_OPT_NB_MAC, (u8*)mac);
}

/*--------------------------------------------------------------
函数名称：UappsMatchMessage
函数功能：比较两个Uapps报文是否匹配
输入参数：pMsg1-待比较的Uapps报文1
        pMsg2-待比较的Uapps报文2
输出参数：无
返回值：  0-不匹配，1-匹配
备 注：
---------------------------------------------------------------*/
u8 UappsMatchMessage(UappsMessage *pMsg1, UappsMessage *pMsg2)
{
	u8 i;

	if (pMsg1->hdr.id != pMsg2->hdr.id)
	{
		return 0;
	}

	for (i = 0; i < pMsg1->hdr.tkl; i++)
	{
		if (pMsg1->token[i] != pMsg2->token[i])
		{
			return 0;
		}
	}

	return 1;

}

/*--------------------------------------------------------------
函数名称：UappsSetId
函数功能：设置Uapps报文中报文头的ID
输入参数：id-待设置的Uapps报文中报文头的ID
输出参数：msg-输出的报文结构体指针
返回值： 无
备 注：
---------------------------------------------------------------*/
void UappsSetId(UappsMessage *msg, u16 id)
{
	msg->hdr.id = id;
}

/*--------------------------------------------------------------
函数名称：UappsSetToken
函数功能：设置Uapps报文的Token
输入参数：token-待设置的Uapps报文的token
         tkl-待设置的Uapps报文的token长度
输出参数：msg-输出的报文结构体指针
返回值：  无
备 注：
---------------------------------------------------------------*/
void UappsSetToken(UappsMessage *msg, u8 *token, u8 tkl)
{
	if(tkl > 8)
	{
		return;
	}
	msg->hdr.tkl = tkl;
	memcpy(msg->token, token, tkl * sizeof(U8));
}


/*--------------------------------------------------------------
函数名称：UappsGetSize
函数功能：获取载荷数据长度
输入参数：pMsg-uapps结构体
         			 size-载荷数据长度
         			 blockFlag-分块标志，0 - 不分块，1 - 分块
         			 blockNum - 块序号
返回值：  错误码
备 注：
---------------------------------------------------------------*/
UappsError UappsGetSize(UappsMessage *pMsg, u16 *size, u8 *blockFlag, u8 *blockNum)
{
	UappsOption *opt;
	
	opt = UappsGetOption(pMsg,UAPPS_OPT_SIZE);
	if (opt == NULL)
	{
		return UAPPS_INVALID;
	}
	if(opt->opt_len == 1)
	{
		*size = opt->opt_val[0];
	}
	else if(opt->opt_len == 2)
	{
		*size = opt->opt_val[1] + ((opt->opt_val[0] & 0x00ff) << 8) + 256;
	}
	else if(opt->opt_len == 3)
	{
		*blockFlag = 1;
		*blockNum = opt->opt_val[0];
		*size = opt->opt_val[2] + ((opt->opt_val[1] & 0x00ff) << 8);
	}
	return UAPPS_OK;
}

/*--------------------------------------------------------------
函数名称：UappsSetSize
函数功能：设置载荷数据长度
输入参数：pMsg-uapps结构体
         			 size-载荷数据长度
         			 blockFlag-分块标志，0 - 不分块，1 - 分块
         			 blockNum - 块序号
返回值：  错误码
备 注：
---------------------------------------------------------------*/
UappsError UappsSetSize(UappsOption *opt, U16 size, U8 blockFlag, U8 blockNum)
{

	if (opt == NULL)
	{
		return UAPPS_INVALID;
	}
	opt->opt_code = UAPPS_OPT_SIZE;

	if(blockFlag)
	{
		opt->opt_len = 3;
		opt->opt_val[0] = blockNum;
		opt->opt_val[1] = size >> 8;
		opt->opt_val[2] = (U8)size;
	}
	else
	{
		if(size < 256)
		{
			opt->opt_len = 1;
			opt->opt_val[0] = (U8)size;
		}
		else
		{
			opt->opt_val[0] = (size & 0xff00) >> 8;
			opt->opt_val[1] = (U8)size;
		}
	}

	return UAPPS_OK;
}

/*--------------------------------------------------------------
函数名称：UappsGetFormat
函数功能：获取载荷数据类型和编码方式
输入参数：pMsg-uapps结构体
         			 type-载荷数据类型指针
         			 encoding-载荷数据编码方式
返回值：  错误码
备 注：
---------------------------------------------------------------*/
UappsError UappsGetFormat(UappsMessage *pMsg, U16 *type, U16 *encoding)
{
	UappsOption *opt;
	U16 extandType = 0;

	
	opt = UappsGetOption(pMsg,UAPPS_OPT_FORMAT);
	if (opt == NULL)
	{
		return UAPPS_INVALID;
	}

	*type = (opt->opt_val[0] & 0xf0) >> 4;
	*encoding = opt->opt_val[0] & 0x0f;

	if(opt->opt_len > 1)
	{
		extandType = opt->opt_val[1];
	}

	*type += (extandType<<4);

	return UAPPS_OK;
}	

/*--------------------------------------------------------------
函数名称：UappsSetSize
函数功能：设置载荷数据长度
输入参数：pMsg-uapps结构体
         			 type-载荷数据类型
         			 encoding-编码方式
返回值： 错误码
备 注：
---------------------------------------------------------------*/
UappsError UappsSetFormat(UappsOption *opt, U16 type, U16 encoding)
{
	U8 temp = 0;
	if (opt == NULL)
	{
		return UAPPS_INVALID;
	}
	opt->opt_code = UAPPS_OPT_FORMAT;
	if(type > FORMAT_EXTAND_TYPE)
	{
		opt->opt_len = 2;
		temp = type&0xf;
		opt->opt_val[0] = (temp << 4) | (U8)encoding;
		opt->opt_val[1] = (type>>4);
	}
	else
	{
		opt->opt_len = 1;
		opt->opt_val[0] = ((U8)type << 4) | (U8)encoding;
	}
	return UAPPS_OK;
}
/**
 * Construct response message
 */
void UappsResponse(UappsMessage *req, UappsMessage *res, u8 type, u8 hdrCode)
{
	// Header
	res->hdr.ver    = UAPPS_VER;
	res->hdr.type   = type;
	res->hdr.tkl    = req->hdr.tkl;
	res->hdr.hdrCode = hdrCode;
	res->hdr.id     = req->hdr.id;
	// Token
	if (res->hdr.tkl > 0) {
		memcpy(res->token, req->token, req->hdr.tkl); 
	}

	// Options
	res->num_opts = 0;
 
		
	// Content
	res->pl_ptr = NULL;
	res->pl_len = 0;
}

u16 UappsGetIdNew()
{
    return UappsGenId();
}

u16 UappsUpdateStrOption(UappsMessage *msg, u16 num, u8 *str)
{
    int i = 0;
    UappsOption *opts =  msg->options;
    UappsError uerr = UAPPS_ERROR;
	if (msg->num_opts >= UAPPS_MAX_OPTIONS)
	{
		return UAPPS_TOO_MANY_OPTS;
	}


	if (str == NULL || strlen((const char*)str) == 0)
	{
		return UAPPS_INVALID;
	}
    //find opton   
	for (i = 0; i < msg->num_opts; i++)
	{
		if (opts[i].opt_code == num)
		{
            memcpy(opts[i].opt_val, str, strlen((char *)str));
            opts[i].opt_len = strlen((char *)str);
            uerr = UAPPS_OK;
            break;
		}
	}    
	return uerr;

}


UappsError UappsAddFromOption(UappsMessage *pUappsMsg, const char *srcMacAddr)
{
    U8 fromOptionString[50];
    U16 len = 0;
    len = strlen((const char *)srcMacAddr);
    if (len == 0 )
    {
        return UAPPS_ERROR;
    }
    len = sprintf((char *)fromOptionString, "@0.%s", srcMacAddr);

    if (UAPPS_OK == UappsPutOption(pUappsMsg, UAPPS_OPT_SRC_RSL, fromOptionString, len))
    {
        return UAPPS_OK;
    }
    else
    {
        return UAPPS_ERROR;
    }
}

UappsError UappsGetNodaFromOptionFROM(UappsMessage *pUappsMsg, char *srcMacAddr)
{
    RSL_t rsl = {0};

    for (U16 i = 0; i < pUappsMsg->num_opts; i++)
    {
        if (pUappsMsg->options[i].opt_code == UAPPS_OPT_SRC_RSL)
        {
            if (RslFromBytes(&rsl, pUappsMsg->options[i].opt_val, pUappsMsg->options[i].opt_len) == UAPPS_OK)
            {
                if (strlen(rsl.noda) == 12)
                {
                    strcpy(srcMacAddr, rsl.noda);
                    return UAPPS_OK;
                }
            }
        }
    }

    return UAPPS_ERROR;
}



UappsMessage * UappsBuildReq(char *rsl, u8 type, u8 hdrCode, char *payload, U32 len)
{
    UappsMessage * msg = NULL;

    msg = Lme_malloc(sizeof(UappsMessage));
    if (msg == NULL)
    {
        LOGD("UappsMessage malloc fail\n");
        return NULL;
    }
    memset(msg, 0, sizeof(UappsMessage));
    UappsCreateMessage(msg, UAPPS_TYPE_CON, UAPPS_REQ_GET, rsl);
    if (payload != NULL)
    {
        UappsPutData(msg, payload, len, UAPPS_FMT_OCTETS, 0);
    }
    return msg ;
}




#ifdef __cplusplus
}
#endif


