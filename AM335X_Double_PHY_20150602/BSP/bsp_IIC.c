/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_iic.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 2014-07-14
**最  新  版  本: V0.1
**描          述: IIC时序程序
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "include.h"
#include "interrupt.h"
#include "edma_event.h"
#include "edma.h"
#include "AM335x_IRQ.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include 	"gpio.h"
#include 	"gpio_v2.h"
#include "cpu.h"
#include "bsp_int.h"
#include "bsp_iic.h"
#include "hw_cm_per.h"
#include    "bsp_gpio.h"
/* Private define-----------------------------------------------------------------------------*/
// IIC硬件配置
#define	SIIC_GPIO				PORT3
#define SIIC_GPIO_SCL			6u
#define SIIC_GPIO_SDA			5u

#define	IIC_DELAY_TIME			100
#define	IIC_DELAY_TIME_LONG		200

#define I2C_TIMEOUT                    1000u
#define I2C_DELAY_CYCLES	           100u
#define I2_INSTANCE_0                  0u
#define I2_INSTANCE_1                  1u
/* Private typedef----------------------------------------------------------------------------*/
typedef void (*I2CDELAY_FN)(INT32U delay);
volatile I2CDELAY_FN i2cDelayFn  = NULL;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
volatile static INT16U i2cDelayVal = I2C_DELAY_CYCLES;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
#if 0
/***********************************************************************************************
* Function		: 
* Description	: i/O模拟I2C 驱动
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20150317   wangyao  
***********************************************************************************************/
static void iic_delay(INT32U time)
{
	DelayUS(time);
}
static void iic_clk_high(void)
{
    GPIOPinWrite(SOC_GPIO_3_REGS, SIIC_GPIO_SCL,Bit_SET); 
	iic_delay(IIC_DELAY_TIME);
}
static void iic_clk_low(void)
{
	GPIOPinWrite(SOC_GPIO_3_REGS, SIIC_GPIO_SCL,Bit_RESET);  
	iic_delay(IIC_DELAY_TIME);
}
static void iic_data_set_in(void)
{
	_BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = SIIC_GPIO;
    GPIO_InitStructure.PinNum = SIIC_GPIO_SDA;
    GPIO_InitStructure.Dir = GPIO_DIR_INPUT;
    GPIO_InitStructure.IntType = GPIO_INT_TYPE_NO;
    //配置SDA
    BSP_GPIOConfig(&GPIO_InitStructure);
    
    // GPIODirModeSet(SOC_GPIO_3_REGS,
    //               SIIC_GPIO_SDA,
    //              GPIO_DIR_INPUT);
	
}
static void iic_data_set_out(void)
{
	_BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = SIIC_GPIO;
    GPIO_InitStructure.PinNum = SIIC_GPIO_SDA;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT;
    //配置SDA
    BSP_GPIOConfig(&GPIO_InitStructure);
    
}
static void iic_data_high(void)
{
	GPIOPinWrite(SOC_GPIO_3_REGS,SIIC_GPIO_SDA,Bit_SET);  
	iic_delay(IIC_DELAY_TIME);
}
static void iic_data_low(void)
{
	GPIOPinWrite(SOC_GPIO_3_REGS,SIIC_GPIO_SDA,Bit_RESET);  
	iic_delay(IIC_DELAY_TIME);
}
static INT8U iic_data_read(void)
{
	iic_delay(IIC_DELAY_TIME);
	return GPIOPinRead(SOC_GPIO_3_REGS,SIIC_GPIO_SDA);
}
// iic 初始化
void iic_init(void)
{ 
    _BSPGPIO_CONFIG GPIO_InitStructure;  

    GPIO_InitStructure.PortNum = SIIC_GPIO;
    GPIO_InitStructure.PinNum = SIIC_GPIO_SCL;
    GPIO_InitStructure.Dir = GPIO_DIR_OUTPUT; 
    BSP_GPIOConfig(&GPIO_InitStructure);
    
    iic_data_high();
	iic_data_set_out();
	iic_clk_high();
}
// iic 开始
void iic_start(void)
{ 
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_data_set_out();
    iic_data_high();
	iic_clk_high();
	iic_data_low();
	iic_clk_low();
}
// iic停止
void iic_stop()
{
	iic_clk_low();
	
	iic_data_set_out();
	iic_data_low();
	iic_clk_high();
	iic_data_high();
}
// 应答:数据接收不成功
void iic_noack()
{
	
	iic_data_set_out();
	iic_data_low();
	iic_clk_high();
}
// 应答:数据接驶成功
void iic_ack()
{
	iic_clk_low();
	iic_data_set_out();
	iic_data_high();
	iic_clk_high();

}
// 发送一个字节
INT8U iic_send_byte(INT8U val)
{
	INT8U i;
	
	iic_clk_low();
	iic_data_set_out();
	for(i=0;i<8;i++)
	{
		iic_clk_low();
		if(val&0x80)
		{
			iic_data_high();
		}
		else
		{
			iic_data_low();
		}
		iic_clk_high();
		//iic_clk_low();
		iic_delay(10);
		val = val<<1;
	}
	
	//iic_clk_high();
	iic_data_set_in();
    iic_delay(10);
	//iic_data_read();
	
	iic_clk_low();
	iic_clk_high();
	i=0;
	while(iic_data_read())// 等待应答ACK
	{
		//iic_clk_low();
		if(++i > 200)
		{
			
			return FALSE;
		}

	}
	//iic_clk_low();
	return TRUE;
}
// 接收一个字节
INT8U iic_rec_byte(void)
{
	INT8U val;
	INT8U i;
	
	val = 0;
	iic_clk_low();
	iic_data_set_in();							// SDA输入，必须要
	for(i=0;i<8;i++)
	{
		iic_delay(IIC_DELAY_TIME_LONG);
		iic_clk_high();
		val = val<<1;
		val |= iic_data_read();
		iic_clk_low();
	}
	return val;
}
// 接收一个16位的值
INT16U iic_rec_2byte()
{
	INT16U val;
	INT8U i;
	
	val = 0;
	iic_clk_low();
	iic_data_set_in();// SDA输入,必须要
	for(i=0;i<16;i++)
	{
		iic_clk_low();
		iic_clk_high();
        
        if(iic_data_read())
        {
          val = val<<1;
          val = val | 0x01;
        }
        else
        {
             val = val<<1;
        }
		//val = val<<1;
		//val |= iic_data_read();
		
	}
	return val;
}
#endif
/***********************************************************************************************
* Function		: i2cDelay
* Description	: I2C延时函数,目前只给I2C驱动用
* Input			: delay  
* Output		: 
* Note(s)		: 
* Contributor	: 王耀 2014-12-03
***********************************************************************************************/
static void i2cDelay(INT32U delay)
{
	volatile INT8U delVar = 0;
        
    if(i2cDelayFn )
    {
        (*i2cDelayFn)(delay);
    }       
    else
    {
        while(delay > 0)
        {
            delVar = 0;
    	    while(delVar < 50)
    	    {
                delVar++;
    	    }
    	    delay--;
        }
    }        
}

/***********************************************************************************************
* Function	: BSP_I2C0Init
* Description	: I2C0 初始化函数
* Input			:   
* Output		: 
* Note(s)		: 
* Contributor	: 王耀 2014-12-03
***********************************************************************************************/
void BSP_I2C0Init(void)
{
    I2C0ModuleClkConfig();

    I2CPinMuxSetup(0);

    /* Put i2c in reset/disabled state */
    I2CMasterDisable(SOC_I2C_0_REGS);

    /* Disable auto Idle functionality */
    I2CAutoIdleDisable(SOC_I2C_0_REGS);

    /* Configure i2c bus speed to 100khz */
    I2CMasterInitExpClk(SOC_I2C_0_REGS, 48000000, 12000000, 100000);

    /* Set i2c OWN address */
	I2COwnAddressSet(SOC_I2C_0_REGS, 0, I2C_OWN_ADDR_3);

    /* Bring I2C out of reset */
    I2CMasterEnable(SOC_I2C_0_REGS);
}

//I2C初始化留用原来的方式
#if 0 
/***********************************************************************************************
* Function		: BSP_PowerI2C
* Description	: I2C power config
* Input			: delay  
* Output		: 
* Note(s)		: 
* Contributor	: 王耀 2014-12-03
***********************************************************************************************/
void BSP_PowerI2C(INT8U  i2c_instance)
{
	if(I2_INSTANCE_1 == i2c_instance )
	{
		HWREG( SOC_PRCM_REGS + CM_PER_I2C1_CLKCTRL)      |= 0x2; 		// I2C1
	}
	else if(I2_INSTANCE_0 == i2c_instance )
	{
		HWREG( SOC_PRCM_REGS + CM_PER_SPI0_CLKCTRL)      |= 0x2; 		// I2C0
	}
}

/***********************************************************************************************
* Function		: BSP_I2CInit
* Description	: I2C初始化函数
* Input			: delay  
* Output		: 
* Note(s)		: 
* Contributor	: 王耀 2014-12-03
***********************************************************************************************/
INT16U BSP_I2CInit( INT8U instance)
{
    INT16U  retVal = 1;
    INT32U  i2c_base_addr = 0;
    
   	BSP_PowerI2C(instance);
    
	/* Assign the I2C register base */
	switch (instance)
	{
        case 0:
            I2C0ModuleClkConfig();	         
        	i2c_base_addr = SOC_I2C_0_REGS;
            break;
        case 1:
            I2C1ModuleClkConfig();
            i2c_base_addr = SOC_I2C_1_REGS;
            break;
        default:
            return 0;
    }

    I2CPinMuxSetup(instance);
    if (1 == retVal)
    {
        /*Setting Clock as Required for I2C to Work*/
        //clockInit(instance);
        /* Reset all the registers */
        HWREG(i2c_base_addr + I2C_CON) = 0;
        HWREG(i2c_base_addr + I2C_PSC) = 0;
        HWREG(i2c_base_addr + I2C_SCLL)= 0;
        HWREG(i2c_base_addr + I2C_SCLH)= 0;
        HWREG(i2c_base_addr + I2C_BUF) = 0;
        HWREG(i2c_base_addr + I2C_SYSC)= 0;
        HWREG(i2c_base_addr + I2C_WE)  = 0;
        HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW)= 0;
        HWREG(i2c_base_addr + I2C_SYSTEST)= 0;

        /* soft-reset the I2c Controller */
       HWREG(i2c_base_addr + I2C_SYSC) |= (I2C_SYSC_SRST & (I2C_SYSC_SRST_RESET <<I2C_SYSC_SRST_SHIFT));

       i2cDelay(i2cDelayVal);

       /* DISABLE THE I@C MODULE LIKE HOLD RESET*/
       HWREG(i2c_base_addr + I2C_CON) |= (I2C_CON_I2C_EN & (I2C_CON_I2C_EN_DISABLE <<I2C_CON_I2C_EN_SHIFT));

       /* IDLE DISABLING AND ON CLOCKS*/
       HWREG(i2c_base_addr + I2C_SYSC) |=
          /*NO IDLE MODE*/
          (I2C_SYSC_AUTOIDLE & (I2C_SYSC_AUTOIDLE_DISABLE
                    << I2C_SYSC_AUTOIDLE_SHIFT))
          |(I2C_SYSC_CLKACTIVITY  &
               (I2C_SYSC_CLKACTIVITY_BOTH<<
                   I2C_SYSC_CLKACTIVITY_SHIFT))
          |(I2C_SYSC_IDLEMODE  &
              (I2C_SYSC_IDLEMODE_NOIDLE<<
                  I2C_SYSC_IDLEMODE_SHIFT));
        /* I2C PRESCALE settings. */
        HWREG(i2c_base_addr + I2C_PSC) = 3;

        HWREG(i2c_base_addr + I2C_SCLL) |=
            (I2C_SCLL_SCLL &
                (33 <<
                    I2C_SCLL_SCLL_SHIFT));

        HWREG(i2c_base_addr + I2C_SCLH) |=
            (I2C_SCLH_SCLH &
                (23 <<
                    I2C_SCLH_SCLH_SHIFT));

        /* Disable  & clear all the Interrupts */
        HWREG(i2c_base_addr + I2C_IRQENABLE_CLR) |= 0x6FF;

        /* Set the self address */
        HWREG(i2c_base_addr + I2C_OA) |=
            (I2C_OA_OA & 0xCC);

        /* Enable the I2C controller */
        HWREG(i2c_base_addr + I2C_CON) |= I2C_CON_I2C_EN;

        /* set the slave address */
        HWREG(i2c_base_addr + I2C_SA) = (short) NULL;;

        /* Set data count */
        HWREG(i2c_base_addr + I2C_CNT) = (short) NULL;
    }
    return (retVal);
}

#endif //I2C初始化留用原来的方式
/***********************************************************************************************
* Function		: BSP_I2CWrite
* Description	: I2C写函数
* Input			:   
* Output		: 
* Note(s)		: 
* Contributor	: 王耀 2014-12-03
***********************************************************************************************/
INT16U BSP_I2CWrite(INT8U instance,INT16U slaveAdd,INT8U *buffer, INT8U  bufLen)
{
    INT16U retVal = 1;
    INT16U  status = 0;
    INT8U	bytesWritten = 0;
    INT32U	timeOut = I2C_TIMEOUT;
    INT32U  i2c_base_addr = 0;

    if ((NULL == buffer) ||  (0 == bufLen) )
    {
       retVal = 0;
       return (retVal);
    }
    /* Assign the I2C register base */
	switch (instance)
	{
        case 0:
            i2c_base_addr = SOC_I2C_0_REGS;
            break;
        case 1:
            i2c_base_addr = SOC_I2C_1_REGS;
            break;
        default:
            return 0;
    }

    /*  set the slave address */
    I2CMasterSlaveAddrSet(i2c_base_addr, slaveAdd);

    /* Configure I2C controller in Master Transmitter mode */
    I2CMasterControl(i2c_base_addr, I2C_CFG_MST_TX | I2C_CON_STP | I2C_CON_STT);

    I2CSetDataCount(i2c_base_addr, bufLen);
    
    /* Loop till we have written all the data or we have timed-out */
    for (timeOut = 0; timeOut < I2C_TIMEOUT; timeOut++)
    {

        status = HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW);

        /* Check for NACK*/
        if (0 == (status & I2C_IRQSTATUS_RAW_NACK))
        {
            /*  ACK received. Now check for data ready */
        	if (I2C_IRQSTATUS_RAW_AL_LOST !=
        	                (status & I2C_IRQSTATUS_RAW_AL))
        	{
        	    /* check for XRDY for data availability */
        	    if (I2C_IRQSTATUS_RAW_XRDY ==(status & I2C_IRQSTATUS_RAW_XRDY))
        	    {

        			/* Write the data and ACK */
        			HWREG(i2c_base_addr + I2C_DATA) =
        					(*buffer++ & I2C_DATA_DATA);
        			bytesWritten++;

        			/* clear the XRDY bits */
		            HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW) |= (I2C_IRQSTATUS_RAW_XRDY_CLEAR
 					                << I2C_IRQSTATUS_RAW_XRDY_SHIFT);
                    if (bytesWritten >= bufLen)
        			{
        				break;
        			}

        	    }
               	i2cDelay(i2cDelayVal);
            }
        	else
        	{
        	    retVal = 0;
                
        	    /* clear the AL bits */
        	    HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW) |=
        	                 I2C_IRQSTATUS_RAW_AL_CLEAR;
        	    break;
        	}


        	status =  HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW);

            /* if we have lost the arbitration */
            if (I2C_IRQSTATUS_RAW_AL_LOST ==
                (HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW) & I2C_IRQSTATUS_RAW_AL))
            {
                retVal = 0;

                /* clear the AL bits */
                HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW) |=
                    I2C_IRQSTATUS_RAW_AL_CLEAR;
                break;
            }

            if (I2C_IRQSTATUS_RAW_NACK == (HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW)
            		& I2C_IRQSTATUS_RAW_NACK))
            {
                retVal = 0;
                
                /* Clear that Bit */
                HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW) |=(I2C_IRQSTATUS_RAW_NACK_CLEAR
                    << I2C_IRQSTATUS_RAW_NACK_SHIFT);
                break;
            }
        }
        else
        {
            retVal = 0;

            /* clear the NACK bits */
            HWREG(i2c_base_addr + I2C_IRQSTATUS_RAW) = (I2C_IRQSTATUS_RAW_NACK_CLEAR
                << I2C_IRQSTATUS_RAW_NACK_SHIFT);
            break;
        }
    }
    if(timeOut >= I2C_TIMEOUT)
    {
        return 0;
    }
    return (retVal);
}
/***********************************************************************************************
* Function		: BSP_I2CRead
* Description	: I2C读函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 王耀 2014-12-03
***********************************************************************************************/
INT16U BSP_I2CRead(INT8U busNum,INT16U	slaveAddr,INT8U	*buffer,INT8U	bufLen,INT8U	*pDataRead)
{
    INT16U retVal = 1;
    INT16U status = 0;
    INT8U  bytesRead = 0;
    INT8U * pDataPtr = NULL; /*Pointer to save received data*/
    INT32U	timeOut = I2C_TIMEOUT;
    INT32U  baseI2c = 0;
    INT8U  previousDatalength = 0;

    /* input parameter validation */
    if ((NULL != buffer) && (0 != bufLen) && (NULL != pDataRead))
    {
        pDataPtr = buffer;
    }
    else
    {
        return 0;
    }
    

    /* Assign the I2C register base */
	switch (busNum)
	{
        case 0:
        	baseI2c = SOC_I2C_0_REGS;
            break;
        case 1:
        	baseI2c = SOC_I2C_1_REGS;
            break;
        default:
            return 0;
    }
   // HWREG(baseI2c + I2C_CON) =(0);
    timeOut = I2C_TIMEOUT;
    while(1)
    {
        status = HWREG(baseI2c + I2C_IRQSTATUS_RAW);
        if((status & I2C_IRQSTATUS_RAW_BB) == I2C_IRQSTATUS_RAW_BB_FREE)
        {
            break;
        }
        timeOut--;
        if(timeOut == 0)
        {
            return 0;
        }
        i2cDelay(i2cDelayVal);
    }
    /* configure the I2C bus as master for receiving mode */
    HWREG(baseI2c + I2C_CON) =
        (I2C_CON_I2C_EN)     // I2C Enable
        | (I2C_CON_MST)      // Master Mode
        | (I2C_CON_TRX_RCV)  // Receive Mode
        | (I2C_CON_STP)        // Stop
        | (I2C_CON_STT);       // Start

    /*  set the slave address */
    HWREG(baseI2c + I2C_SA) = slaveAddr & I2C_SA_SA;
    /* set the count */
    HWREG(baseI2c + I2C_CNT) = bufLen;

    i2cDelay(i2cDelayVal);
    /* in THIS loop till we have TO read all the data or we have timed-out,
     * read the data from I2C data register and return
     */

    for (timeOut = 0; timeOut < I2C_TIMEOUT; timeOut++)
    {
        status = HWREG(baseI2c + I2C_IRQSTATUS_RAW);

        /* Check for NACK*/
        if (0 == (status & I2C_IRQSTATUS_RAW_NACK))
        {
             /* if we have lost the arbitration */
            if (I2C_IRQSTATUS_RAW_AL_LOST !=
                     (status & I2C_IRQSTATUS_RAW_AL))
            {
                /* if we have lost the arbitration */

                   /* check for RDR for Last data availability */
                    if (I2C_IRQSTATUS_RAW_RDR == (status & I2C_IRQSTATUS_RAW_RDR ))
                    {
                        previousDatalength =  (HWREG(baseI2c + I2C_BUFSTAT) &
                            I2C_BUFSTAT_RXSTAT) >> I2C_BUFSTAT_RXSTAT_SHIFT;
                        while(previousDatalength > 0)
                        {
                            /* Previous data is available. Read it now and ACK */
                            *pDataPtr = (INT8U)(HWREG(baseI2c + I2C_DATA)) & I2C_DATA_DATA;
                            pDataPtr++;
                            bytesRead++;
                            bufLen++;
                            previousDatalength = previousDatalength - 1;
                        }
                        /* CLEAR RDR FLAG*/
                        HWREG(baseI2c + I2C_IRQSTATUS_RAW) |= (I2C_IRQSTATUS_RAW_RDR_CLEAR
                                                   << I2C_IRQSTATUS_RAW_RDR_SHIFT);
                    }
                    /*CHECKING FOR NEW DATA */
                    else if(I2C_IRQSTATUS_RAW_RRDY == (status & I2C_IRQSTATUS_RAW_RRDY ))
                    {

                        /* check if we have buffer space available */
                        if (bytesRead < bufLen)
                        {
                            /* data is available. Read it now and ACK */
                            *pDataPtr = (INT8U)(HWREG(baseI2c + I2C_DATA)) &
                                I2C_DATA_DATA;
                            pDataPtr++;
                            bytesRead++;
                            /* clear the RRDY bits */
                            HWREG(baseI2c + I2C_IRQSTATUS_RAW) |= (I2C_IRQSTATUS_RAW_RRDY_CLEAR
                               << I2C_IRQSTATUS_RAW_RRDY_SHIFT);
                        }
                        if(bytesRead >= bufLen)
                        {
                        	break;
                        }
                    i2cDelay(i2cDelayVal);
                    }


             }
            else
            {
                /* clear the AL bits */
                HWREG(baseI2c + I2C_IRQSTATUS_RAW) |=
                    I2C_IRQSTATUS_RAW_AL_CLEAR;
                /* configure the I2C bus as master for receiving mode */
                /* AFTER NACK STP AND STT ARE CLEARED BY HARDWARE*/

                HWREG(baseI2c + I2C_CON) =
                  (I2C_CON_I2C_EN)     // I2C Enable
                  | (I2C_CON_MST)        // Master Mode
                  | (I2C_CON_TRX_RCV << I2C_CON_TRX_SHIFT)        // Receive Mode
                  | (I2C_CON_STP)        // Stop
                  | (I2C_CON_STT);       // Start

                /*  set the slave address */
                HWREG(baseI2c + I2C_SA) = slaveAddr & I2C_SA_SA;

                /* set the count */
                HWREG(baseI2c + I2C_CNT) = bufLen;

                i2cDelay(i2cDelayVal);
                status = HWREG(baseI2c + I2C_CON);
                if((status & I2C_CON_STT) != I2C_CON_STT_STT )
                {
                    if(status & I2C_CON_STP)
                    {
                        break;
                    }
                    else
                    {
                       HWREG(baseI2c + I2C_CON) =
                             (I2C_CON_I2C_EN)     // I2C Enable
                             | (I2C_CON_MST)        // Master Mode
                             | (I2C_CON_TRX_RCV << I2C_CON_TRX_SHIFT)        // Receive Mode
                             | (I2C_CON_STP)        // Stop
                             | (I2C_CON_STT);       // Start
                    }
                }
            }
        }
        else
        {

            /* clear the NACK bits */
            HWREG(baseI2c + I2C_IRQSTATUS_RAW) |=
                (I2C_IRQSTATUS_RAW_NACK_CLEAR
                << I2C_IRQSTATUS_RAW_NACK_SHIFT);
            /* configure the I2C bus as master for receiving mode */
            /* AFTER NACK STP AND STT ARE CLEARED BY HARDWARE*/

            HWREG(baseI2c + I2C_CON) =
              (I2C_CON_I2C_EN)     // I2C Enable
              | (I2C_CON_MST)        // Master Mode
              | (I2C_CON_TRX_RCV << I2C_CON_TRX_SHIFT)        // Receive Mode
              | (I2C_CON_STP)        // Stop
              | (I2C_CON_STT);       // Start

            /*  set the slave address */
            HWREG(baseI2c + I2C_SA) = slaveAddr & I2C_SA_SA;

            /* set the count */
            HWREG(baseI2c + I2C_CNT) = bufLen;

            i2cDelay(i2cDelayVal);
            status = HWREG(baseI2c + I2C_CON);
            if((status & I2C_CON_STT) != I2C_CON_STT_STT )
            {
                if(status & I2C_CON_STP)
                {
                    break;
                }
                else
                {
                   HWREG(baseI2c + I2C_CON) =
                         (I2C_CON_I2C_EN)     // I2C Enable
                         | (I2C_CON_MST)        // Master Mode
                         | (I2C_CON_TRX_RCV << I2C_CON_TRX_SHIFT)        // Receive Mode
                         | (I2C_CON_STP)        // Stop
                         | (I2C_CON_STT);       // Start
                }
            }
        }
    }
    if(timeOut >= I2C_TIMEOUT)
    {
      return 0;
    }
    /* before returning, update the return counts too */
    if (NULL != pDataRead)
        *pDataRead = bytesRead;
    return (retVal);
}

/************************(C)COPYRIGHT 2010 浙江方泰*****END OF FILE****************************/
