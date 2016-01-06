/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: Canprotocol.h
* Author			: �γ�
* Date First Issued	: 30/01/2015
* Version			: V
* Description		:
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
	INT16U DataID;       // ���ݱ�ʶ
	INT16U (*DealwithDataIDFunction)(InterfaceStruct *controlstrcut);  //id��Ӧ�Ĳ���
	INT8U  (*Updatafunction)(void);//ִ�������õȲ�������Ҫ���µĲ���ִ�к���,һЩС�Ĳ����ĸ��²���������
}_PROTOCOL_CAN_MANAGE_TABLE;

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

_PROTOCOL_CAN_MANAGE_TABLE  Can_Para_Manage_Table[CANPARATABLELEN] =
{
	//ID    //���յ��Ժ���  //���͸�MC�Ĵ���
    {0x1001,CanProtocolATOperation,NULL},//�㳮ģʽ��Ϣ
    
    {0x1002,CanProtocolATOperation,NULL},//����ֵ
    
    {0x1003,CanProtocolATOperation,NULL},//��￪ͣ��
    
    {0x1004,CanProtocolATOperation,NULL},//��α����
    
    {0x1005,CanProtocolATOperation,NULL},//��α�ȼ����ص�һ��
    
    {0x1006,CanProtocolATOperation,NULL},//��α�ȼ����صڶ��� 
    
    {0x1007,CanProtocolATOperation,NULL},//�����ֵȼ�   
    
    {0x1008,CanProtocolATOperation,NULL},//�����ϸ  
    
    {0x1009,CanProtocolATOperation,NULL},//�˳��ڹ��ֺ���鿴 
    
    {0x100A,CanProtocolATOperation,NULL},//�ӳ��ڹ��ֺ���鿴  
    
    {0x100B,CanProtocolMianMenuATOperation,NULL},//�ܳ���Ϣ  
    
    {0x100C,CanProtocolATOperation,NULL},//������Ϣ
    
    {0x100D,CanProtocolATOperation,NULL},//�汾��Ϣ 
    
    {0x100E,CanProtocolATOperation,NULL},//�������
        
    {0x100F,CanProtocolATOperation,NULL},//�����ӳ�AD���� 
            
    {0x1010,CanProtocolATOperation,NULL},//���ⶨλ��AD
    
    {0x1011,CanProtocolATOperation,NULL},//��С��ͷAD
    
    {0x1012,CanProtocolATOperation,NULL},//ӫ��AD
 
    {0x1013,CanProtocolATOperation,NULL},//�߳�
    
    {0x1014,CanProtocolATOperation,NULL},//MT����
    
    {0x10F0,CanProtocolATOperation,NULL},//MT ARM������Ϣ
};
extern _PROTOCOL_CAN_MANAGE_TABLE  Can_Para_Manage_Table[CANPARATABLELEN];

struct CanFramePROStruct CanFrameInformation;

extern struct CanFramePROStruct CanFrameInformation;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/

