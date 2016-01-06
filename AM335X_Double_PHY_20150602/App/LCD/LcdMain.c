/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: lcdmain.c
**Ӳ          ��: am335x
**��    ��    ��: wangyao
**��  ��  ��  ��: 131029
**��  ��  ��  ��: V0.1
**��          ��: LCD��ʾ���������ļ�
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "guisprint.h"
#include "display.h"
#include "key.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
typedef enum
{
	//0~9���ּ�
	BSP_KEY_0,	    
	BSP_KEY_1,		
	BSP_KEY_2,
	BSP_KEY_3,
	BSP_KEY_4,
	BSP_KEY_5,
	BSP_KEY_6,
	BSP_KEY_7,
	BSP_KEY_8,//��Ϊ���ؼ�ʹ��
	BSP_KEY_9,
}_BSP_KEYMSG;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
tContext sContext;//�ı�
extern const unsigned char g_pucImage[];
extern const unsigned char g_TILogo[];
extern const unsigned char key2[];

OS_EVENT *LCDDisplay_prevent;	//����Һ����ʾ����
void *LCDDisplay[8];		    //8����Ϣ���Դ洢
//static FILINFO FileInfo;
//static FILINFO FileInfo1;
/* Private function prototypes----------------------------------------------------------------*/
void  KeyMessageDealWith(INT8U *pdata);
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: DummyKeyInit 
* Description	: ���ⰴ�����ô��ڴ���
* Input			: none
* Output		: none
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT8U TxKeyBuff[10]={0};
INT8U RxKeyBuff[10]={0};
void DummyKeyInit(void)
{
    _BSPUART_CONFIG UART_Config;

    LCDDisplay_prevent = OSQCreate(LCDDisplay,8);
    UART_Config.Baudrate = 115200;		            // ������
    UART_Config.Parity = BSPUART_PARITY_NO;			// У��λ
    UART_Config.StopBits = BSPUART_STOPBITS_1;		// ֹͣλ
    UART_Config.WordLength = BSPUART_WORDLENGTH_8D;	// ����λ��
    UART_Config.Work = BSPUART_WORK_FULLDUPLEX;		// ����ģʽ
    UART_Config.TxRespond = BSPUART_RESPOND_INT;	// �ж�ģʽ
    UART_Config.pEvent = LCDDisplay_prevent;	    // ��Ϣ�¼�
    UART_Config.MaxTxBuffer = 10;	                // ���ͻ�������
    UART_Config.MaxRxBuffer = 10;	                // ���ջ�������
    UART_Config.pTxBuffer = TxKeyBuff;	// ���ͻ���ָ��
    UART_Config.pRxBuffer = RxKeyBuff;	// ���ջ���ָ��
    UART_Config.TxSpacetime = SYS_DELAY_20ms;	// ����֡���
    UART_Config.RxOvertime = SYS_DELAY_50ms;	// ����֡���

    BSP_UARTConfig(0,&UART_Config);    
}
/*****************************************************************   
�������ƣ�INT32U IntHexToBcd(INT32U Source)
���������Source-��ת����Hex����   
���������ת���������BCD������
Լ����������
���������������͵�ʮ��������ת����BCD��
��    �ߣ�                     ����:  
��    �ģ�                     ����:  
��    ����  
*****************************************************************/
INT32U IntHexToBcd(INT32U Source)
{
	INT32U Result, tBase[8], Temp1, Temp2;
	int i, j;

	tBase[0] = 10000000;
	tBase[1] = 1000000;
	tBase[2] = 100000;
	tBase[3] = 10000;
	tBase[4] = 1000;
	tBase[5] = 100;
	tBase[6] = 10;
	tBase[7] = 1;

	Result = 0;		
	for(i = 0; i < 8; i++)
	{
		Temp1 = 0;  
		Temp2 = 0;
		for(j = 0; j < 10; j++)
		{
			Temp1 += tBase[i];
			if(Source < Temp1)
			{
				Result = (Result << 4);
				Result = (Result | j);
				Source -= Temp2;
				break;
			}
			Temp2 = Temp1;			
		}
	}		
	return Result;
}

/***********************************************************************************************
* Function		: void  KeyMessageDealWith(INT8U *pdata)
* Description	: ������Ϣ����������Ӧ��������
* Input			: *pdata ����ֵ
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void  KeyMessageDealWith(INT8U *pdata)
{
    _BSP_KEYMSG keyval;
    
	keyval = (_BSP_KEYMSG)(*(pdata)&0x0F);//ʮ����ʮ�����Ʒ��͹���������
	switch(keyval)
	{
		case BSP_KEY_0:
			break;	    
		case BSP_KEY_1:
            Key_one(keyval);
			break;		
		case BSP_KEY_2:
            Key_two(keyval);
			break;
		case BSP_KEY_3:
			break;
		case BSP_KEY_4:
			break;
		case BSP_KEY_5:
			break;
		case BSP_KEY_6:
			break;
		case BSP_KEY_7:
			break;
        case BSP_KEY_8:
            Key_eight(keyval);
			break;    
		default:
			break;
	}
}

/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/