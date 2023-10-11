/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved。
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC。
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code。
 *
 * FileName    : LmeUtils.c
 * Author      :
 * Date        : 2020-05-09
 * Version     : 1.0
 * Description : APS层的工具的源文件
 *             :
 * Others      :
 * ModifyRecord:
 *
********************************************************************************/
#include "Common.h"

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
u8 str2bcd(char *str, u8 len, u8 *buf)
{
	int i;
	u8 sz, remainder, j;

	sz = len >> 1;
    remainder = len & 0x01;
	j = 0;
   	// 转BCD码
   	for (i = 0; i < sz; i++) 
	{
    	buf[i] = ((str[j++] - '0') << 4);
		buf[i]|= (str[j++]-'0');
	}
   	// 如果存在余数，需要填f
   	if (remainder > 0) 
	{
		buf[i] = ((str[j++] - '0') << 4) | 0x0f;
	}

   	return (sz+remainder);
}

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
u8 bcd2str(u8 *buf, u8 len, char *str)
{
	int i;
	u8 j, b;

	j = 0;
	for (i = 0; i < len; i++) 
	{
		b = buf[i];
		str[j++] = '0'+ (b >> 4);
		b &= 0x0f;
		if (b == 15)
			break;
		str[j++] = '0' + b;
	}

	str[j] = 0;

	return j;
}

// Decimal value of hex digit   
u8 hex_val(char hex)
{
	if ((hex >= '0') && (hex <= '9'))
		return (hex - '0');
	else if ((hex >= 'a') && (hex <= 'f'))
		return (hex - 'a' + 10);
	else if ((hex >= 'A') && (hex <= 'F'))
		return (hex - 'A' + 10);

	return 0;
}


/**
 * Encode hex string "0123456789abcdef" into bytes, 2 digits into one byte.
 * len must be even. 
 * The string may starts with "0x" or "0X"
 * 返回编码后的字节数 
*/
/*--------------------------------------------------------------
函数名称：str2hex
函数功能：将字符串转为十六进制格式（HEX）。
输入参数：str-待转换的字符串
        len-字符串长度
输出参数：buf-转换成HEX格式的字节数组
返回值：  转换后的BCD数组长度
备 注：    待转换的字符串必须以"0x" or "0X"开头
---------------------------------------------------------------*/
U32 str2hex(char *str, U32 len, u8 buf[])
{
	int i;
	U32 sz, j;
	char *p;

	p = str;
	if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X'))
		p += 2;

	sz = len >> 1;
	j = 0;
   	// 转HEX码
   	for (i = 0; i < sz; i++) 
	{
    	buf[i] = (hex_val(p[j++]) << 4);
		buf[i] |= hex_val(p[j++]);
	}
 	// 
   	return sz;
}

/**
 * Decode HEX in buffer to string。
 * 返回解码后的长度
 */
/*--------------------------------------------------------------
函数名称：hexTostr
函数功能：将十六进制格式（HEX）转为字符串。
输入参数：buf-待转换的HEX数组
        len-HEX数组长度
        prefix - 转换后的字符串前缀
输出参数：str-转换成字符串
返回值：  转换后的字符串长度
备 注：    待转换的字符串必须以"0x" or "0X"开头
---------------------------------------------------------------*/
U32 hexTostr(u8 *buf, U32 len, char *prefix, char *str)
{
	U32 i, j;
	U32 b;

	j = 0;
	if (prefix != NULL)
	{
		strcpy(str,prefix);
		j = strlen(prefix);
	}
		
	for (i = 0; i < len; i++) 
	{
		b = buf[i] >> 4;
		if (b <= 9)
			str[j++] = '0'+ b;
		else {
			str[j++] = 'a' + b-10;
		}

		b = buf[i] & 0x0f;
		if (b <= 9)
			str[j++] = '0'+ b;
		else {
			str[j++] = 'a' + b-10;
		}
	}

	str[j] = 0; 

	return j;
}

/*--------------------------------------------------------------
函数名称：isNumber
函数功能：判断字符是否代表数字。
输入参数：s-待判断的字符
输出参数：无
返回值：  1-是数字，0-不是数字
备 注：    无
---------------------------------------------------------------*/
u8 isNumber(char *s)
{
	char c;
	while(*s != 0)
	{
		c = *s++;
		if (c < '0' || c > '9')
			return 0;
	} 

	return 1;
}

/*--------------------------------------------------------------
函数名称：isNumber
函数功能：判断字符是否代表十六进制数字。
输入参数：s-待判断的字符
输出参数：无
返回值：  1-是十六进制数字，0-不是十六进制数字
备 注：    无
---------------------------------------------------------------*/
u8 isHex(char *s)
{
	const char *hex = "0123456789abcdefABCDEF";
	char c;

	if (*s != '0' || (*(s+1) != 'x' && *(s+1) != 'X')) 
		return 0;

	s += 2;
	while (*s != 0) 
	{ 
		c = *s++;
		if (strchr(hex,c) == NULL)
			return 0;
	}

	return 1;
}

/*--------------------------------------------------------------
函数名称：isHexString
函数功能：判断字符是否代表十六进制数字。
输入参数：s-待判断的字符
	   inlen-输入的字符串长度
输出参数：无
返回值：  1-是十六进制字符串，0-不是十六进制字符串
备 注：   add by Leonard
---------------------------------------------------------------*/
int isHexString(char *s, uint32_t inlen)
{
    if (s == NULL) {
        return 0;
    }

    for (int i = 0; i < inlen; i++) {
        if ((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'A' && s[i] <= 'F') || (s[i] >= 'a' && s[i] <= 'f')) {
            return 0;
        }
    }

    return 1;
}

/*--------------------------------------------------------------
函数名称：Itoa
函数功能：将数字转成字符。
输入参数：n-待转换的数
输出参数：s-转换后的字符串
返回值：  无
备 注：    无
---------------------------------------------------------------*/
void Itoa(long n,char s[])
{
	int i,j;
	long sign;
	char tmp[12];

	if((sign=n)<0)//记录符号
		n=-n;//使n成为正数

	i=0;
	do {
		tmp[i++]=n%10+'0';//取下一个数字
	} while ((n/=10)>0);//删除该数字

	if(sign<0)
		tmp[i++]='-';
	
	i--;
	j = 0;
	while(i >= 0)
		s[j++] = tmp[i--];

	s[j] = '\0';

}


#define isUpper(x) (x>='A' && x<='Z') //判断是大写字符。
#define isLower(x) (x>='a' && x<='z') //判断是小写字符。
#define toLowerCase(x) (x-'A'+'a')    //转为小写
#define toUpperCase(x) (x-'a'+'A')    //转为大写

/*--------------------------------------------------------------
函数名称：toLower
函数功能：转为小写字母。
输入参数：s-待转换的字母
输出参数：s-转换后的小写字母
返回值：  无
备 注：    无
---------------------------------------------------------------*/
void toLower(char *s)
{
	int i;

	if (s[0] == 0)
	{
		return;
	}

	for(i = 0; s[i]; i++)
	{
		if (isUpper(s[i])) 
		{
			s[i] = toLowerCase(s[i]); //如果是大写字符，转为小写。
		}
	}
}

/*--------------------------------------------------------------
函数名称：toUpper
函数功能：转为大写字母。
输入参数：s-待转换的字母
输出参数：s-转换后的大写字母
返回值：  无
备 注：    无
---------------------------------------------------------------*/
void toUpper(char *s)
{
	int i;

	if (s[0] == 0)
	{
		return;
	}

	for(i = 0; s[i]; i++)
	{
		if (isLower(s[i])) 
		{
			s[i] = toUpperCase(s[i]); 
		}
	}
}








