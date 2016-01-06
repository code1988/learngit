/**
 * \file   mcspi.c
 *
 * \brief  This file contains functions which configure the pin muxing and
 *         module clock configurations for mcspi peripheral.
 *
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
#include "evmAM335x.h"
#include "hw_cm_per.h"
#include "hw_types.h"
#include "mcspi.h"
#include "error.h"
/****************************************************************************
**                       MACRO DEFINITIONS
****************************************************************************/

/* Supported McSPI instance. */
#define MCSPI_INSTANCE                 (0)

/* EVM Profile setting for McSPI */
#define MCSPI_EVM_PROFILE              (2)


/**
 * \brief   This function selects the McSPI pins for use. The McSPI pins
 *          are multiplexed with pins of other peripherals in the SoC
 *
 * \param   instanceNum       The instance number of the McSPI instance to be
 *                            used.
 * \return  Returns the value S_PASS if the desired functionality is met else
 *          returns the appropriate error value. Error values can be
 *          1) E_INST_NOT_SUPP - McSPI instance not supported
 *          2) E_INVALID_PROFILE - Invalid profile setting of EVM for McSPI
 *
 * \note    This muxing depends on the profile in which the EVM is configured.
 */
int McSPIPinMuxSetup(unsigned int instanceNum)
{
    int status = E_INST_NOT_SUPP;

    switch (instanceNum)
    {
        case 0:																					//SPI0
        case 1:
          HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_SCLK) =
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));		//A17	
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_D0) =
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));		//B17
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_D1) =
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));		//B16
            status = S_PASS;
            break;
        case 2:																					//SPI1
        	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_ECAP0_IN_PWM0_OUT) =
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));		//C18
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_CTSN(0)) =
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));		//E18
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RTSN(0)) =
                (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));		//E17
            status = S_PASS;
            break;
        default:
            status = E_INVALID_PROFILE;
            break;
    }    
    return status;
}

/**
 * \brief   This function selects the McSPI CS's for use. The McSPI CS pins
 *          are multiplexed with pins of other peripherals in the SoC
 *
 * \param   csPinNum       The Chip select of the McSPI instance to be
 *                         used.
 * \return  Returns the value S_PASS if the desired functionality is met else
 *          returns the appropriate error value. Error values can be
 *          1) E_INVALID_CHIP_SEL - Chip select not supported
 *          2) E_INVALID_PROFILE - Invalid profile setting of EVM for McSPI
 *
 * \note    This muxing depends on the profile in which the EVM is configured.
            将CS0/CS1全部打开,其中CS1片选管脚有多路可选，这里采用缺省情况D18/D17    20150119
 */
 void McSPICSPinMuxSetup(unsigned int instanceNum)
{
    if((0 == instanceNum)||(1 == instanceNum))
    {
        HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS0) =
                     (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//A16
        HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS1) =
                     (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//C15
    }
    else if(2 == instanceNum)
    {
        //HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_CTSN(1)) =
        //             (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//D18
        HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RTSN(1)) =
                     (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//D17
    }
}
 #if 0
void McSPICSPinMuxSetup(unsigned int instanceNum,unsigned int csPinNum)
{
    if((0 == instanceNum)||(1 == instanceNum))
    {

        switch(csPinNum)
        {
            case 0:
            	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS0) =
                     (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//A16
                break;
            case 1:
                HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS1) =
                     (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(0));	//C15
            	break;
            default:
				break;
        }
    }
    else if(2 == instanceNum) 
    {
		switch(csPinNum)
        {
            case 0:
            	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_CTSN(1)) =
                     (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//D18
                break;
            case 1:
                HWREG(SOC_CONTROL_REGS + CONTROL_CONF_UART_RTSN(1)) =
                     (CONTROL_CONF_PULLUPSEL | CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(4));	//D17
            	break;
            default:
				break;
        }
    }

}
 #endif

/**
 * \brief   This function will configure the required clocks for McSPI instance.
 *
 * \return  None.
 *
 */
void McSPIModuleClkConfig(unsigned instanceNum )
{
    HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) =
                             CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    while((HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
     CM_PER_L3S_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

    HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) =
                             CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    while((HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
     CM_PER_L3_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

    HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) =
                             CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE;

    while((HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) &
                               CM_PER_L3_INSTR_CLKCTRL_MODULEMODE) !=
                                   CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE);

    HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) =
                             CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE;

    while((HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) &
        CM_PER_L3_CLKCTRL_MODULEMODE) != CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE);

    HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) =
                             CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    while((HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
                              CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL) !=
                                CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

    HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) =
                             CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    while((HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
                             CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL) !=
                               CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

    HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKCTRL) =
                             CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE;

    while((HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKCTRL) &
      CM_PER_L4LS_CLKCTRL_MODULEMODE) != CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE);

	if((0 == instanceNum)||(1 == instanceNum))
	{
	    HWREG(SOC_CM_PER_REGS + CM_PER_SPI0_CLKCTRL) &= ~CM_PER_SPI0_CLKCTRL_MODULEMODE;
	    HWREG(SOC_CM_PER_REGS + CM_PER_SPI0_CLKCTRL) |=
	                             CM_PER_SPI0_CLKCTRL_MODULEMODE_ENABLE;					// 使能SPI0时钟
	    while((HWREG(SOC_CM_PER_REGS + CM_PER_SPI0_CLKCTRL) &
	      CM_PER_SPI0_CLKCTRL_MODULEMODE) != CM_PER_SPI0_CLKCTRL_MODULEMODE_ENABLE);
	}
	else if(instanceNum == 2)
	{
	    HWREG(SOC_CM_PER_REGS + CM_PER_SPI1_CLKCTRL) &= ~CM_PER_SPI1_CLKCTRL_MODULEMODE;
	    HWREG(SOC_CM_PER_REGS + CM_PER_SPI1_CLKCTRL) |=
	                             CM_PER_SPI1_CLKCTRL_MODULEMODE_ENABLE;
	    while((HWREG(SOC_CM_PER_REGS + CM_PER_SPI1_CLKCTRL) &
	      CM_PER_SPI0_CLKCTRL_MODULEMODE) != CM_PER_SPI1_CLKCTRL_MODULEMODE_ENABLE);	// 使能SPI1时钟
	}

    while(!(HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
            CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK));

    while(!(HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
            CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK));

    while(!(HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
           (CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK |
            CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK)));

    while(!(HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           (CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_L4LS_GCLK |
            CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_SPI_GCLK)));

}
