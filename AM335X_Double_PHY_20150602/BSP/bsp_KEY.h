/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_KEY.h
**��    ��    ��: wangyao
**��  ��  ��  ��: 141203
**��  ��  ��  ��: V0.1
**��          ��: KEY��ͷ�ļ�
	ʹ�ò���:
		1 ����Ҫʹ��key��ģ�鶨���¼�ָ��(����),������������
		2 ����BSP_KEYInit()������ʼ��key,�������涨����¼�ָ��
		3 �ȴ��¼�
		4 �ڳ��ֺϷ�������,�����ᷢ��һ���¼���Ϣ,��Ϣ�����ݾ��Ǽ�ֵ(_BSPKEY_VALUE�ж���),
		Ӧ�ó�����Ӧ���¼�,��������Ϣ���ݴ�����.
		ע:��Ϣ����ʹ��ͳһ�Ľṹ_BSP_MESSAGE,����ֻʹ����DivNum��,���������ֵ
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��: ��ֲ��AM335x
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_KEY_H_
#define	__BSP_KEY_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/

// key��������
#define	KEY_MAX_NUM				6				// �������Ÿ���(��������)
#define	KEY_DELAY				20				// ȥ��ʱ��
#define	KEY_DELAY_LONG			70				// ��������ʱʱ��
#define	KEY_ENABLE_LONG			0				// 1:��������(�����ͷ���Ӧ);0:ֻ�ж̰���(����������Ӧ,�����Ӧ���)
/* Private typedef----------------------------------------------------------------------------*/
//��������
#if 0
#define	BSPKEY_VALUE_ONE     0xFFDF			
#define	BSPKEY_VALUE_TOW     0xFFBF		
#define	BSPKEY_VALUE_THREE   0xFF7F			
#define	BSPKEY_VALUE_FOUR    0xFEFF
#define BSPKEY_VALUE_FIVE    0xBFFF				
#define	BSPKEY_VALUE_SIX     0xDFFF					
#define	BSPKEY_VALUE_SEVEN   0x6FFF				
#define	BSPKEY_VALUE_EIGHT   0xF7FF						
#define	BSPKEY_VALUE_NINE    0xFDFF					
#define	BSPKEY_VALUE_TEN     0xFBFF					
#define	BSPKEY_VALUE_ELEVEN  0x7FFF
#define	BSPKEY_VALUE_TWELVE  0x0004
#endif

#define	BSPKEY_VALUE_ONE     0xFE			
#define	BSPKEY_VALUE_TOW     0xFD		
#define	BSPKEY_VALUE_THREE   0xFB			
#define	BSPKEY_VALUE_FOUR    0xF7
#define BSPKEY_VALUE_FIVE    0x7F				
#define	BSPKEY_VALUE_SIX     0xBF					
#define	BSPKEY_VALUE_SEVEN   0xDF					
#define	BSPKEY_VALUE_EIGHT   0xEF							
#define	BSPKEY_VALUE_NINE    0x07					
#define	BSPKEY_VALUE_TEN     0x0B					
#define	BSPKEY_VALUE_ELEVEN  0x0D
#define	BSPKEY_VALUE_TWELVE  0x0E
/***********************************************************************************************
* Function		: BSP_165Init,BSP_165Read
* Description	: 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 140701 Wang WenGang
***********************************************************************************************/
INT8U KEYRead(void);

/***********************************************************************************************
* Function		: BSP_KEYScan
* Description	: ����ɨ�����,��⵽���������ź���.
	Ŀǰ���ܴ��������������,���ܵİ���ֵ��_BSPKEY_VALUE�ж���
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 081202	
***********************************************************************************************/
void BSP_KEYScan(void);
/***********************************************************************************************
* Function		: BSP_KEYHWInit
* Description	: ����Ӳ����ʼ��
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void BSP_KEYHWInit(void);
/***********************************************************************************************
* Function		: BSP_KEYInit
* Description	: ������ʼ��,��Ҫ����������
* Input			: *Event:�¼�(����)ָ��,ʹ��keyģ����û�����ָ��һ���������ݰ�����Ϣ�Ͱ���ֵ���¼�,
* Output		: 
* Note(s)		: 
* Contributor	: 081202	
***********************************************************************************************/
void BSP_KEYInit(OS_EVENT *Event);

void G1PinMuxSetup(unsigned int pinNo);
#endif	//__BSP_KEY_H_
/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/
