/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.c
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013	        : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "BaseDisp.h"
#include "DwinPotocol.h"
#include "CoodinateInit.h"
#include "DisplayMain.h"
#include "grlib.h"
#include "RecevCanPotocol.h"
/* Private define-----------------------------------------------------------------------------*/
#define SENDTODISPLEN (SendToDisp[2]+3)
#define __UART2_ENABLE

#define CLEAR 0x5A
#define MATRIX 0x59

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
INT8U count = 0;
INT8U SendToUART2[100] = {0};

INT8U Setdata[30];//设置测量点号用全局变量
INT8U Sconfirm = 0;//list菜单按下ok,back,cancel以后重新描画菜单
ST_Menu *gps_CurMenu;     //当前菜单
INT32S (*LcdPage)(INT16U *MeterNo,INT32U type);//页面显示函数指针
SetDec gps_data = {64,1,16,8,1,0,0,0,Setdata};//设置页面用结构体
INT8U gCursorInMenu;    //光标在当前菜单中的序号
ST_Cursor  gs_CursorInPage;   //当前页中的光标位置
//参数修改菜单现在指定的参数位置
INT8U Paramhorizontal = 0;
INT8U Paramvertical = 1;
INT8U EditParamPage = 0;
INT8U CheckParamPage = 0;
//键盘坐标
INT8U KeyBoardhorizontal = 0;
INT8U KeyBoardvertical = 0;
//用户信息显示第几条LIST
INT8U InfomationPage = 0;
INT8U InfomationList = 0;

INT8U AuthenticationDataPages = 0;
INT8U CursorSize = 2;
extern _BSP_MESSAGE Nandsend_message; 

//坐标参数
extern tRectangle BaseMainMenuNumCoordinate[3][10];
extern tRectangle BaseInletTitle[2];
extern tRectangle BaseModeIcon[9];
extern tRectangle BaseNetIcon[2];
extern tRectangle BaseOutLetCoordinate[4][3];
extern tRectangle BaseTotalPage[4];
extern tRectangle BaseTotalSum[7];
extern tRectangle BaseTotalSumCoordinate;
extern tRectangle BaseNetCoordinate;
extern tRectangle BaseMainMenuModeCoordinate[2];
extern tRectangle BaseOutLetParam[7][10];
extern tRectangle BaseCheckParam[10][10];
extern tRectangle BaseEditParam[10][12];
extern tRectangle BaseEditParamColor[4];
extern tRectangle BaseKeyBoardSettingsCoordinate[8][12];
extern tRectangle BaseBlackListCoordinate[7];
extern tRectangle BaseBlackListNumberCoordinate[7];
extern tRectangle BaseInfomationCoordinate[3][7];
extern tRectangle BaseCleanLevelSettingsSelectCoordinate[6];
extern tRectangle BaseAuthenticationSettingsSelectCoordinate[7];
//鉴伪参数
extern INT8U AuthenticationDataSettings[3][90];
//马达状态
extern INT8U MTStatus;
//菜单标志
extern INT8U MainMenuStatus;
//黑名单的纵坐标
extern INT8U BlackListhorizontal;
//清分等级的纵坐标值（用来定位光标）
extern INT8U ClearLevelhorizontal;
//鉴伪等级的纵坐标值（用来定位光标）
extern INT8U AuthenticationLevelhorizontal;
//当前菜单
extern ST_Menu *gps_CurMenu;     
//发送给CAN的消息
extern _BSP_MESSAGE Dsend_message; 
//批量值
extern INT16U TemBatchNum;
//用户信息查看参数
extern INT8U NetPostTime[6];
extern INT8U NetPostPage[2];
extern INT8U BlankName[16];
extern INT8U AreaNum[16];
extern INT8U BranchNum[16];
extern INT8U LatticePoint[11];
extern INT8U MachineNo[16];
extern INT8U IP[13];
extern INT8U Mac[13];
extern INT8U MachineNo[16];
extern INT8U BanTechnician[3][16];
extern INT8U Time[6];
extern INT8U Mask[4];
extern INT8U GateWay[4];
extern INT8U LocalPort[4];
extern INT8U DiskPort[4];
extern INT8U DiskIP[13];
extern INT8U BlackLists[2][7][13];
extern INT8U BlackListPage;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: printIcon
* Description	: 显示图标用到的函数
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printIcon(INT8U IMAGE,tRectangle Icon,tRectangle Coordinate)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x71); //0x71 剪切图标到背景图片上
    TXBYTE(IMAGE); //0x64图标1库上的图标，0x65，图标库2里面的图标
    //发送图标的坐标
    TXBYTE(Icon.sXMin>>8);
    TXBYTE(Icon.sXMin&0xFF);
    TXBYTE(Icon.sYMin>>8);
    TXBYTE(Icon.sYMin&0xFF);
    TXBYTE(Icon.sXMax>>8);
    TXBYTE(Icon.sXMax&0xFF);
    TXBYTE(Icon.sYMax>>8);
    TXBYTE(Icon.sYMax&0xFF);
    
    //发送图标的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printCharStrings16
* Description	: 直接输出16*16的字符
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printCharStrings16(tRectangle Coordinate,INT8U *s)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while(*s) //发送字符串内容
    {
        TXBYTE(*s);
        s++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printStrings16
* Description	: 显示参数菜单的文字的函数
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[4];
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    bin_to_char(*s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printOutLetValueStrings16
* Description	: 显示出钞口查看和退钞口查看的面额
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printOutLetValueStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[4];
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    JcSpeed_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: print2charStrings16
* Description	: 显示参数菜单的文字的函数(2个字符)
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void print2charStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[3];
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    bin_to_2char(*s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: print2charStrings16
* Description	: 显示参数菜单的文字的函数(2个字符)
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void print1charStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[2];
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    bin_to_1char(*s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printCrownWordStrings16
* Description	: 冠字号码显示
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printCrownWordStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while(i<10)
    {
        TXBYTE(*(s++));
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printCrownWordStrings16
* Description	: 冠字号码显示
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printMainMenuTimeStrings32(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x6F); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while(*s != 0)
    {
        TXBYTE(*(s++));
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printMixNums16
* Description	: 显示混点明细菜单的张数和金额
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printMixNums16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[4];
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x6F); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    Mix_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printStrings16
* Description	: 显示参数菜单的文字的函数
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printStrings32(tRectangle Coordinate,INT8U *s)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    TXBYTE(*s);
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}


/***********************************************************************************************
* Function		: printStrings16
* Description	: 显示版本信息的函数
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printVersionStrings32(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x54); //0x54 显示16*16的字
    //发送显示文字的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while((s[i])&&(s[i] != 0xFF))
    {
        TXBYTE(s[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: FetchBackColor
* Description	: 取得背景色
* Input		: Coordinate 取得颜色的坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void FetchBackColor(tRectangle Coordinate)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x42); //0x54 显示16*16的字
    //发送坐标的坐标
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: PrintCursor
* Description	: 显示光标
* Input		: Coordinate 取得颜色的坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void PrintCursor(ST_Cursor Coordinate)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x44); //0x54 显示16*16的字
    TXBYTE(0x01); 
    //发送光标的坐标
    Coordinate.YCoor = Coordinate.YCoor -1;
    TXBYTE(Coordinate.XCoor>>8);
    TXBYTE(Coordinate.XCoor&0xFF);
    TXBYTE(Coordinate.YCoor>>8);
    TXBYTE(Coordinate.YCoor&0xFF);
    switch(CursorSize)
    {
        case 0x01:
            TXBYTE(0x07); //显示的光标长度
            break;
        case 0x02:
            TXBYTE(0x0F); //显示的光标长度
            break;
        case 0x03:
            TXBYTE(0x17); //显示的光标长度
            break;
    }
    TXBYTE(0x0F); //显示的光标高度 
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: CloseCursor
* Description	: 关闭光标显示
* Input		: Coordinate 取得的坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void CloseCursor(ST_Cursor Coordinate)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x44); //0x54 显示16*16的字
    TXBYTE(0x00); 
    //发送光标的坐标
    TXBYTE(Coordinate.XCoor>>8);
    TXBYTE(Coordinate.XCoor&0xFF);
    TXBYTE(Coordinate.YCoor>>8);
    TXBYTE(Coordinate.YCoor&0xFF);
	TXBYTE(0x16); //显示的光标长度
    TXBYTE(0x0F); //显示的光标高度
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printBackImage
* Description	: 显示背景图片(这里画面ID和图片保存在屏上的顺序保持一致)
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 12/11/2014	songchao
***********************************************************************************************/
void PraPerprintBackImage(INT8U IMAGE)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x70); //0x7 显示第几张图片
    TXBYTE(IMAGE); //0x64图标1库上的图标，0x65，图标库2里面的图标
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: printBackImage
* Description	: 显示背景图片(这里画面ID和图片保存在屏上的顺序保持一致)
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 12/11/2014	songchao
***********************************************************************************************/
void printBackImage(INT8U IMAGE)
{
    INT8U MTmode[2] = {0};
    if(IMAGE == 0)
    {
          PraPerprintBackImage(IMAGE);
          RefreshMainMenu();
          PraperMianMenu(); 
          MainMenuStatus = INMAINMENU;
          MTmode[0] = MTStatus;
          MTmode[1] = MainMenuStatus;
          Dsend_message.MsgID = APP_COMFROM_UI;
          Dsend_message.DivNum = UIACANMOTORSET;
          Dsend_message.DataLen = 2;
          Dsend_message.pData = MTmode;
          OSQPost (canEvent,&Dsend_message);
          OSTimeDlyHMSM(0,0,0,10);
    }
    else
    {
          PraPerprintBackImage(IMAGE);
    }
}

/***********************************************************************************************
* Function		: printsstring8
* Description	: 显示8*8大小的字符串
* Input			:   x       横坐标
                    y       纵坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring8(INT16U x,INT16U y,INT8U *s)
{
    prints(0x53,x,y,s);
} 

/***********************************************************************************************
* Function		: printsstring32
* Description	: 显示32*32大小的字符串
* Input			:   x       横坐标
                    y       纵坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring32(INT16U x,INT16U y,INT8U *s)
{
    prints(0x55,x,y,s);
}

/***********************************************************************************************
* Function	: printsJcSpeedstring32
* Description	: 显示32*32大小的字符串
* Input		:   x       横坐标
                    y       纵坐标
* Output	: 
* Note(s)	: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsJcSpeedstring32(INT16U x,INT16U y,INT8U *s)
{
    JcSpeedprints(0x55,x,y,s);
}

/***********************************************************************************************
* Function		: printsJcPwmstring32
* Description	: 显示32*32大小的字符串
* Input			:   x       横坐标
                    y       纵坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsJcPwmstring32(INT16U x,INT16U y,INT8U *s)
{
    JcPwmprints(0x55,x,y,s);
}

/***********************************************************************************************
* Function		: printsstring12
* Description	: 显示12*12大小的字符串
* Input			:   x       横坐标
                    y       纵坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring12(INT16U x,INT16U y,INT8U *s)
{
    prints(0x6E,x,y,s);
}

/***********************************************************************************************
* Function		: printsstring24
* Description	: 显示24*24大小的字符串
* Input			:   x       横坐标
                    y       纵坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring24(INT16U x,INT16U y,INT8U *s)
{
    prints(0x6F,x,y,s);
}

/***********************************************************************************************
* Function		: prints
* Description	: 显示字符串
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void prints(INT8U size,INT16U x,INT16U y,INT8U *s)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(size); //0x53=8点阵ASKII，0x54=16点阵字符串，0x55=32点阵
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x坐标
    TXWORD(y); //y坐标
    while(*s) //发送字符串内容
    {
        TXBYTE(*s);
        s++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

//相关子程序
void TXBYTE( INT8U i) //向串口发送一个字节
{
    SendToUART2[count++] = i;
    //BSP_UARTWrite(UART2,&i,1);
}

void TXWORD( INT16U i) //向串口发送一个字
{
    TXBYTE(((i>>8)&0xFF));
    TXBYTE((i&0xFF)); 
}
void frame_end() //发送帧结束符 cc 33 c3 3c
{
    TXBYTE(0xcc);
    TXBYTE(0x33);
    TXBYTE(0xc3);
    TXBYTE(0x3c);
}

/***********************************************************************************************
* Function		: JcSpeedprints
* Description	: 显示字符串
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void JcSpeedprints(INT8U size,INT16U x,INT16U y,INT8U *s)
{
    INT8U String[4] = {0x00};
    INT8U i = 0;
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(size); //0x53=8点阵ASKII，0x54=16点阵字符串，0x55=32点阵
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x坐标
    TXWORD(y); //y坐标
    JcSpeed_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: JcPwmprints
* Description	: 显示字符串
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void JcPwmprints(INT8U size,INT16U x,INT16U y,INT8U *s)
{
    INT8U String[4];
    INT8U i = 0;
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(size); //0x53=8点阵ASKII，0x54=16点阵字符串，0x55=32点阵
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x坐标
    TXWORD(y); //y坐标
    JcPWM_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: Clear
* Description	: 清除固定区域的字符串
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void Clear(INT8U size,INT16U x,INT16U y)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x5A); //0x53=8点阵ASKII，0x54=16点阵字符串，0x55=32点阵
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x坐标
    TXWORD(y); //y坐标
    TXWORD(x+size); //x坐标
    TXWORD(y+size); //x坐标
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_1ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: handshake
* Description	: 清除固定区域的字符串
* Input			: size 字体大小
                  x     横坐标
                  y     纵坐标
                  s     显示的字符串的内容
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void handshake(void)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x00); //握手协议
    frame_end(); //发送帧结束符
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_1ms); //确保发送完毕，必须有
}

/***********************************************************************************************
* Function		: setcolor
* Description	: 设置前景色和背景色
* Input			: color1 前景色
                  color2 背景色
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void setcolor(INT16U color1,INT16U color2)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(0x40); //设置颜色
    TXWORD(color1); //x坐标
    TXWORD(color2); //y坐标
    frame_end(); //发送帧结束符
}

/***********************************************************************************************
* Function		: fillw
* Description	: 填充区域
* Input			: color1 前景色
                  color2 背景色
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void fillw(INT8U code,INT16U x1,INT16U y1,INT16U x2,INT16U y2)
{
    TXBYTE(0xAA); //帧头0xAA
    TXBYTE(code); //设置矩形框
    TXWORD(x1); //x坐标
    TXWORD(y1); //y坐标
    TXWORD(x2); //x坐标
    TXWORD(y2); //y坐标
    frame_end(); //发送帧结束符
}

/********************************************************************
* Function Name : DisplayMianMenu() 
* Description   : 主菜单显示函数
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayMianMenu(struct st_menu* pMenu, INT8U cursor)
{
    gps_CurMenu = pMenu;//当前菜单指针赋值

    if(pMenu->Menu_Flag==0)//显示主页
    {	
        MainMenuStatus = INMAINMENU;
        printBackImage(pMenu->FrameID);
        //显示主菜单
        PraperMianMenu();      
    }
}

/********************************************************************
* Function Name : DisplayCommonMenu() 
* Description   : 普通菜单显示函数
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayCommonMenu(struct st_menu* pMenu, INT8U cursor)
{    
    gps_CurMenu = pMenu;//当前菜单指针赋值

    printBackImage(pMenu->FrameID);
}

/********************************************************************
* Function Name : PraperAuthenticationDataSettings() 
* Description   : 鉴伪参数显示参数名称
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperAuthenticationDataSettings()
{   
    INT8U horizontal = 0;
    INT8U vertical = 0;  
    INT8U AuthenticationItem1[3][10][8] = {{{"币型"},{"套别"},{"版本"},{"面值"},{"面向"},{"新旧"},{"D06"},{"D07"},{"D08"},{"D09"}},
                                          {{"D10"},{"D11"},{"D12"},{"D13"},{"D14"},{"D15"},{"D16"},{"D17"},{"D18"},{"D19"}},
                                          {{"D20"},{"D21"},{"D22"},{"D23"},{"D24"},{"D25"},{"D26"},{"D27"},{"D28"},{"D29"}},};
   
    INT8U AuthenticationItem2[3][10][8] = {{{"D30"},{"D31"},{"D32"},{"D33"},{"D34"},{"D35"},{"D36"},{"D37"},{"D38"},{"D39"}},
                                          {{"D40"},{"D41"},{"D42"},{"D43"},{"D44"},{"D45"},{"D46"},{"D47"},{"D48"},{"D49"}},
                                          {{"D50"},{"D51"},{"D52"},{"D53"},{"D54"},{"D55"},{"D56"},{"D57"},{"D58"},{"D59"}},};

    INT8U AuthenticationItem3[3][10][8] = {{{"D60"},{"D61"},{"D62"},{"D63"},{"D64"},{"D65"},{"D66"},{"D67"},{"D68"},{"D69"}},
                                          {{"D70"},{"D71"},{"D72"},{"D73"},{"D74"},{"D75"},{"D76"},{"D77"},{"D78"},{"D79"}},
                                          {{"D80"},{"D81"},{"D82"},{"D83"},{" "},{" "},{" "},{" "},{" "},{" "}},};

    
    //背景色
    setcolor(0x0000,0x0000);
    
    switch(AuthenticationDataPages)
    {
        case 0x00:
            for(vertical = 0;vertical < 3;vertical++)
            {
                for(horizontal = 0;horizontal < 10;horizontal++)
                {
                    printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem1[vertical][horizontal][0]);
                }
            }
            PraperParmSet();
            break;
        case 0x01:
            for(vertical = 0;vertical < 3;vertical++)
            {
                for(horizontal = 0;horizontal < 10;horizontal++)
                {
                    printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem2[vertical][horizontal][0]);
                }
            }
            PraperParmSet();
            break;
        case 0x02:
            for(vertical = 0;vertical < 2;vertical++)
            {
                for(horizontal = 0;horizontal < 10;horizontal++)
                {
                    printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem3[vertical][horizontal][0]);
                }
            }
            for(horizontal = 0;horizontal < 4;horizontal++)
            {
                printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem3[vertical][horizontal][0]);
            }
            PraperParmSet();
            break;
        default:
          break;
    }  
}

/********************************************************************
* Function Name : PraperSpeedSetMenu() 
* Description   : 速度调试菜单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperSpeedSetMenu()
{       
    //8个按键显示
    setcolor(0x0000,0xFFFF);
    printsstring32(3,5,"进钞");
    printsstring32(3,68,"出钞");
  
    printsstring32(400,5,"+1");
    printsstring32(400,68,"-1");
    
    printsstring32(400,136,"保存");
    printsstring32(400,204,"返回");
    
    
    //表格
    printsstring32(80,32,"进钞");
    printsstring32(80,76,"出钞");
    
    printsstring32(294,2,"PWM");
    printsstring32(180,2,"SPEED");
    
    
    printsstring32(60,152,"进钞");    
    printsstring32(60,196,"出钞");
    
    //对应的转速信息
    printsstring16(150,125,"750");
    printsstring16(210,125,"850");
    printsstring16(270,125,"950");
    printsstring16(320,125,"1050");    
}

/********************************************************************
* Function Name : PraperMcDebugMainMenu() 
* Description   : 速度调试菜单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMcDebugMainMenu()
{   
  
  	//设置字体颜色
	setcolor(0x0000,0xFFFF);
    
    //背景色    
    printsstring16(3,5,"电机速度");
    printsstring16(3,68,"进钞接钞AD");
  
    printsstring16(3,138,"红外AD管");
    printsstring16(3,208,"大小磁头AD");
    
    printsstring16(400,5,"荧光AD");
    printsstring16(400,68,"走钞信息");
    
    printsstring16(400,138,"CS校验");  

    printsstring16(400,204,"返回");
  
}

/********************************************************************
* Function Name : PraperInLetADMenu() 
* Description   : 进出超口AD采样数据
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInLetADMenu()
{   
    //表格
    printsstring32(3,32,"进钞");
    printsstring32(3,76,"出钞");
    printsstring32(3,120,"疑币H");
    printsstring32(3,164,"疑币A");
    printsstring32(3,208,"码盘");

    
    
    printsstring32(100,2,"PWM");
    printsstring32(200,2,"原始值");
    printsstring32(200,2,"遮挡值"); 
   
}

/********************************************************************
* Function Name : PraperIRADMenu() 
* Description   : 红外D采样数据
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperIRADMenu()
{       
    //表格
    printsstring32(3,32,"L1");
    printsstring32(3,72,"R1");
    printsstring32(3,112,"L2");
    printsstring32(3,152,"R2");
    printsstring32(3,192,"L3");
    printsstring32(3,232,"R3");
    
    
    printsstring32(240,32,"L4");
    printsstring32(240,72,"R4");
    printsstring32(240,112,"L5");
    printsstring32(240,152,"R5");  
    
    printsstring16(80,2,"原始值");
    printsstring16(170,2,"采样值");
    printsstring16(320,2,"原始值"); 
    printsstring16(410,2,"采样值"); 
}

/********************************************************************
* Function Name : PraperMTADMenu() 
* Description   : 大小磁头采样数据
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMTADMenu()
{     
    //表格
    printsstring16(3,32,"MT1");
    printsstring16(3,52,"MT2");
    printsstring16(3,72,"MG1");
    printsstring16(3,92,"MG2");
    printsstring16(3,112,"MG3");
    printsstring16(3,132,"MG4");
    printsstring16(3,152,"UV1");
    printsstring16(3,172,"UV2");
    
    printsstring16(100,2,"原始值");
    printsstring16(170,2,"采样值");
}

/********************************************************************
* Function Name : PraperUVADMenu() 
* Description   :荧光采样
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperUVADMenu()
{    
    //表格
    printsstring16(3,32,"UV1");
    printsstring16(3,72,"UV2");
    
    printsstring16(100,2,"原始值");
    printsstring16(170,2,"采样值");
}

/********************************************************************
* Function Name : PraperInfoIRADMenu() 
* Description   :走钞IR采样
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInfoIRADMenu()
{       
    //表格
    printsstring16(3,40,"1");
    printsstring16(3,65,"2"); 
    printsstring16(3,90,"3");
    printsstring16(3,115,"4");  
    printsstring16(3,140,"5");
    printsstring16(3,165,"6");
 
    
    printsstring16(52,2,"E1"); 
    printsstring16(101,2,"E2");
    printsstring16(150,2,"E3");  
    printsstring16(200,2,"E4");
    printsstring16(250,2,"E5");
    printsstring16(300,2,"E6");
    printsstring16(350,2,"E7");
}

/********************************************************************
* Function Name : PraperCheckCSStart() 
* Description   :开始CS校验
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSStart()
{     
    //表格
    printsstring32(50,100,"CS校验开始，请放纸");
}

/********************************************************************
* Function Name : PraperCheckCSMenu() 
* Description   :CS校验
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSMenu()
{      
    //表格
    printsstring32(50,100,"进入CS校验模式，请等待");
}

/********************************************************************
* Function Name : PraperCheckCSFail() 
* Description   :CS校验失败
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSFail()
{  
    //表格
    printsstring32(50,100,"CS校验失败，请重新放纸");
}

/********************************************************************
* Function Name : PraperParmSet() 
* Description   :鉴伪参数修改
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperParmSet()
{       
    setcolor(0xFFFF,0x0000);
	
    switch(AuthenticationDataPages)
    {
        case 0x00:
            //显示标题
            printsstring32(100,2,"鉴伪参数第一页");
            break;
        case 0x01:
            //显示标题
            printsstring32(100,2,"鉴伪参数第二页");
            break;
        case 0x02:
            //显示标题
            printsstring32(100,2,"鉴伪参数第三页");
            break;
        default:
            //显示标题
            printsstring32(100,2,"鉴伪参数第一页");
            break;
    }
}

/********************************************************************
* Function Name : PraperAmountOver() 
* Description   :批量值越界
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperAmountOver()
{  	
    if(TemBatchNum>899)
    {
        printsstring32(100,2,"您输入的批量值太大了");
    }
    else if(TemBatchNum<101)
    {
        printsstring32(100,2,"您输入的批量值太小了");
    }
}

/********************************************************************
* Function Name : PraperCheckCSImageSuccess() 
* Description   :CS校验图像成功
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSImageSuccess()
{      
    //表格字符打印
    setcolor(0xFFFF,0x0000);    
    
    //表格
    printsstring32(50,100,"CS校验图像成功,请重启机器");
}

/********************************************************************
* Function Name : PraperCheckCSStage1Success() 
* Description   :CS校验第一阶段成功
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSStage1Success()
{      
    //表格
    printsstring32(50,100,"CS校验第一阶段成功，请继续放纸,再按启动按键");
}

/********************************************************************
* Function Name : PraperCheckCSStage2Start() 
* Description   :CS校验第二阶段开始
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSStage2Start()
{      
    //表格
    printsstring32(50,100,"CS校验第二阶段开始");
}

/********************************************************************
* Function Name : PraperInfoMTADMenu() 
* Description   :走钞MT采样
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInfoMTADMenu()
{     
    //表格
    printsstring16(240,2,"MT");
    
    printsstring16(40,25,"脉冲个数");
    printsstring16(240,25,"面额真值");
}

/********************************************************************
* Function Name : PraperInfoMGADMenu() 
* Description   :走钞MG采样
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInfoMGADMenu()
{      
    //表格
    printsstring16(3,32,"MT1");
    printsstring16(3,52,"MT2");
    printsstring16(3,72,"MG1");
    printsstring16(3,92,"MG2");
    printsstring16(3,112,"MG3");
    printsstring16(3,132,"MG4");
    printsstring16(3,152,"UV1");
    printsstring16(3,172,"UV2");
    
    printsstring16(80,2,"1");
    printsstring16(130,2,"2");
    printsstring16(180,2,"3");
    printsstring16(230,2,"4");
    printsstring16(280,2,"5");
    printsstring16(330,2,"6");
    printsstring16(380,2,"7");
}

/********************************************************************
* Function Name : PraperSpeedSetMenu() 
* Description   : 速度调试菜单进钞模式
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperSpeedSetMenuINMode()
{  
    //进钞模式显示
    setcolor(0xF800,0xFFE0);
    printsstring32(3,5,"进钞");
    
}

/********************************************************************
* Function Name : PraperSpeedSetMenu() 
* Description   : 速度调试菜单出钞模式
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperSpeedSetMenuOutMode()
{  
    //进钞模式显示
    setcolor(0xF800,0xFFE0);
    printsstring32(3,68,"出钞");
}

/********************************************************************
* Function Name : DisplayDebugMenu() 
* Description   : 调试菜单显示
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperOldTest()
{      
    //设置字体颜色
	setcolor(0x0000,0xFFFF);
    
    //表格
    printsstring32(70,72,"进钞");
    printsstring32(70,110,"清分");
    printsstring32(70,148,"电磁铁");
    printsstring32(70,186,"小电机");
    printsstring32(70,224,"LCD");

    //表格行
    printsstring32(190,36,"PWM");
    printsstring32(280,36,"SPEED"); 
}

/********************************************************************
* Function Name : PraperMTDispTest() 
* Description   : MT波形数据的显示
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMTDispTest()
{  
    setcolor(0x0000,0xFFFF);

    printsstring32(130,2,"MT数据查看");
     //表格
    printsstring16(3,82,"1");
    printsstring16(3,112,"2");
    printsstring16(3,142,"3");
    printsstring16(3,172,"4");
    printsstring16(3,202,"5");
    printsstring16(3,232,"6");

    printsstring16(80,52,"1");
    printsstring16(130,52,"2");
    printsstring16(180,52,"3");
    printsstring16(230,52,"4");
    printsstring16(280,52,"5");
    printsstring16(330,52,"6");
    printsstring16(380,52,"7");      
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : 升级菜单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUp()
{  
    setcolor(0x0000,0xFFFF);
    
    printsstring32(3,5,"UI ARM");
    printsstring32(3,68,"DSP");
    
    printsstring32(3,136,"模板");
    printsstring32(3,204,"FPGA");
    
    printsstring32(350,5,"MT ARM");
    printsstring32(350,68,"  ");
    
    printsstring32(350,136,"");
    printsstring32(350,204,"返回");
      
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : 升级菜单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUpING()
{  
    //背景色
    setcolor(0x0000,0xFFFF);
    fillw(MATRIX,0,0,479,271);
    fillw(CLEAR,3,2,478,270);
    
    printsstring32(100,130,"升级中，请等待");
    printsstring32(100,170,"升级完成前请不要关机");      
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : 升级菜单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUpED()
{  
    //背景色
    printsstring32(100,130,"升级成功请重启");
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : 升级菜单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUpFail()
{  
    //背景色
    printsstring32(100,130,"升级失败请重启后重新升级");
}


/********************************************************************
* Function Name : PraperPasswordError() 
* Description   : 密码出错菜单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperPasswordError()
{      
    setcolor(0xFFFF,0x0000);
    printsstring32(160,120,"密码出错");
}

/********************************************************************
* Function Name : PraperNetPostTime() 
* Description   : 网发时间设置
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperNetPostTime()
{  
    printsstring32(180,150,"网发时间");
}

/********************************************************************
* Function Name : PraperNetPostPages() 
* Description   : 网发张数设置
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperNetPostPages()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"网发张数");
}

/********************************************************************
* Function Name : PraperBlankName() 
* Description   : 银行简称
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBlankName()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"银行简称");
}

/********************************************************************
* Function Name : PraperAreaCode() 
* Description   : 地区号
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperAreaCode()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"地区号");
}

/********************************************************************
* Function Name : PraperBranchNumber() 
* Description   : 支行号
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBranchNumber()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"支行号");
}

/********************************************************************
* Function Name : PraperNetworkNumber() 
* Description   : 网点号
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperNetworkNumber()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"网点号");
}

/********************************************************************
* Function Name : PraperIPNumber() 
* Description   : IP
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperIPNumber()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"IP");
}

/********************************************************************
* Function Name : PraperBanTechnician1() 
* Description   : 禁止网发1
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBanTechnician1()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"禁止网发1");
}

/********************************************************************
* Function Name : PraperBanTechnician2() 
* Description   : 禁止网发2
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBanTechnician2()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"禁止网发2");
}

/********************************************************************
* Function Name : PraperBanTechnician3() 
* Description   : 禁止网发2
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBanTechnician3()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"禁止网发3");
}

/********************************************************************
* Function Name : PraperMask() 
* Description   : Mask
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMask()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"Mask");
}

/********************************************************************
* Function Name : PraperGateWay() 
* Description   : GateWay
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperGateWay()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"GateWay");
}

/********************************************************************
* Function Name : PraperLocalPort() 
* Description   : 本地端口
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLocalPort()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"本地端口");
}

/********************************************************************
* Function Name : PraperDistalPort() 
* Description   : 远端端口
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperDistalPort()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"远端端口");
}

/********************************************************************
* Function Name : PraperBlackList() 
* Description   : 黑名单
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBlackList()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"黑名单");
}

/********************************************************************
* Function Name : PraperBlackListPage() 
* Description   : 黑名单页码
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBlackListPage()
{  
    setcolor(0xFFFF,0x0000);
	switch(BlackListPage)
	{
		case 0x00:
  			printsstring16(370,6,"第一页");
			break;
		case 0x01:
			printsstring16(370,6,"第二页");
			break;
		default:
			printsstring16(370,6,"第一页");
			break;
	}
}


/********************************************************************
* Function Name : PraperDistalIP() 
* Description   : 远端IP
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperDistalIP()
{  
    //背景色
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"远端IP");
}

/********************************************************************
* Function Name : PraperInformation3Page() 
* Description   : 显示用户信息的第三页
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInformation3Page()
{    
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,88,27,184,244);
    printsstring16(117,36,"MASK ");
    
    printsstring16(117,64,"GW ");
    
    printsstring16(117,96,"本地端口");
    
    printsstring16(117,125,"远端端口");
    
    printsstring16(117,155,"远端IP");

}

/********************************************************************
* Function Name : DisplayCommonMenu() 
* Description   : 普通菜单显示函数
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayStartMenu(struct st_menu* pMenu, INT8U cursor)
{
    printBackImage(30);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(31);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(32);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(33);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(34);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(35);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(36);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(37);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(38);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(39);
    OSTimeDlyHMSM(0,0,2,0);
}

/********************************************************************
* Function Name : DisplayPasswordMenu() 
* Description   : 密码显示函数
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayPasswordMenu(INT8U cursor)
{
        setcolor(0xFFFF,0x0000);
        switch(cursor)
        {
            case 0x01:
                printsstring32(160,128,"*");
                break;
            case 0x02:
                printsstring32(200,128,"*");
                break;
            case 0x03:
                printsstring32(240,128,"*");
                break;
            case 0x04:
                printsstring32(280,128,"*");
                break;
            default:
                break;
        }
}
/********************************************************************
* Function Name : DisplayParaMenu() 
* Description   : 显示参数
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayParaMenu(void)
{
    INT8U Param[12] = {0x10,0x11,0x12,0x13,0x21,0x22,0x23,0x24,0x31,0x32,0x33,0x34};
    
    INT8U horizontal = 0;
    INT8U vertical = 0;
    for(horizontal = 0;horizontal < 10;horizontal++)
    {
        for(vertical = 0;vertical < 10;vertical++)
        {
            FetchBackColor(EDITPARAMCOLOR01);
            printStrings16(BaseCheckParam[horizontal][vertical],&Param[vertical]);
        }
    }
}

/********************************************************************
* Function Name : DisplayEditParaMenu() 
* Description   : 显示并修改参数
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayEditParaMenu(void)

{
    INT8U horizontal = 0;
    INT8U vertical = 0;
    INT8U i = 0;
    DisplayCursor();
      
    //背景色
    setcolor(0x0000,0x0000);    

    if(AuthenticationDataPages == 2)
    {
        for(vertical = 0;vertical < 2;vertical++)
        {
            for(horizontal = 0;horizontal < 10;horizontal++)
            {
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3];
                if(i>99)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3] = 99;
                    i = 99;
                }
                print2charStrings16(BaseEditParam[horizontal][vertical*4+1],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+1];
                printStrings16(BaseEditParam[horizontal][vertical*4+2],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2];
                if(i>1)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2] = 0;
                    i = 0;
                }
                print1charStrings16(BaseEditParam[horizontal][vertical*4+3],&i);
            }
        }
        for(horizontal = 0;horizontal < 4;horizontal++)
        {
            i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3];
            if(i>99)
            {
                AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3] = 99;
                i = 99;
            }
            print2charStrings16(BaseEditParam[horizontal][vertical*4+1],&i);
            i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+1];
            printStrings16(BaseEditParam[horizontal][vertical*4+2],&i);
            i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2];
            if(i>1)
            {
                AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2] = 0;
                i = 0;
            }
            print1charStrings16(BaseEditParam[horizontal][vertical*4+3],&i);
        }
    }
    else
    {
        for(vertical = 0;vertical < 3;vertical++)
        {
            for(horizontal = 0;horizontal < 10;horizontal++)
            {
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3];
                if(i>99)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3] = 99;
                    i = 99;
                }
                print2charStrings16(BaseEditParam[horizontal][vertical*4+1],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+1];
                printStrings16(BaseEditParam[horizontal][vertical*4+2],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2];
                if(i>1)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2] = 0;
                    i = 0;
                }
                print1charStrings16(BaseEditParam[horizontal][vertical*4+3],&i);
            }
        }
    }
}

/********************************************************************
* Function Name : DisplayCursor() 
* Description   : 光标更新
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayCursor(void)
{
    gs_CursorInPage.XCoor = BaseEditParam[Paramhorizontal][Paramvertical].sXMin;
    gs_CursorInPage.YCoor = BaseEditParam[Paramhorizontal][Paramvertical].sYMin;
    PrintCursor(gs_CursorInPage);
}

/********************************************************************
* Function Name : ClearCursor() 
* Description   : 消除原有光标
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearCursor(void)
{
    gs_CursorInPage.XCoor = BaseEditParam[Paramhorizontal][Paramvertical].sXMin;
    gs_CursorInPage.YCoor = BaseEditParam[Paramhorizontal][Paramvertical].sYMin;
    CloseCursor(gs_CursorInPage);
}

/********************************************************************
* Function Name : DisplayClearLevelmark() 
* Description   : 清分等级和鉴伪等级选中勾更新
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayClearLevelmark()
{
    if(gps_CurMenu->FrameID == 6)
    {
        //清分等级
        tRectangle Checkmark,DispMark;
        DispMark.sXMin = BaseBlackListCoordinate[1].sXMin;
        DispMark.sXMax = BaseBlackListCoordinate[1].sXMax-5;
        DispMark.sYMin = BaseBlackListCoordinate[1].sYMin;
        DispMark.sYMax = BaseBlackListCoordinate[1].sYMax-4;
        
        Checkmark.sXMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMax;
        printIcon(BLACKLISTMARK,DispMark,Checkmark);
    
    }
    else if(gps_CurMenu->FrameID == 7)
    {
        //鉴伪等级
        tRectangle Checkmark,DispMark;
        DispMark.sXMin = BaseBlackListCoordinate[1].sXMin+2;
        DispMark.sXMax = BaseBlackListCoordinate[1].sXMax-6;
        DispMark.sYMin = BaseBlackListCoordinate[1].sYMin;
        DispMark.sYMax = BaseBlackListCoordinate[1].sYMax-4;
        
        Checkmark.sXMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMax;
        printIcon(BLACKLISTMARK,DispMark,Checkmark);
    }

}

/********************************************************************
* Function Name : ClearClearLevelmark() 
* Description   : 清分等级和鉴伪等级选中勾消除
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearClearLevelmark()
{
    tRectangle Checkmark;
    if(gps_CurMenu->FrameID == 6)
    {
        //清分等级
        Checkmark.sXMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMax;
        printIcon(BLACKLISTBACK,Checkmark,Checkmark);
    }
    else if(gps_CurMenu->FrameID == 7)
    {
        //清分等级
        Checkmark.sXMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMax;
        printIcon(BLACKLISTBACK,Checkmark,Checkmark);
    
    }
}

/********************************************************************
* Function Name : DisplayCheckmark() 
* Description   : 黑名单菜单选中勾更新
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayCheckmark(void)
{
    tRectangle Checkmark,DispMark;
    DispMark.sXMin = BaseBlackListCoordinate[1].sXMin;
    DispMark.sXMax = BaseBlackListCoordinate[1].sXMax;
    DispMark.sYMin = BaseBlackListCoordinate[1].sYMin;
    DispMark.sYMax = BaseBlackListCoordinate[1].sYMax;
    
    Checkmark.sXMin = BaseBlackListCoordinate[BlackListhorizontal].sXMin;
    Checkmark.sXMax = BaseBlackListCoordinate[BlackListhorizontal].sXMax;
    Checkmark.sYMin = BaseBlackListCoordinate[BlackListhorizontal].sYMin;
    Checkmark.sYMax = BaseBlackListCoordinate[BlackListhorizontal].sYMax;
    printIcon(BLACKLISTMARK,DispMark,Checkmark);
}

/********************************************************************
* Function Name : ClearCheckmark() 
* Description   : 黑名单菜单选中勾消除
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearCheckmark(void)
{
    tRectangle Checkmark;
    Checkmark.sXMin = BaseBlackListCoordinate[BlackListhorizontal].sXMin;
    Checkmark.sXMax = BaseBlackListCoordinate[BlackListhorizontal].sXMax;
    Checkmark.sYMin = BaseBlackListCoordinate[BlackListhorizontal].sYMin;
    Checkmark.sYMax = BaseBlackListCoordinate[BlackListhorizontal].sYMax;
    printIcon(BLACKLISTBACK,Checkmark,Checkmark);
}

/********************************************************************
* Function Name : DisplayKeyBoard() 
* Description   : 按键选中状态更新
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayKeyBoard(void)
{
    tRectangle KeyBoard;
    KeyBoard.sXMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMin;
    KeyBoard.sXMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMax;
    KeyBoard.sYMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMin;
    KeyBoard.sYMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMax;
    printIcon(KEYBOARDACRTIVE,KeyBoard,KeyBoard);
}

/********************************************************************
* Function Name : ClearKeyBoard() 
* Description   : 按键选中状态消除
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearKeyBoard(void)
{
    tRectangle KeyBoard;
    KeyBoard.sXMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMin;
    KeyBoard.sXMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMax;
    KeyBoard.sYMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMin;
    KeyBoard.sYMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMax;
    printIcon(KEYBOARDIMAGE,KeyBoard,KeyBoard);
}


/***********************************************************************************************
* Function		: printsstring16
* Description	: 显示16*16大小的字符串
* Input			:   x       横坐标
                    y       纵坐标
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring16(INT16U x,INT16U y,INT8U *s)
{
    prints(0x54,x,y,s);
}


/********************************************************************
* Function Name : DisplayInfomationList() 
* Description   : 显示当前选中第几条
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayInfomationList(void)
{
    gs_CursorInPage.XCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sXMin;
    gs_CursorInPage.YCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sYMin;
    PrintCursor(gs_CursorInPage);
    InformationDisp();
}

/********************************************************************
* Function Name : DisplayInfomationList() 
* Description   : 显示当前选中第几条
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void CleanInfomationList(void)
{
    gs_CursorInPage.XCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sXMin;
    gs_CursorInPage.YCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sYMin;
    CloseCursor(gs_CursorInPage);
}

/********************************************************************
* Function Name : InformationDisp() 
* Description   : 用户信息显示
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void InformationDisp(void)
{   
    setcolor(0xFFFF,0x0000);
    
    switch(InfomationPage)
    {
        case 0x00:
            //网发时间
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][0],NetPostTime);
            //网发张数
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][1],NetPostPage);
            //银行简称
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][2],BlankName);
            //地区号
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][3],AreaNum);
            //支行号
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][4],BranchNum);
            //网点号
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][5],LatticePoint);
            //IP
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][6],IP);
            break;
        case 0x01:
            //MAC
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][0],Mac);
            //机器序列号
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][1],MachineNo);
            //禁止网发1
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][2],BanTechnician[0]);
            //禁止网发2
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][3],BanTechnician[1]);
            //禁止网发3
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][4],BanTechnician[2]);
            //时间
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][5],Time);
            break;
        case 0x02:
            //MASK
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][0],Mask);
            //GW
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][1],GateWay);
            //本地端口
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][2],LocalPort);
            //远端端口
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][3],DiskPort);
            //远端IP
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][4],DiskIP);
            break;   
        default:
            break; 
    }
}

/********************************************************************
* Function Name : blackListDisp() 
* Description   : 黑名单显示
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void blackListDisp(void)
{   
    INT8U i = 0;
    
    setcolor(0xFFFF,0x0000);
    
    for(i=0;i<7;i++)
    {
        //黑名单发送
        printVersionStrings32(BaseBlackListNumberCoordinate[i],BlackLists[BlackListPage][i]);
    }
}

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/
