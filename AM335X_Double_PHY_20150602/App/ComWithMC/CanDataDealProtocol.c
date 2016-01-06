/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		    : mmcsdmain.c
* Author		    : 
* Date First Issued	: 130722   
* Version		    : V应用程序
* Description		: FLASH读写
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
#include "canprotocoltable.h"

//接收到的马达板数据
#define CANIN 0x05
//发送给马达板的数据
#define CANOUT 0x06

//UIA接收CAN数据的回复
#define UIACANRECEVOK   0x00
#define UIACANRECEVFAIL   0x01
#define UIACANMUSTREPLY 0x55

//协议保留帧
#define DEFAULTBYTE 0xFF

//Can发送的后续还有多少帧
INT8U CanTotal = 0xFF;//初始化为0xFF是因为当CanTotal为0时开始做处理
//主菜单的帧保存用作下一次对比，当帧有改变时候才发送给UI_ARM
INT8U MianMenuBuff[24] = {0x00};

INT8U CanDataRec[8] = {0x00};
INT8U CanTotalRxBuff[1024] = {0x00};
/***********************************************************************************************
* Function		: CanGetCanFrameInformation
* Description	: 获取接收到帧的信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 15/1/2015	songchao
***********************************************************************************************/
void CanGetCanFrameInformation(INT8U *pdata)
{    
    CanFrameInformation.DataID = (*(pdata+1)|((*pdata)<<8));//ID号
}

/***********************************************************************************************
* Function		: SearchParaManagementTable 
* Description	        : 搜索表格中数据项位置，返回位置值
* Input			: DataID---搜索的ID号
*                        flag-----搜索参数表==0
*                           -----搜索数据表>0
* Output		: 0xff-----表中无搜索数据项存在 
* Note(s)		: 
* Contributor	: 15/1/2015	songchao
***********************************************************************************************/
INT8U CanSearchParaManagementTable(INT16U DataId,INT8U flag)
{
	INT8U count;
	INT8U allnum=0;
	const _PROTOCOL_CAN_MANAGE_TABLE *p;

    allnum = CANPARATABLELEN;	//参数个数
    
    p = Can_Para_Manage_Table;
    
	for(count=0;count<allnum;count++)
	{
	 	if(DataId == p[count].DataID)
		{
			  return count;
		}
	}
	return 0xff;
}

/***********************************************************************************************
* Function		: CanProtocolATOperation
* Description	: 按数据ID,对接收和发送CAN数据
* Input			: controlstrcut--入参传递控制结构体
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT16U CanProtocolATOperation(InterfaceStruct *terminal_para)
{
    _BSP_MESSAGE Can_send_UI_message;  
  
	if(NULL == terminal_para)
        DBG_DIS("err info:terminal_para is NULL\r\n");
	switch(terminal_para->RWChoose)
	{
		case CANIN:
            Can_send_UI_message.MsgID = APP_DISP_COMFROM_MC;
            Can_send_UI_message.DataLen = 1024;
            Can_send_UI_message.pData = terminal_para->ptr;
            OSQPost (DispTaskEvent,&Can_send_UI_message); 
            OSTimeDlyHMSM(0,0,0,5); 
            if(terminal_para->ptr[6] == UIACANMUSTREPLY)
            {
                Reply_CAN_Frame(terminal_para->ptr);
            }
            return TRUE;
		case CANOUT:
            //组发送协议帧
            return TRUE;
		default:
            break;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: CanProtocolATOperation
* Description	: 按数据ID,对接收和发送CAN数据
* Input			: controlstrcut--入参传递控制结构体
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT16U CanProtocolMianMenuATOperation(InterfaceStruct *terminal_para)
{
    _BSP_MESSAGE Can_send_UI_message;  
    INT8U SetMainMenuMessgaeFlag = TRUE;
    
	if(NULL == terminal_para)
        DBG_DIS("err info:terminal_para is NULL\r\n");

    
    SetMainMenuMessgaeFlag = memcmp(MianMenuBuff,terminal_para->ptr,24);
    
    if(!SetMainMenuMessgaeFlag)
    {
        //DoNothing
    } 
    else
    {
        memcpy(MianMenuBuff,terminal_para->ptr,24);
        Can_send_UI_message.MsgID = APP_DISP_COMFROM_MC;
        Can_send_UI_message.DataLen = 1024;
        Can_send_UI_message.pData = terminal_para->ptr;
        OSQPost (DispTaskEvent,&Can_send_UI_message); 
        OSTimeDlyHMSM(0,0,0,5); 
    }
    
	return TRUE;
}

/***********************************************************************************************
* Function		: ParaAppfunction
* Description	: 供flash任务包外的task
* Input			: terminal_para->RWChoose//读写功能
				  terminal_para->DataID  //数据ID
				  terminal_para->len     //数据长度
				  terminal_para->offset  //在表格中的偏移量
				  terminal_para->*ptr    //具体数据
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT16U CanParaAppfunction(InterfaceStruct *terminal_para)
{
	if(terminal_para->DataID != 0xFFFF)
	{
		terminal_para->offset=CanSearchParaManagementTable(terminal_para->DataID,0);
		if(terminal_para->offset==0xff)
			return FALSE;
	}
	return CanProtocolATOperation(terminal_para);
}

/***********************************************************************************************
* Function		: CombinedCanFrame
* Description	: 组合Can的帧
* Input			:   DisplayTxBuff       接收到的按键回传的信息   
                    Len                 数据长度
* Output		: 
                     CHECK1Buff      转换出来的LCD格式转换的字符串
* Note(s)		: 
* Contributor	: 16/07/2014	songchao 
***********************************************************************************************/
void CombinedCanFrame()
{
    if((CanDataRec[7] == 0)&&(CanDataRec[1] == 0x09)&&(CanDataRec[0] == 0x10))
    {
         CanTotal= 5;
         memcpy((CanTotalRxBuff+CanDataRec[7]*8),CanDataRec,8);
    }  
    if((CanDataRec[7] == 0)&&(CanDataRec[0] == 0x10))
    {
         CanTotal= CanDataRec[2]+1;
         memcpy((CanTotalRxBuff+CanDataRec[7]*8),CanDataRec,8);
    }    
    if((CanDataRec[1] == 0x14)&&(CanDataRec[0] == 0x10))
    {
         CanTotal= 1;
         memcpy((CanTotalRxBuff+CanDataRec[7]*8),CanDataRec,8);
    }   
    if(CanTotal<0xE0)
    {
          CanTotal--;
          memcpy((CanTotalRxBuff+CanDataRec[7]*8),CanDataRec,8);
    }
}

/***********************************************************************************************
* Function		: CombinedCanFrame
* Description	: 组合Can的帧
* Input			:   DisplayTxBuff       接收到的按键回传的信息   
                    Len                 数据长度
* Output		: 
                     CHECK1Buff      转换出来的LCD格式转换的字符串
* Note(s)		: 
* Contributor	: 16/07/2014	songchao 
***********************************************************************************************/
void McDataControl(INT8U *CanData)
{
    InterfaceStruct  parastruct;
    INT8U tablepos = 0;
    
    memcpy(CanDataRec,CanData,8);
    CombinedCanFrame();
    
    if(CanTotal == 0)
    {
        CanGetCanFrameInformation(CanTotalRxBuff);
        tablepos = CanSearchParaManagementTable(CanFrameInformation.DataID,0);
        if(tablepos < CANPARATABLELEN)
        {
            parastruct.RWChoose = CANIN;
            parastruct.DataID = CanFrameInformation.DataID;
            parastruct.offset=tablepos;
            parastruct.ptr = CanTotalRxBuff;
            if(Can_Para_Manage_Table[tablepos].DealwithDataIDFunction(&parastruct))
            {
            //处理成功    
            }  
        }

    }
}

/***********************************************************************************************
* Function		: Reply_CAN
* Description	: CAN的回复帧
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 7/11/2014	songchao
***********************************************************************************************/
void Reply_CAN_Frame(INT8U *CanData)
{
    static _BSPDCAN_CONFIG CANWriteConfig;
    INT32U CanReply[2] = {0x00};
    
    memcpy(CanReply,CanData,8);  
    
    if(CanData[7])
    {
        CanData[2] = UIACANRECEVOK;
        CanData[3] = DEFAULTBYTE;
        CanData[4] = DEFAULTBYTE;
        CanData[5] = DEFAULTBYTE;
        CanData[6] = DEFAULTBYTE;
        
        memcpy(CanReply,CanData,8); 
        CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
        CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
        CANWriteConfig.entry.dlc = 8;
        CANWriteConfig.entry.data = CanReply;                           
        BSP_DCANWrite(CANWriteConfig);

    }
    else
    {
        CanData[2] = UIACANRECEVOK; 
        CanData[3] = DEFAULTBYTE;
        CanData[4] = DEFAULTBYTE;
        CanData[5] = DEFAULTBYTE;
        CanData[6] = CanData[7];
        CanData[7] = DEFAULTBYTE;
		
    	memcpy(CanReply,CanData,8);    
        CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// 标准数据帧方向为发送
        CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//标准帧
        CANWriteConfig.entry.dlc = 8;
        CANWriteConfig.entry.data = CanReply;                           
        BSP_DCANWrite(CANWriteConfig);
    }
}


/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/