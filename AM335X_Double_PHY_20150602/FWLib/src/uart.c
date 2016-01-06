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
 * \note    This pin multiplexing depends on ZVZ��װ
 *          ����PIN�Ÿ��ã�ֻ������UART0��UART2(RX_K18/TX_L18)
 */
void UARTPinMuxSetup(unsigned int instanceNum)
{
	switch(instanceNum)
	{
		case 0:
			/* RXD -- E16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RXD(0)) = 
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//�ܽŸ���0�Ź���	
            /* TXD -- E15 */
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_TXD(0)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(0));								//�ܽŸ���0�Ź���
            break;
		case 1:
			/* RXD -- D16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RXD(1)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//�ܽŸ���0�Ź���
            /* TXD -- D15 */
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_TXD(1)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(0));								//�ܽŸ���0�Ź���
            break;
		case 2:
			/* RXD -- K18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXCLK) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//�ܽŸ���1�Ź���
			/* TXD -- L18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXCLK) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//�ܽŸ���1�Ź���
            #if 0
            /* RXD -- A17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS0) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//�ܽŸ���1�Ź���
            /* RXD -- G17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_CLK) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//�ܽŸ���3�Ź���
            /* RXD -- H17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_CRS) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(6));	//�ܽŸ���6�Ź���
                
			/* TXD -- B17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_D0) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//�ܽŸ���1�Ź���
            /* TXD -- G18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_CMD) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//�ܽŸ���3�Ź���
            /* TXD -- J15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXERR) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(6));								//�ܽŸ���6�Ź���
            #endif
            break;
		case 3:
			/* RXD -- C15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS1) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//�ܽŸ���1�Ź���
			/* TXD -- C18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_ECAP0_IN_PWM0_OUT) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//�ܽŸ���1�Ź���
            #if 0
            /* RXD -- G15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_DAT1) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//�ܽŸ���3�Ź���
            /* RXD -- L17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD3) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//�ܽŸ���1�Ź���
                
			/* TXD -- G16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MMC0_DAT0) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//�ܽŸ���3�Ź���
            /* TXD -- L16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD2) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//�ܽŸ���1�Ź���
            #endif
            break;
		case 4:
			/* RXD -- E18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_CTSN(0)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1));	//�ܽŸ���1�Ź���
			/* TXD -- E17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RTSN(0)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(1));								//�ܽŸ���1�Ź���
            #if 0
            /* RXD -- J18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD3) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//�ܽŸ���3�Ź���
            /* RXD -- T17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_WAIT0) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(6));	//�ܽŸ���6�Ź���
                
			/* TXD -- K15 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD2) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//�ܽŸ���3�Ź���
            /* TXD -- U17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_WPN) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(6));								//�ܽŸ���6�Ź���
            #endif
            break;
		case 5:
			/* RXD -- H16 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_COL) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3));	//�ܽŸ���3�Ź���
			/* TXD -- H18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_RMII1_REFCLK) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//�ܽŸ���3�Ź���
            #if 0
            /* RXD -- M17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_DATA) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(2));	//�ܽŸ���2�Ź���
            /* RXD -- U2 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_LCD_DATA(9)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//�ܽŸ���4�Ź���
            /* RXD -- V4 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_LCD_DATA(14)) =
                (CONTROL_CONF_PULLUPSEL |  CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//�ܽŸ���4�Ź���
                
			/* TXD -- J17 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXDV) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(3));								//�ܽŸ���3�Ź���
            /* TXD -- M18 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_CLK) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(2));								//�ܽŸ���2�Ź���
            /* TXD -- U1 */
			HWREG(SOC_CONTROL_REGS + CONTROL_CONF_LCD_DATA(8)) = 
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_MUXMODE(4));								//�ܽŸ���4�Ź���
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
		          CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE;			//-- 1 --����ʱ��ģ��CM_PER��CM_PER_L3_CLKCTRL�Ĵ���MODULEMODE��λ[1:0]д0x2,ʹ�ܻ���ʱ��				
		    while(CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) &
		           CM_PER_L3_CLKCTRL_MODULEMODE));				//ȷ��MODULEMODE�Ѿ�ʹ��
			
		    HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) |=
		          CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE;	//-- 2 --����ʱ��ģ��CM_PER��CM_PER_L3_INSTR_CLKCTRL�Ĵ���MODULEMODE��λ[1:0]д0x2,ʹ��INSTRʱ��
		    while(CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) &
		           CM_PER_L3_INSTR_CLKCTRL_MODULEMODE));		//ȷ��MODULEMODE�Ѿ�ʹ��

		    HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) |=
		          CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;		//-- 3 --����ʱ��ģ��CM_PER��CM_PER_L3_CLKSTCTRL�Ĵ���CLKTRCTRL��λ[1:0]д0x2,SW_WKUP

		    while(CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
		           CM_PER_L3_CLKSTCTRL_CLKTRCTRL));				//ȷ��CLKTRCTRL�Ѿ�д��

		    HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) |=
		          CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;	//-- 4 --����ʱ��ģ��CM_PER��CM_PER_OCPWP_L3_CLKSTCTRL�Ĵ���CLKTRCTRL��λ[1:0]д0x2,SW_WKUP
		    while(CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
		           CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL));		//ȷ��CLKTRCTRL�Ѿ�д��

		    HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) |=
		          CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP;		//-- 5 --����ʱ��ģ��CM_PER��CM_PER_L3S_CLKSTCTRL�Ĵ���CLKTRCTRL��λ[1:0]д0x2,SW_WKUP
		    while(CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
		           CM_PER_L3S_CLKSTCTRL_CLKTRCTRL));			//ȷ��CLKTRCTRL�Ѿ�д��

			
		    /* Checking fields for necessary values.  */
		    while((CM_PER_L3_CLKCTRL_IDLEST_FUNC << CM_PER_L3_CLKCTRL_IDLEST_SHIFT)!=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) &
		           CM_PER_L3_CLKCTRL_IDLEST));								//-- 1 --�������ʱ��ģ��CM_PER��CM_PER_L3_CLKCTRL�Ĵ���IDLEST״̬λ
			
		    while((CM_PER_L3_INSTR_CLKCTRL_IDLEST_FUNC <<
		           CM_PER_L3_INSTR_CLKCTRL_IDLEST_SHIFT)!=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) &
		           CM_PER_L3_INSTR_CLKCTRL_IDLEST));						//-- 2 --�������ʱ��ģ��CM_PER��CM_PER_L3_INSTR_CLKCTRL�Ĵ���IDLEST״̬λ

		    while(CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
		           CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK));				//-- 3 --�������ʱ��ģ��CM_PER��CM_PER_L3_CLKSTCTRL�Ĵ���CLKACTIVITY_L3_GCLK״̬λ
			
		    while(CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
		           CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK));	//-- 4 --�������ʱ��ģ��CM_PER��CM_PER_OCPWP_L3_CLKSTCTRL�Ĵ���CLKACTIVITY_OCPWP_L3_GCLK״̬λ

		    while(CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK !=
		          (HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
		          CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK));				//-- 5 --�������ʱ��ģ��CM_PER��CM_PER_L3S_CLKSTCTRL�Ĵ���CLKACTIVITY_L3S_GCLK״̬λ


		    /* Configuring registers related to Wake-Up region. */
		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CONTROL_CLKCTRL) |=
		          CM_WKUP_CONTROL_CLKCTRL_MODULEMODE_ENABLE;				//-- 1 --����ʱ��ģ��CM_WKUP��CM_WKUP_CONTROL_CLKCTRL�Ĵ���MODULEMODE��λ[1:0]д0x2
		    while(CM_WKUP_CONTROL_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CONTROL_CLKCTRL) &
		           CM_WKUP_CONTROL_CLKCTRL_MODULEMODE));					//ȷ��MODULEMODE�Ѿ�д��
			
		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) |=
		          CM_WKUP_CLKSTCTRL_CLKTRCTRL_SW_WKUP;						//-- 2 --����ʱ��ģ��CM_WKUP��CM_PER_L3S_CLKSTCTRL�Ĵ���CLKTRCTRL��λ[1:0]д0x2
		    while(CM_WKUP_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
		           CM_WKUP_CLKSTCTRL_CLKTRCTRL));							//ȷ��CLKTRCTRL�Ѿ�д��

		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L3_AON_CLKSTCTRL) |=
		          CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKTRCTRL_SW_WKUP;			//-- 3 --����ʱ��ģ��CM_WKUP��CM_L3_AON_CLKSTCTRL�Ĵ���CLKTRCTRL��λ[1:0]д0x2
		    while(CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L3_AON_CLKSTCTRL) &
		           CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKTRCTRL));					//ȷ��CLKTRCTRL�Ѿ�д��

		    HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL) |=
		          CM_WKUP_UART0_CLKCTRL_MODULEMODE_ENABLE;					//-- 4 --����ʱ��ģ��CM_WKUP��CM_WKUP_UART0_CLKCTRL�Ĵ���MODULEMODE��λ[1:0]д0x2,UART0ʱ��ʹ��
		    while(CM_WKUP_UART0_CLKCTRL_MODULEMODE_ENABLE !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL) &
		           CM_WKUP_UART0_CLKCTRL_MODULEMODE));						//ȷ��MODULEMODE�Ѿ�д��

		    /* Verifying if the other bits are set to required settings. */
		    while((CM_WKUP_CONTROL_CLKCTRL_IDLEST_FUNC <<
		           CM_WKUP_CONTROL_CLKCTRL_IDLEST_SHIFT) !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CONTROL_CLKCTRL) &
		           CM_WKUP_CONTROL_CLKCTRL_IDLEST));								//-- 1 --��⻽��ʱ��ģ��CM_WKUP��CM_WKUP_CONTROL_CLKCTRL�Ĵ���IDLEST״̬λ

		    while(CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L3_AON_CLKSTCTRL) &
		           CM_WKUP_CM_L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK));			//-- 2 --��⻽��ʱ��ģ��CM_WKUP��CM_L3_AON_CLKSTCTRL�Ĵ���CLKACTIVITY_L3_AON_GCLK״̬λ
		           
		    while((CM_WKUP_L4WKUP_CLKCTRL_IDLEST_FUNC <<
		           CM_WKUP_L4WKUP_CLKCTRL_IDLEST_SHIFT) !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_L4WKUP_CLKCTRL) &
		           CM_WKUP_L4WKUP_CLKCTRL_IDLEST));									//-- 3 --��⻽��ʱ��ģ��CM_WKUP��CM_WKUP_L4WKUP_CLKCTRL�Ĵ���IDLEST״̬λ

		    while(CM_WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
		           CM_WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK));					//-- 4 --��⻽��ʱ��ģ��CM_WKUP��CM_WKUP_CLKSTCTRL�Ĵ���CLKACTIVITY_L4_WKUP_GCLK״̬λ
			
		    while(CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL) &
		           CM_WKUP_CM_L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK));	//-- 5 --��⻽��ʱ��ģ��CM_WKUP��CM_L4_WKUP_AON_CLKSTCTRL�Ĵ���CLKACTIVITY_L4_WKUP_AON_GCLK״̬λ

		    while(CM_WKUP_CLKSTCTRL_CLKACTIVITY_UART0_GFCLK !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_CLKSTCTRL) &
		           CM_WKUP_CLKSTCTRL_CLKACTIVITY_UART0_GFCLK));						//-- 6 --��⻽��ʱ��ģ��CM_WKUP��CM_WKUP_CLKSTCTRL�Ĵ���CLKACTIVITY_UART0_GFCLK״̬λ
			
		    while((CM_WKUP_UART0_CLKCTRL_IDLEST_FUNC <<
		           CM_WKUP_UART0_CLKCTRL_IDLEST_SHIFT) !=
		          (HWREG(SOC_CM_WKUP_REGS + CM_WKUP_UART0_CLKCTRL) &	
		           CM_WKUP_UART0_CLKCTRL_IDLEST));									//-- 7 --��⻽��ʱ��ģ��CM_WKUP��CM_WKUP_UART0_CLKCTRL�Ĵ���IDLEST״̬λ
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
		    	case 1:																	//ʹ��UART1ʱ�ӼĴ���
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART1_CLKCTRL) |=
		    							CM_PER_UART1_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART1_CLKCTRL) &
		    			CM_PER_UART1_CLKCTRL_MODULEMODE) != CM_PER_UART1_CLKCTRL_MODULEMODE_ENABLE);
		    		break;		    	
		    	case 2:																	//ʹ��UART2ʱ�ӼĴ���
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART2_CLKCTRL) |=
		    							CM_PER_UART2_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART2_CLKCTRL) &
		    			CM_PER_UART2_CLKCTRL_MODULEMODE) != CM_PER_UART2_CLKCTRL_MODULEMODE_ENABLE);		    	
		    		break;		    	
		    	case 3:																	//ʹ��UART3ʱ�ӼĴ���
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART3_CLKCTRL) |=
		    							CM_PER_UART3_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART3_CLKCTRL) &
		    			CM_PER_UART3_CLKCTRL_MODULEMODE) != CM_PER_UART3_CLKCTRL_MODULEMODE_ENABLE);
		    		break;		    	
		    	case 4:																	//ʹ��UART4ʱ�ӼĴ���
		    		HWREG(SOC_PRCM_REGS + CM_PER_UART4_CLKCTRL) |=
		    							CM_PER_UART4_CLKCTRL_MODULEMODE_ENABLE;
		    		while((HWREG(SOC_PRCM_REGS + CM_PER_UART4_CLKCTRL) &
		    			CM_PER_UART4_CLKCTRL_MODULEMODE) != CM_PER_UART4_CLKCTRL_MODULEMODE_ENABLE);
		    		break;
		    	case 5:																	//ʹ��UART5ʱ�ӼĴ���
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
		            CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_UART_GFCLK)));			//UART1~5�˴�������ͬ
			break;
		default:
			break;
	}
}

/****************************** End of file *********************************/
