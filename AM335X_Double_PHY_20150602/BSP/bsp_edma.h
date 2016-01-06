/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_MMCSD.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 131029
**最  新  版  本: V0.1
**描          述: SD卡驱动管理文件，文件系统暂时也做在这里，
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_EDMA_H_
#define __BSP_EDMA_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "bsp.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
/* Private macro------------------------------------------------------------------------------*/
#define EDMAAPP_MAX_ACOUNT        (10u) /* MAX ACOUNT */
#define EDMAAPP_MAX_BCOUNT        (1u)  /* MAX BCOUNT */
#define EDMAAPP_MAX_CCOUNT        (1u)  /* MAX CCOUNT */
#define EDMAAPP_MAX_BUFFER_SIZE   (EDMAAPP_MAX_ACOUNT * EDMAAPP_MAX_BCOUNT * \
                                   EDMAAPP_MAX_CCOUNT)

#define EDMAAPP_IRQ_STATUS_XFER_INPROG     (0u)
#define EDMAAPP_IRQ_STATUS_XFER_COMP       (1u)
#define EDMAAPP_IRQ_STATUS_DMA_EVT_MISS    (-1)
#define EDMAAPP_IRQ_STATUS_QDMA_EVT_MISS   (-2)
#define EDMAAPP_EDMACC_BASE_ADDRESS        SOC_EDMA30CC_0_REGS

// Data parameters required by application
#define EDMAAPP_DMA_CH_TYPE     EDMA3_CHANNEL_TYPE_DMA
#define EDMAAPP_DMA_CH_NUM      (63u)
#define EDMAAPP_DMA_TCC_NUM     (63u)
#define EDMAAPP_DMA_SYNC_TYPE   EDMA3_SYNC_A
#define EDMAAPP_DMA_TRIG_MODE   EDMA3_TRIG_MODE_MANUAL
#define EDMAAPP_DMA_EVTQ        (0u)

#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = SOC_CACHELINE_SIZE_MAX
extern volatile char SrcBuff[EDMAAPP_MAX_BUFFER_SIZE];
extern volatile char DstBuff[EDMAAPP_MAX_BUFFER_SIZE];
#endif

extern volatile int IrqRaised;
extern void (*EDMAAppCallbackFxn[EDMA3_NUM_TCC])(unsigned int status);
/* Private function prototypes----------------------------------------------------------------*/
void _EDMAAppRegisterEdma3Interrupts();
void _EDMAAppEdma3ccComplIsr();
void _EDMAAppEdma3ccErrIsr();
void EDMAAppEDMA3Test();
void EDMAAppCallback(unsigned int status);

#endif
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/