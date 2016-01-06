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
#include "flashinterface.h"
#include "save_para.h"
#define  PARA_TABLE_C
    #include "flashtable.h"
#include "flashmain.h"
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
INT8U SearchParaManagementTable(INT16U DataId,INT8U flag)
{
	INT8U count;
	INT8U allnum=0;
	const _PROTOCOL_MANAGE_TABLE *p;

    allnum = PARATABLELEN;	//参数个数

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
* Description	: 按数据ID,对参数的存和读flash函数统一接口
* Input			: controlstrcut--入参传递控制结构体
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
			/*根据写下来的数据进行相应复位操作*/
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
* Description	:把从UI收到的DATA存到FLASH中
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 150114	宋超
***********************************************************************************************/
void DealWithUIDATA(INT8U Port,INT8U *pData,INT16U Len)
{
    INT8U tablepos = 0;
   InterfaceStruct  parastruct;

    GetFrameInformation(pData);
    tablepos = SearchParaManagementTable(FrameInformation.DataID,0);
    if(tablepos!=0xFF)
    {
        parastruct.RWChoose = WRITE_ORDER;//写Flash
        parastruct.DataID = FrameInformation.DataID;
        parastruct.len = Para_Manage_Table[tablepos].DataLen;
        parastruct.offset=tablepos;
        parastruct.ptr = FrameInformation.pdata;

        if(Para_Manage_Table[tablepos].DealwithDataIDFunction(&parastruct))
        {
        //存储成功

        }

    }

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
**名称:int flash_para_opt_read(uint16 index_id,uint8 *buf)
**功能:通过id,读参数
* 入口:无
* 出口:成功返回1,出错返回0
**auth:hxj, date: 2015-1-30 16:39
*****************************************************************************************************/
int flash_para_opt_read(uint16 index_id,uint8 *buf)
{
    InterfaceStruct  parastruct;
    INT16U ret=0;

    if(NULL == buf) return 0;


    parastruct.RWChoose = READ_ORDER;//读Flash
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
**名称:int flash_para_opt_write(uint16 index_id,uint8 *buf)
**功能:通过id,写参数
* 入口:无
* 出口:成功返回1,出错返回0
**auth:hxj, date: 2015-1-30 16:39
*****************************************************************************************************/
int flash_para_opt_write(uint16 index_id,uint8 *buf)
{
    InterfaceStruct  parastruct;
    INT16U ret=0;

    if(NULL == buf) return 0;

    parastruct.RWChoose = WRITE_ORDER;//写Flash
    parastruct.DataID = index_id;
    parastruct.ptr = buf;

    ret=ParaAppfunction(&parastruct);
    if(TRUE==ret)
    {
        return 1;
    }

    return 0;

}












/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
