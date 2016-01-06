/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_nand.c
**��    ��    ��: code
**��  ��  ��  ��: 140908
**��  ��  ��  ��: V0.1
**��          ��: nand����
                  �ͺ�        MT29F2G16ABAEAWP
                  ҳ�ߴ�      1056words��1024+32��
                  ��ߴ�      64pages��128k+4k��
                  ��ߴ�      1024blocks
                  �����ߴ�    2planes��256MB+8MB��
                  
**				  
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**�� �� ǰ �� ��: 
**��          ��: 20150211 - ���������⡢�������������   by code
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "Bsp_nand.h"

/* Private variables--------------------------------------------------------------------------*/  
static INT8U BlockTab[2048];		// ��״̬�����		Ŀǰ�õ�[7]:������	[6]:��������	[5:0]����

/* Private function--------------------------------------------------------------------------*/  
static INT32U NAND_BadBlocksCheck(NandInfo_t *nandInfo,INT32U strblock,INT32U blkNum);
static INT32U NAND_BlocksErase(NandInfo_t *nandInfo,INT32U strblock,INT32U blkNum);
static void NAND_TimingInfoInit(GPMCNANDTimingInfo_t *TimingInfo);

/***********************************************************************************************
* Function	:   NAND_BadBlocksCheck
* Description	: ������
* Input		:   *nandInfo   nand������Ϣ�ṹ��ָ��
                strblock    ��������ʼ��
                blkNum      ���Ŀ�����
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
* Description	: �����
* Input		:   *nandInfo   nand������Ϣ�ṹ��ָ��
                strblock    �������ʼ��ַ
                blkNum      �����Ŀ�����
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
* Description	: nandͨѶ�е�ʱ�����
* Input		:   ʱ����Ϣ�ṹ��
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
* Description	: ��nandflash
* Input		:   *nandInfo   nand������Ϣ�ṹ��ָ��
                src_addr   	nand����ַ
                *dest_addr  �ļ�ָ��
                size		�ļ�����
* Return	: 
* Note(s)	: 	nand��ַsrc_addr���뱣֤2048�ֽڶ���
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
* Description	: дnandflash
* Input		:   *nandInfo   nand������Ϣ�ṹ��ָ��
                dest_addr  	nandд���ַ
                *src_addr   ����ָ��
                size		���ݳ���
* Return	: 
* Note(s)	:  	nand��ַdest_addr���뱣֤2048�ֽڶ���
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

	// ������
	NAND_BadBlocksCheck(nandInfo,BlockNum,EndBlock-BlockNum+1);

	// �����
	NAND_BlocksErase(nandInfo,BlockNum,EndBlock-BlockNum+1);

	// ����д��
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
* Description	: nand��ʼ��
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
	nandinfo->eccType			= NAND_ECC_ALGO_BCH_8BIT;		// ECC����BCH�㷨��8λ����
	nandinfo->chipSelectCnt		= 1;							// 1��Ƭѡ
	nandinfo->dieCnt			= 1;							// 1��die����
	nandinfo->chipSelects[0]	= NAND_CHIP_SELECT_0;			// 0��Ƭѡ��
	nandinfo->busWidth			= NAND_BUSWIDTH_16BIT;			// 16bit����
	nandinfo->pageSize			= NAND_PAGESIZE_2048BYTES;		// ҳ��С2048byte(��1024word)
    nandinfo->blkSize			= NAND_BLOCKSIZE_128KB;			// ��ߴ�128KB
    nandinfo->manId				= NAND_MANUFATURER_MICRON_ID;	// ����ID þ��
    nandinfo->devId				= NAND_DEVICE_ID;				// �豸ID
    nandinfo->dataRegAddr		= (SOC_GPMC_0_REGS + 
    							GPMC_NAND_DATA(GPMC_CHIP_SELECT_0));	// ���ݼĴ�����ַ
    nandinfo->addrRegAddr		= (SOC_GPMC_0_REGS +
    							GPMC_NAND_ADDRESS(GPMC_CHIP_SELECT_0));	// ��ַ�Ĵ�����ַ
    nandinfo->cmdRegAddr		= (SOC_GPMC_0_REGS +
								GPMC_NAND_COMMAND(GPMC_CHIP_SELECT_0));	// ����Ĵ�����ַ

	/* Init the NAND Controller Info struct */
	hNandCtrlInfo->CtrlInit 				= GPMCNANDInit;					// ָ����GPMCNANDInit�׵�ַ
	hNandCtrlInfo->WaitPinStatusGet			= GPMCNANDWaitPinStatusGet;		// ָ����GPMCNANDWaitPinStatusGet�׵�ַ
	hNandCtrlInfo->WriteBufReady			= GPMCNANDWriteBufReady;		// ָ����GPMCNANDWriteBufReady�׵�ַ
	hNandCtrlInfo->currChipSelect			= NAND_CHIP_SELECT_0;			// 0��Ƭѡ��
	hNandCtrlInfo->baseAddr					= SOC_GPMC_0_REGS;				// GPMCģ�����ַ
	hNandCtrlInfo->eccSupported				= (NAND_ECC_ALGO_HAMMING_1BIT |
												NAND_ECC_ALGO_BCH_4BIT |
												NAND_ECC_ALGO_BCH_8BIT |
												NAND_ECC_ALGO_BCH_16BIT);	// ECC����֧��4��8��16λbch�㷨��֧��hanming�㷨
	hNandCtrlInfo->waitPin					= GPMC_WAIT_PIN0;				// 0�ŵȴ���
	hNandCtrlInfo->waitPinPol				= GPMC_WAIT_PIN_POLARITY_LOW;	// �ȴ��߼���Ϊ�͵�ƽ
	hNandCtrlInfo->wpPinPol					= GPMC_WP_PIN_LEVEL_HIGH;		// д�����߼���Ϊ�ߵ�ƽ
	hNandCtrlInfo->chipSelectBaseAddr[0]	= NAND_CS0_BASEADDR; 			// Ƭѡ����ַ
	hNandCtrlInfo->chipSelectRegionSize[0]	= GPMC_CS_SIZE_256MB;			// Ƭѡ��С256MB
	NAND_TimingInfoInit(hNandCtrlInfo->hNandTimingInfo);					// ʱ���������

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

