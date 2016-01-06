/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_KEY.c
**创    建    人: wangyao
**创  建  日  期: 081201
**最  新  版  本: V0.2
**描          述: KEY控制程序
	              74165的驱动也做在这里
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: wangyao
**日          期: 141203
**版          本: V0.2
**描          述: TCA6416 基于I2C的KEY驱动
**----------------------------------------------------------------------------------------------
**修    改    人:
**日          期: 
**版          本: V0.
**描          述: 本版本针对TI AM3352修改,并优化
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
#define TIMER6_NUM_MS                     100//开启的毫秒数
#define TIMER6_INITIAL_COUNT             (0xFFFFFFFF - 24000*TIMER6_NUM_MS)//(0xFF000000u)//
#define TIMER6_RLD_COUNT                 (0xFFFFFFFF - 24000*TIMER6_NUM_MS)//(0xFF000000u)//

#define		TCA6416A_ADDR	0x20	/*从机TCA6416A的地址*/
//TCA6416的控制寄存器定义
//读取管脚输入状态寄存器；只读
#define		IN_CMD0			0x00	
#define		IN_CMD1			0x01
//控制管脚输出状态寄存器；R/W
#define		OUT_CMD0		0x02	
#define		OUT_CMD1		0x03
//反向控制管脚输出状态寄存器；R/W
#define		PIVS_CMD0		0x04	
#define		PIVS_CMD1		0x05
//管脚方向控制：1：In；0:：Out。
#define		CFG_CMD0		0x06	
#define		CFG_CMD1		0x07
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
// 事件消息结构重定义,用来发送消息数据的结构,这个在bsp_conf.h中统一定义
#define	_KEY_MESSAGE			_BSP_MESSAGE	// 详见_GLB_MESSAGE的定义
INT8U KEYInitDone = 0;
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
	_KEY_MESSAGE Message;						// 保存消息
	OS_EVENT *Event;							// 键值传递用事件指针
}_KEY_CONTROL;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static _KEY_CONTROL KEY_Control;				// key模块控制变量定义
//_BSP_MESSAGE Keysend_message; 
INT8U keyvalue = 0;
INT8U countkey1 = 0;
INT8U countkey2 = 0;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//按键发送消息
void KEYRxOver(INT8U keyvalue)
{
    KEY_Control.Message.MsgID = BSP_MSGID_KEY;
    KEY_Control.Message.DivNum = keyvalue;//设备号用来记录按键值
    SYSPost(KEY_Control.Event,&KEY_Control.Message);
}
#if 0
/***********************************************************************************************
* Function		: BSP_KEYScan
* Description	: 按键扫描程序,检测到按键后发送信号量.不能处理多个按键的情况,
* Input			: 
* Output		: 
* Note(s)		: 用户无需使用此函
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
            switch(keynum)								// 判断消息
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
        BSP_I2CWrite(0,0x20,temp,2); //发送读命令     
        BSP_I2CRead(0,0x20,Keyval,2,&temp[0]);//读取按键值	
        if(Keyval[1] != 0xFF)
        {                      
            switch(Keyval[1])								// 判断消息
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
            switch(Keyval[0])								// 判断消息
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
* Description	: 按键硬件初始化,TCA6416复位，低电平复位，考虑这颗新使用的芯片，此功能外部用户在
*                 按键出现异常时候可以看情况进行硬件复位。                 
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
    OSTimeDlyHMSM(0,0,0,50);   //延时50MS
    GPIOPinWrite(SOC_GPIO_1_REGS, 27,GPIO_PIN_HIGH); 
    OSTimeDlyHMSM(0,0,0,20); 
}


/*
** Do the necessary DMTimer configurations on to AINTC.
*/
static void DMTimer6AintcConfigure(void)
{
    /* Registering DMTimerIsr */
    BSP_IntVectReg(SYS_INT_TINT6, Timer6_IRQHandler);//定时器6 
    
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
    DMTimerCounterSet(SOC_DMTIMER_6_REGS, TIMER6_INITIAL_COUNT);//计数器寄存器赋值

    /* Load the load register with the reload count value */
    DMTimerReloadSet(SOC_DMTIMER_6_REGS, TIMER6_RLD_COUNT);

    /* Configure the DMTimer for Auto-reload and compare mode */
    DMTimerModeConfigure(SOC_DMTIMER_6_REGS, DMTIMER_AUTORLD_NOCMP_ENABLE);
}

  /***********************************************************************************************
* Function		: Bsp_Timer6Init 
* Description	: 按键扫描
* Input			: none
* Output		: none
* Note(s)		: 采用定时器6作为扫描 
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
* Description	: 驱动初始化,主要是引脚配置
* Input			: *Event:事件(消息队列)指针,使用key模块的用户必须指定一个用来传递按键信息和按键值的事件,
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void BSP_KEYInit(OS_EVENT *Event)
{
    INT8U temp[2] = {0};
    
    //保存邮箱消息
    KEY_Control.Event = Event;
    //初始化
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
* Description	: 驱动初始化,主要是引脚配置
* Input			: *Event:事件(消息队列)指针,使用key模块的用户必须指定一个用来传递按键信息和按键值的事件,
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void BSP_KEYInit(OS_EVENT *Event)
{
    INT8U temp[2] = {0};
    
    //保存邮箱消息
    KEY_Control.Event = Event;
    //初始化
    KEY_Control.Message.DataLen = 1;
    KEY_Control.Message.pData = (INT8U *)0;
	BSP_I2C0Init();
    //BSP_I2CInit(0);
    //port0全部为输入状态，置1；
    temp[0] = CFG_CMD0;
    temp[1] = 0xff; 
    BSP_I2CWrite(0,TCA6416A_ADDR,temp,2);  
    
    //port1暂时也置为输入，后续再改
    temp[0] = CFG_CMD1;
    temp[1] = 0xff; //port0全部为输入状态，置1；
    BSP_I2CWrite(0,TCA6416A_ADDR,temp,2);  
    //BSP_KEYScan();
    KEYInitDone = 1;
    Bsp_Timer6Init();
    
}
/************************(C)COPYRIGHT 2008 浙江方泰*****END OF FILE****************************/
