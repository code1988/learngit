/*
 * \file   cpsw.c
 *
 * \brief  This file contains functions which configure CPSW instance
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

#include "soc_AM335x.h"
#include "hw_control_AM335x.h"
#include "hw_types.h"
#include "beaglebone.h"
#include "hw_cm_per.h"

/******************************************************************************
**                       INTERNAL MACRO DEFINITIONS
******************************************************************************/
#define CPSW_RGMII_SEL_MODE                   (0x02u)
#define CPSW_MDIO_SEL_MODE                    (0x00u)
#define LEN_MAC_ADDR                          (0x06u)
#define OFFSET_MAC_ADDR                       (0x30u)
#define I2C_SLAVE_EEPROM                      (0xA0u)
#define CPSW_RMII_SEL_MODE					  (0x01u)

//#define CPSW_MII_SEL_MODE                     (0x00u)
//#define CPSW_MDIO_SEL_MODE                    (0x00u)
//#define LEN_MAC_ADDR                          (0x06u)
//#define OFFSET_MAC_ADDR                       (0x30u)

/******************************************************************************
**                          FUNCTION DEFINITIONS
******************************************************************************/
/**
 * \brief   This function selects the CPSW pins for use in MII mode.
 *          
 * \param   None
 *
 * \return  None.
 *
 */
void CPSWPinMuxSetup(void)
{
	//百兆，RMII接口
	// RMII1
	#if 1
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_CRS) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_RMII1_REFCLK) = CONTROL_CONF_RXACTIVE;						    //50mhz时钟有外部提供，所以该管脚必须配成输入模式
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD0) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD1) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXERR) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD0) 	= CONTROL_CONF_MUXMODE(1);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD1) 	= CONTROL_CONF_MUXMODE(1);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXEN) 	= CONTROL_CONF_MUXMODE(1);
    #endif
    // RMII2
    #if 1
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_CSN(3)) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(2);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_COL)     = CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(1);	//50mhz时钟有外部提供，所以该管脚必须配成输入模式
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(11)) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(10)) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3);
   	//HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_WPN) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_MUXMODE(3);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(5)) 	= CONTROL_CONF_MUXMODE(3);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(4)) 	= CONTROL_CONF_MUXMODE(3);
   	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(0)) 	= CONTROL_CONF_MUXMODE(3);
    #endif
    // MDIO
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_DATA) 	= CONTROL_CONF_RXACTIVE | CONTROL_CONF_PULLUPSEL;
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_CLK) 	= CONTROL_CONF_PULLUPSEL;
    
#if 0
	//千兆，MII接口
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_COL) =
                     CONTROL_CONF_MII1_COL_CONF_MII1_COL_RXACTIVE
                     | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_CRS) =
                     CONTROL_CONF_MII1_CRS_CONF_MII1_CRS_RXACTIVE
                     | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXERR) =
                     CONTROL_CONF_MII1_RXERR_CONF_MII1_RXERR_RXACTIVE
                     | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXEN) =   
                      CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXDV) =  
                      CONTROL_CONF_MII1_RXDV_CONF_MII1_RXDV_RXACTIVE 
                      | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD3) =  
                      CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD2) =  
                      CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD1) =  
                      CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXD0) =  
                      CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_TXCLK) =  
                      CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXCLK) =
                      CONTROL_CONF_MII1_RXCLK_CONF_MII1_RXCLK_RXACTIVE 
                      | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD3) = 
                      CONTROL_CONF_MII1_RXD3_CONF_MII1_RXD3_RXACTIVE 
                      | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD2) = 
                      CONTROL_CONF_MII1_RXD2_CONF_MII1_RXD2_RXACTIVE 
                      | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD1) =  
                      CONTROL_CONF_MII1_RXD1_CONF_MII1_RXD1_RXACTIVE 
                      | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MII1_RXD0) =  
                      CONTROL_CONF_MII1_RXD0_CONF_MII1_RXD0_RXACTIVE 
                      | CPSW_RGMII_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_RMII1_REFCLK) =  
                      CONTROL_CONF_RMII1_REFCLK_CONF_RMII1_REFCLK_RXACTIVE; 
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_DATA) =
                      CONTROL_CONF_MDIO_DATA_CONF_MDIO_DATA_RXACTIVE 
                      | CONTROL_CONF_MDIO_DATA_CONF_MDIO_DATA_PUTYPESEL
                      | CPSW_MDIO_SEL_MODE;
            HWREG(SOC_CONTROL_REGS + CONTROL_CONF_MDIO_CLK) =
                      CONTROL_CONF_MDIO_CLK_CONF_MDIO_CLK_PUTYPESEL
                      | CPSW_MDIO_SEL_MODE;
#endif
}

/**
 * \brief   Enables CPSW clocks
 *
 * \param   None
 *
 * \return  None.
 */
void CPSWClkEnable(void)
{
//    HWREG(SOC_PRCM_REGS + CM_PER_CPGMAC0_CLKCTRL) =
//                      CM_PER_CPGMAC0_CLKCTRL_MODULEMODE_ENABLE;
//
//    while(0 != (HWREG(SOC_PRCM_REGS + CM_PER_CPGMAC0_CLKCTRL)
//                & CM_PER_CPGMAC0_CLKCTRL_IDLEST));
// 
//    HWREG(SOC_PRCM_REGS + CM_PER_CPSW_CLKSTCTRL) =
//                      CM_PER_CPSW_CLKSTCTRL_CLKTRCTRL_SW_WKUP;
//
//    while(0 == (HWREG(SOC_PRCM_REGS + CM_PER_CPSW_CLKSTCTRL)
//                & CM_PER_CPSW_CLKSTCTRL_CLKACTIVITY_CPSW_125MHZ_GCLK)); 
    HWREG(SOC_PRCM_REGS + CM_PER_CPGMAC0_CLKCTRL) =
                      CM_PER_CPGMAC0_CLKCTRL_MODULEMODE_ENABLE;

    while(0 != (HWREG(SOC_PRCM_REGS + CM_PER_CPGMAC0_CLKCTRL)
                & CM_PER_CPGMAC0_CLKCTRL_IDLEST));    
    
    
    
} 

/**
 * \brief   This function sets the MII/RMII/GMII mode for both ports
 *
 * \param   None 
 *
 * \return  None.
 * \note	  TI官方的sillicon errata中有一处该处的问题指出
 */
#define  RGMIIMODE                0x0a
#define  RMIIMODE                 0xF5
#define  GMIIMODE                 0x00
void EVMPortRGMIIModeSelect(void)//以太网硬件平台的MAC层和PHY层之间，MII接口,RGMII简化的板
{
    /* Select MII, Internal Delay mode */
    HWREG(SOC_CONTROL_REGS + CONTROL_GMII_SEL) = RMIIMODE;   	// AM335X通过PLL生成的50mhz时钟抖动过大，所以时钟必须有外部提供
}
/** * \brief   This function sets the RGMII mode for both ports * * \param   None  * * \return  None. */
//void EVMPortRGMIIModeSelect(void){    
/* Select RGMII, Internal Delay mode */    
//HWREG(SOC_CONTROL_REGS + CONTROL_GMII_SEL) = 0x0A;  }


/**
 * \brief   This function returns the MAC address for the EVM
 *
 * \param   addrIdx    the MAC address index. 
 * \param   macAddr    the Pointer where the MAC address shall be stored
 *     'addrIdx' can be either 0 or 1
 *
 * \return  None.
 */
void EVMMACAddrGet(unsigned int addrIdx, unsigned char *macAddr)
{
    macAddr[0] =  (HWREG(SOC_CONTROL_REGS + CONTROL_MAC_ID_LO(addrIdx)) 
                   >> 8) & 0xFF;
    macAddr[1] =  (HWREG(SOC_CONTROL_REGS + CONTROL_MAC_ID_LO(addrIdx)))
                  & 0xFF;
    macAddr[2] =  (HWREG(SOC_CONTROL_REGS + CONTROL_MAC_ID_HI(addrIdx))
                   >> 24) & 0xFF;
    macAddr[3] =  (HWREG(SOC_CONTROL_REGS + CONTROL_MAC_ID_HI(addrIdx))
                   >> 16) & 0xFF;
    macAddr[4] =  (HWREG(SOC_CONTROL_REGS + CONTROL_MAC_ID_HI(addrIdx))
                   >> 8) & 0xFF;
    macAddr[5] =  (HWREG(SOC_CONTROL_REGS + CONTROL_MAC_ID_HI(addrIdx)))
                  & 0xFF;
}

/****************************** End Of File *********************************/
