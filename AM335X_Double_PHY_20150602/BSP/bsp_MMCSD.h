/*****************************************Copyright(C)******************************************
*******************************************浙江万马*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		: bsp_mmcsd.h
* Author	        : 王耀
* Date First Issued	: 10/29/2013
* Version		: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History		:
* //2013		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_MMCSD_H_
#define	__BSP_MMCSD_H_
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp_conf.h"
#include "bsp_beep.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "ff.h"//与文件系统的结合
#include "hs_mmcsd.h"
#include "interrupt.h"
#include "edma_event.h"
#include "mmcsd_proto.h"
#include "hs_mmcsdlib.h"
#include "cpu.h"
#include "bsp_int.h"
#include "edma.h"
#include "am335x_irq.h"
#include "bsp.h"
/* Private define-----------------------------------------------------------------------------*/
#define MMCSD_BASE                     SOC_MMCHS_0_REGS
//#define MMCSD_DMA_BASE                 SOC_EDMA30CC_0_REGS

#define MMCSD_IN_FREQ                  96000000 /* 96MHz */
#define MMCSD_INIT_FREQ                400000   /* 400kHz */

#define HSMMCSD_CARD_DETECT_PINNUM     6

//#define MMCSD_DMA_CHA_TX               24
//#define MMCSD_DMA_CHA_RX               25
//#define MMCSD_DMA_QUE_NUM              0
//#define MMCSD_DMA_REGION_NUM           0
#define MMCSD_BLK_SIZE                 512
#define MMCSD_OCR                      (SD_OCR_VDD_3P0_3P1 | SD_OCR_VDD_3P1_3P2)                                     

#define PATH_BUF_SIZE                  512
#define DATA_BUF_SIZE                  1024*(1024)	//2K，对应nandflash2048byte的页大小

/* EDMA3 Event queue number. */
#define EVT_QUEUE_NUM                  0
/* MMCSD instance related macros. */
#define MMCSD_INST_BASE                (SOC_MMCHS_0_REGS)
#define MMCSD_INT_NUM                  (SYS_INT_MMCSD0INT)

/* EDMA instance related macros. */
#define EDMA_INST_BASE                 (SOC_EDMA30CC_0_REGS)
#define EDMA_COMPLTN_INT_NUM           (SYS_INT_EDMACOMPINT)
#define EDMA_ERROR_INT_NUM             (SYS_INT_EDMAERRINT) 

/* EDMA Events */
#define MMCSD_TX_EDMA_CHAN             (EDMA3_CHA_MMCSD0_TX)
#define MMCSD_RX_EDMA_CHAN             (EDMA3_CHA_MMCSD0_RX)
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
extern FATFS g_sFatFs;
extern FIL g_sFileObject;
extern DIR g_sDirObject;

#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=128
extern unsigned char g_cTmpBuf[DATA_BUF_SIZE];

#endif
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_MMCSDInit()
* Description	        : SD卡初始化
* Input			: none
* Output		: none
* Note(s)		: 
* Contributor	        : 10/12/2013	wangyao
***********************************************************************************************/
void BSP_MMCSDInit(void);
void Edma3CompletionIsr(void);
void Edma3CCErrorIsr(void);
void HSMMCSDIsr(void);
#endif	//__BSP_MMCSD_H_
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

