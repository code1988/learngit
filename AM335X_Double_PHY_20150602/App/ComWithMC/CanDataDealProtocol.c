/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName		    : mmcsdmain.c
* Author		    : 
* Date First Issued	: 130722   
* Version		    : VӦ�ó���
* Description		: FLASH��д
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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

//���յ�����������
#define CANIN 0x05
//���͸����������
#define CANOUT 0x06

//UIA����CAN���ݵĻظ�
#define UIACANRECEVOK   0x00
#define UIACANRECEVFAIL   0x01
#define UIACANMUSTREPLY 0x55

//Э�鱣��֡
#define DEFAULTBYTE 0xFF

//Can���͵ĺ������ж���֡
INT8U CanTotal = 0xFF;//��ʼ��Ϊ0xFF����Ϊ��CanTotalΪ0ʱ��ʼ������
//���˵���֡����������һ�ζԱȣ���֡�иı�ʱ��ŷ��͸�UI_ARM
INT8U MianMenuBuff[24] = {0x00};

INT8U CanDataRec[8] = {0x00};
INT8U CanTotalRxBuff[1024] = {0x00};
/***********************************************************************************************
* Function		: CanGetCanFrameInformation
* Description	: ��ȡ���յ�֡����Ϣ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 15/1/2015	songchao
***********************************************************************************************/
void CanGetCanFrameInformation(INT8U *pdata)
{    
    CanFrameInformation.DataID = (*(pdata+1)|((*pdata)<<8));//ID��
}

/***********************************************************************************************
* Function		: SearchParaManagementTable 
* Description	        : ���������������λ�ã�����λ��ֵ
* Input			: DataID---������ID��
*                        flag-----����������==0
*                           -----�������ݱ�>0
* Output		: 0xff-----������������������� 
* Note(s)		: 
* Contributor	: 15/1/2015	songchao
***********************************************************************************************/
INT8U CanSearchParaManagementTable(INT16U DataId,INT8U flag)
{
	INT8U count;
	INT8U allnum=0;
	const _PROTOCOL_CAN_MANAGE_TABLE *p;

    allnum = CANPARATABLELEN;	//��������
    
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
* Description	: ������ID,�Խ��պͷ���CAN����
* Input			: controlstrcut--��δ��ݿ��ƽṹ��
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
            //�鷢��Э��֡
            return TRUE;
		default:
            break;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: CanProtocolATOperation
* Description	: ������ID,�Խ��պͷ���CAN����
* Input			: controlstrcut--��δ��ݿ��ƽṹ��
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
* Description	: ��flash��������task
* Input			: terminal_para->RWChoose//��д����
				  terminal_para->DataID  //����ID
				  terminal_para->len     //���ݳ���
				  terminal_para->offset  //�ڱ���е�ƫ����
				  terminal_para->*ptr    //��������
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
* Description	: ���Can��֡
* Input			:   DisplayTxBuff       ���յ��İ����ش�����Ϣ   
                    Len                 ���ݳ���
* Output		: 
                     CHECK1Buff      ת��������LCD��ʽת�����ַ���
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
* Description	: ���Can��֡
* Input			:   DisplayTxBuff       ���յ��İ����ش�����Ϣ   
                    Len                 ���ݳ���
* Output		: 
                     CHECK1Buff      ת��������LCD��ʽת�����ַ���
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
            //����ɹ�    
            }  
        }

    }
}

/***********************************************************************************************
* Function		: Reply_CAN
* Description	: CAN�Ļظ�֡
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
        CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// ��׼����֡����Ϊ����
        CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//��׼֡
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
        CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// ��׼����֡����Ϊ����
        CANWriteConfig.entry.id = BSP_CANTX_MSG_STD_ID;//��׼֡
        CANWriteConfig.entry.dlc = 8;
        CANWriteConfig.entry.data = CanReply;                           
        BSP_DCANWrite(CANWriteConfig);
    }
}


/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/