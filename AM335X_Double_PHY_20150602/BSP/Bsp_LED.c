/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName		    : bsp_num.c
* Author		    : Wang WenGang
* Date First Issued	: 2014-7-10 8:39:49   
* Version		    : v1
* Description		: СLED�����
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History		    :
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "include.h"
#include "interrupt.h"
#include "edma_event.h"
#include "edma.h"
#include "AM335x_IRQ.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "cpu.h"
#include "bsp_iic.h"
#include "bsp_LED.h"
#include "bsp_gpio.h"
#include "gpio.h"
#include "gpio_v2.h"

/* Private define-----------------------------------------------------------------------------*/
INT16U u16_LED_NUM = 0;
const INT8U vfdmap[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
const INT8U vfdmap1[]={0x5F,0x44,0x2F,0x6E,0x74,0x7A,0x7B,0x4C,0x7F,0x7E,0x7d,0x00};// (0~A,��)
const INT8U vfdmap2[]={0x5F,0x44,0x2F,0x6E,0x74,0x7A,0x7B,0x4C,0x7F,0x7E,00};
#define SPI_GPIO_SCL 7u
#define SPI_GPIO_STB 12u
#define SPI_GPIO_DIN 8u
#define LED_OUTSIDE_GPIO_SCL 17u
#define LED_OUTSIDE_GPIO_STB 25u
#define LED_OUTSIDE_GPIO_DIN 24u
#define	SPI_DELAY_TIME			3
#define	SPI_DELAY_TIME_LONG		5

static void vfd_delay(INT32U time)
{
	DelayUS(time);
}

void led_outside_clock_gpio_init(void)
{
    _BSPGPIO_CONFIG GPIO_InitStructure;  
      
    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = LED_OUTSIDE_GPIO_SCL;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT; 
    BSP_GPIOConfig(&GPIO_InitStructure);
}

void led_outside_stb_gpio_init(void)
{
    _BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = LED_OUTSIDE_GPIO_STB;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT; 
    BSP_GPIOConfig(&GPIO_InitStructure);
}

void led_outside_din_gpio_init(void)
{
    _BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = LED_OUTSIDE_GPIO_DIN;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT; 
    BSP_GPIOConfig(&GPIO_InitStructure);
}

void led_clock_gpio_init(void)
{
    _BSPGPIO_CONFIG GPIO_InitStructure;  
      
    GPIO_InitStructure.PortNum = PORT0;
    GPIO_InitStructure.PinNum = SPI_GPIO_SCL;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT; 
    BSP_GPIOConfig(&GPIO_InitStructure);
}

void led_stb_gpio_init(void)
{
    _BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = PORT0;
    GPIO_InitStructure.PinNum = SPI_GPIO_STB;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT; 
    BSP_GPIOConfig(&GPIO_InitStructure);
}

void led_din_gpio_init(void)
{
    _BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = SPI_GPIO_DIN;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT; 
    BSP_GPIOConfig(&GPIO_InitStructure);
}

// gpio��ʼ��
void Led_gpio_init(void)
{ 
    led_clock_gpio_init();
    led_stb_gpio_init();
    led_din_gpio_init();
    led_outside_clock_gpio_init();
    led_outside_stb_gpio_init();
    led_outside_din_gpio_init();
}

//����
static void led_clock_high(void)
{
    GPIOPinWrite(SOC_GPIO_0_REGS, SPI_GPIO_SCL,Bit_SET); 
	vfd_delay(SPI_DELAY_TIME);
}
static void led_clock_low(void)
{
	GPIOPinWrite(SOC_GPIO_0_REGS, SPI_GPIO_SCL,Bit_RESET);  
	vfd_delay(SPI_DELAY_TIME);
}
static void led_stb_high(void)
{
    GPIOPinWrite(SOC_GPIO_0_REGS, SPI_GPIO_STB,Bit_SET); 
	vfd_delay(SPI_DELAY_TIME);
}
static void led_stb_low(void)
{
	GPIOPinWrite(SOC_GPIO_0_REGS, SPI_GPIO_STB,Bit_RESET);  
	vfd_delay(SPI_DELAY_TIME);
}
static void led_din_high(void)
{
    GPIOPinWrite(SOC_GPIO_1_REGS, SPI_GPIO_DIN,Bit_SET); 
	vfd_delay(SPI_DELAY_TIME);
}
static void led_din_low(void)
{
	GPIOPinWrite(SOC_GPIO_1_REGS, SPI_GPIO_DIN,Bit_RESET);  
	vfd_delay(SPI_DELAY_TIME);
}

//����
static void led_outside_clock_high(void)
{
    GPIOPinWrite(SOC_GPIO_1_REGS, LED_OUTSIDE_GPIO_SCL,Bit_SET); 
	vfd_delay(SPI_DELAY_TIME);
}
static void led_outside_clock_low(void)
{
	GPIOPinWrite(SOC_GPIO_1_REGS, LED_OUTSIDE_GPIO_SCL,Bit_RESET);  
	vfd_delay(SPI_DELAY_TIME);
}
static void led_outside_stb_high(void)
{
    GPIOPinWrite(SOC_GPIO_1_REGS, LED_OUTSIDE_GPIO_STB,Bit_SET); 
	vfd_delay(SPI_DELAY_TIME);
}
static void led_outside_stb_low(void)
{
	GPIOPinWrite(SOC_GPIO_1_REGS, LED_OUTSIDE_GPIO_STB,Bit_RESET);  
	vfd_delay(SPI_DELAY_TIME);
}
static void led_outside_din_high(void)
{
    GPIOPinWrite(SOC_GPIO_1_REGS, LED_OUTSIDE_GPIO_DIN,Bit_SET); 
	vfd_delay(SPI_DELAY_TIME);
}
static void led_outside_din_low(void)
{
	GPIOPinWrite(SOC_GPIO_1_REGS, LED_OUTSIDE_GPIO_DIN,Bit_RESET);  
	vfd_delay(SPI_DELAY_TIME);
}

void ta6932_cmd_a (INT8U vfd_cmd0)  

{
	INT8U u8_i;

	// led_stb_high();
	led_clock_high();
	vfd_delay(SPI_DELAY_TIME);
	for ( u8_i=0;u8_i<8;u8_i++ )
	{
		led_stb_low();
		vfd_delay(SPI_DELAY_TIME);
		led_clock_low();
		vfd_delay(SPI_DELAY_TIME);
		if (vfd_cmd0 & 0x01)
		{
			led_din_high();
		}
		else
		{
			led_din_low();
		}
		vfd_delay(SPI_DELAY_TIME);
		led_clock_high();
		vfd_cmd0 >>= 1;
		vfd_delay(SPI_DELAY_TIME);
	}
	
	// led_stb_high();
	vfd_delay(SPI_DELAY_TIME);
	led_clock_high();
	return;
}


void ta6932_cmd (INT8U vfd_cmd0)                   /*    */
{
	INT8U u8_i;

	led_stb_high();
	led_clock_high();
	vfd_delay(SPI_DELAY_TIME);
	for ( u8_i=0;u8_i<8;u8_i++ )
	{
		led_stb_low();
		vfd_delay(SPI_DELAY_TIME);
		led_clock_low();
		vfd_delay(SPI_DELAY_TIME);
		if (vfd_cmd0 & 0x01)
		{
			led_din_high();
		}
		else
		{
			led_din_low();
		}
		vfd_delay(SPI_DELAY_TIME);		
		led_clock_high();
		vfd_cmd0 >>= 1;
		vfd_delay(SPI_DELAY_TIME);
	}
	
	led_stb_high();
	vfd_delay(SPI_DELAY_TIME);
	led_clock_high();
	return;
}
void ta6932_Send (INT8U vfd_wrdata0)  

{
	INT8U u8_i;

	for(u8_i=0;u8_i<8;u8_i++)
	{
		led_clock_high();
		vfd_delay(SPI_DELAY_TIME);
		led_clock_low();
		vfd_delay(SPI_DELAY_TIME);
	if (vfd_wrdata0 & 0x01)
		{
			led_din_high();
		}
		else
		{
			led_din_low();
		}
	led_clock_high();
	
	vfd_delay(SPI_DELAY_TIME);
	vfd_wrdata0 >>= 1;
	}
	// led_stb_low();
	led_clock_high();
	return;
}


void ta6932_wrdata (INT8U vfd_wrdata0)               /*  */
{
	INT8U u8_i;
#ifdef NEW_LED_PORT

	led_stb_low();
	vfd_delay(SPI_DELAY_TIME);	
	for(u8_i=0;u8_i<8;u8_i++)
	{
		led_clock_low();	
		vfd_delay(SPI_DELAY_TIME);
		if (vfd_wrdata0 & 0x01)
		{
			led_din_high();
		}
		else
		{
			led_din_low();
		}
		vfd_delay(SPI_DELAY_TIME);					
		led_clock_high(); 
		vfd_delay(SPI_DELAY_TIME);		
		vfd_wrdata0 >>= 1;
		vfd_delay(SPI_DELAY_TIME);										
}
led_clock_high();


#else
	for(u8_i=0;u8_i<8;u8_i++)
	{
		led_clock_high();
		vfd_delay(SPI_DELAY_TIME);
		led_stb_low();
		vfd_delay(SPI_DELAY_TIME);
		led_clock_low();
		vfd_delay(SPI_DELAY_TIME);
	if (vfd_wrdata0 & 0x01)
		{
			led_din_high();
		}
		else
		{
			led_din_low();
		}
	led_clock_high();
	
	vfd_delay(SPI_DELAY_TIME);
	vfd_wrdata0 >>= 1;
	}
	led_stb_low();
	led_clock_high();
#endif
	return;
}

void vfd_cmd1 (INT8U vfd_cmd0)  
{
	INT8U u8_i;

	led_outside_stb_high();// stb ����
	vfd_delay(SPI_DELAY_TIME);
	led_outside_clock_high();
	vfd_delay(SPI_DELAY_TIME);	
	led_outside_stb_low();// stb ����
	vfd_delay(SPI_DELAY_TIME);
	for ( u8_i=0;u8_i<8;u8_i++ )
	{
		led_outside_clock_low();	// clk ����	
		vfd_delay(SPI_DELAY_TIME);					
		if (vfd_cmd0 & 0x01)
		{
			led_outside_din_high();
		}
		else
		{
			led_outside_din_low();
		}
		vfd_delay(SPI_DELAY_TIME);				
		led_outside_clock_high();// clk ����
		vfd_cmd0 >>= 1;
		vfd_delay(SPI_DELAY_TIME);	
	}
	led_outside_clock_high();
	return;
}

void vfd_cmd (INT8U vfd_cmd0)  
{
	INT8U u8_i;

	led_stb_high();// stb ����
	vfd_delay(SPI_DELAY_TIME);
	led_clock_high();
	vfd_delay(SPI_DELAY_TIME);	
	led_stb_low();// stb ����
	vfd_delay(SPI_DELAY_TIME);
	for ( u8_i=0;u8_i<8;u8_i++ )
	{
		led_clock_low();	// clk ����	
		vfd_delay(SPI_DELAY_TIME);					
		if (vfd_cmd0 & 0x01)
		{
			led_din_high();
		}
		else
		{
			led_din_low();
		}
		vfd_delay(SPI_DELAY_TIME);			
		led_clock_high();// clk ����
		vfd_cmd0 >>= 1;
		vfd_delay(SPI_DELAY_TIME);
	}
	led_clock_high();
	return;
}

void vfd_wrdata1(INT8U vfd_wrdata0)           
{
	INT8U u8_i;

	led_outside_stb_low();
	vfd_delay(SPI_DELAY_TIME);
	for(u8_i=0;u8_i<8;u8_i++)
	{
		led_outside_clock_low();
		vfd_delay(SPI_DELAY_TIME);		
		if (vfd_wrdata0 & 0x01)
		{
			led_outside_din_high();
		}
		else
		{
			led_outside_din_low();
		}
		vfd_delay(SPI_DELAY_TIME);					
		led_outside_clock_high();	
		vfd_delay(SPI_DELAY_TIME);		
		vfd_wrdata0 >>= 1;
		vfd_delay(SPI_DELAY_TIME);									
	}
	led_outside_clock_high();
	return;
}

void vfd_wrdata(INT8U vfd_wrdata0)           
{
	INT8U u8_i;

	led_stb_low();
	vfd_delay(SPI_DELAY_TIME);	
	for(u8_i=0;u8_i<8;u8_i++)
	{
		led_clock_low();
		vfd_delay(SPI_DELAY_TIME);		
		if (vfd_wrdata0 & 0x01)
		{
			led_din_high();
		}
		else
		{
			led_din_low();
		}
		vfd_delay(SPI_DELAY_TIME);					
		led_clock_high();	
		vfd_delay(SPI_DELAY_TIME);		
		vfd_wrdata0 >>= 1;		
		vfd_delay(SPI_DELAY_TIME);								
	}
	led_clock_high();
	return;
}

void vfd_init1(void)
{
	vfd_cmd1(0x06);                               
	vfd_cmd1(0x40);                                     
	vfd_cmd1(0xC0);
	vfd_wrdata1(0x5f);	// 1st char	

	vfd_wrdata1(0x5f);	// 1st char	

	vfd_wrdata1(0x5f);	// 1st char	

	vfd_delay(SPI_DELAY_TIME);	
	vfd_cmd1(0x8f);             			
	led_outside_stb_high();		
	return;
}

void vfd_init(void)
{
	vfd_cmd(0x06);                                      
	vfd_cmd(0x40);  
	vfd_cmd(0xC0);// wwg �滻����һ��
	// vfd_wrdata(0xC0);         //                         
	return;
}

void ta6932_init(void)
{
	vfd_cmd(0x8c);
	vfd_cmd(0x40);	
}

void LEDTestFunction(INT16U testnum)// ���ⲿ��ʾ
{
	int i;
        
	vfd_cmd(0x06);        /* 10digits��12segments��ʾģʽ  */
	vfd_cmd(0x40);        /* ��ʾ����ģʽ�����ݵ�ַ����  */
	vfd_cmd(0xCD);        // ���õ�ַ��00H��ʼ  

	i = (testnum)%10;
	vfd_wrdata(vfdmap[i]);

    i = ((testnum)/10)%10;
	vfd_wrdata(vfdmap[i]);
	

	i = (testnum)/100;
	vfd_wrdata(vfdmap[i]|(0x80));	// 1st char		

	vfd_delay(SPI_DELAY_TIME);	
	vfd_cmd(0x8f);              /*  ��ʾ��,14/16���� */	
	vfd_delay(SPI_DELAY_TIME);	
	led_stb_high();	
}

/****************
����led�ĸ�����λ�ĵ�ַ :

��һ��4��:  C1  C0  C3  C2 
�ڶ���6��:  C4  C5  C6  C7  C8  C9

// GRID 1  -- zhi neng
// GRID 2 -- fen ban
// GRID 3 -- hun ban
// GRID 4 -- ji ben
// GRID 12 -- yi bi - 0XCB
// GRID 11 -- wai bi - 0XCA

*****************/
/***********************************************************************************************
* Function		: LEDTestFunction
* Description	: LED��000 111 2222 .... 999 ��ʾ
* Input			: null
* Output		: null
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void LEDTestFunction1(INT16U testnum)// �ڲ���ʾ
{
	int i;
	
	vfd_cmd1(0x06);        /* 10digits��12segments��ʾģʽ  */
	vfd_cmd1(0x40);        /* ��ʾ����ģʽ�����ݵ�ַ����  */
	vfd_cmd1(0xC0);        // ���õ�ַ��00H��ʼ  
	i = (testnum)/100;
	vfd_wrdata1(vfdmap[0]);	// 2st char	
	
	vfd_wrdata1(vfdmap[1]);	// 1st char	
    vfd_wrdata1(vfdmap[2]);	// 4st char	
    vfd_wrdata1(vfdmap[3]);	// 3st char	
    vfd_wrdata1(vfdmap[4]);	// 5st char	
    vfd_wrdata1(vfdmap[5]);	// 6st char	
    vfd_wrdata1(vfdmap[6]);	// 7st char	
    vfd_wrdata1(vfdmap[7]);	// 8st char	
    vfd_wrdata1(vfdmap[8]);	// 9st char	
    vfd_wrdata1(vfdmap[9]);	// 10st char	
    vfd_wrdata1(vfdmap[10]);// 11st char	
                                        
        
	vfd_delay(SPI_DELAY_TIME);	
	vfd_cmd1(0x8f);              /*  ��ʾ��,14/16���� */	
	vfd_delay(SPI_DELAY_TIME);	
	led_outside_stb_high();	
}
extern INT16U u16_ybpage;
void vfd_wrtime1(void)// ����
{
	int i;
	INT16U u16_num=0;
	u16_num =u16_ybpage;// u16_LED_NUM;
	vfd_cmd1(0x06);                                      /*  10digits��12segments��ʾģʽ  */
	vfd_cmd1(0x40);                                      /* ��ʾ����ģʽ�����ݵ�ַ����  */
	vfd_cmd1(0xC0);                                   //    ���õ�ַ��00H��ʼ    
	
	#ifdef NEW_LED_PORT

	i = (u16_num)%10;
	vfd_wrdata1(vfdmap1[i]);

	i = ((u16_num)/10)%10;
	vfd_wrdata1(vfdmap1[i]);

	i = ((u16_num)/100)%10;// back_u16_ybpage
	vfd_wrdata1(vfdmap1[i]);	// 1st char	


	#else
	i = (u16_LED_NUM)/100;// back_u16_ybpage
	vfd_wrdata1(vfdmap1[i]);	// 1st char	
	
	vfd_wrdata1(0x0);
	i = ((u16_LED_NUM)/10)%10;
	vfd_wrdata1(vfdmap1[i]);
	vfd_wrdata1(0x0);
	i = (u16_LED_NUM)%10;
	vfd_wrdata1(vfdmap1[i]);
	vfd_wrdata1(0x0);
	#endif
	vfd_delay(SPI_DELAY_TIME);	
	vfd_cmd1(0x8f);              /*  ��ʾ��,14/16���� */	
	vfd_delay(SPI_DELAY_TIME);	
	led_stb_high();	
}

/****************
����led�ĸ�����λ�ĵ�ַ :

��һ��4��:  C1  C0  C3  C2 
�ڶ���6��:  C4  C5  C6  C7  C8  C9

// GRID 1  -- zhi neng
// GRID 2 -- fen ban
// GRID 3 -- hun ban
// GRID 4 -- ji ben
// GRID 12 -- yi bi - 0XCB
// GRID 11 -- wai bi - 0XCA

*****************/


INT16U led_blank = 0;
extern INT16U u16_ybpage;
extern INT16U u16_LED_NUM;
extern volatile INT32U TOTALMOMEY;

void vfd_wrtime(void)// ����
{
	int i;
	ta6932_init();
    

	return;
}
