/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_nand.c
**创    建    人: code
**创  建  日  期: 140908
**最  新  版  本: V0.1
**描          述: nand驱动
                  型号        MT29F2G16ABAEAWP
                  页尺寸      1056words（1024+32）
                  块尺寸      64pages（128k+4k）
                  面尺寸      1024blocks
                  整个尺寸    2planes（256MB+8MB）
                  
**				  
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**修 改 前 版 本: 
**描          述: 20150211 - 修正坏块检测、块擦除函数错误   by code
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "Bsp_nand.h"

/* Private variables--------------------------------------------------------------------------*/  
static INT8U BlockTab[2048];		// 块状态管理表		目前用到[7]:坏块检测	[6]:块擦除检测	[5:0]保留

/* Private function--------------------------------------------------------------------------*/  
static INT32U NAND_BadBlocksCheck(NandInfo_t *nandInfo,INT32U strblock,INT32U blkNum);
static INT32U NAND_BlocksErase(NandInfo_t *nandInfo,INT32U strblock,INT32U blkNum);
static void NAND_TimingInfoInit(GPMCNANDTimingInfo_t *TimingInfo);

/***********************************************************************************************
* Function	:   NAND_BadBlocksCheck
* Description	: 坏块检测
* Input		:   *nandInfo   nand配置信息结构体指针
                strblock    坏块检测起始块
                blkNum      检测的块数量
* Return	: 
* Note(s)	: 
* Contributor	:140908 code
***********************************************************************************************/
static INT32U NAND_BadBlocksCheck(NandInfo_t *nandInfo,INT32U strblock,INT32U blkNum)
{
	INT32U i;
    
	for(i=strblock;i<(blkNum+strblock);i++)
	{
		if(BlockTab[i] >> 7)
			continue;
		if(NANDBadBlockCheck(nandInfo, i) != NAND_BLOCK_GOOD)
		{
			return E_FAIL;
		}
		BlockTab[i] |= 0x80;
	}
	return E_PASS;
}

/***********************************************************************************************
* Function	:   NAND_BlocksErase
* Description	: 块擦除
* Input		:   *nandInfo   nand配置信息结构体指针
                strblock    块擦除起始地址
                blkNum      擦除的块数量
* Return	: 
* Note(s)	: 
* Contributor	:140908 code
***********************************************************************************************/
static INT32U NAND_BlocksErase(NandInfo_t *nandInfo,INT32U strblock,INT32U blkNum)
{
	INT32U i;

	for(i=strblock;i<(blkNum+strblock);i++)
	{
		if(BlockTab[i] & 0x40)
			continue;
		if(NANDBlockErase(nandInfo, i) != NAND_STATUS_PASSED)
		{
			return E_FAIL;
		}
		BlockTab[i] |= 0x40;
	}
	return E_PASS;
}

/***********************************************************************************************
* Function	:   NAND_TimingInfoInit
* Description	: nand通讯中的时间管理
* Input		:   时间信息结构体
* Return	: 
* Note(s)	: 
* Contributor	:140908 code
***********************************************************************************************/
static void NAND_TimingInfoInit(GPMCNANDTimingInfo_t *TimingInfo)
{
    TimingInfo->CSWrOffTime               = NAND_CSWROFFTIME;
    TimingInfo->CSRdOffTime               = NAND_CSRDOFFTIME;
    TimingInfo->CSExtDelayFlag            = GPMC_CS_EXTRA_NODELAY;
    TimingInfo->CSOnTime                  = NAND_CSONTIME;

    TimingInfo->ADVAADMuxWrOffTime        = NAND_ADVAADMUXWROFFTIME;
    TimingInfo->ADVAADMuxRdOffTime        = NAND_ADVAADMUXRDOFFTIME;
    TimingInfo->ADVWrOffTime              = NAND_ADVWROFFTIME;
    TimingInfo->ADVRdOffTime              = NAND_ADVRDOFFTIME;
    TimingInfo->ADVExtDelayFlag           = GPMC_ADV_EXTRA_NODELAY;
    TimingInfo->ADVAADMuxOnTime           = NAND_ADVAADMUXONTIME;
    TimingInfo->ADVOnTime                 = NAND_ADVONTIME;

    TimingInfo->WEOffTime                 = NAND_WEOFFTIME;
    TimingInfo->WEExtDelayFlag            = GPMC_WE_EXTRA_NODELAY;
    TimingInfo->WEOnTime                  = NAND_WEONTIME;
    TimingInfo->OEAADMuxOffTime           = NAND_OEAADMUXOFFTIME;
    TimingInfo->OEOffTime                 = NAND_OEOFFTIME;
    TimingInfo->OEExtDelayFlag            = GPMC_OE_EXTRA_NODELAY;
    TimingInfo->OEAADMuxOnTime            = NAND_OEAADMUXONTIME;
    TimingInfo->OEOnTime                  = NAND_OEONTIME;

    TimingInfo->rdCycleTime               = NAND_RDCYCLETIME;
    TimingInfo->wrCycleTime               = NAND_WRCYCLETIME;
    TimingInfo->rdAccessTime              = NAND_RDACCESSTIME;
    TimingInfo->pageBurstAccessTime       = NAND_PAGEBURSTACCESSTIME;

    TimingInfo->cycle2CycleDelay          = NAND_CYCLE2CYCLEDELAY;
    TimingInfo->cycle2CycleDelaySameCSCfg = NAND_CYCLE2CYCLESAMECSEN;
    TimingInfo->cycle2CycleDelayDiffCSCfg = NAND_CYCLE2CYCLEDIFFCSEN;
    TimingInfo->busTAtime                 = NAND_BUSTURNAROUND;
}

/***********************************************************************************************
* Function	:   BSP_NandRead
* Description	: 读nandflash
* Input		:   *nandInfo   nand配置信息结构体指针
                src_addr   	nand读地址
                *dest_addr  文件指针
                size		文件长度
* Return	: 
* Note(s)	: 	nand地址src_addr必须保证2048字节对齐
* Contributor	:140908 code
***********************************************************************************************/
INT32U BSP_NandRead(NandInfo_t *nandInfo,INT32U src_addr,volatile INT8U *dest_addr,INT32U size)
{
	INT32U BlockNum;
	INT32U PageNum;
	INT32U EndPage;
	INT8U eccdata[64];

	BlockNum 	= src_addr / NAND_BLOCKSIZE_128KB;
	PageNum 	= src_addr / NAND_PAGESIZE_2048BYTES;
	EndPage		= (src_addr + size - 1) / NAND_PAGESIZE_2048BYTES;

	do
	{
		if(NANDPageRead(nandInfo,BlockNum,(PageNum&0x3f),dest_addr,eccdata) != NAND_STATUS_PASSED)
			return E_FAIL;
		dest_addr += NAND_PAGESIZE_2048BYTES;
		PageNum++;
		if(!(PageNum & 0x3f))
			BlockNum++;
	}while(PageNum <= EndPage);

	return E_PASS;
}

/***********************************************************************************************
* Function	:   BSP_NandWrite
* Description	: 写nandflash
* Input		:   *nandInfo   nand配置信息结构体指针
                dest_addr  	nand写入地址
                *src_addr   数据指针
                size		数据长度
* Return	: 
* Note(s)	:  	nand地址dest_addr必须保证2048字节对齐
* Contributor	:140908 code
***********************************************************************************************/
INT32U BSP_NandWrite(NandInfo_t *nandInfo,INT32U dest_addr,volatile INT8U *src_addr,INT32U size)
{
	INT32U BlockNum;
	INT32U EndBlock;
	INT32U PageNum;
	INT32U EndPage;
	INT8U eccdata[64];
	
	BlockNum 	= dest_addr / NAND_BLOCKSIZE_128KB;
	PageNum 	= dest_addr / NAND_PAGESIZE_2048BYTES;
	EndBlock	= (dest_addr + size - 1) / NAND_BLOCKSIZE_128KB;
	EndPage		= (dest_addr + size - 1) / NAND_PAGESIZE_2048BYTES;	

	// 坏块检测
	NAND_BadBlocksCheck(nandInfo,BlockNum,EndBlock-BlockNum+1);

	// 块擦除
	NAND_BlocksErase(nandInfo,BlockNum,EndBlock-BlockNum+1);

	// 数据写入
	do
	{
		if(NANDPageWrite(nandInfo,BlockNum,(PageNum&0x3f),src_addr,eccdata) != NAND_STATUS_PASSED)
			return E_FAIL;
		src_addr += NAND_PAGESIZE_2048BYTES;
		PageNum++;
		if(!(PageNum & 0x3f))
			BlockNum++;
	}while(PageNum <= EndPage);

	return E_PASS;
}

/***********************************************************************************************
* Function Name	: BSP_NandConfig
* Description	: nand初始化
* Input			: InitStructure
* Return		: void
* Note(s)		: 
* Contributor	: 140804	code
***********************************************************************************************/
NandStatus_t BSP_NandConfig(NandInfo_t *nandinfo)
{
    NandStatus_t retVal;
    NandCtrlInfo_t *hNandCtrlInfo = nandinfo->hNandCtrlInfo;
    NandDmaInfo_t  *hNandDmaInfo  = nandinfo->hNandDmaInfo;
    NandEccInfo_t  *hNandEccInfo  = nandinfo->hNandEccInfo;
	
	/* Pin mux and clock setting */
    GPMCPinMuxSetup();
    GPMCClkConfig();
    EDMAModuleClkConfig();	

    
    
    /* Init the NAND Device Info */
	nandinfo->opMode 			= NAND_XFER_MODE_CPU;			// CPU
	nandinfo->eccType			= NAND_ECC_ALGO_BCH_8BIT;		// ECC纠错，BCH算法，8位处理
	nandinfo->chipSelectCnt		= 1;							// 1个片选
	nandinfo->dieCnt			= 1;							// 1个die数量
	nandinfo->chipSelects[0]	= NAND_CHIP_SELECT_0;			// 0号片选线
	nandinfo->busWidth			= NAND_BUSWIDTH_16BIT;			// 16bit带宽
	nandinfo->pageSize			= NAND_PAGESIZE_2048BYTES;		// 页大小2048byte(即1024word)
    nandinfo->blkSize			= NAND_BLOCKSIZE_128KB;			// 块尺寸128KB
    nandinfo->manId				= NAND_MANUFATURER_MICRON_ID;	// 厂商ID 镁光
    nandinfo->devId				= NAND_DEVICE_ID;				// 设备ID
    nandinfo->dataRegAddr		= (SOC_GPMC_0_REGS + 
    							GPMC_NAND_DATA(GPMC_CHIP_SELECT_0));	// 数据寄存器地址
    nandinfo->addrRegAddr		= (SOC_GPMC_0_REGS +
    							GPMC_NAND_ADDRESS(GPMC_CHIP_SELECT_0));	// 地址寄存器地址
    nandinfo->cmdRegAddr		= (SOC_GPMC_0_REGS +
								GPMC_NAND_COMMAND(GPMC_CHIP_SELECT_0));	// 命令寄存器地址

	/* Init the NAND Controller Info struct */
	hNandCtrlInfo->CtrlInit 				= GPMCNANDInit;					// 指向函数GPMCNANDInit首地址
	hNandCtrlInfo->WaitPinStatusGet			= GPMCNANDWaitPinStatusGet;		// 指向函数GPMCNANDWaitPinStatusGet首地址
	hNandCtrlInfo->WriteBufReady			= GPMCNANDWriteBufReady;		// 指向函数GPMCNANDWriteBufReady首地址
	hNandCtrlInfo->currChipSelect			= NAND_CHIP_SELECT_0;			// 0号片选线
	hNandCtrlInfo->baseAddr					= SOC_GPMC_0_REGS;				// GPMC模块基地址
	hNandCtrlInfo->eccSupported				= (NAND_ECC_ALGO_HAMMING_1BIT |
												NAND_ECC_ALGO_BCH_4BIT |
												NAND_ECC_ALGO_BCH_8BIT |
												NAND_ECC_ALGO_BCH_16BIT);	// ECC纠错，支持4、8、16位bch算法，支持hanming算法
	hNandCtrlInfo->waitPin					= GPMC_WAIT_PIN0;				// 0号等待线
	hNandCtrlInfo->waitPinPol				= GPMC_WAIT_PIN_POLARITY_LOW;	// 等待线极性为低电平
	hNandCtrlInfo->wpPinPol					= GPMC_WP_PIN_LEVEL_HIGH;		// 写保护线极性为高电平
	hNandCtrlInfo->chipSelectBaseAddr[0]	= NAND_CS0_BASEADDR; 			// 片选基地址
	hNandCtrlInfo->chipSelectRegionSize[0]	= GPMC_CS_SIZE_256MB;			// 片选大小256MB
	NAND_TimingInfoInit(hNandCtrlInfo->hNandTimingInfo);					// 时间参数配置

	 /* Init the NAND Ecc Info */
	hNandEccInfo->baseAddr				= SOC_ELM_0_REGS;						
	hNandEccInfo->ECCInit				= GPMCNANDECCInit;						
	hNandEccInfo->ECCEnable				= GPMCNANDECCEnable;					
	hNandEccInfo->ECCDisable			= GPMCNANDECCDisable;					
	hNandEccInfo->ECCWriteSet			= GPMCNANDECCWriteSet;					
	hNandEccInfo->ECCReadSet			= GPMCNANDECCReadSet;					
	hNandEccInfo->ECCCalculate			= GPMCNANDECCCalculate;					
	hNandEccInfo->ECCCheckAndCorrect	= GPMCNANDECCCheckAndCorrect;		

	/* Init the NAND DMA info */
	hNandDmaInfo->DMAXfer				= GPMCNANDDMAXfer;
	hNandDmaInfo->DMAInit				= GPMCNANDEdmaInit;
	hNandDmaInfo->DMAXferSetup			= GPMCNANDXferSetup;
	hNandDmaInfo->DMAXferStatusGet		= GPMCNANDXferStatusGet;
	 
    retVal = NANDOpen(nandinfo);
    
    return retVal;
}

