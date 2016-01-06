/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: flashtable.h
* Author			: ��ҫ
* Date First Issued	: 26/01/2015
* Version			: V
* Description		:
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2013		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__FLASH_TABLE_H_
#define	__FLASH_TABLE_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "flashinterface.h"
#include "fLashaddrsconfig.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
	INT16U DataID;       // ���ݱ�ʶ
	INT32U DataSaveAddr; // �������豸�ݵ��ϴγ������ݴ��ƫ�Ƶ�ַ
	INT32U DataLen;      // ���ݳ���
	INT16U (*DealwithDataIDFunction)(InterfaceStruct *controlstrcut);  //id��Ӧ�Ĳ���
	INT8U  (*Updatafunction)(void);//ִ�������õȲ�������Ҫ���µĲ���ִ�к���,һЩС�Ĳ����ĸ��²���������
}_PROTOCOL_MANAGE_TABLE;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
#ifdef   PARA_TABLE_C
_PROTOCOL_MANAGE_TABLE  Para_Manage_Table[PARATABLELEN]=
{	//ID    //flash����λ��     //flash��������     //ִ�к���
    {PARA_1001_ID_SAVE_BAT_VAL,PARA_1001_FLASHADDR,PARA_1001_FLASHLEN,ProtocolATOperation,NULL},//������ֵ

    {PARA_1002_ID_FAKE_LEVEL_DATA,PARA_1002_FLASHADDR,PARA_1002_FLASHLEN,ProtocolATOperation,NULL},//��ֵȼ�����

    {PARA_1003_ID_FZ_LEVEL_DATA,PARA_1003_FLASHADDR,PARA_1003_FLASHLEN,ProtocolATOperation,NULL},//��ȡ��α�ȼ�����

    {PARA_1004_ID_LOCAL_IP,PARA_1004_FLASHADDR,PARA_1004_FLASHLEN,ProtocolATOperation,NULL},//IP��ַ

    {PARA_1005_ID_LOCAL_MK,PARA_1005_FLASHADDR,PARA_1005_FLASHLEN,ProtocolATOperation,NULL},//mask

    {PARA_1006_ID_LOCAL_GW,PARA_1006_FLASHADDR,PARA_1006_FLASHLEN,ProtocolATOperation,NULL},//GW

    {PARA_1007_ID_LOCAL_MAC,PARA_1007_FLASHADDR,PARA_1007_FLASHLEN,ProtocolATOperation,NULL},//MAC

    {PARA_1008_ID_LOCAL_PORT,PARA_1008_FLASHADDR,PARA_1008_FLASHLEN,ProtocolATOperation,NULL},//���ض˿�

    {PARA_1009_ID_SERVER_IP,PARA_1009_FLASHADDR,PARA_1009_FLASHLEN,ProtocolATOperation,NULL},//Զ��IP

    {PARA_100A_ID_SERVER_PORT,PARA_100A_FLASHADDR,PARA_100A_FLASHLEN,ProtocolATOperation,NULL},//Զ�˶˿�

    {PARA_100B_ID_NET_SWITCH,PARA_100B_FLASHADDR,PARA_100B_FLASHLEN,ProtocolATOperation,NULL},//��������

    {PARA_100C_ID_NET_PROTOCOL_TYPE,PARA_100C_FLASHADDR,PARA_100C_FLASHLEN,ProtocolATOperation,NULL},//��������

    {PARA_100D_ID_NET_PROTOCOL_MODE,PARA_100D_FLASHADDR,PARA_100D_FLASHLEN,ProtocolATOperation,NULL},//����ģʽ

    {PARA_100E_ID_NET_STOP_TIME,PARA_100E_FLASHADDR,PARA_100E_FLASHLEN,ProtocolATOperation,NULL},//������ʱ��

    {PARA_100F_ID_SD_REMAIN_SIZE_THR,PARA_100F_FLASHADDR,PARA_100F_FLASHLEN,ProtocolATOperation,NULL},//SD��ʣ�������澯ֵ

    {PARA_1010_ID_SD_ALARM_EN,PARA_1010_FLASHADDR,PARA_1010_FLASHLEN,ProtocolATOperation,NULL},//SD��ʣ�������澯ʹ��

    {PARA_1011_ID_SD_ALARM_TIME,PARA_1011_FLASHADDR,PARA_1011_FLASHLEN,ProtocolATOperation,NULL},//SD��ʣ�������澯����

    {PARA_1012_ID_SD_SEND_OK_DAY,PARA_1012_FLASHADDR,PARA_1012_FLASHLEN,ProtocolATOperation,NULL},//SD�����ͳɹ����ļ���������

    {PARA_1013_ID_SD_SEND_ERR_DAY,PARA_1013_FLASHADDR,PARA_1013_FLASHLEN,ProtocolATOperation,NULL},//SD������ʧ�ܵ��ļ���������
    
    {PARA_1014_ID_AUTHENTICATIO_PARA,PARA_1014_FLASHADDR,PARA_1014_FLASHLEN,ProtocolATOperation,NULL},//���α����
    
    {PARA_1015_ID_BLACK_LIST,PARA_1015_FLASHADDR,PARA_1015_FLASHLEN,ProtocolATOperation,NULL},//�������洢
};
#endif 	//PARA_TABLE_C
extern _PROTOCOL_MANAGE_TABLE  Para_Manage_Table[PARATABLELEN];
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

#endif	//__FLASH_TABLE_H_
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/

