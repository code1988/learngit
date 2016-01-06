/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_nand.h
**创    建    人: code
**创  建  日  期: 140804
**最  新  版  本: V0.1
**描          述: nand支持
-----------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_NAND_H_
#define __BSP_NAND_H_

#include "bsp.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "bsp_int.h"
#include "am335x_irq.h"
#include "gpmc.h"
#include "nandlib.h"
#include "nand_gpmc.h"
#include "nandDma.h"


#define NAND_MANUFATURER_MICRON_ID              (0x2C)
#define NAND_DEVICE_ID                          (0xCA)

/*****************************************************************************/
/*
** Macros which defines the chip select base address and cs region size.
**
*/
#define NAND_CS0_BASEADDR                       (0x10000000)

/*****************************************************************************/
/*
** Macros which defines the NAND timing info.
**
*/
#define NAND_CSWROFFTIME                        (30)
#define NAND_CSRDOFFTIME                        (31)
#define NAND_CSONTIME                           (0)

#define NAND_ADVONTIME                          (0)
#define NAND_ADVAADMUXONTIME                    (0)
#define NAND_ADVRDOFFTIME                       (31)
#define NAND_ADVWROFFTIME                       (31)
#define NAND_ADVAADMUXRDOFFTIME                 (0)
#define NAND_ADVAADMUXWROFFTIME                 (0)

#define NAND_WEOFFTIME                          (31)
#define NAND_WEONTIME                           (3)
#define NAND_OEAADMUXOFFTIME                    (31)
#define NAND_OEOFFTIME                          (31)
#define NAND_OEAADMUXONTIME                     (3)
#define NAND_OEONTIME                           (1)

#define NAND_RDCYCLETIME                        (31)
#define NAND_WRCYCLETIME                        (31)
#define NAND_RDACCESSTIME                       (28)
#define NAND_PAGEBURSTACCESSTIME                (0)

#define NAND_BUSTURNAROUND                      (0)
#define NAND_CYCLE2CYCLEDIFFCSEN                (0)
#define NAND_CYCLE2CYCLESAMECSEN                (1)
#define NAND_CYCLE2CYCLEDELAY                   (0)
#define NAND_WRDATAONADMUXBUS                   (15)
#define NAND_WRACCESSTIME                       (22)
/* Private functions--------------------------------------------------------------------------*/
NandStatus_t BSP_NandConfig(NandInfo_t *nandinfo);
INT32U BSP_NandRead(NandInfo_t *nandInfo,INT32U src_addr,volatile INT8U *dest_addr,INT32U size);
INT32U BSP_NandWrite(NandInfo_t *nandInfo,INT32U dest_addr,volatile INT8U *src_addr,INT32U size);

#endif
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/