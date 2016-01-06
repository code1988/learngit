/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_nand.h
**��    ��    ��: code
**��  ��  ��  ��: 140804
**��  ��  ��  ��: V0.1
**��          ��: nand֧��
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
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/