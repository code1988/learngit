/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: key.c
* Author			: 王耀
* Date First Issued	: 02/20/2014 
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "DispkeyFunction.h"
#include "display.h"
#include "DwinPotocol.h"
#include "CoodinateInit.h"
#include "DisplayMain.h"
/* Private define-----------------------------------------------------------------------------*/
#define BLACKLISTMAXPAGES 0x02
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//鉴伪参数的横坐标
extern INT8U Paramvertical;
//鉴伪参数的纵坐标
extern INT8U Paramhorizontal;

//键盘光标显示的横坐标
extern INT8U KeyBoardvertical;
//键盘光标显示的纵坐标
extern INT8U KeyBoardhorizontal;
//光标大小
extern INT8U CursorSize;

//黑名单输入
extern INT8U BlackListMaxPages;
extern INT8U BlackListPage;
extern INT8U BlackListhorizontal;
//鉴伪信息页码计数
extern INT8U AuthenticationDataPages;
//鉴伪参数的修改用到的数组
extern INT8U AuthenticationDataSettings[3][90];

//鉴伪参数暂存变量
INT8U TempAuthenticationDataSettings[3][90];

extern tRectangle BaseEditParam[10][12];
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: DealWithParm
*Description	: 处理鉴伪函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 14/01/2015	宋超
***********************************************************************************************/
void DealWithParm(INT8U PlusFlag)
{
    if(PlusFlag == 1)
    {
        if(Paramvertical%4 == 1)
        {
            if(TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]<99)
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]++;
            }
            else
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)] = 0;
            }
            print2charStrings16(BaseEditParam[Paramhorizontal][Paramvertical],&TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]);
        }
        if(Paramvertical%4 == 2)
        {
            if(TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]<255)
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]++;
            }
            else
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)] = 0;
            }
            printStrings16(BaseEditParam[Paramhorizontal][Paramvertical],&TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]);
        }
        if(Paramvertical%4 == 3)
        {
            if(TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]<1)
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]++;
            }
            else
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)] = 0;
            }
            print1charStrings16(BaseEditParam[Paramhorizontal][Paramvertical],&TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]);
        }
    }
    else if(PlusFlag == 0)
    {
        if(Paramvertical%4 == 1)
        {
            if(TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]>0)
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]--;
            }
            else
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)] = 99;
            }
            print2charStrings16(BaseEditParam[Paramhorizontal][Paramvertical],&TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]);
        }
        if(Paramvertical%4 == 2)
        {
            if(TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]>0)
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]--;
            }
            else
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)] = 254;
            }
            printStrings16(BaseEditParam[Paramhorizontal][Paramvertical],&TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]);
        }
        if(Paramvertical%4 == 3)
        {
            if(TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]>0)
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]--;
            }
            else
            {
                TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)] = 1;
            }
            print1charStrings16(BaseEditParam[Paramhorizontal][Paramvertical],&TempAuthenticationDataSettings[AuthenticationDataPages][(Paramhorizontal*3+(Paramvertical-(Paramvertical/4)*4)-1+Paramvertical/4*30)]);
        }

    }
}

/***********************************************************************************************
* Function		: DealCursor
*Description	: 光标处理函数
* Input			: 
* Output		: 
* Note(s)		:  
* Contributor	: 20/12/2014	宋超
***********************************************************************************************/
void DealCursor(INT8U Cursor)
{
    switch(Cursor)
    {
        case 0x01:
            //对鉴伪参数修改的最后一页的出错保护
            if((AuthenticationDataPages == 0x02)&&(Paramhorizontal > 0x03)&&(Paramvertical == 0x07))
            {
                break;
            }
            //光标出错
            ClearCursor();
            if(Paramvertical == 11)
            {
                Paramvertical = 1;
            }
            else
            {
                Paramvertical++;         
                if(Paramvertical == 4)Paramvertical = 5;
                if(Paramvertical == 8)Paramvertical = 9;
            }
			DealCursorSize();
            DisplayCursor();
            break;
        case 0x02:
            //对鉴伪参数修改的最后一页的出错保护
            if((AuthenticationDataPages == 0x02)&&(Paramhorizontal > 0x03)&&(Paramvertical == 0x01))
            {
                break;
            }
            //光标出错
            ClearCursor();
            if(Paramvertical == 1)
            {
                Paramvertical = 11;
            }
            else
            {
                Paramvertical--;
                if(Paramvertical == 4)Paramvertical = 3;
                if(Paramvertical == 8)Paramvertical = 7;
            }
            DealCursorSize();
            DisplayCursor();
            break;
        case 0x03:
            //对鉴伪参数修改的最后一页的出错保护
            if((AuthenticationDataPages == 0x02)&&(Paramhorizontal == 0x03)&&(Paramvertical>0x07))
            {
                break;
            }
            //光标出错
            ClearCursor();
            if(Paramhorizontal == 9)
            {
                Paramhorizontal = 0;
            }
            else
            {
                Paramhorizontal++;
            }
            DisplayCursor();
            break;
        case 0x04:
            //对鉴伪参数修改的最后一页的出错保护
            if((AuthenticationDataPages == 0x02)&&(Paramhorizontal == 0x00)&&(Paramvertical>0x07))
            {
                break;
            }
            //光标出错
            ClearCursor();
            if(Paramhorizontal == 0)
            {
                Paramhorizontal = 9;
            }
            else
            {
                Paramhorizontal--;
            }
            DisplayCursor();
            break;
    }
}

/***********************************************************************************************
* Function		: DealCursorSize
*Description	: 光标处理函数
* Input			: 
* Output		: 
* Note(s)		:  
* Contributor	: 2/3/2015	宋超
***********************************************************************************************/
void DealCursorSize(void)
{
     switch(Paramvertical%4 - 1)
     {
         case 0x00:
             CursorSize = 2;
             break;
         case 0x01:
             CursorSize = 3;
             break;
         case 0x02:
             CursorSize = 1;
             break;
        default:
             CursorSize = 3;
             break;
     }  
}

/***********************************************************************************************
* Function	: DealKeyBoard
*Description	: 键盘光标处理
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
***********************************************************************************************/
void DealKeyBoard(INT8U Cursor)
{
    switch(Cursor)
    {
        case 0x01:
            ClearKeyBoard();
            if((KeyBoardhorizontal == 2)&&(KeyBoardvertical == 10))
            {
                KeyBoardvertical = 11;
            }
            else if(KeyBoardvertical >9)
            {
                KeyBoardvertical = 0;
                if(KeyBoardhorizontal == 3)
                {
                    KeyBoardhorizontal = 0;
                }
                else
                {
                    KeyBoardhorizontal++;
                }
            }
            else
            {
                KeyBoardvertical++;
            }
            DisplayKeyBoard();
            break;
        case 0x02:
            ClearKeyBoard();
            if((KeyBoardhorizontal == 3)&&(KeyBoardvertical == 0))
            {
                KeyBoardvertical = 11;
                KeyBoardhorizontal = 2;
            }
            else if(KeyBoardvertical < 1)
            {
                KeyBoardvertical = 10;
                if(KeyBoardhorizontal == 0)
                {
                    KeyBoardhorizontal = 3;
                }
                else
                {
                    KeyBoardhorizontal--;
                }
            }
            else
            {
                KeyBoardvertical--;
            }
            DisplayKeyBoard();
            break;
        case 0x03:
            ClearKeyBoard();
            if(KeyBoardhorizontal == 3)
            {
                KeyBoardhorizontal = 0;
                if(KeyBoardvertical >9)
                {
                    KeyBoardvertical = 0;
                }
                else
                {
                    KeyBoardvertical++;
                }
            }
            else if((KeyBoardhorizontal == 2)&&(KeyBoardvertical == 11))
            {
                KeyBoardhorizontal++;
                KeyBoardvertical = 10;
            }
            else if((KeyBoardhorizontal == 2)&&(KeyBoardvertical == 10))
            {
                KeyBoardvertical = 11;
            }
            else
            {
                KeyBoardhorizontal++;
            }
            DisplayKeyBoard();
            break;
        case 0x04:
            //翻页
            break;
    }
}
 
/***********************************************************************************************
* Function		: IPNumtoChar
*Description	: 把IP从数字转化为字符串
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 14/01/2015	宋超
***********************************************************************************************/
void IPNumtoChar(INT8U *Ptr,INT8U *IP)
{
    INT8U i,totalnum;    
    
    for(i = 0;i < 4;i++)
    {
        totalnum = Ptr[i];
        if(totalnum/100)
        {
            IP[2+i*3] = totalnum%10 + '0';
            totalnum = totalnum/10;
            IP[1+i*3] = totalnum%10 + '0';
            totalnum = totalnum/10;
            IP[0+i*3] = totalnum + '0';
        }
        else if(totalnum/10)
        {
            IP[0+i*3] = '0';
            IP[1+i*3] = totalnum/10 + '0';
            totalnum = totalnum%10;
            IP[2+i*3] = totalnum + '0';
        }
        else
        {
            IP[0+i*3] = '0';
            IP[1+i*3] = '0';
            IP[2+i*3] = totalnum + '0';
        }
    }
}

/***********************************************************************************************
* Function		: IPChartoNum
*Description	: 把IP从字符串转化为数字
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 14/01/2015	宋超
***********************************************************************************************/
void IPChartoNum(INT8U *Ptr,INT8U *IP)
{
    INT8U i;
    
    for(i = 0;i < 4;i++)
    {
            Ptr[i+2] = (IP[i*3]-0x30)*100+(IP[i*3+1]-0x30)*10+(IP[i*3+2]-0x30);
    }
}

/***********************************************************************************************
* Function		: PortNumtoChar
*Description	: 把端口号数字转化为字符
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 14/01/2015	宋超
***********************************************************************************************/
void PortNumtoChar(INT8U *Ptr,INT8U *Port)
{
    INT16U PortNum;
    
    PortNum = Ptr[1]*256 + Ptr[0];
    
    if(PortNum/1000)
    {
        Port[3] = PortNum%10 + '0';
        PortNum = PortNum/10;
        Port[2] = PortNum%10 + '0';
        PortNum = PortNum/10;
        Port[1] = PortNum%10 + '0';
        PortNum = PortNum/10;
        Port[0] = PortNum + '0';
    }
    else if(PortNum/100)
    {
        Port[0] = ' ';
        Port[3] = PortNum%10 + '0';
        PortNum = PortNum/10;
        Port[2] = PortNum%10 + '0';
        PortNum = PortNum/10;
        Port[1] = PortNum + '0';
    }
    else if(PortNum/10)
    {
        Port[0] = ' ';
        Port[1] = ' ';
        Port[2] = PortNum/10 + ' ';
        PortNum = PortNum%10;
        Port[3] = PortNum + ' ';
    }
    else
    {
        Port[0] = ' ';
        Port[1] = ' ';
        Port[2] = ' ';
        Port[3] = PortNum + ' ';
    }

}

/***********************************************************************************************
* Function		: PortChartoNum
*Description	: 把端口号转化为数字
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 14/01/2015	宋超
***********************************************************************************************/
void PortChartoNum(INT8U *Ptr,INT8U *Port)
{
    INT16U PortNum;
    
    PortNum = (Port[0]-0x30)*1000+(Port[1]-0x30)*100+(Port[2]-0x30)*10+(Port[3]-0x30);
    
    Ptr[2] = PortNum&0xFF;
    Ptr[3] = PortNum>>8;

}

/***********************************************************************************************
* Function		: MacNumtoChar
*Description	: 把Mac数字转化为字符
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 14/01/2015	宋超
***********************************************************************************************/
void MacNumtoChar(INT8U *Ptr,INT8U *LocalMac)
{
    INT8U i;
    INT8U MacHigh;
    
    for(i = 0;i<6;i++)
    {
        MacHigh = Ptr[i]/16;
        if(Ptr[i]%16>9)
        {
            LocalMac[i*2+1] = Ptr[i]%16 - 10 + 'A';
        }else
        {
            LocalMac[i*2+1] = Ptr[i]%16 + '0';
        }
        
        if(MacHigh>9)
        {
            LocalMac[i*2] = MacHigh- 10 + 'A';
        }else
        {
            LocalMac[i*2] = MacHigh + '0';
        }
    }
}



/***********************************************************************************************
* Function		: Dealblacklist
*Description	        : 黑名单的光标处理函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 12/01/2015	宋超
***********************************************************************************************/
void Dealblacklist(INT8U Cursor)
{
    switch(Cursor)
    {
        case 0x01:
            //向前翻页
            BlackListhorizontal = 0;
            if(BlackListPage == 0)
            {
                BlackListPage = (BLACKLISTMAXPAGES-1);
            }
            else
            {
                BlackListPage--;
            }
            DisplayCommonMenu(&gsMenu_23,0);
            DisplayCheckmark();
            blackListDisp();
            break;            
        case 0x02:
            ClearCheckmark();
            if(BlackListhorizontal == 0)
            {
                BlackListhorizontal = 6;
            }
            else
            {
                BlackListhorizontal--;
            }
            DisplayCheckmark();
            break;
        case 0x03:
            ClearCheckmark();
            if(BlackListhorizontal == 6)
            {
                BlackListhorizontal = 0;
            }
            else
            {
                BlackListhorizontal++;
            }
            DisplayCheckmark();
            break;
        case 0x04:
            //向后翻页
            BlackListhorizontal = 0;
            if(BlackListPage == (BLACKLISTMAXPAGES-1))
            {
                BlackListPage = 0;
            }
            else
            {
                BlackListPage++;
            }
            DisplayCommonMenu(&gsMenu_23,0);
            DisplayCheckmark();
            blackListDisp();
            break;  
        default:
            break;
    }
}

/***********************************************************************************************
* Function		: DeleteBlack
*Description	        : 删除黑名单（想采取链表的方式）
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 14/01/2015	宋超
***********************************************************************************************/
void DeleteBlack(void)
{
    
}



/************************(C)COPYRIGHT 2010 浙江方泰****END OF FILE****************************/
