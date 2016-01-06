/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		    : CanSendUIData.c
* Author		    : 
* Date First Issued	: 150204   
* Version		    : V应用程序
* Description		: CAN发送UIDATA
*----------------------------------------历史版本信息-------------------------------------------
* History		    :
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "appstruct.h"
#include "CanSendUIData.h"

/***********************************************************************************************
* Function		: UIA_Send_MC_Authentication_Shark
* Description	        : 鉴伪参数握手
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_Authentication_Shark(INT8U *CanData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANAUTHENTICATION;
    *(CanData + UIACANINFO) = UIACANAUTHENTICATIONTOTALPAGES;
    *(CanData + UIACANINFO + 1) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 2) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = UIACANPAGE0;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_Authentication_Param
* Description	        : 鉴伪参数设定
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_Authentication_Param(INT8U *UIData,INT8U Page,INT8U *CanData)
{
    *(CanData + UIACANLIST01) = UIData[UIACANLIST01];
    *(CanData + UIACANLIST02) = UIData[UIACANLIST02];
    *(CanData + UIACANLIST03) = UIData[UIACANLIST03];
    *(CanData + UIACANLIST04) = UIData[UIACANLIST04];
    *(CanData + UIACANLIST05) = UIData[UIACANLIST05];
    *(CanData + UIACANLIST06) = UIData[UIACANLIST06];
    *(CanData + UIACANLIST07) = UIData[UIACANLIST07];
    *(CanData + UIACANLIST08) = Page;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_Motor_Flag_Set
* Description   	: 马达板是否能点钞设置
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 4/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_Motor_Flag_Set(INT8U *CanData,INT8U *UIData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANMOTORSET;
    *(CanData + UIACANINFO) = UIData[0];
    *(CanData + UIACANINFO + 1) = UIData[1];
    *(CanData + UIACANINFO + 2) = UIData[2];
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_Mode
* Description	        : 点钞模式
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 4/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_Mode(INT8U *CanData,INT8U *UIData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANMODE;
    *(CanData + UIACANINFO) = UIData[0];
    *(CanData + UIACANINFO + 1) = UIData[1];
    *(CanData + UIACANINFO + 2) = UIData[2];
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_JCSPEEDEDIT_Search
* Description	: 马大速度调试内容
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_JCSPEEDEDIT_Search(INT8U *CanData,INT8U *UIData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANJCSPEED;
    *(CanData + UIACANINFO) = UIData[0];
    *(CanData + UIACANINFO + 1) = UIData[1];
    *(CanData + UIACANINFO + 2) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_Mixed_Search
* Description	        : 混点明细查询
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_Mixed_Shark(INT8U *CanData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANMIXEDSEARCH;
    *(CanData + UIACANINFO) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 1) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 2) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_OutLet_Search
* Description	        : 退钞口冠字号码查看
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_OutLet_Search(INT8U *CanData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANOUTLETSEARCH;
    *(CanData + UIACANINFO) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 1) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 2) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_InLets_Search
* Description	        : 进钞口冠字号码查看
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_InLets_Search(INT8U *CanData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANINLERSSEARCH ;
    *(CanData + UIACANINFO) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 1) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 2) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_batch
* Description	: 批量值设置
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 4/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_batch(INT8U *CanData,INT8U *UIData)
{
    *(CanData + UIACANHEAD) = UIACANID;
    *(CanData + UIACANHEAD + 1) = UIACANBATCH;
    *(CanData + UIACANINFO) = UIData[0];
    *(CanData + UIACANINFO + 1) = UIData[1];
    *(CanData + UIACANINFO + 2) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 3) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 4) = DEFAULTBYTE;
    *(CanData + UIACANINFO + 5) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_AuthenticSwitch_Param
* Description	        : 鉴伪功能开关设定
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_AuthenticSwitch_Param1(INT8U *CanData,INT8U *UIData)
{
    *(CanData + UIACANLIST01) = UIACANID;
    *(CanData + UIACANLIST02) = UIACANAUTHENTICLEVEL1;
    *(CanData + UIACANLIST03) = UIData[UIACANLIST01];
    *(CanData + UIACANLIST04) = UIData[UIACANLIST02];
    *(CanData + UIACANLIST05) = UIData[UIACANLIST03];
    *(CanData + UIACANLIST06) = UIData[UIACANLIST04];
    *(CanData + UIACANLIST07) = UIData[UIACANLIST05];
    *(CanData + UIACANLIST08) = UIData[UIACANLIST06];
}

/***********************************************************************************************
* Function		: UIA_Send_MC_AuthenticSwitch_Param
* Description	        : 鉴伪功能开关设定
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_AuthenticSwitch_Param2(INT8U *CanData,INT8U *UIData)
{
    *(CanData + UIACANLIST01) = UIACANID;
    *(CanData + UIACANLIST02) = UIACANAUTHENTICLEVEL2;
    *(CanData + UIACANLIST03) = UIData[UIACANLIST07];
    *(CanData + UIACANLIST04) = DEFAULTBYTE;
    *(CanData + UIACANLIST05) = DEFAULTBYTE;
    *(CanData + UIACANLIST06) = DEFAULTBYTE;
    *(CanData + UIACANLIST07) = DEFAULTBYTE;
    *(CanData + UIACANLIST08) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_ClearLevel_Param
* Description	        : 清分等级
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_ClearLevel_Param(INT8U *CanData,INT8U *UIData)
{
    *(CanData + UIACANLIST01) = UIACANID;
    *(CanData + UIACANLIST02) = UIACANCLEARLEVEL;
    *(CanData + UIACANLIST03) = UIData[UIACANLIST06];
    *(CanData + UIACANLIST04) = UIData[UIACANLIST05];
    *(CanData + UIACANLIST05) = UIData[UIACANLIST04];
    *(CanData + UIACANLIST06) = UIData[UIACANLIST03];
    *(CanData + UIACANLIST07) = UIData[UIACANLIST02];
    *(CanData + UIACANLIST08) = UIData[UIACANLIST01];
}

/***********************************************************************************************
* Function		: UIA_Send_MC_LevelUp
* Description	: MT ARM升级
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_LevelUp_Start(INT8U *CanData)
{
    *(CanData + UIACANLIST01) = UIACANID;
    *(CanData + UIACANLIST02) = UIACANLEVELUPSTART;
    *(CanData + UIACANLIST03) = DEFAULTBYTE;
    *(CanData + UIACANLIST04) = DEFAULTBYTE;
    *(CanData + UIACANLIST05) = DEFAULTBYTE;
    *(CanData + UIACANLIST06) = DEFAULTBYTE;
    *(CanData + UIACANLIST07) = DEFAULTBYTE;
    *(CanData + UIACANLIST08) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: UIA_Send_MC_LevelUp
* Description	: MT ARM升级
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 5/11/2014	songchao
***********************************************************************************************/				
void UIA_Send_MC_LevelUp(INT8U *CanData,INT32U *UIData)
{
    *(CanData + UIACANLIST01) = UIACANID;
    *(CanData + UIACANLIST02) = UIACANLEVELUPDATA;
    *(CanData + UIACANLIST03) = (*UIData)>>16;
    *(CanData + UIACANLIST04) = ((*UIData)>>8)&0xFF;
    *(CanData + UIACANLIST05) = (*UIData)&0xFF;
    *(CanData + UIACANLIST06) = DEFAULTBYTE;
    *(CanData + UIACANLIST07) = DEFAULTBYTE;
    *(CanData + UIACANLIST08) = DEFAULTBYTE;
}

/***********************************************************************************************
* Function		: DealWitchUIData
* Description	: 处理UI发送过来的数据传送给马达ARM
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 3/2/2014	songchao
***********************************************************************************************/
void DealWitchUIData(INT8U *UIData,INT16U DivNum,_BSPDCAN_CONFIG CANWriteConfig)
{
    INT8U CanData[8];
    INT32U SenCanBuff[2] = {0x00};
    INT8U Page;
     
    switch(DivNum)
    {
        case UIACANMODE:  
            UIA_Send_MC_Mode(CanData,UIData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);          
            break;
        case UIACANBATCH:
            UIA_Send_MC_batch(CanData,UIData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);    
            break;
        case UIACANMOTORSET:  
            UIA_Send_MC_Motor_Flag_Set(CanData,UIData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);        
            break;
        case UIACANAUTHENTICATION:     
            //鉴伪参数发送
            UIA_Send_MC_Authentication_Shark(CanData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);    
            OSTimeDlyHMSM(0,0,0,1);    
            
            for(Page = 0; Page < 28;Page++) 
            {
                UIA_Send_MC_Authentication_Param((UIData+7*(Page)),(Page+1),CanData);   
                memcpy(SenCanBuff,CanData,8);
                CANWriteConfig.entry.dlc = 8;
                CANWriteConfig.entry.data = SenCanBuff;                           
                BSP_DCANWrite(CANWriteConfig);   
                OSTimeDlyHMSM(0,0,0,1);    
            }
            break;
        case UIACANAUTHENTICLEVEL1:
			UIA_Send_MC_AuthenticSwitch_Param1(CanData,UIData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);    
			UIA_Send_MC_AuthenticSwitch_Param2(CanData,UIData);
			memcpy(SenCanBuff,CanData,8);
			CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);     
            break;    
        case UIACANCLEARLEVEL:
			UIA_Send_MC_ClearLevel_Param(CanData,UIData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);    
            break;
        case UIACANMIXEDSEARCH:            
            UIA_Send_MC_Mixed_Shark(CanData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);     
            break;
        case UIACANOUTLETSEARCH:
            UIA_Send_OutLet_Search(CanData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);    
            break;
        case UIACANINLERSSEARCH:
            UIA_Send_InLets_Search(CanData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);    
            break;
        case UIACANJCSPEED:
            UIA_Send_JCSPEEDEDIT_Search(CanData,UIData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);    
            break;
        case UIACANLEVELUPSTART:
            UIA_Send_MC_LevelUp_Start(CanData);
            memcpy(SenCanBuff,CanData,8);
            CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
            CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
            CANWriteConfig.entry.dlc = 8;
            CANWriteConfig.entry.data = SenCanBuff;                           
            BSP_DCANWrite(CANWriteConfig);
            OSTimeDlyHMSM(0,0,0,1);    
            break;
    }
}

/***********************************************************************************************
* Function		: DealwithMCLevelUp
* Description	: 马达ARM升级
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/
void DealwithMCLevelUp(INT8U *MTLevelUP,INT32U *MTLevelUPSize,_BSPDCAN_CONFIG CANWriteConfig)
{
    INT8U CanData[8];
    INT32U SenCanBuff[2] = {0x00};  
    INT32U MTLevelUPPages = 0;
    INT32U i = 0;
    
    if((*MTLevelUPSize)%8)
    {
        MTLevelUPPages = (*MTLevelUPSize)/8 + 1;
    }
    else
    {
        MTLevelUPPages = (*MTLevelUPSize)/8;
    }
    
    UIA_Send_MC_LevelUp(CanData,&MTLevelUPPages);
    memcpy(SenCanBuff,CanData,8);
    CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
    CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
    CANWriteConfig.entry.dlc = 8;
    CANWriteConfig.entry.data = SenCanBuff;                     
    BSP_DCANWrite(CANWriteConfig);
    NOP();  

    for(i = 0;i<MTLevelUPPages;i++)
    {
//        memcpy(SenCanBuff,(MTLevelUP+i*8),8);
//        CANWriteConfig.entry.data = SenCanBuff;
//        BSP_DCANWrite(CANWriteConfig);
//        OSTimeDlyHMSM(0,0,0,1);
    } 
}

/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/