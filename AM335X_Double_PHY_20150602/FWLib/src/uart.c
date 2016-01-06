/*
 * \file   uart.c
 *
 * \brief  This file contains functions which does the platform specific
 *         configurations for UART.
 */

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
*/
/*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#include "hw_control_AM335x.h"
#include "soc_AM335x.h"
#include "hw_cm_wkup.h"
#include "hw_cm_per.h"
#include "beaglebone.h"
#include "hw_types.h"

/**
 * \brief   This function selects the UART pins for use. The UART pins
 *          are multiplexed with pins of other peripherals in the SoC
 *          
 * \param   instanceNum       The instance number of the UART to be used.
 *
 * \return  None.
 *
 * \note    This pin multiplexing depends on ZVZ封装
 *          以下PIN脚复用，只测试了UART0、UART2(RX_K18/TX_L18)
 */
void UARTPinMuxSetup(unsigned int instanceNum)
{
	switch(instanceNum)
	{
		case 0:
			/* RXD -- E16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RXD(0)) = 
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//管脚复用0号功能	
            /* TXD -- E15 */
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_TXD(0)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(0));								//管脚复用0号功能
            break;
		case 1:
			/* RXD -- D16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RXD(1)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//管脚复用0号功能
            /* TXD -- D15 */
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_TXD(1)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(0));								//管脚复用0号功能
            break;
		case 2:
			/* RXD -- K18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXCLK) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//管脚复用1号功能
			/* TXD -- L18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXCLK) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//管脚复用1号功能
            #if 0
            /* RXD -- A17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS0) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//管脚复用1号功能
            /* RXD -- G17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_CLK) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//管脚复用3号功能
            /* RXD -- H17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_CRS) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(6));	//管脚复用6号功能
                
			/* TXD -- B17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_D0) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//管脚复用1号功能
            /* TXD -- G18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_CMD) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//管脚复用3号功能
            /* TXD -- J15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXERR) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(6));								//管脚复用6号功能
            #endif
            break;
		case 3:
			/* RXD -- C15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS1) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//管脚复用1号功能
			/* TXD -- C18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_ECAP0_IN_PWM0_OUT) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//管脚复用1号功能
            #if 0
            /* RXD -- G15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_DAT1) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//管脚复用3号功能
            /* RXD -- L17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD3) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//管脚复用1号功能
                
			/* TXD -- G16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_DAT0) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//管脚复用3号功能
            /* TXD -- L16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD2) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//管脚复用1号功能
            #endif
            break;
		case 4:
			/* RXD -- E18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_CTSN(0)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//管脚复用1号功能
			/* TXD -- E17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RTSN(0)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//管脚复用1号功能
            #if 0
            /* RXD -- J18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD3) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//管脚复用3号功能
            /* RXD -- T17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_WAIT0) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(6));	//管脚复用6号功能
                
			/* TXD -- K15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD2) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//管脚复用3号功能
            /* TXD -- U17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_WPN) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(6));								//管脚复用6号功能
            #endif
            break;
		case 5:
			/* RXD -- H16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_COL) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//管脚复用3号功能
			/* TXD -- H18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_RMII1_REFCLK) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//管脚复用3号功能
            #if 0
            /* RXD -- M17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_DATA) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(2));	//管脚复用2号功能
            /* RXD -- U2 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_LCD_DATA(9)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//管脚复用4号功能
            /* RXD -- V4 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_LCD_DATA(14)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//管脚复用4号功能
                
			/* TXD -- J17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXDV) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//管脚复用3号功能
            /* TXD -- M18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_CLK) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(2));								//管脚复用2号功能
            /* TXD -- U1 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_LCD_DATA(8)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(4));								//管脚复用4号功能
            #endif
            break;
		default:
            break;
	}
}

/*
** This function enables the system L3 and system L4_WKUP clocks.
** This also enables the clocks for UART0 instance.
** note: UART1~5 don't support device wake-up feature
*/

void UARTModuleClkConfig(unsigned int instanceNum )
{
	switch(instanceNum)
	{
		case 0:
		    /* Configuring L3 Interface Clocks. */
		    HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) |=
		          CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE;			//-- 1 --外设时钟模块CM_PER中CM_PER_L3_CLKCTRL寄存器MODULEMODE段位[1:0]写0x2,使能互联时钟				
		    while(CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) &
		           CM_PER_L3_CLKCTRL_MODULEMODE));				//确认MODULEMODE已经使能
			
		    HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) |=
		          CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE;	//-- 2 --外设时钟模块CM_PER中CM_PER_L3_INSTR_CLKCTRL寄存器MODULEMODE段位[1:0]写0x2,使能INSTR时钟
		    while(CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) &
		           CM_PER_L3_INSTR_CLKCTRL_MODULEMODE));		//确认MODULEMODE已经使能

		    HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) |=
		          CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;		//-- 3 --外设时钟模块CM_PER中CM_PER_L3_CLKSTCTRL寄存器CLKTRCTRL段位[1:0]写0x2,SW_WKUP

		    while(CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
		           CM_PER_L3_CLKSTCTRL_CLKTRCTRL));				//确认CLKTRCTRL已经写入

		    HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) |=
		          CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;	//-- 4 --外设时钟模块CM_PER中CM_PER_OCPWP_L3_CLKSTCTRL寄存器CLKTRCTRL段位[1:0]写0x2,SW_WKUP
		    while(CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
		           CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL));		//确认CLKTRCTRL已经写入

		    HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) |=
		          CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP;		//-- 5 --外设时钟模块CM_PER中CM_PER_L3S_CLKSTCTRL寄存器CLKTRCTRL段位[1:0]写0x2,SW_WKUP
		    while(CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
		           CM_PER_L3S_CLKSTCTRL_CLKTRCTRL));			//确认CLKTRCTRL已经写入

			
		    /* Checking fields for necessary values.  */
		    while((CM_PER_L3_CLKCTRL_IDLEST_FUNC << CM_PER_L3_CLKCTRL_IDLEST_SHIFT)!=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) &
		           CM_PER_L3_CLKCTRL_IDLEST));								//-- 1 --检测外设时钟模块CM_PER中CM_PER_L3_CLKCTRL寄存器IDLEST状态位
			
		    while((CM_PER_L3_INSTR_CLKCTRL_IDLEST_FUNC <<
		           CM_PER_L3_INSTR_CLKCTRL_IDLEST_SHIFT)!=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) &
		           CM_PER_L3_INSTR_CLKCTRL_IDLEST));						//-- 2 --检测外设时钟模块CM_PER中CM_PER_L3_INSTR_CLKCTRL寄存器IDLEST状态位

		    while(CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
		           CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK));				//-- 3 --检测外设时钟模块CM_PER中CM_PER_L3_CLKSTCTRL寄存器CLKACTIVITY_L3_GCLK状态位
			
		    while(CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
		           CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK));	//-- 4 --检测外设时钟模块CM_PER中CM_PER_OCPWP_L3_CLKSTCTRL寄存器CLKACTIVITY_OCPWP_L3_GCLK状态位

		    while(CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
		          CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK));				//-- 5 --检测外设时钟模块CM_PER中CM_PER_L3S_CLKSTCTRL寄存器CLKACTIVITY_L3S_GCLK状态位


		    /* Configuring registers related to Wake-Up region. */
		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CONTROL_CLKCTRL) |=
		          CM_WKUP_CONTROL_CLKCTRL_MODULEMODE_ENABLE;				//-- 1 --唤醒时钟模块CM_WKUP中CM_WKUP_CONTROL_CLKCTRL寄存器MODULEMODE段位[1:0]写0x2
		    while(CM_WKUP_CONTROL_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CONTROL_CLKCTRL) &
		           CM_WKUP_CONTROL_CLKCTRL_MODULEMODE));					//确认MODULEMODE已经写入
			
		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) |=
		          CM_WKUP_CLKSTCTRL_CLKTRCTRL_SW_WKUP;						//-- 2 --唤醒时钟模块CM_WKUP中CM_PER_L3S_CLKSTCTRL寄存器CLKTRCTRL段位[1:0]写0x2
		    while(CM_WKUP_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
		           CM_WKUP_CLKSTCTRL_CLKTRCTRL));							//确认CLKTRCTRL已经写入

		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L3_AON_CLKSTCTRL) |=
		          CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKTRCTRL_SW_WKUP;			//-- 3 --唤醒时钟模块CM_WKUP中CM_L3_AON_CLKSTCTRL寄存器CLKTRCTRL段位[1:0]写0x2
		    while(CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L3_AON_CLKSTCTRL) &
		           CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKTRCTRL));					//确认CLKTRCTRL已经写入

		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL) |=
		          CM_WKUP_UART0_CLKCTRL_MODULEMODE_ENABLE;					//-- 4 --唤醒时钟模块CM_WKUP中CM_WKUP_UART0_CLKCTRL寄存器MODULEMODE段位[1:0]写0x2,UART0时钟使能
		    while(CM_WKUP_UART0_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL) &
		           CM_WKUP_UART0_CLKCTRL_MODULEMODE));						//确认MODULEMODE已经写入

		    /* Verifying if the other bits are set to required settings. */
		    while((CM_WKUP_CONTROL_CLKCTRL_IDLEST_FUNC <<
		           CM_WKUP_CONTROL_CLKCTRL_IDLEST_SHIFT) !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CONTROL_CLKCTRL) &
		           CM_WKUP_CONTROL_CLKCTRL_IDLEST));								//-- 1 --检测唤醒时钟模块CM_WKUP中CM_WKUP_CONTROL_CLKCTRL寄存器IDLEST状态位

		    while(CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L3_AON_CLKSTCTRL) &
		           CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK));			//-- 2 --检测唤醒时钟模块CM_WKUP中CM_L3_AON_CLKSTCTRL寄存器CLKACTIVITY_L3_AON_GCLK状态位
		           
		    while((CM_WKUP_L4WKUP_CLKCTRL_IDLEST_FUNC <<
		           CM_WKUP_L4WKUP_CLKCTRL_IDLEST_SHIFT) !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_L4WKUP_CLKCTRL) &
		           CM_WKUP_L4WKUP_CLKCTRL_IDLEST));									//-- 3 --检测唤醒时钟模块CM_WKUP中CM_WKUP_L4WKUP_CLKCTRL寄存器IDLEST状态位

		    while(CM_WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
		           CM_WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK));					//-- 4 --检测唤醒时钟模块CM_WKUP中CM_WKUP_CLKSTCTRL寄存器CLKACTIVITY_L4_WKUP_GCLK状态位
			
		    while(CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL) &
		           CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK));	//-- 5 --检测唤醒时钟模块CM_WKUP中CM_L4_WKUP_AON_CLKSTCTRL寄存器CLKACTIVITY_L4_WKUP_AON_GCLK状态位

		    while(CM_WKUP_CLKSTCTRL_CLKACTIVITY_UART0_GFCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
		           CM_WKUP_CLKSTCTRL_CLKACTIVITY_UART0_GFCLK));						//-- 6 --检测唤醒时钟模块CM_WKUP中CM_WKUP_CLKSTCTRL寄存器CLKACTIVITY_UART0_GFCLK状态位
			
		    while((CM_WKUP_UART0_CLKCTRL_IDLEST_FUNC <<
		           CM_WKUP_UART0_CLKCTRL_IDLEST_SHIFT) !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL) &	
		           CM_WKUP_UART0_CLKCTRL_IDLEST));									//-- 7 --检测唤醒时钟模块CM_WKUP中CM_WKUP_UART0_CLKCTRL寄存器IDLEST状态位
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			HWREG(SOC_PRCM_REGS + CM_PER_L3S_CLKSTCTRL) |= 
                             			CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP;				//-- 1 --CM_PER_L3S_CLKSTCTRL
		    while((HWREG(SOC_PRCM_REGS + CM_PER_L3S_CLKSTCTRL) & 
		     CM_PER_L3S_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
		
		    HWREG(SOC_PRCM_REGS + CM_PER_L3_CLKSTCTRL) |= 
		                             CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;					//-- 2 --CM_PER_L3_CLKSTCTRL
		    while((HWREG(SOC_PRCM_REGS + CM_PER_L3_CLKSTCTRL) & 
		     CM_PER_L3_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
		
		    HWREG(SOC_PRCM_REGS + CM_PER_L3_INSTR_CLKCTRL) |= 
		                             CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE;				//-- 3 --CM_PER_L3_INSTR_CLKCTRL
		    while((HWREG(SOC_PRCM_REGS + CM_PER_L3_INSTR_CLKCTRL) & 
		                               CM_PER_L3_INSTR_CLKCTRL_MODULEMODE) != 
		                                   CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE);
		
		    HWREG(SOC_PRCM_REGS + CM_PER_L3_CLKCTRL) |= 
		                             CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE;					//-- 4 --CM_PER_L3_CLKCTRL
		    while((HWREG(SOC_PRCM_REGS + CM_PER_L3_CLKCTRL) & 
		        CM_PER_L3_CLKCTRL_MODULEMODE) != CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE);
		
		    HWREG(SOC_PRCM_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) |= 
		                             CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;			//-- 5 --CM_PER_OCPWP_L3_CLKSTCTRL
		    while((HWREG(SOC_PRCM_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) & 
		                              CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL) != 
		                                CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
		
		    HWREG(SOC_PRCM_REGS + CM_PER_L4LS_CLKSTCTRL) |= 
		                             CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP;				//-- 6 --CM_PER_L4LS_CLKSTCTRL
		    while((HWREG(SOC_PRCM_REGS + CM_PER_L4LS_CLKSTCTRL) & 
		                             CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL) != 
		                               CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
		
		    HWREG(SOC_PRCM_REGS + CM_PER_L4LS_CLKCTRL) |= 
		                             CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE;					//-- 7 --CM_PER_L4LS_CLKCTRL
		    while((HWREG(SOC_PRCM_REGS + CM_PER_L4LS_CLKCTRL) & 
		      CM_PER_L4LS_CLKCTRL_MODULEMODE) != CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE);
		      
		    switch(instanceNum)
		    {
		    	case 1:																	//使能UART1时钟寄存器
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART1_CLKCTRL) |=
		    							CM_PER_UART1_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART1_CLKCTRL) &
		    			CM_PER_UART1_CLKCTRL_MODULEMODE) != CM_PER_UART1_CLKCTRL_MODULEMODE_ENABLE);
		    		break;		    	
		    	case 2:																	//使能UART2时钟寄存器
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART2_CLKCTRL) |=
		    							CM_PER_UART2_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART2_CLKCTRL) &
		    			CM_PER_UART2_CLKCTRL_MODULEMODE) != CM_PER_UART2_CLKCTRL_MODULEMODE_ENABLE);		    	
		    		break;		    	
		    	case 3:																	//使能UART3时钟寄存器
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART3_CLKCTRL) |=
		    							CM_PER_UART3_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART3_CLKCTRL) &
		    			CM_PER_UART3_CLKCTRL_MODULEMODE) != CM_PER_UART3_CLKCTRL_MODULEMODE_ENABLE);
		    		break;		    	
		    	case 4:																	//使能UART4时钟寄存器
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART4_CLKCTRL) |=
		    							CM_PER_UART4_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART4_CLKCTRL) &
		    			CM_PER_UART4_CLKCTRL_MODULEMODE) != CM_PER_UART4_CLKCTRL_MODULEMODE_ENABLE);
		    		break;
		    	case 5:																	//使能UART5时钟寄存器
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART5_CLKCTRL) |=
		    							CM_PER_UART5_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART5_CLKCTRL) &
		    			CM_PER_UART5_CLKCTRL_MODULEMODE) != CM_PER_UART5_CLKCTRL_MODULEMODE_ENABLE);	
		    		break;
		    	default:
		    		break;		    		
		    }
		    		    
		    while(!(HWREG(SOC_PRCM_REGS + CM_PER_L3S_CLKSTCTRL) & 
            		CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK));

		    while(!(HWREG(SOC_PRCM_REGS + CM_PER_L3_CLKSTCTRL) & 
		            CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK));
		
		    while(!(HWREG(SOC_PRCM_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) & 
		           (CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK | 
		            CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK)));
		            
			while(!(HWREG(SOC_PRCM_REGS + CM_PER_L4LS_CLKSTCTRL) & 
		           (CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_L4LS_GCLK | 
		            CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_UART_GFCLK)));			//UART1~5此处配置相同
			break;
		default:
			break;
	}
}

/****************************** End of file *********************************/
