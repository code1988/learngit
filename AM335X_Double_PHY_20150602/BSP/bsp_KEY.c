/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_KEY.c
**��    ��    ��: wangyao
**��  ��  ��  ��: 081201
**��  ��  ��  ��: V0.2
**��          ��: KEY���Ƴ���
	              74165������Ҳ��������
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: wangyao
**��          ��: 141203
**��          ��: V0.2
**��          ��: TCA6416 ����I2C��KEY����
**----------------------------------------------------------------------------------------------
**��    ��    ��:
**��          ��: 
**��          ��: V0.
**��          ��: ���汾���TI AM3352�޸�,���Ż�
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include	"sysconfig.h"
#include    "bsp_conf.h"
#include	"soc_AM335x.h"
#include 	"evmAM335x.h"
#include 	"gpio.h"
#include 	"gpio_v2.h"
#include 	"am335x_irq.h"
#include    "pin_mux.h"
#include	"bsp_KEY.h"
#include    "hw_types.h"
#include    "bsp_iic.h"
#include    "bsp_gpio.h"
#include    "bsp_LED.h"
#include    "app.h"


#include "bsp.h"
#include "dmtimer.h"
/* Private define-----------------------------------------------------------------------------*/
#define TIMER6_NUM_MS                     100//�����ĺ�����
#define TIMER6_INITIAL_COUNT             (0xFFFFFFFF - 24000*TIMER6_NUM_MS)//(0xFF000000u)//
#define TIMER6_RLD_COUNT                 (0xFFFFFFFF - 24000*TIMER6_NUM_MS)//(0xFF000000u)//

#define		TCA6416A_ADDR	0x20	/*�ӻ�TCA6416A�ĵ�ַ*/
//TCA6416�Ŀ��ƼĴ�������
//��ȡ�ܽ�����״̬�Ĵ�����ֻ��
#define		IN_CMD0			0x00	
#define		IN_CMD1			0x01
//���ƹܽ����״̬�Ĵ�����R/W
#define		OUT_CMD0		0x02	
#define		OUT_CMD1		0x03
//������ƹܽ����״̬�Ĵ�����R/W
#define		PIVS_CMD0		0x04	
#define		PIVS_CMD1		0x05
//�ܽŷ�����ƣ�1��In��0:��Out��
#define		CFG_CMD0		0x06	
#define		CFG_CMD1		0x07
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
// �¼���Ϣ�ṹ�ض���,����������Ϣ���ݵĽṹ,�����bsp_conf.h��ͳһ����
#define	_KEY_MESSAGE			_BSP_MESSAGE	// ���_GLB_MESSAGE�Ķ���
INT8U KEYInitDone = 0;
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
	_KEY_MESSAGE Message;						// ������Ϣ
	OS_EVENT *Event;							// ��ֵ�������¼�ָ��
}_KEY_CONTROL;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static _KEY_CONTROL KEY_Control;				// keyģ����Ʊ�������
//_BSP_MESSAGE Keysend_message; 
INT8U keyvalue = 0;
INT8U countkey1 = 0;
INT8U countkey2 = 0;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//����������Ϣ
void KEYRxOver(INT8U keyvalue)
{
    KEY_Control.Message.MsgID = BSP_MSGID_KEY;
    KEY_Control.Message.DivNum = keyvalue;//�豸��������¼����ֵ
    SYSPost(KEY_Control.Event,&KEY_Control.Message);
}
#if 0
/***********************************************************************************************
* Function		: BSP_KEYScan
* Description	: ����ɨ�����,��⵽���������ź���.���ܴ��������������,
* Input			: 
* Output		: 
* Note(s)		: �û�����ʹ�ô˺�
* Contributor	: 
***********************************************************************************************/
INT16U BSP_KEYRead(void)
{
	INT16U keynum = 0;
	
	iic_start();
	if(iic_send_byte(0x81)==TRUE)
	{		
		keynum=iic_rec_2byte();
		iic_ack();
		iic_stop();
		return keynum;//^0xffff;
	}
	else
	{
		iic_stop();
		return 0;
	}
}
static INT16U  lastkeynum = 0;
void BSP_KEYScan(void)
{
    
    INT16U keynum = 0;
    
    if(KEYInitDone)
    {    
        keynum =  BSP_KEYRead();
        
        if(lastkeynum != keynum)
        {
            switch(keynum)								// �ж���Ϣ
            {
                case BSPKEY_VALUE_ONE:
                    keyvalue = 1;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case  BSPKEY_VALUE_TOW:
                    keyvalue = 2;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_THREE:
                    keyvalue = 3;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break; 
                case BSPKEY_VALUE_FOUR:
                    keyvalue = 4;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                       KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                     break;
                case BSPKEY_VALUE_FIVE:
                    keyvalue = 5;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                     break;
                case BSPKEY_VALUE_SIX:
                    keyvalue = 6;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_SEVEN:
                    keyvalue = 7;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_EIGHT:
                    keyvalue = 8;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_NINE:
                    keyvalue = 9;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_TEN:
                    keyvalue = 10;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_ELEVEN:
                    keyvalue = 11;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_TWELVE:
                    keyvalue = 12;
                    //if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    //countkey1++;
                    NOP();
                    break;
                default:
                    break;
            }
        }
        lastkeynum = keynum;
       
    }

}
#endif
void BSP_KEYScan(void)
{
    INT8U Keyval[2] = {0};
    INT8U temp[2] = {0};
    if(KEYInitDone)
    {
        temp[0] = 0x00;
        temp[1] = 0x41;         
        BSP_I2CWrite(0,0x20,temp,2); //���Ͷ�����     
        BSP_I2CRead(0,0x20,Keyval,2,&temp[0]);//��ȡ����ֵ	
        if(Keyval[1] != 0xFF)
        {                      
            switch(Keyval[1])								// �ж���Ϣ
            {
                case BSPKEY_VALUE_ONE:
                    keyvalue = 1;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                    break;
                case  BSPKEY_VALUE_TOW:
                    keyvalue = 2;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_THREE:
                    keyvalue = 3;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                    break; 
                case BSPKEY_VALUE_FOUR:
                    keyvalue = 4;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                       KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                     break;
                case BSPKEY_VALUE_FIVE:
                    keyvalue = 5;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                     break;
                case BSPKEY_VALUE_SIX:
                    keyvalue = 6;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_SEVEN:
                    keyvalue = 7;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                    break;
                case BSPKEY_VALUE_EIGHT:
                    keyvalue = 8;
                    if((countkey1 == 0)||(countkey1 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey1++;
                    NOP();
                    break;
                default:
                    break;
            }
        }
        else
        {
            countkey1 = 0;
        }
        if(Keyval[0] != 0x0F)
        {
            switch(Keyval[0])								// �ж���Ϣ
            {
                case BSPKEY_VALUE_NINE:
                    keyvalue = 9;
                    if((countkey2 == 0)||(countkey2 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey2++;
                    NOP();
                     break;
                case BSPKEY_VALUE_TEN:
                    keyvalue = 10;
                    if((countkey2 == 0)||(countkey2 > 5))
                    {
                       KEYRxOver(keyvalue);
                    }
                    countkey2++;
                    NOP();
                    break;
                case BSPKEY_VALUE_ELEVEN:
                    keyvalue = 11;
                    if((countkey2 == 0)||(countkey2 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey2++;
                    NOP();
                    break;
                case BSPKEY_VALUE_TWELVE:
                    keyvalue = 12;
                    if((countkey2 == 0)||(countkey2 > 5))
                    {
                        KEYRxOver(keyvalue);
                    }
                    countkey2++;
                    NOP();
                    break;
                default:
                    break;
            }
        }
        else
        {
            countkey2 = 0;
        }
    }

}

/***********************************************************************************************
* Function		: BSP_KEYHWInit
* Description	: ����Ӳ����ʼ��,TCA6416��λ���͵�ƽ��λ�����������ʹ�õ�оƬ���˹����ⲿ�û���
*                 ���������쳣ʱ����Կ��������Ӳ����λ��                 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void BSP_KEYHWInit(void)
{
    //KEY RESET
    _BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = 27;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT;
    //reset
    BSP_GPIOConfig(&GPIO_InitStructure);
    
    GPIOPinWrite(SOC_GPIO_1_REGS, 27,GPIO_PIN_LOW);   
    OSTimeDlyHMSM(0,0,0,50);   //��ʱ50MS
    GPIOPinWrite(SOC_GPIO_1_REGS, 27,GPIO_PIN_HIGH); 
    OSTimeDlyHMSM(0,0,0,20); 
}


/*
** Do the necessary DMTimer configurations on to AINTC.
*/
static void DMTimer6AintcConfigure(void)
{
    /* Registering DMTimerIsr */
    BSP_IntVectReg(SYS_INT_TINT6, Timer6_IRQHandler);//��ʱ��6 
    
    /* Set the priority */
    IntPrioritySet(SYS_INT_TINT6, 18, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enable the system interrupt */
    IntSystemEnable(SYS_INT_TINT6);
}

/*
** Setup the timer for one-shot and compare mode.
*/
static void DMTimer6SetUp(void)
{
    /* Load the counter with the initial count value */
    DMTimerCounterSet(SOC_DMTIMER_6_REGS, TIMER6_INITIAL_COUNT);//�������Ĵ�����ֵ

    /* Load the load register with the reload count value */
    DMTimerReloadSet(SOC_DMTIMER_6_REGS, TIMER6_RLD_COUNT);

    /* Configure the DMTimer for Auto-reload and compare mode */
    DMTimerModeConfigure(SOC_DMTIMER_6_REGS, DMTIMER_AUTORLD_NOCMP_ENABLE);
}

  /***********************************************************************************************
* Function		: Bsp_Timer6Init 
* Description	: ����ɨ��
* Input			: none
* Output		: none
* Note(s)		: ���ö�ʱ��6��Ϊɨ�� 
* Contributor	: 
***********************************************************************************************/
void  Bsp_Timer6Init(void)
{  
    /* This function will enable clocks for the DMTimer6 instance */
    DMTimer6ModuleClkConfig();
    /* Enable IRQ in CPSR */
    IntMasterIRQEnable();

    /* Register DMTimer6 interrupts on to AINTC */
    DMTimer6AintcConfigure();

    /* Perform the necessary configurations for DMTimer */
    DMTimer6SetUp();

    /* Enable the DMTimer interrupts */
    DMTimerIntEnable(SOC_DMTIMER_6_REGS, DMTIMER_INT_OVF_EN_FLAG);

    /* Start the DMTimer */
    DMTimerEnable(SOC_DMTIMER_6_REGS);      
}
#if 0
/***********************************************************************************************
* Function		: BSP_KEYInit
* Description	: ������ʼ��,��Ҫ����������
* Input			: *Event:�¼�(��Ϣ����)ָ��,ʹ��keyģ����û�����ָ��һ���������ݰ�����Ϣ�Ͱ���ֵ���¼�,
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void BSP_KEYInit(OS_EVENT *Event)
{
    INT8U temp[2] = {0};
    
    //����������Ϣ
    KEY_Control.Event = Event;
    //��ʼ��
    KEY_Control.Message.DataLen = 1;
    KEY_Control.Message.pData = (INT8U *)0;
	//BSP_I2C0Init();
    
    iic_init();
     
    //BSP_KEYScan();
    KEYInitDone = 1;
    Bsp_Timer6Init();
    
}
#endif
/***********************************************************************************************
* Function		: BSP_KEYInit
* Description	: ������ʼ��,��Ҫ����������
* Input			: *Event:�¼�(��Ϣ����)ָ��,ʹ��keyģ����û�����ָ��һ���������ݰ�����Ϣ�Ͱ���ֵ���¼�,
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void BSP_KEYInit(OS_EVENT *Event)
{
    INT8U temp[2] = {0};
    
    //����������Ϣ
    KEY_Control.Event = Event;
    //��ʼ��
    KEY_Control.Message.DataLen = 1;
    KEY_Control.Message.pData = (INT8U *)0;
	BSP_I2C0Init();
    //BSP_I2CInit(0);
    //port0ȫ��Ϊ����״̬����1��
    temp[0] = CFG_CMD0;
    temp[1] = 0xff; 
    BSP_I2CWrite(0,TCA6416A_ADDR,temp,2);  
    
    //port1��ʱҲ��Ϊ���룬�����ٸ�
    temp[0] = CFG_CMD1;
    temp[1] = 0xff; //port0ȫ��Ϊ����״̬����1��
    BSP_I2CWrite(0,TCA6416A_ADDR,temp,2);  
    //BSP_KEYScan();
    KEYInitDone = 1;
    Bsp_Timer6Init();
    
}
/************************(C)COPYRIGHT 2008 �㽭��̩*****END OF FILE****************************/
