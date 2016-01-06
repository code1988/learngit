/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: Canprotocol.h
* Author			: 宋超
* Date First Issued	: 30/01/2015
* Version			: V
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "APPSTRUCT.h"
#include "canIDlen.h"
#include "CanDataDealProtocol.h"
/* Private define-----------------------------------------------------------------------------*/
#define CANPARATABLELEN 21
/* Private typedef----------------------------------------------------------------------------*/
struct CanFramePROStruct
{		 
    INT16U DataID;
    INT16U FrameLength;		 
    INT8U  *pdata;	        
};

typedef struct
{
	INT16U DataID;       // 数据标识
	INT16U (*DealwithDataIDFunction)(InterfaceStruct *controlstrcut);  //id对应的操作
	INT8U  (*Updatafunction)(void);//执行完设置等操作后，需要更新的参数执行函数,一些小的参数的更新操作放在这
}_PROTOCOL_CAN_MANAGE_TABLE;

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

_PROTOCOL_CAN_MANAGE_TABLE  Can_Para_Manage_Table[CANPARATABLELEN] =
{
	//ID    //接收到以后处理  //发送给MC的处理
    {0x1001,CanProtocolATOperation,NULL},//点钞模式信息
    
    {0x1002,CanProtocolATOperation,NULL},//批量值
    
    {0x1003,CanProtocolATOperation,NULL},//马达开停机
    
    {0x1004,CanProtocolATOperation,NULL},//鉴伪参数
    
    {0x1005,CanProtocolATOperation,NULL},//鉴伪等级开关第一组
    
    {0x1006,CanProtocolATOperation,NULL},//鉴伪等级开关第二组 
    
    {0x1007,CanProtocolATOperation,NULL},//面额清分等级   
    
    {0x1008,CanProtocolATOperation,NULL},//混点明细  
    
    {0x1009,CanProtocolATOperation,NULL},//退钞口冠字号码查看 
    
    {0x100A,CanProtocolATOperation,NULL},//接钞口冠字号码查看  
    
    {0x100B,CanProtocolMianMenuATOperation,NULL},//跑钞信息  
    
    {0x100C,CanProtocolATOperation,NULL},//出错信息
    
    {0x100D,CanProtocolATOperation,NULL},//版本信息 
    
    {0x100E,CanProtocolATOperation,NULL},//电机调试
        
    {0x100F,CanProtocolATOperation,NULL},//进钞接钞AD调试 
            
    {0x1010,CanProtocolATOperation,NULL},//红外定位管AD
    
    {0x1011,CanProtocolATOperation,NULL},//大小磁头AD
    
    {0x1012,CanProtocolATOperation,NULL},//荧光AD
 
    {0x1013,CanProtocolATOperation,NULL},//走钞
    
    {0x1014,CanProtocolATOperation,NULL},//MT数据
    
    {0x10F0,CanProtocolATOperation,NULL},//MT ARM升级信息
};
extern _PROTOCOL_CAN_MANAGE_TABLE  Can_Para_Manage_Table[CANPARATABLELEN];

struct CanFramePROStruct CanFrameInformation;

extern struct CanFramePROStruct CanFrameInformation;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

