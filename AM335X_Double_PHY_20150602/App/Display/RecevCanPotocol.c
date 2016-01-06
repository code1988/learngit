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
#include "display.h"
#include "RecevCanPotocol.h"
#include "CoodinateInit.h"
#include "DisplayMain.h"
#include "DwinPotocol.h"
#include "DealWithError.h"
/* Private define-----------------------------------------------------------------------------*/
#define SENDTODISPLEN (SendToDisp[2]+3)
#define CLEAR 0x5A
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
INT8U RecevToCAN[100] = {0};
extern INT8U CLEARINGMODE;
extern INT8U DEPWITHDRAWMODE;
extern INT8U SUMMATIONMODE;

extern INT8U DataSettings[252];
extern INT8U AuthenticationLevel[7];
extern INT8U AuthenticationSwitch[14];
extern INT8U ClearLevel[6];
extern INT8U MainMenuDisp[13];
extern INT8U MixedDisp[18];
extern INT8U OUTLETDisp[200][24];
extern INT8U INLETDisp[300][24];
extern INT8U EditionDisp[7];
extern INT8U JcSpeedDisp[6];
extern INT8U InLetADDisp[15]; 
extern INT8U IRADDisp[30];
extern INT8U MTDisp[30];
extern INT8U UVDisp[30];
extern INT8U InfoIRDisp[50];
extern INT8U InfoMTDisp[50];
extern INT8U InfoMGDisp[6][50];
extern INT8U MTwaveform[50];

extern VAL_VOL int g_dsp_send_money_status;

INT8U OutLetPageNum;
INT8U OutLetTotalPage = 0;
INT8U InLetTotalPage = 0;
INT8U InLetPageNum;
INT8U TotalCANPage = 0;
INT8U MCCanID = 0;
INT8U InfoADpage = 0;
INT8U StartRun = 0;

//查询显示的菜单
INT8U SelectMode = 0;

INT8U Reply_CAN[8] = {0};
extern _BSP_MESSAGE Dsend_message; 
extern ST_Menu *gps_CurMenu;     //当前菜单
//按键状态
extern INT8U KeyStatus;
//主菜单批量值
extern INT16U BatchNum;

extern INT8U INchangflag;
INT8U FLASHflag = 0;
extern INT8U OUTchangflag;
extern INT32U DenomNum;
extern INT8U ErrorFlag;

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: DealWithMC
* Description	        : 处理马达板的信息
* Input			: 
* Output		:  
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/	
void DealWithMC(INT8U *CanData)
{
        if(CanData[UIACANLIST02] == UIACANMAINMENU)
        {
            if(gps_CurMenu->Menu_Flag == 0)
            {
           		StartRun = 0;
                switch(CanData[UIACANLIST04])
                {
                    case 0x00:
                        //马达待机中
                        TotalCANPage = CanData[UIACANLIST03];
                        KeyStatus = KEYENABLE;
                        ErrorFlag = FALSE;
                        PraPerprintBackImage(0);
                        PraperMianMenu();
                        UIA_Recev_Main_Menu(CanData);
                        break;
                    case 0x01:
                        //dianchao 
                        TotalCANPage = CanData[UIACANLIST03];
                        if(ErrorFlag == TRUE)
                        {
                            ErrorFlag = FALSE;
                            PraPerprintBackImage(0);
                            PraperMianMenu();
                        }
                        KeyStatus = KEYDISABLE;
                        UIA_Recev_Main_Menu(CanData);
                        break;
                    case 0x02:
                        //马达暂停
                        TotalCANPage = CanData[UIACANLIST03];
                        KeyStatus = KEYDISABLE;
                        if(ErrorFlag == TRUE)
                        {
                            ErrorFlag = FALSE;
                            PraPerprintBackImage(0);
                            PraperMianMenu();
                        }
                        break;
                    case 0x03:
                        //报警出错
                        KeyStatus = KEYENABLE;
                        ErrorFlag = TRUE;
                        ErrorDisp(CanData[UIACANLIST04]);
                        break;
                    case 0x04:
                        //菜单设置
                        TotalCANPage = CanData[UIACANLIST03];
                        KeyStatus = KEYENABLE;
                        break;
                    default:
                        //DoNothing
                        TotalCANPage = CanData[UIACANLIST03];
                        break;
                }
            }
        }
        else
        {
                switch(CanData[UIACANLIST02])
                {
                    case UIACANMIXEDSEARCH:
                        UIA_Recev_Mixed(CanData);
                        break;
                    case UIACANOUTLETSEARCH:
                        UIA_Recev_OUTLET(CanData);
                        break;
                    case UIACANINLERSSEARCH:
                        UIA_Recev_INLET(CanData);
                        break;
                    case UIACANERRORCODE:
                        //detail error message   
                        break;
                    case UIACANEDITION:
                        UIA_Recev_Edition(CanData);
                        break;
                    case UIACANJCSPEED:
                        UIA_Recev_JCSPEED(CanData);
                        break;
                    case UIACANINLETAD:
                        UIA_Recev_Inlet_AD(CanData);
                        break;
                    case UIACANIRAD:
                        UIA_Recev_IR_AD(CanData);
                        break;
                    case UIACANMGAD:
                        UIA_Recev_MT_AD(CanData);
                        break;
                    case UIACANUVAD:
                        UIA_Recev_UV_AD(CanData);
                        break;
                    case UIACANINFOAD:
                        UIA_Recev_Info_AD(CanData);
                        break;
                    case UIACANLEVELUPSTART:
                        UIA_Recev_Level_Up(CanData);
                        break;
                    case UIACANWAVEFORM:
                        UIA_Recev_Waveform(CanData);
                        break;
                    default:
                        break;
                }
        }
}

/***********************************************************************************************
* Function		: UIA_Recev_Main_Menu
* Description	        : 主菜单跑钞实时信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_Main_Menu(INT8U *CanData)
{
     memcpy(MainMenuDisp,CanData+8,6);

     memcpy(MainMenuDisp+6,CanData+16,7);
          
     RefreshMainMenu();
}

/***********************************************************************************************
* Function		: UIA_Recev_Mixed
* Description	        : 接收到的混点明细数据解析
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_Mixed(INT8U *CanData)
{
    memcpy(MixedDisp,CanData+8,6);
    
    memcpy(MixedDisp+6,CanData+16,6);
    
    memcpy(MixedDisp+12,CanData+24,6);
    

    if(gps_CurMenu->FrameID == 3)
    {
        gps_CurMenu = &gsMenu0_3;
        printBackImage(gps_CurMenu->FrameID);
        //刷新混点明细
        MixMenuNumDisp();
    }

}

/***********************************************************************************************
* Function		: UIA_Recev_Mixed
* Description	        : 接收到的退钞口冠字号解析
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_OUTLET(INT8U *CanData)
{
    char time[20]={0};
	if(StartRun == 0)
	{
       g_dsp_send_money_status = 1;
	}
	
	 StartRun = 1;
     memcpy(MainMenuDisp,CanData+8,6);

     memcpy(MainMenuDisp+6,CanData+16,7);
     
    if(CanData[29] == 0)
    {
        //200张清零
        if(InLetTotalPage>199)
        {
            InLetTotalPage = 0;
            memset(INLETDisp,0,7200);
        }
        memcpy(&INLETDisp[InLetTotalPage][InLetPageNum],CanData+24,7);
        memcpy(&INLETDisp[InLetTotalPage][InLetPageNum+7],CanData+32,7);
        memcpy(&INLETDisp[InLetTotalPage][InLetPageNum+14],CanData+40,7);
        InLetTotalPage++;
    }
    else
    {
        //200张清零
        if(OutLetTotalPage>198)
        {
            OutLetTotalPage = 0;
            memset(OUTLETDisp,0,7200);
        }
        memcpy(&OUTLETDisp[OutLetTotalPage][OutLetPageNum],CanData+24,7);
        memcpy(&OUTLETDisp[OutLetTotalPage][OutLetPageNum+7],CanData+32,7);
        memcpy(&OUTLETDisp[OutLetTotalPage][OutLetPageNum+14],CanData+40,7);
        OutLetTotalPage++;
    }

    if(gps_CurMenu->FrameID == 0)
    {
        RefreshMainMenu();
    }
}

/***********************************************************************************************
* Function		: UIA_Recev_INLET
* Description	        : 接收到的接钞口冠字号解析
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_INLET(INT8U *CanData)
{    

}

/***********************************************************************************************
* Function		: UIA_Recev_Edition
* Description	        : 版本信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_Edition(INT8U *CanData)
{

}

/***********************************************************************************************
* Function		: UIA_Recev_JCSPEED
* Description	        :马达速度调试
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_JCSPEED(INT8U *CanData)
{
    memcpy(JcSpeedDisp,CanData+8,6);
    JcSpeedDispMenu();
}

/***********************************************************************************************
* Function		: UIA_Recev_Inlet_AD
* Description	:进钞，接钞AD采样     
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_Inlet_AD(INT8U *CanData)
{
    INT8U InletADpages = 0;
    INT8U i = 1;
    InletADpages = CanData[UIACANLIST03];
    while(InletADpages>0)
    {
        memcpy(InLetADDisp+5*(i-1),CanData+i*8,5);
        InletADpages--;
        i++;
    
    }
    InLetADDispMenu();
}

/***********************************************************************************************
* Function		: UIA_Recev_IR_AD
* Description	:红外AD采样     
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_IR_AD(INT8U *CanData)
{
    memcpy(IRADDisp,CanData+8,7);
    memcpy(IRADDisp+7,CanData+16,3);
    
    memcpy(IRADDisp+15,CanData+24,7);
    memcpy(IRADDisp+22,CanData+32,3);
  
    IRADDispMenu();
}

/***********************************************************************************************
* Function		: UIA_Recev_MT_AD
* Description	:大小磁头AD采样     
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_MT_AD(INT8U *CanData)
{    
    memcpy(MTDisp,CanData+8,6);
    memcpy(MTDisp+10,CanData+16,6);
    
    MTADDispMenu();
}

/***********************************************************************************************
* Function		: UIA_Recev_UV_AD
* Description	:荧光AD采样     
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_UV_AD(INT8U *CanData)
{    
    memcpy(UVDisp,CanData+8,2);
    memcpy(UVDisp+10,CanData+16,2);
    
    UVADDispMenu();
}

/***********************************************************************************************
* Function		: UIA_Recev_Info_AD
* Description	:走钞信息查看     
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_Info_AD(INT8U *CanData)
{    
    memcpy(InfoIRDisp,CanData+8,7);
    memcpy(InfoMTDisp,CanData+16,8);
    memcpy(&InfoMGDisp[0][0],CanData+24,8);
    memcpy(&InfoMGDisp[0][8],CanData+32,8);
    memcpy(&InfoMGDisp[1][0],CanData+40,8);
    memcpy(&InfoMGDisp[1][8],CanData+48,8);  
    
    if(InfoADpage == 0)
    {
        InfoIRADDispMenu();
    }
    else if(InfoADpage == 1)
    {
        InfoMTADDispMenu();
    }
    else if(InfoADpage == 2)
    {
        InfoMGADDispMenu();
    }
}

/***********************************************************************************************
* Function		: UIA_Recev_Level_Up
* Description	:马达升级信息     
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_Level_Up(INT8U *CanData)
{       
    switch(CanData[3])
    {
        case 0x01:
            //升级成功
            PraperLevelUpED();
            break;
        case 0x02:
            //升级失败
            PraperLevelUpFail();
            break;
        default:
            //升级失败
            PraperLevelUpFail();
            break;
    }
}

/***********************************************************************************************
* Function		: UIA_Recev_Waveform
* Description	:接收到DAT文件生成帧   
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/				
void UIA_Recev_Waveform(INT8U *CanData)
{       
    save_cur_money_info();
}

/***********************************************************************************************
* Function		: RefreshMainMenu
* Description	        : 刷新主菜单(实时跑钞)
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 17/06/2010	wangyao
***********************************************************************************************/	
void RefreshMainMenu(void)
{
    if(((MainMenuDisp[OUTLETHIGHBYTE]*256 + MainMenuDisp[OUTLETLOWBYTE])==0)&&(OUTchangflag==1))FLASHflag = 1;
    if(((MainMenuDisp[INLETHIGHBYTE]*256 + MainMenuDisp[INLETLOWBYTE])==0)&&(INchangflag==1))FLASHflag = 1; 
    if(((MainMenuDisp[DENOMHIGHBYTE]*256 + MainMenuDisp[DENOMLOWBYTE])!=DenomNum))FLASHflag = 1; 
    if(FLASHflag == 1)
    {
        PraPerprintBackImage(0);
        PraperMianMenu();
        FLASHflag = 0;
        INchangflag = 0;
        OUTchangflag = 0;
    }
    MianMenuAmountNumDisp();
    MianMenuInletNumDisp();
    MianMenuOutletNumDisp();
    MianMenuBatchNumDisp();
    MianMenuDenomNumDisp();
    MianMenuTotalNumDisp();
}

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


