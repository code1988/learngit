/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: GUIsprint.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 131029
**最  新  版  本: V0.1
**描          述: 调用grlib接口函数实现方便液晶显示的一些函数，主要实现功能为print函数。
                  比如英文与汉字同时显示，打印不同格式的数据等。所有函数基本都是围绕sContext
                  原本想直接做在BSP驱动里，但考虑到这来有sContext形式，以及字库比较多，所以做
                  在应用编程里。
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "guisprint.h"
/* Private define-----------------------------------------------------------------------------*/
#define	BSPGUI_HFONT_MAX		((BSPGUI_XSIZE/BSPGUI_HFONT_X)*(BSPGUI_YSIZE/(BSPGUI_HFONT_Y+BSPGUI_HFONT_Y_A)))	// 最多可显示的半角字数
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes-------------------------
---------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : BSP_GUIDispChars
* Description	: 从起始位置开始显示半角字符串
* Input		    : x0:X坐标
		          y0:y坐标
	              len:数据长度
                  *pData:数据指针
* Output	    : TRUE/FALSE
* Note(s)	    : 这里传递的数据是ASCII码表,不是字库数据指针
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT16U BSP_GUIDispChars(INT16U x0,INT16U y0,INT16U len,INT8U *pData)
{
    
    while(len--)
    {
        if(*pData<0x80) //半角，这里用grlib带的20字体 
        {
            GrContextFontSet(&sContext, &g_sFontCm20); 
            GrStringDraw(&sContext, (const char *)pData,1, x0, y0+2,0);
            x0+=12;
            pData++;//半角往后偏移一个字节
        }
        else
        {
            GrContextFontSet(&sContext, &g_sFontCH24);
            GrStringDraw(&sContext,(const char *)pData,1,x0,y0,0);  
            x0+=24;
            pData+=2;//中文往后偏移2个字节
            if(len)
                len--;
        }          
    }  
    return len;
}
/***********************************************************************************************
* Function	    : BSP_GUISprint
* Description	: 显示一个数字(有点类似sprinf函数,简化了许多)
* Input		    : x0:x起始坐标(0 ~ BSPGUI_XSIZE-1)
                     y0:y起始坐标(0 ~ BSPGUI_YSIZE-1)
                      *p:控制参数,用来表明显示格式的,有点类似标准库函数printf()的参数用法
                            *p="u.vwx",由以下几部分组成
                                    第一部分'u'字符是表明显示的位数,1~BSPGUI_SPRINT_DISP_NUM,可以两位('12')也可以一位('6')
                                    第二部分'.v'字符是表明小数点位数,如果没有则表明是整数
                                    第三部分'w'字符是显示类型控制
                                            'd':带符号十进制(正的不显示符号,负的显示'-'),最大显示位数10
                                            'u':无符号十进制,最大显示位数10
                                            'p':带小数点的大写的十六进制数(其实是BCD码显示),如果出现大于9的字符,则全部显示'-',小数点保留
                                            'x':小写的十六进制
                                            'X':大写的十六进制
                                            'f':浮点数(没做)
                                    第四部分'x'字符用于一些特殊功能
                                            'Z':显示高位的0
                                            'L':去除高位的0,并且数字往左靠
                                            无:高位的0不显示,但是位置保留
                                    注:小数点占一个位置
                      *pData:数据,类型(INT8U *),对于十进制数,必须先转换成INT32,再传递,并且最大显示位数为10
                      例子:
                            1 无符号十进制
                                    INT8U data = 0x12;
                                    INT32U temp;
                                    temp = (INT32U)data;
                                    BSP_GUISprint(x0,y0,"2u",(INT8U *)&temp);
                                    显示:"18";返回:2
                            2 有符号十进制
                                    INT8U data = -1;
                                    INT32S temp;
                                    temp = (INT32S)data;
                                    BSP_GUISprint(x0,y0,"3d",(INT8U *)&temp);
                                    显示:" -1";返回:3
                            3 十六进制
                                    INT16U data = 0x0123;
                                    BSP_GUISprint(x0,y0,"4x",(INT8U *)&data);
                                    显示:" 123";返回:4
                                    BSP_GUISprint(x0,y0,"4xL",(INT8U *)&data);
                                    显示:"123";返回:3
                                    BSP_GUISprint(x0,y0,"4xZ",(INT8U *)&data);
                                    显示:"0123";返回:4
                            4 带小数点的十六进制
                                    INT16U data = 0x0123;
                                    BSP_GUISprint(x0,y0,"3.1p",(INT8U *)&data);
                                    显示:" 12.3";返回:5
                                    BSP_GUISprint(x0,y0,"1.3p",(INT8U *)&data);
                                    显示:"0.003";返回:5
                                    BSP_GUISprint(x0,y0,"5.7p",(INT8U *)&data);
                                    显示:"0.0000123";返回:9
                                    BSP_GUISprint(x0,y0,"2.4p",(INT8U *)&data);
                                    显示:"0.0023";返回:6
* Output	    : 实际显示的长度
* Note(s)	    : 
* Contributor	: 081205  wangyao	
***********************************************************************************************/
INT8U BSP_GUISprint(INT16U x0,INT16U y0,const tFont *pFont,INT8U *p,INT8U *pData)
{
    INT8U i,temp;
    INT8U dispBuff[BSPGUI_SPRINT_DISP_NUM];
    INT8U dispBitNum;							// 显示位数
    INT8U dispFloat;							// 小数点位置
    INT8U dispType;								// 显示类型
    INT8U dispPlace;							// 位置控制
    INT8U dispError=0;							// 1:错误的数字,比如在'p'显示中出现大于9的数字
    INT8U placeObject=0;						// 显示位置,用在有符号数的符号显示上
    INT32U data=0;
    
    // 得到数据显示长度
    dispBitNum = 0;
    i = p[1]-'0';
    if(i<=9)
    {
        dispBitNum = (p[0] - '0')*10 + i;
        i = 2;
    }
    else
    {
        dispBitNum += p[0] - '0';
        i = 1;
    }
    if(dispBitNum > BSPGUI_SPRINT_DISP_NUM)
        dispBitNum = BSPGUI_SPRINT_DISP_NUM;
    if(dispBitNum == 0)
        dispBitNum = 1;
    // 得到浮点数据小数位数
    dispFloat = 0;
    if(p[i] == '.')
    {
        i++;
        dispFloat = p[i++] - '0';
        if((dispFloat > 9)||(dispFloat >= (BSPGUI_SPRINT_DISP_NUM-1)))
            return 0;
    }
    // 小数点也要占一个位置
    if(dispFloat)
    {
        if(dispBitNum >= BSPGUI_SPRINT_DISP_NUM)	// 小数占一位
            dispBitNum = BSPGUI_SPRINT_DISP_NUM - 1;
    }
    // 得到数据显示类型
    dispType = p[i++];
    // 得到位置控制参数(去除高位0,左移等)
    dispPlace = p[i++];
    // 组织数据到缓存
    switch(dispType)
    {
        case 'd':								// 有符号十进制数
        {
            i = 4;
            if(dispBitNum > 10)					// 不得大于10位,因为32位的数据最大表示10位十进制值
                dispBitNum=10;
            if(dispBitNum < BSPGUI_SPRINT_DISP_NUM)	// 加一个符号位置
                dispBitNum++;
            placeObject = 1;					// 第一位置是符号
            // 整理数据
            data = pData[3];
            data = data << 8;
            data += pData[2];
            data = data << 8;
            data += pData[1];
            data = data << 8;
            data += pData[0];
            if(pData[i-1] & 0x80)				// 是否是负的
            {
                dispBuff[0] = '-';				// 显示'-'
                data = ((-data) + 1);			// 数据变成正的
            }
            else								// 正的
            {
                dispBuff[0] = ' ';				// 不显示'+'
            }
            // 这里没有break的
        }
        case 'u':								// 无符号十进制数
        {
			if(placeObject == 0)
			{
				// 整理数据
				data = pData[3];
				data = data << 8;
				data += pData[2];
				data = data << 8;
				data += pData[1];
				data = data << 8;
				data += pData[0];
				if(dispBitNum > 10)				// 不得大于10位,因为32位的数据最大表示10位十进制值
					dispBitNum=10;
			}
			// 将数据放入buff,数据最高位放在buff[0],其他依次,并转换成ASCII码
			for(i=dispBitNum;i>placeObject;i--)
			{
                dispBuff[i-1] = data % 10;
                dispBuff[i-1] = dispBuff[i-1] + '0';
                data = data / 10;					
			}
			break;
        }
        case 'p':	// 带小数点的小写的十六进制数(其实是BCD码显示)
                // 这里没有break的
        case 'x':	// 小写的十六进制数
                // 这里没有break的
        case 'X':  // 大写的十六进制数
        {
            INT8U hex='A';						// 用来区别大小写,这样后面的代码可以统一
            if(dispType == 'x')
                    hex = 'a';						// 小写
            if(dispBitNum & 0x01)				// 如果显示位数为奇数,则先单独处理最高位数据
            {
                // 低4位
                temp = pData[dispBitNum>>1]&0x0f;
                if(temp < 0x0a)
                    temp = temp + '0';			// 0~9
                else
                {
                    if(dispType == 'p')			// BCD码不能出现大于9
                        dispError = 1;
                    temp = temp - 0x0a + hex;	// a~f
                }
                dispBuff[0] = temp;
            }
            // 将数据放入buff,数据最高位放在buff[0],其他依次,并转换成ASCII码
            for(i=(dispBitNum>>1);i>0;i--)		// 按字节处理(2个位)
            {
                // 高4位
                temp = pData[i-1]>>4;
                if(temp < 0x0a)
                    temp = temp + '0';			// 0~9
                else
                {
                    if(dispType == 'p')			// BCD码不能出现大于9
                        dispError = 1;
                    temp = temp - 0x0a + hex;	// a~f
                }
                dispBuff[dispBitNum-(i<<1)] = temp;
                // 低4位
                temp = pData[i-1]&0x0f;
                if(temp < 0x0a)
                    temp = temp + '0';			// 0~9
                else
                {
                    if(dispType == 'p')			// BCD码不能出现大于9
                        dispError = 1;
                    temp = temp - 0x0a + hex;	// a~f
                }
                dispBuff[dispBitNum-(i<<1)+1] = temp;
            }
            break;
        }
        case 'f':								// 浮点数
        {
            break;
        }
        default:
            return 0;
    }
	// 添加小数点
	if(dispFloat)
	{
		if(dispFloat < dispBitNum)		// 显示位数大于小数个数的情况
		{
			// 小数后的数据都右移一位
			for(i=0;i<dispFloat;i++)
				dispBuff[dispBitNum-i] = dispBuff[dispBitNum-i-1];
			// 添加小数点
			dispBuff[dispBitNum-dispFloat] = '.';
			dispBitNum++;
		}
		else							// 小数位数大于等于显示位数的情况
		{
			// 小数后的数据都右移一位
			for(i=0;i<dispBitNum;i++)
				dispBuff[dispFloat-i+1] = dispBuff[dispBitNum-i-1];
			// 高位添'0'
			for(i=0;i<dispFloat-dispBitNum+2;i++)
				dispBuff[i] = '0';
			// 添加小数点
			dispBuff[1]='.';
			dispBitNum = dispFloat+2;
		}
	}
	// 位置处理(高位的0是否显示,是否需要左移)
	if(dispPlace == 'L')						// 左移
	{
		// 如果第一个位置是'-',那么从第二个位置开始左移
		if(dispBuff[0]=='-')
			placeObject = 1;
		else
			placeObject = 0;
		i=placeObject;
		temp = 0;
		// 计算需要移动的位数
		while(i<dispBitNum-1)
		{
			if((dispBuff[i]=='0')||(dispBuff[i]==' '))
				temp++;
			else if(dispBuff[i]=='.')			// 高位出现小数点
			{
				// 小数点前一位写成'0'
				if(i&&temp)
				{
					temp--;
					dispBuff[i-1]='0';
					break;
				}
				else							// 不能出现在最高位
					return 0;
			}
			else
				break;
			i++;
		}
		// 移动
		if(temp)
		{
			for(i=placeObject;i<dispBitNum-temp;i++)
				dispBuff[i] = dispBuff[i+temp];
		}
		dispBitNum = dispBitNum - temp;
	}
	else if(dispPlace != 'Z')				// 去掉高位的零
	{
		// 如果第一个位置是'-',那么从第二个位置开始左移
		if(dispBuff[0]=='-')
			placeObject = 1;
		else
			placeObject = 0;
		i=placeObject;
		// 变成' '
		while(i<dispBitNum-1)
		{
			if(dispBuff[i]=='0')
				dispBuff[i] = ' ';
			else if(dispBuff[i]=='.')			// 高位出现小数点
			{
				// 小数点前一位写成'0'
				if(i)
				{
					dispBuff[i-1]='0';
					break;
				}
				else							// 不能出现在最高位
					return 0;
			}
			else
				break;
			i++;
		}
	}
	// 错误处理
	if(dispError)
	{
		for(i=0;i<dispBitNum;i++)
		{
			// 所有不是小数点的数都变成'-'
			if(dispBuff[i] != '.')
				dispBuff[i] = '-';
		}
	}
	// 显示
	GrContextFontSet(&sContext, pFont); //打印20字体大小g_sFontCm20g_sFontCmss48b
    GrStringDraw(&sContext, (const char *)dispBuff,dispBitNum, x0, y0,0);
	return dispBitNum;
}
/***********************************************************************************************
* Function		: GUIPrintf
* Description	: 显示字符串函数,用法基本同标准printf函数,只是前面多了x,y坐标
* Input			: x0:x起始坐标(0 ~ BSPGUI_XSIZE-1)
				  y0:y起始坐标(0 ~ BSPGUI_YSIZE-1)
				  *pStr,...:用法同标准printf函数
* Output		: 实际显示的数据长度,0:出错
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT16U GUIPrintf(INT16U x0,INT16U y0,const char *pStr,...)
{
	INT16U len;
	char Buff[40];
	va_list arg_ptr;
	va_start(arg_ptr,pStr);
	len = vsprintf(Buff,pStr,arg_ptr);
	va_end(arg_ptr);
	//assert_param(len<BSPGUI_HFONT_MAX);// 参数检查,这里的错误比较严重,缓冲区溢出
	return BSP_GUIDispChars(x0,y0,len,(INT8U *)Buff);
}
/***********************************************************************************************
* Function		: BSP_GUIFill
* Description	: LCD区域填充
* Input			: x0:x起始坐标
				  y0:y起始坐标
				  x1:x结束坐标
				  y1:y结束坐标
				  color:需要填充的数据
* Output		: 
* Note(s)		: 调用grlib的tRectangle制作该函数。
* Contributor	: 140221	wangyao
***********************************************************************************************/
void BSP_GUIFill(INT16U x0,INT16U y0,INT16U x1,INT16U y1,INT32U color)
{
    tRectangle sRect; 
    
    // Fill the top 24 rows of the screen with blue to create the banner.
    sRect.sXMin = x0;
    sRect.sYMin = y0;
    sRect.sXMax = x1;
    sRect.sYMax = y1;
    GrContextForegroundSet(&sContext, color);//ClrLightSteelBlue);//
    GrRectFill(&sContext, &sRect);
}
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/