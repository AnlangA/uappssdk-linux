
/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : Uapps.h
 * Author      : baod
 * Date        : 2020-05-09
 * Version     : 1.0
 * Description : APS层的Uapps API的头文件
 *             :
 * Others      :
 * ModifyRecord: 
 *
********************************************************************************/
#ifndef		_UAPPS_H
#define		_UAPPS_H
#include "Common.h"
#ifdef __cplusplus
extern "C"{
#endif

/**********************************宏定义 开始*************************************/
#define MAXOPT 16

#define URI_HOST_LEN	20	// Max uncoded str length
#define URI_PATH_LEN	50	//
#define URI_QUERY_LEN	20
#define PATHSEG_MAX_LEN		10

#define UAPPS_VER			1	// Uapps version
#define UAPPS_TKL			2	// TKL default

// Uapps Configurations
#define UAPPS_MAX_OPTIONS	5	// max options in a message
#define UAPPS_MAX_OPTLEN		48	// opt value max size
// Escapse sequence
#define ESC_CHAR	 		'\x1b'  // ESC char
//#define ESC_STRING			"\n\?\x1b\x1b\x1b\x1b\n\a"	/*0A 3F 1B 1B 1B 1B 0A 07*/
#define ESC_STRING			"\x1b\x1b\x1b\x1b\x1b\n\x1b\n"	/*1B 1B 1B 1B 1B 0A 1B 0A*/

#define ESC_LEN				8

// Node type (role)
#define PLC_MASTER	    (1)
#define PLC_SLAVE	    (0)

// Protocol
#define nPLBus		    (1)
#define hPLBus		    (2)
#define PLBus512	    (3)
#define gHPLC		    (4)
#define sHPLC		    (5)

// Profile
#define APP_HOME		'h'
#define APP_INDUSTRY	'i'

// Index into config buffer
#define INDX_ROLE		(0)
#define INDX_PROTO		(1)
#define INDX_APP		(2)
#define INDX_VER		(3)
#define INDX_YEAR		(4)
#define INDX_MONTH		(6)
#define INDX_DAY		(7)
#define INDX_ID			(8)
#define INDX_APS		(9)
#define INDX_NWK		(10)
#define INDX_LL			(11)


#define AEI_MAX_LEN 		12		// AEI最大长度
#define NODA_MAX_LEN		14
#define GWI_MAX_LEN			12
#define RES_PATH_MAXSZ		12
#define RSI_MAXSZ_MAXSZ		8
// String compression type
#define ENC_TYPE_STR		0
#define ENC_TYPE_BCD		1
#define ENC_TYPE_HEX		2


/**********************************宏定义 结束*************************************/


/**********************************枚举声明 开始************************************/
// Uapps error code
typedef enum {
	UAPPS_OK			= 0,
	UAPPS_ERROR			= 1, // general error
	UAPPS_INVALID		= 2, // invalid parameters or data
	UAPPS_NULL			= 3, // null error
	UAPPS_TOO_MANY_OPTS	= 4, // too many options
	UAPPS_EMPTY_OPTIONS	= 5, // 0 options in message
	UAPPS_OPT_NOT_FOUND	= 6, // specified opt not found in opt list
	UAPPS_RSL_INVALID   = 7,
	UAPPS_RSL_SYNTAXERR = 8	// RSL_t syntax error
} UappsError;

// Message type
typedef enum {
	UAPPS_TYPE_CON	= 0,
	UAPPS_TYPE_NON 	= 1,
	UAPPS_TYPE_ACK	= 2,
	UAPPS_TYPE_RST	= 3
} Uapps_TypeEnum;

// Method code
// 0.dd
typedef enum {
	UAPPS_EMPTY_MSG  	=	0x00,	//0.0
	UAPPS_REQ_GET		=	0x01,	//0.1
	UAPPS_REQ_POST		=	0x02,	//0.2
	UAPPS_REQ_PUT		=	0x03,	//0.3
	UAPPS_REQ_DELETE		=	0x04	//0.4
} Uapps_MethodEnum;

// 2.dd, positive response
typedef enum {
	UAPPS_ACK_CREATED	=	0x41,	//2.01, succefully created
	UAPPS_ACK_DELETED	=	0x42,	//2.02, deleted
	UAPPS_ACK_VALID		=	0x43,	//2.03, valid
	UAPPS_ACK_CHANGED	=	0x44,	//2.04, changed
	UAPPS_ACK_CONTENT	=	0x45	//2.05, content
} Uapps_AckCode;

typedef enum {
	// 4.dd, request error
	UAPPS_BAD_REQUEST	=	0x80,	//4.0, bad request
	UAPPS_UNAUTHORIZED	=	0x81,	//4.1
	UAPPS_BAD_OPTION		=	0x82,	//4.2
	UAPPS_FORBIDDEN		=	0x83,
	UAPPS_NOT_FOUND		=	0x84,
	UAPPS_BAD_METHOD		=	0x85,
	UAPPS_BAD_DATA		=	0x86,	// data not accepted
	UAPPS_BAD_FORMAT		=	0x8f,
	// 5.dd, server error
	UAPPS_SERVER_ERROR		= 0xa0,	//5.00, Error in server
	UAPPS_NOT_IMPLEMENTED	= 0xa1,	//5.01, not supported by server
	UAPPS_SERVICE_UNAVAIL	= 0xa3,	//5.03, service currently not available
	UAPPS_INVALID_PROXY		= 0xa5	//5.05, proxying not supported
} Uapps_ResCode;

// Options
typedef enum {
	UAPPS_OPT_RSV0			= 0,	// reserved
	UAPPS_OPT_MATCH			= 1,	// If match
	UAPPS_OPT_URI_HOST		= 3,	// uri-host
	UAPPS_OPT_ETAG			= 4,	// ETag
	UAPPS_OPT_NONE_MATCH		= 5,	// If none match
	UAPPS_OPT_URI_PORT		= 7,	// Uri-port
	UAPPS_OPT_LOC_PATH		= 8,	// Location-path
	UAPPS_OPT_URI_PATH		= 10,	// Uri-path
	UAPPS_OPT_FORMAT			= 11,	// Content-format
	UAPPS_OPT_SIZE			= 12,	// 内容数据长度
	UAPPS_OPT_SRC_RSL		= 13,	// 源资源和服务项寻址
	UAPPS_OPT_MAX_AGE		= 14,	// Max-age
	UAPPS_OPT_URI_QUERY		= 15,	// Uri-query
	//add by Leonard
	UAPPS_OPT_NB_MAC		    = 16,	// ZAIDAI Specail option
	//end add
	UAPPS_OPT_ACCEPT			= 17,	// Accept
	UAPPS_OPT_MCU_GROUP      = 18,    // 用于外部MCU群组控制时存储aei。
	UAPPS_OPT_LOC_QUERY		= 20,	// Location-query
	UAPPS_OPT_BLOCK_SIZE2    = 28,   // 分块传输
	UAPPS_OPT_PROXY_URI		= 35,	// Proxy-uri
	UAPPS_OPT_PROXY_SCHEME	= 39,	// Proxy-scheme
	UAPPS_OPT_SIZE1			= 60	// Size1
} Uapps_OptEnum;
 
// Content format
typedef enum
{
	UAPPS_FMT_TEXT		= 0,
	UAPPS_FMT_OCTETS     = 1,	//Application/octet-stream
	UAPPS_FMT_INT		= 2,
	UAPPS_FMT_FLOAT		= 3,
	UAPPS_FMT_STR		= 4,
	UAPPS_FMT_HTML		= 5,
	UAPPS_FMT_XML		= 6,
	UAPPS_FMT_JSON		= 7
} Uapps_FormatEnum;


/**********************************枚举声明 结束************************************/

/**********************************结构体声明 开始***********************************/
typedef struct
{
	u8 host[URI_HOST_LEN+1]; 	// Uncoded str, add 0x00 to end
	u8 hasPort;
	u16 port;
	u8 path[URI_PATH_LEN+1]; 	// byte array, need to convert to RESI
	u8 query[URI_QUERY_LEN+1]; 	// Uncoded str, add 0x00 to end
} UappsUri;

// Message header
typedef struct
{
	u8 tkl:4;
	u8 type:2;
	u8 ver:2;
	u8 hdrCode;
	u16 id; 		// Note: transmit in network byte order, i.e., MSB first
} UappsHeader;

// Message option
typedef struct
{
	u16 opt_code;  	// opt code
	u8 opt_len;
	u8 opt_val[UAPPS_MAX_OPTLEN+1]; // hold value bytes
} UappsOption;

// Uapps message struct
typedef struct
{
	UappsHeader hdr;// header
	u8 token[8];//When sending, the actual length is determined by hdr.tkl
	u8 num_opts;// Number of options
	UappsOption options[UAPPS_MAX_OPTIONS];//Variable options list, up to UAPPS_MAX_OPTIONS
	u8 numOfStandardUappsOption;
	UappsOption optionsOfStandardUappsMsg[UAPPS_MAX_OPTIONS];
	u8 *pl_ptr; // Payload data pointer. Therefore, the data itself is not included in the structure, but the pointer points to the data buffer
	u16 pl_len; // payload length
	u8 txState;	//发送状态，1 - 发送中，0 - 发送完成
	u8 txResult; // 发送结果
	u16 txTimeOut; // 单次发送超时
	u8 maxTries;	// 剩余发送尝试次数。发送一次后减1，若为0则不再尝试
} UappsMessage;


typedef struct
{
	u8 bf;   //分块标志位：0-单块，1-多块
	u8 blkn; //分块序号
	u16 len; //分块数据的长度
} UappsBlk_t;

#define UAPPS_MSG_CODE(msg)          (msg->hdr.hdrCode)
#define UAPPS_MSG_IS_EMPTY(msg)      (UAPPS_MSG_CODE(msg) == 0)
#define UAPPS_MSG_IS_REQUEST(msg)    (!UAPPS_MSG_IS_EMPTY(msg) && UAPPS_MSG_CODE(msg) < 32)
#define UAPPS_MSG_IS_RESPONSE(msg)   (UAPPS_MSG_CODE(msg) >= 64 && UAPPS_MSG_CODE(msg) < 224)
#define UAPPS_MSG_IS_SIGNALING(msg)  (UAPPS_MSG_CODE(msg) >= 224)
#define UAPPS_MSG_IS_CON(msg)        (msg->hdr.type == UAPPS_TYPE_CON)

//add by Leonard
#define UAPPS_MSG_IS_NON(msg)        (msg->hdr.type == UAPPS_TYPE_NON)
//end add

/*
typedef struct
{
	u8 baudrate; //波特率， 0x00表示1200,0x01表示2400，0x02表示4800,0x03表示9600,
	            //0x04表示19200，0x05表示38400，0x06表示57600，0x07表示115200
	u8 parity;	   //校验位，0-none, 1=odd, 2=even
	u8 uart_save_flag; //保存标志，0-不存到flash中，1-保存到flash中
} UART_CONFIG;
typedef struct
{
    u8 role;          //0=slave, 1=master
    u8 proto;         //1=nPLBus,表示窄带载波
                      //2=hPLBus,表示高速载波
                      //3=PLBus512,
                      //4=gHLC,
                      //5=sHPLC,  应用场景用.x表示,.hn?表示?家庭网络,.i表示工业网络
    u8 app_profile;   //’h’ - home, 'i' = industry
    u8 chip_type;     //芯片类型，0-3460CC,1-34600DE
    u8 fw_ver[6];
                      //fw_ver[0]版本高位
                      //fw_ver[1]版本低位
                      //fw_ver[2]年高位、fw_ver[3]年低位
                      //fw_ver[4]表示月份，fw_ver[5]表示日，时间信息按照BCD编码，MSB传输
    u8 nwk_ver[3];    //表示网络层版本信息,Nxxxx
    u8 layer_ver[2];  //表示底层版本信息，xxxx 
    u8 reserv[8];     //保留信息
} NODE_INFO;

typedef struct
{
   	  u16 did;		// did
	  u16 addr;		// short addr
      u8 bw;			// 1=3Mhz,2=2.2Mhz
	  u8 mode;		//1=low speed, 2=mid spped, 3=hi speed
	  u8 tx_power;	//0=low,1=mid,2=hi
	  u8 reserv[5];	// reserved		
} PLC_CONFIG;
*/

typedef struct {
	u8 valid;
	u8 hasPort;
	u16 port;
	char aei[AEI_MAX_LEN+1];
	char noda[NODA_MAX_LEN+1];
	char gwi[GWI_MAX_LEN+1];
	char resPath[RES_PATH_MAXSZ+1];
	char resName[RSI_MAXSZ_MAXSZ+1];
}RSL_t;


typedef struct
{
    u8 ver;                		/* Uapps version number */
    u8 t;                  		/* Uapps Message Type */
    u8 tkl;                     /* Token length: indicates length of the Token field */
    u8 codeOfUappsHead;          /* Uapps status codeOfUappsHead. Can be request (0.xx), success reponse (2.xx),
                                 * client error response (4.xx), or rever error response (5.xx) */
    u8 id[2];
} uapps_header_t; //报文头


typedef struct
{
    u8 *p;
    u16 len;
} uapps_buffer_t; //报文数据

typedef struct
{
    u16 num;                /* Option number. */
    uapps_buffer_t buf;      /* Option value */
} uapps_option_t; //报文选项

typedef struct
{
    uapps_header_t hdr;          /* Header of the packet */
    uapps_buffer_t tok;          /* Token value, size as specified by hdr.tkl */
    u8 numopts;                 //内容头选项数量
    uapps_option_t opts[MAXOPT]; //内容头选项
	u8 numOfOptionStandardUappsMsg;	//标准uapps选项数量
	uapps_option_t optionOfStandardUappsMsg[MAXOPT];  //标准uapps选项
    uapps_buffer_t payload;      /* Payload carried by the packet */
} uapps_packet_t; //报文

typedef struct
{
    u8 *p;
    u16 len;
} uapps_rw_buffer_t;

/**********************************结构体声明 结束***********************************/


/**********************************静态变量声明 开始**********************************/


/**********************************静态变量声明 结束**********************************/

/**********************************函数列表  开始************************************


***********************************函数列表  结束***********************************/



/***********************************函数声明 开始***********************************/

/*-----------------------Uapps 开始---------------------------------------*/
/**
 * Construct Uapps message struct for specified type, hdrCode and uri 
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
UappsError UappsCreateMessage(UappsMessage *msg, u8 type, u8 hdrCode, char *rslStr);

/*-----------------------Uapps 开始---------------------------------------*/
/**
 * Construct Uapps request message struct for specified type, hdrCode and uri with payload 
 */
/*--------------------------------------------------------------
函数名称：UappsBuildReq
函数功能：创建Uapps报文结构体
输入参数：type-Uapps报文头的报文类型
         hdrCode-Uapps报文头的code码
         rslStr-Uapps报文中的RSL
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/

UappsMessage * UappsBuildReq(char *rsl, u8 type, u8 hdrCode, char *payload, U32 len);

/**
 * Add string option to message's opt list. 
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
UappsError UappsPutStrOption(UappsMessage *msg, u16 opt_code, u8 *str);

/**
 * Add UINT option to message's opt list
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
UappsError UappsPutIntOption(UappsMessage *msg, u16 opt_code, u16 val);

/**
 * Add option into message's option list. val is in byte array.
 * Return UAPPS_TOO_MANY_OPTS if list is full.
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
UappsError UappsPutOption(UappsMessage *msg, u16 opt_code, u8 *val, u8 len);

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
UappsError UappsPutData(UappsMessage *msg, u8 *buf, u16 len, u16 fmt, u16 encoding);

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
UappsError UappsPutStrData(UappsMessage *msg, char *str);

/*--------------------------------------------------------------
函数名称：UappsPutData
函数功能：向Uapps报文结构体中添加分块数据载荷选项
输入参数：bf-数据载荷字符串
输出参数：msg-生成的Uapps报文结构体
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsPutDataBlock(UappsMessage *msg,u8 bf, u8 *buf, u16 len, u16 fmt, u16 encoding);

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
UappsMessage* UappsSimpleResponse(UappsMessage *reqMsg, UappsMessage *resMsg, u8 type, u8 resCode);

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
		u8* buf, u16 len, u16 fmt, u16 encoding);

/**
 * Convert Uapps message struct into bytes for transmit.
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
u16 Uapps2Bytes(UappsMessage *msg, u8 *buf);

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
UappsError UappsFromBytes(u8 *buf, u16 buf_len, UappsMessage *msg);

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
UappsOption *UappsGetOption(UappsMessage *msg, u16 opt_code);

/*--------------------------------------------------------------
函数名称：UappsGetIntOpt
函数功能：从Uapps报文结构中提取指定选项码的数值型选项值
输入参数：msg-待处理的Uapps报文结构体
        opt_code-待提取的选项码
输出参数：val - 提取指定选项码的数值型选项值
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsGetIntOpt(UappsMessage *pMsg, u16 opt_code, u16 *val);

/*--------------------------------------------------------------
函数名称：UappsGetStrOpt
函数功能：从Uapps报文结构中提取指定选项码的字符串型选项值
输入参数：msg-待处理的Uapps报文结构体
        opt_code-待提取的选项码
输出参数：s - 提取指定选项码的字符串型选项值
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsGetStrOpt(UappsMessage *pMsg, u16 opt_code, char *s);

/*--------------------------------------------------------------
函数名称：UappsGetRsl
函数功能：从Uapps报文结构中提取RSL
输入参数：msg-待处理的Uapps报文结构体指针
输出参数：pRsl - 提取的RSL结构体指针
返回值：  0-成功，非0-失败
备 注：
---------------------------------------------------------------*/
UappsError UappsGetRsl(UappsMessage *pMsg, RSL_t *pRsl);

/*--------------------------------------------------------------
函数名称：UappsGetNbMac
函数功能：从Uapps报文结构中提取载带的MAC地址
输入参数：msg-待处理的Uapps报文结构体指针
输出参数：mac - 保存提取出来的MAC地址[Out]
返回值：  0-成功，非0-失败
备 注： add by Leonard
---------------------------------------------------------------*/
UappsError UappsGetNbMac(UappsMessage *pMsg, char* mac);

/*--------------------------------------------------------------
函数名称：UappsSetNbMac
函数功能：给Uapps报文结构中设置载带的MAC地址
输入参数：msg-待处理的Uapps报文结构体指针
输出参数：mac - 待设置的MAC地址[Out]
返回值：  0-成功，非0-失败
备 注： add by Fu Baicheng
---------------------------------------------------------------*/
UappsError UappsSetNbMac(UappsMessage *pMsg, char* mac);

/*--------------------------------------------------------------
函数名称：UappsMatchMessage
函数功能：比较两个Uapps报文是否匹配
输入参数：pMsg1-待比较的Uapps报文1
        pMsg2-待比较的Uapps报文2
输出参数：无
返回值：  0-不匹配，1-匹配
备 注：
---------------------------------------------------------------*/
u8 UappsMatchMessage(UappsMessage *pMsg1, UappsMessage *pMsg2);


/*--------------------------------------------------------------
函数名称：UappsSetId
函数功能：设置Uapps报文中报文头的ID
输入参数：id-待设置的Uapps报文中报文头的ID
输出参数：msg-输出的报文结构体指针
返回值： 无
备 注：
---------------------------------------------------------------*/
void UappsSetId(UappsMessage *msg, u16 id);

/*--------------------------------------------------------------
函数名称：UappsSetToken
函数功能：设置Uapps报文的Token
输入参数：token-待设置的Uapps报文的token
         tkl-待设置的Uapps报文的token长度
输出参数：msg-输出的报文结构体指针
返回值：  无
备 注：
---------------------------------------------------------------*/
void UappsSetToken(UappsMessage *msg, u8 *token, u8 tkl);

/*-------------------------------------------------------------------------------
函数名称：	uapps_parse
函数功能：	报文解析函数
输入参数：       buf-待解析报文字节流的指针
            buflen - 待解析报文字节流长度
输出参数：	pkt-解析后的报文结构体
返回值：		0 - 成功，非0-组帧失败
备注说明：
--------------------------------------------------------------------------------*/
u16 uapps_parse(uapps_packet_t *pkt, u8 *buf, u16 buflen);
/*-------------------------------------------------------------------------------
函数名称：	uapps_build
函数功能：	报文组帧函数
输入参数：	pkt-待组帧的报文结构体
输出参数：       buf-组帧后的字节流指针
            buflen - 组帧后的字节流长度
返回值：		0 - 成功，非0-组帧失败
备注说明：
--------------------------------------------------------------------------------*/
u16 uapps_build(u8 *buf, u16 *buflen, uapps_packet_t *pkt);

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
UappsError UappsGetSize(UappsMessage *pMsg, u16 *size, u8 *blockFlag, u8 *blockNum);

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
UappsError UappsSetSize(UappsOption *opt, u16 size, u8 blockFlag, u8 blockNum);

/*--------------------------------------------------------------
函数名称：UappsGetFormat
函数功能：获取载荷数据类型和编码方式
输入参数：pMsg-uapps结构体
         			 type-载荷数据类型指针
         			 encoding-载荷数据编码方式
返回值：  错误码
备 注：
---------------------------------------------------------------*/
UappsError UappsGetFormat(UappsMessage *pMsg, u16 *type, u16 *encoding);

/*--------------------------------------------------------------
函数名称：UappsSetSize
函数功能：设置载荷数据长度
输入参数：pMsg-uapps结构体
         			 type-载荷数据类型
         			 encoding-编码方式
返回值： 错误码
备 注：
---------------------------------------------------------------*/
UappsError UappsSetFormat(UappsOption *opt, u16 type, u16 encoding);

/*-----------------------Uapps 结束---------------------------------------*/

/*-----------------------LmeUtil 开始---------------------------------------*/

/**
 * Decode string to BCD bytes, return BCD bytes length
 */

/*--------------------------------------------------------------
函数名称：str2bcd
函数功能：将字符串转为十进制格式（BCD）。
输入参数：str-待转换的字符串
        len-字符串长度
输出参数：buf-转换成BCD格式的字节数组
返回值：  转换后的BCD数组长度
备 注：
---------------------------------------------------------------*/
u8 str2bcd(char *str, u8 len, u8 *buf);

/**
 * Decode BCD bytes to string, return string length
 */
/*--------------------------------------------------------------
函数名称：bcd2str
函数功能：将十进制格式（BCD）转换为字符串。
输入参数：buf-待转换的BCD格式字节数组
        len-BCD格式字节数组大小
输出参数：str-转换后的字符串
返回值：  转换后字符串长度
备 注：
---------------------------------------------------------------*/
u8 bcd2str(u8 *buf, u8 len, char *str);


/**
 * hex/string convertion
 */
/*--------------------------------------------------------------
函数名称：str2hex
函数功能：将字符串转为十六进制格式（HEX）。
输入参数：str-待转换的字符串
        len-字符串长度
输出参数：buf-转换成HEX格式的字节数组
返回值：  转换后的BCD数组长度
备 注：
---------------------------------------------------------------*/
U32 str2hex(char *str, U32 len, u8 *buf);

/*--------------------------------------------------------------
函数名称：hex2str
函数功能：将十六进制格式（HEX）转为字符串。
输入参数：buf-待转换的HEX数组
        len-HEX数组长度
        prefix - 转换后的字符串前缀
输出参数：str-转换成字符串
返回值：  转换后的字符串长度
备 注：    待转换的字符串必须以"0x" or "0X"开头
---------------------------------------------------------------*/
U32 hexTostr(u8 *buf, U32 len, char *prefix, char *str);

/*--------------------------------------------------------------
函数名称：isNumber
函数功能：判断字符是否代表数字。
输入参数：s-待判断的字符
输出参数：无
返回值：  1-是数字，0-不是数字
备 注：    无
---------------------------------------------------------------*/
u8 isNumber(char *str);

/*--------------------------------------------------------------
函数名称：isNumber
函数功能：判断字符是否代表十六进制数字。
输入参数：s-待判断的字符
输出参数：无
返回值：  1-是十六进制数字，0-不是十六进制数字
备 注：    无
---------------------------------------------------------------*/
u8 isHex(char *str);

/*--------------------------------------------------------------
函数名称：isHexString
函数功能：判断字符是否代表十六进制数字。
输入参数：s-待判断的字符
	   inlen-输入的字符串长度
输出参数：无
返回值：  1-是十六进制字符串，0-不是十六进制字符串
备 注：   add by Leonard
---------------------------------------------------------------*/
int isHexString(char *s, uint32_t inlen);

/*--------------------------------------------------------------
函数名称：Itoa
函数功能：将数字转成字符。
输入参数：n-待转换的数
输出参数：s-转换后的字符串
返回值：  无
备 注：    无
---------------------------------------------------------------*/
void Itoa(long n, char s[]);

/*--------------------------------------------------------------
函数名称：toLower
函数功能：转为小写字母。
输入参数：s-待转换的字母
输出参数：s-转换后的小写字母
返回值：  无
备 注：    无
---------------------------------------------------------------*/
void toLower(char *s);

/*--------------------------------------------------------------
函数名称：toUpper
函数功能：转为大写字母。
输入参数：s-待转换的字母
输出参数：s-转换后的大写字母
返回值：  无
备 注：    无
---------------------------------------------------------------*/
void toUpper(char *s);

/*-----------------------LmeUtil 结束---------------------------------------*/

/*-----------------------RSL_t 开始---------------------------------------*/
/**
 * Convert RSL_t struct from string expression. Return OK or error code
 * String format: [[AEI][@port]]/seg1/seg2/.../name
 */
/*--------------------------------------------------------------
函数名称：RslFromStr
函数功能：将RSL字符串转为RSL结构体。
输入参数：rslStr-待转换的RSL字符串
输出参数：rsl-转换后RSL结构体
返回值：  0-成功，非0-失败
备 注：    无
---------------------------------------------------------------*/
UappsError RslFromStr(RSL_t *rsl, char *str);

/**
 * Convert RSL_t struct to string. Return ok or error code
 */
/*--------------------------------------------------------------
函数名称：Rsl2Str
函数功能：将RSL结构体转为RSL字符串。
输入参数：rsl-待转换的RSL结构体
输出参数：buf-转换后的RSL字符串
返回值：  0-成功，非0-失败
备 注：    无
---------------------------------------------------------------*/
UappsError Rsl2Str(RSL_t *rsl, char* buf);

/**
 * Convert RSL_t struct to bytes, return byte array length
 */
/*--------------------------------------------------------------
函数名称：Rsl2Bytes
函数功能：将RSL结构体转为RSL字节流。
输入参数：rsl-待转换的RSL结构体
         enc-编码方式：0-不压缩编码，1-压缩编码
输出参数：buf-转换后的RSL字节流
返回值：  0-成功，非0-失败
备 注：    无
---------------------------------------------------------------*/
u8 Rsl2Bytes(RSL_t *pRsl, u8* buf, u8 enc);

/**
 * Construct RSL_t struct from bytes, return OK or error code
 */
/*--------------------------------------------------------------
函数名称：RslFromBytes
函数功能：将RSL字节流转为RSL结构体。
输入参数：buf-待转换的RSL字节流数组
        buf_len-RSL字节流长度
输出参数：rsl-转换后的RSL结构体
返回值：  0-成功，非0-失败
备 注：    无
---------------------------------------------------------------*/
UappsError RslFromBytes(RSL_t *pRsl, u8* buf, u8 buf_len);

/**
 * Convert RSL_t struct to option struct, return OK or error code
 */
/*--------------------------------------------------------------
函数名称：Rsl2Option
函数功能：将RSL结构体转换为Uapps协议的Option。
输入参数：rsl-待转换的RSL结构体
         enc-RSL的编码方式：0-不压缩编码，1-压缩编码
         opt_code - 选项码
输出参数：opt-转换后的Uapps协议选项结构体
返回值：  0-成功，非0-失败
备 注：    无
---------------------------------------------------------------*/
UappsError Rsl2Option(RSL_t *pRsl, u8 opt_code, UappsOption *opt, u8 enc);

/**
 * Convert UappsOpton (URI_PATH) into RSL_t, return OK or error 
 */
/*--------------------------------------------------------------
函数名称：RslFromOpt
函数功能：将Uapps协议的Option转为RSL结构体。
输入参数：opt-待转换的Uapps协议的Option结构体
输出参数：rsl-转换后的RSL结构体
返回值：  0-成功，非0-失败
备 注：    无
---------------------------------------------------------------*/
UappsError RslFromOpt(RSL_t *gRsl, UappsOption *opt);
/**
 * Construct response message
 */
void UappsResponse(UappsMessage *req, UappsMessage *res,u8 type,u8 hdrCode);

#if 1
/**
 * Get resPath in supplied buffer. 
 * Return UAPPS_NULL if resPath is empty, otherwise return UAPPS_OK
 */
/*--------------------------------------------------------------
函数名称：RslGetPath
函数功能：从RSL字符串中提取RSL的路径。
输入参数：buf-RSL字符串
输出参数：rsl-提取路径后的RSL结构体
返回值：  0-成功，非0-失败
备 注：    无
---------------------------------------------------------------*/
UappsError RslGetPath(RSL_t *rsl, char buf[]);

/**
 * Return resName ptr in rsl. Return NULL if empty. 
 */
/*--------------------------------------------------------------
函数名称：RslGetName
函数功能：从RSL结构体中提取资源名。
输入参数：rsl-待处理的RSL结构体
输出参数：无
返回值：  提取的资源名
备 注：    无
---------------------------------------------------------------*/
char *RslGetName(RSL_t *rsl);

void UappsGenToken(u8* buf, u8 tkl);

u16 UappsGetIdNew();
u16 UappsUpdateStrOption(UappsMessage *msg, u16 opt_code, u8 *str);

UappsError UappsAddFromOption(UappsMessage *pUappsMsg, const char *srcMacAddr);

UappsError UappsGetNodaFromOptionFROM(UappsMessage *pUappsMsg, char *srcMacAddr);


#endif
/*-----------------------RSL_t 结束---------------------------------------*/

#ifdef __cplusplus
}
#endif

/***********************************函数声明 开始***********************************/
#endif
