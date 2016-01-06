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
#include "flashinterface.h"
#include "save_para.h"
#define  PARA_TABLE_C
    #include "flashtable.h"
#include "flashmain.h"
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
INT8U SearchParaManagementTable(INT16U DataId,INT8U flag)
{
	INT8U count;
	INT8U allnum=0;
	const _PROTOCOL_MANAGE_TABLE *p;

    allnum = PARATABLELEN;	//��������

    p=Para_Manage_Table;

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
* Function		: ProtocolATOperation
* Description	: ������ID,�Բ����Ĵ�Ͷ�flash����ͳһ�ӿ�
* Input			: controlstrcut--��δ��ݿ��ƽṹ��
* Output		:
* Note(s)		:
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT16U ProtocolATOperation(InterfaceStruct *terminal_para)
{
	if(NULL == terminal_para)
	{
        DBG_DIS("err info:terminal_para is NULL\r\n");
        return FALSE;
	}

	switch(terminal_para->RWChoose)
	{
		case   WRITE_ORDER:
			/*����д���������ݽ�����Ӧ��λ����*/
			if(BSP_WriteDataToAT(Para_Manage_Table[terminal_para->offset].DataSaveAddr,\
								 terminal_para->ptr,\
								 terminal_para->len))
				return  terminal_para->len;
			else
				return FALSE;
		case   READ_ORDER:
		    if(BSP_ReadDataFromAT(Para_Manage_Table[terminal_para->offset].DataSaveAddr,\
							      terminal_para->ptr,\
								  Para_Manage_Table[terminal_para->offset].DataLen))
				return  terminal_para->len;
			else
				return FALSE;
		default:
            break;
	}
	return TRUE;
}
/***********************************************************************************************
* Function	: DealWithUIDATA
* Description	:�Ѵ�UI�յ���DATA�浽FLASH��
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 150114	�γ�
***********************************************************************************************/
void DealWithUIDATA(INT8U Port,INT8U *pData,INT16U Len)
{
    INT8U tablepos = 0;
   InterfaceStruct  parastruct;

    GetFrameInformation(pData);
    tablepos = SearchParaManagementTable(FrameInformation.DataID,0);
    if(tablepos!=0xFF)
    {
        parastruct.RWChoose = WRITE_ORDER;//дFlash
        parastruct.DataID = FrameInformation.DataID;
        parastruct.len = Para_Manage_Table[tablepos].DataLen;
        parastruct.offset=tablepos;
        parastruct.ptr = FrameInformation.pdata;

        if(Para_Manage_Table[tablepos].DealwithDataIDFunction(&parastruct))
        {
        //�洢�ɹ�

        }

    }

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
INT16U ParaAppfunction(InterfaceStruct *terminal_para)
{
	if(terminal_para->DataID != 0xFFFF)
	{
		terminal_para->offset=SearchParaManagementTable(terminal_para->DataID,0);
		if(terminal_para->offset==0xff)
			return FALSE;
	}
	return ProtocolATOperation(terminal_para);
}



/****************************************************************************************************
**����:int flash_para_opt_read(uint16 index_id,uint8 *buf)
**����:ͨ��id,������
* ���:��
* ����:�ɹ�����1,������0
**auth:hxj, date: 2015-1-30 16:39
*****************************************************************************************************/
int flash_para_opt_read(uint16 index_id,uint8 *buf)
{
    InterfaceStruct  parastruct;
    INT16U ret=0;

    if(NULL == buf) return 0;


    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = index_id;
    parastruct.ptr = buf;

    ret=ParaAppfunction(&parastruct);

    if(TRUE==ret)
    {
        return 1;
    }

    return 0;

}






/****************************************************************************************************
**����:int flash_para_opt_write(uint16 index_id,uint8 *buf)
**����:ͨ��id,д����
* ���:��
* ����:�ɹ�����1,������0
**auth:hxj, date: 2015-1-30 16:39
*****************************************************************************************************/
int flash_para_opt_write(uint16 index_id,uint8 *buf)
{
    InterfaceStruct  parastruct;
    INT16U ret=0;

    if(NULL == buf) return 0;

    parastruct.RWChoose = WRITE_ORDER;//дFlash
    parastruct.DataID = index_id;
    parastruct.ptr = buf;

    ret=ParaAppfunction(&parastruct);
    if(TRUE==ret)
    {
        return 1;
    }

    return 0;

}












/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
