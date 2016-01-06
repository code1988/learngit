/***************文件信息********************************************************************************
**文   件   名: compile_time.c
**说        明: 编译的时间源文件
**创   建   人: hxj
**创   建 日期: 2011-11-10 10:54
*******************************************************************************************************/
#include "compile_time.h"


/****************************************************************************************************
**名称:void get_compile_time_my_format ( char *time_st )
**功能:获取编译时间,时间格式为:2011-10-05 15:07:15
* 入口:time_st-返回编译时间字符串指针(长度为20个字符,包括结束符)
* 出口:无
**auth:hxj, date: 2011-11-10 14:15
*****************************************************************************************************/
void get_compile_time_my_format ( char *time_st )
{
    volatile char *comp_time[4] =
	{
		"compile time:",
		__DATE__,		//Nov 10 2011
		__TIME__,		//20:13:10
		"Jan 1,Feb 2,Mar 3,Apr 4,May 5,Jun 6,Jul 7,Aug 8,Sep 9,Oct 10,Nov 11,Dec 12",
	};
    char *p_dst=0;
    char *p_src=0;
    char i=0;
    
    if(0==time_st) return;


#define MY_CPY_LEN(PDST,PSRC,LEN)   \
    do                                  \
    {                                   \
        p_dst=(char *)(PDST);           \
        p_src=(char *)(PSRC);           \
        for(i=0;i<(LEN);++i )           \
        {                               \
            *p_dst++=*p_src++;          \
        }                               \
    }while(0)

    
	MY_CPY_LEN(time_st,comp_time[1],3);
    if('F'==time_st[0])
    {
        time_st[5]='0';
		time_st[6]='2';
    }
    else if('M'==time_st[0])
    {
        if('r'==time_st[2])
        {
            time_st[5]='0';
			time_st[6]='3';
        }
        else
        {
            time_st[5]='0';
			time_st[6]='5';
        }
    }
    else if('A'==time_st[0])
    {
        if('p'==time_st[1])
        {
            time_st[5]='0';
			time_st[6]='4';
        }
        else
        {
            time_st[5]='0';
			time_st[6]='8';
        }  
    }
    else if('J'==time_st[0])
    {
        if('a'==time_st[1])
        {
            time_st[5]='0';
			time_st[6]='1';
        }
        else
        {
            if('n'==time_st[2])
            {
                time_st[5]='0';
				time_st[6]='6';
            }
            else
            {
                time_st[5]='0';
				time_st[6]='7';
            }
        }
    }
    else if('S'==time_st[0])
    {
        time_st[5]='0';
		time_st[6]='9';
    }
    else if('O'==time_st[0])
    {
        time_st[5]='1';
        time_st[6]='0';
    }
    else if('N'==time_st[0])
    {
        time_st[5]='1';
        time_st[6]='1';
    }
    else if('D'==time_st[0])
    {
        time_st[5]='1';
        time_st[6]='2';
    }
  
	MY_CPY_LEN(time_st,comp_time[1]+7,4);
	MY_CPY_LEN(&time_st[8],comp_time[1]+4,2);
    if(' '==time_st[8]) time_st[8]='0';
	time_st[4]='-';
	time_st[7]='-';
	time_st[10]=' ';
	MY_CPY_LEN(&time_st[11],comp_time[2],8);
	time_st[19]=0;
    
}



/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

