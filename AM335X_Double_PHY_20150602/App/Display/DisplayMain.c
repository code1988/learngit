/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: main.c
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
#include "DisplayMain.h"
#include "BaseDisp.h"
#include "DwinPotocol.h"
#include "RecevCanPotocol.h"
#include "CoodinateInit.h"
#include <string.h>
#include "sysconfig.h"
#include "main.h"
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>
#include  <ucos_ii.h>
#include "bsp.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "gpio.h"
#include "gpio_v2.h"
#include "Dispkey.h"
#include "appstruct.h"
#include "flashinterface.h"
#include "DispkeyFunction.h"
#include "DispDSP.h"

//#include "Bsp_rtc.h"

/* Private define-----------------------------------------------------------------------------*/
#define I2C_SLAVE_ADDR_READ                 (0x41)
#define SENDTODISPLEN (SendToDisp[2]+3)
#define CLEAR 0x5A
#define MATRIX 0x59
#define __UART2_ENABLE
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
void *DisplayOSQ[16];					                 // ��Ϣ����
OS_EVENT *DispTaskEvent;				                // ʹ�õ��¼�

OS_EVENT *KeyTaskEvent;				                // ʹ�õ��¼�
void *KeyOSQ[4];					                 // ��Ϣ����
//��ʼ�����ڲ鿴�˵���ҳ��Ϊ��һҳ
INT8U ERRORDETAILPAGE = 0x01;
/* Private variables--------------------------------------------------------------------------*/
//��ʼ�����պͷ��͵��ַ���
INT8U DisplayTxBuff[DISPBUFF_LENGTH] = {0};
INT8U DisplayRxBuff[DISPBUFF_LENGTH] = {0};
INT8U display_recv_info[DISPBUFF_LENGTH] = {0};


INT8U DisplayRxKEYBuff[DISPBUFF_LENGTH] = {0};
INT8U CanRxBuff[1024] = {0};
INT8U CanTxBuff[1024] = {0};

INT8U SendToDisp[100] = {0};

//���˵���ʾ
INT8U MainMenuDisp[13] = {0};
//�����ϸ��ѯ
INT8U MixedDisp[18] = {0};
//�˳��ڹ��ֺŲ�ѯ
INT8U OUTLETDisp[200][24] = {0};
//�ӳ��ڹ��ֺŲ�ѯ
INT8U INLETDisp[300][24] = {0};
//�汾��Ϣ
INT8U EditionDisp[7] = {0};
//��������Ϣ
INT8U JcSpeedDisp[6] = {0};
//���������汾��Ϣ
INT8U InLetADDisp[15] = {0};
//����AD��Ϣ
INT8U IRADDisp[30] = {0};
//��С��ͷAD��Ϣ
INT8U MTDisp[30] = {0};
//ӫ���������
INT8U UVDisp[30] = {0};
//�߳�����
INT8U InfoIRDisp[50] = {0};
//�߳�MT
INT8U InfoMTDisp[50] = {0};
//�߳�MG
INT8U InfoMGDisp[6][50] = {0};
//MT������ʾ
INT8U MTwaveform[50] = {0};
//��α�����趨
INT8U AuthenticationDataSettings[3][90] = {0x00};
//��α�ȼ��趨
INT8U AuthenticationLevel[7] = {0x01,0x02,0x03,0x04,0x05,0x04,0x03};
//��α���ܿ���
INT8U AuthenticationSwitch[14] = {0};
//��ֵȼ��趨
INT8U ClearLevel[6] = {0x01,0x02,0x03,0x04,0x05,0x05};
//���ģʽ
INT8U CLEARINGMODE = INTELLIGENTMODE;
//��ȡ��ģʽ
INT8U DEPWITHDRAWMODE = DEPOSITMODE;
//�ۼ�ģʽ
INT8U SUMMATIONMODE = SUMMATIONOFF;
//���翪��
INT8U NETFLAG = NETUNABL;
//����ֵ
INT16U BatchNum = 0;
//��￪ͣ����־
INT8U MTStatus = INMAINMENU;
//�˵���־
INT8U MainMenuStatus = MTENABLE;
//�����Ƿ���Ч
INT8U KeyStatus = KEYENABLE;
//�����ʾ
INT8U ErrorFlag = TRUE;
//CSУ���ʾ
INT8U CSCheckFlag = FALSE;
//�����ʾ
INT8U BlackList[200][12] = {0};


extern INT8U MainMenuNumCoordinate[3][10][8];
extern INT8U InletTitle[2][8];
extern INT8U ModeIcon[9][8];
extern INT8U NetIcon[2][8];
extern INT8U OutLetCoordinate[4][3][4];
extern INT8U TotalPage[4][4];
extern INT8U TotalSum[7][4];
extern INT8U TotalSumCoordinate[4];
extern INT8U NetCoordinate[4];
extern INT8U MainMenuModeCoordinate[2][4];
extern INT8U Paramhorizontal;
extern INT8U Paramvertical;

//�������
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
extern tRectangle BaseBatchParam[3];
extern tRectangle BaseBatchParamColor;

extern INT8U AuthenticationDataSettings[3][90];

//������������ֵȼ��Լ���α�ȼ�����
extern INT16U TemBatchNum;
extern INT8U TemClearLevel[6];
extern INT8U TemAuthenticationLevel[7];
extern INT32U numOfBytes;
//�û���Ϣ�鿴����
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
extern INT8U Mask[13];
extern INT8U GateWay[13];
extern INT8U LocalPort[5];
extern INT8U DiskPort[4];
extern INT8U DiskIP[13];
//������
extern INT8U BlackLists[2][7][13];
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: DisplayParaInit
* Description	: �ϵ��ʼ����ȡflash�����������ʾ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 11/7/2014	songchao
***********************************************************************************************/
void DisplayParaInit(void)
{
    InterfaceStruct  parastruct;
    INT8U Get_Flash_IP[4] = {0};
    INT8U Get_Flash_MASK[4] = {0};
    INT8U Get_Flash_GateWay[4] = {0};
    INT8U Get_Flash_Mac[6] = {0x12,0x55,0xAA,0xBB,0xCC,0x66};
    INT8U Get_Flash_LocalPort[2] = {0};
    INT8U Get_Flash_DiskIP[4] = {0};
    INT8U Get_Flash_DiskPort[2] = {0};
    
    //��ֵȼ�
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1002;
    parastruct.ptr = ClearLevel; 
    ParaAppfunction(&parastruct);
    memcpy(TemClearLevel,ClearLevel,6);
    
    //��α�ȼ�
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1003;
    parastruct.ptr = AuthenticationLevel; 
    ParaAppfunction(&parastruct);  
    memcpy(TemAuthenticationLevel,AuthenticationLevel,7);
    
    //��α����
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1014;
    parastruct.ptr = (INT8U *)AuthenticationDataSettings; 
    ParaAppfunction(&parastruct);
    
    //�û���Ϣ��ȡ    
    //IP 
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1004;
    parastruct.ptr = (INT8U *)Get_Flash_IP; 
    ParaAppfunction(&parastruct);
    IPNumtoChar(Get_Flash_IP,IP);
    
    //MASK
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1005;
    parastruct.ptr = (INT8U *)Get_Flash_MASK; 
    ParaAppfunction(&parastruct);
    IPNumtoChar(Get_Flash_MASK,Mask);
    
    //GateWay
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1006;
    parastruct.ptr = (INT8U *)Get_Flash_GateWay; 
    ParaAppfunction(&parastruct);
    IPNumtoChar(Get_Flash_GateWay,GateWay);
        
    //Mac
    MacNumtoChar(Get_Flash_Mac,Mac);
    
    //���ض˿�
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1008;
    parastruct.ptr = (INT8U *)Get_Flash_LocalPort; 
    ParaAppfunction(&parastruct);
    PortNumtoChar(Get_Flash_LocalPort,LocalPort);
    
    //Զ��IP 
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1009;
    parastruct.ptr = (INT8U *)Get_Flash_DiskIP; 
    ParaAppfunction(&parastruct);
    IPNumtoChar(Get_Flash_DiskIP,DiskIP);
    
    //Զ�˶˿�
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x100A;
    parastruct.ptr = (INT8U *)Get_Flash_DiskPort; 
    ParaAppfunction(&parastruct);
    PortNumtoChar(Get_Flash_DiskPort,DiskPort);   
    
    //������
    parastruct.RWChoose = READ_ORDER;//��Flash
    parastruct.DataID = 0x1015;
    parastruct.ptr = (INT8U *)BlackLists; 
    ParaAppfunction(&parastruct);
}
/***********************************************************************************************
* Function		: TaskDisplay
* Description	: ��������������������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 11/7/2014	songchao
***********************************************************************************************/
_BSP_MESSAGE Dsend_message; 
void TaskDisplay(void *pdata)
{   
    INT8U err;
    INT8U keyval;
    _BSP_MESSAGE *pMsg;	
   
    _BSPUART_CONFIG UART2_config;                       // �ñ����������޸ģ���Ϊ��Bsp_UART.c�ļ��б���ͬ����Ӱ���Ķ��ж�
    
    DispTaskEvent = OSQCreate(DisplayOSQ,16);
    UART2_config.Baudrate = 115200;		                // ������
    UART2_config.Parity = BSPUART_PARITY_NO;			// У��λ
    UART2_config.StopBits = BSPUART_STOPBITS_1;		    // ֹͣλ
    UART2_config.WordLength = BSPUART_WORDLENGTH_8D;	// ����λ��
    UART2_config.Work = BSPUART_WORK_FULLDUPLEX;		// ����ģʽ
    UART2_config.TxRespond = BSPUART_RESPOND_INT;	    // �ж�ģʽ
    UART2_config.pEvent = DispTaskEvent;	            // ��Ϣ�¼�
    UART2_config.MaxTxBuffer = DISPBUFF_LENGTH;		    // ���ͻ�������
    UART2_config.MaxRxBuffer = DISPBUFF_LENGTH;		    // ���ջ�������
    UART2_config.pTxBuffer = DisplayTxBuff;				// ���ͻ���ָ��
    UART2_config.pRxBuffer = DisplayRxBuff;				// ���ջ���ָ��
    UART2_config.TxSpacetime = 0;			// ����֡���
    UART2_config.RxOvertime = 0;			// ����֡���
    
#ifdef __UART2_ENABLE
    BSP_UARTConfig(UART2,&UART2_config);
#else 
    BSP_UARTConfig(UART0,&UART2_config);
#endif //__UART2_ENABLE   
    
    BSP_KEYHWInit();
    BSP_KEYInit(DispTaskEvent);//������ʼ��
    
    OSTimeDlyHMSM(0,0,0,500);
        
    InitMianMenu();
    
    DisplayParaInit();
    
   //�����˵�
    DisplayStartMenu(&gsMenu_30,0);
    
    //���˵���ʾ
    DisplayMianMenu(&gsMenu0,0);

    DBG_DIS("in TaskDisplay 1:while start\r\n");

    while(1)
    {	
        // ��Ϣָ��
        pMsg = OSQPend(DispTaskEvent,SYS_DELAY_1s,(INT8U *)&err);
        if(err == OS_NO_ERR)							// �յ���Ϣ
        {
            switch(pMsg->MsgID)								// �ж���Ϣ
            {
                case BSP_MSGID_UART_TXOVER:					// RS232�������
                    NOP();
                    break;
                case BSP_MSGID_UART_RXOVER:					// RS232�������
                    if(pMsg->DataLen > 600)
                    {
                        pMsg->DataLen=0;						    
                        break;
                    }
                    memcpy(DisplayTxBuff,pMsg->pData,pMsg->DataLen);
                    BSP_UARTRxClear(UART2);
                    break;
                case APP_DISP_COMFROM_MC:
                    memcpy(CanRxBuff,pMsg->pData,pMsg->DataLen);
                    //�����������յ�������
                    if(CSCheckFlag == FALSE)
                    {
                        DealWithMC(CanRxBuff);
                    }
                    break;
                case BSP_MSGID_KEY://���Եײ�İ���keyֵ
                   keyval = pMsg->DivNum;//ȡ����ֵ
                    //����������
                    if(KeyStatus == KEYENABLE)
                    {
                        DealWithKey(&keyval);
                    }
                    break;
                case APP_DISP_COMFROM_DSP:
                    {
                        NETFLAG = *(pMsg->pData);
                        //��������˵��͸������˵�������״̬
                        if(gps_CurMenu->Menu_Flag==0)
                        {
                            DisplayMianMenu(&gsMenu0,0);
                        }
                    }
                    break;
                case APP_DISP_DSP_VER:
                    {
                        DBG_DIS("in TaskDisplay 4:display recv ver info:DataLen=%d\r\n",pMsg->DataLen);
                        memcpy(display_recv_info,pMsg->pData,pMsg->DataLen);
                        if(pMsg->DataLen == 25)
                        {
                            DealWithDsp(display_recv_info);
                        }
                    }
                    break;
                case APP_DISP_DSP_CHECK_CS:
                    {
                        DBG_DIS("in TaskDisplay 4:display recv check cs info:DataLen=%d\r\n",pMsg->DataLen);
                        memcpy(display_recv_info,pMsg->pData,pMsg->DataLen);
                        CSCheckFlag = TRUE;
                        DealWitchCheckCS(display_recv_info);
                    }
                    break;
                default:
                break;
            }
        }

    }
}

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/
