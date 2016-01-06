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
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_mmcsd.h"

/* Private define-----------------------------------------------------------------------------*/
#define CMD_BUF_SIZE            512 //buf大小
//A macro to make it easy to add result codes to the table.
#define FRESULT_ENTRY(f)        { (f), (#f) }
//A macro that holds the number of result codes.
#define NUM_FRESULT_CODES (sizeof(g_sFresultStrings) / sizeof(tFresultString))
#define HSMMCSD_INT_STATUS_FLAG       (0xFFFFFFFF)



/*****************************************************************************
Defines the size of the buffers that hold the path, or temporary data from
the memory card.  There are two buffers allocated of this size.  The buffer
size must be large enough to hold the longest expected full path name,
including the file name, and a trailing null character.
******************************************************************************/
/* Private typedef----------------------------------------------------------------------------*/
/* Fat 设备注册 */
#ifndef fatDevice
typedef struct _fatDevice
{
    /* Pointer to underlying device/controller */
    void *dev;
    /* File system pointer */
    FATFS *fs;
    /* state */
    unsigned int initDone;
}fatDevice;
#endif
extern fatDevice fat_devices[2];
/* Private macro------------------------------------------------------------------------------*/
/* EDMA callback function array */
static void (*cb_Fxn[EDMA3_NUM_TCC]) (unsigned int tcc, unsigned int status);
/* Private variables--------------------------------------------------------------------------*/
/* Global flags for interrupt handling */
volatile unsigned int callbackOccured = 0; 
volatile unsigned int xferPend = 0;
volatile unsigned int cmdCompFlag = 0;
volatile unsigned int xferCompFlag = 0; 
volatile unsigned int dataTimeout = 0;
volatile unsigned int cmdTimeout = 0;
volatile unsigned int errFlag = 0;
volatile unsigned int sdBlkSize = MMCSD_BLK_SIZE;
unsigned int hsmmcsd_dataLen = 0;
volatile unsigned char *hsmmcsd_buffer = NULL;
/*****************************************************************************
The buffer that holds the command line.
******************************************************************************/
volatile unsigned int g_sPState = 0;
volatile unsigned int g_sCState = 0;

/* SD card info structure */
#ifdef __IAR_SYSTEMS_ICC__

#pragma data_alignment=32
mmcsdCardInfo sdCard;
#pragma data_alignment=32
mmcsdCtrlInfo ctrlInfo;
#pragma data_alignment=128
unsigned char g_cTmpBuf[DATA_BUF_SIZE];

#endif

/*****************************************************************************
Current FAT fs state.
******************************************************************************/
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=SOC_CACHELINE_SIZE
FATFS g_sFatFs;
FIL g_sFileObject;
DIR g_sDirObject;
#else
#error "Unsupported Compiler. \r\n"

#endif

//FATFS g_sFatFs;
//FIL g_sFileObject;

/*****************************************************************************
A structure that holds a mapping between an FRESULT numerical code,
and a string representation.  FRESULT codes are returned from the FatFs
FAT file system driver.

******************************************************************************/
typedef struct
{
    FRESULT fresult;
    char *pcResultStr;
}
tFresultString;

/*****************************************************************************
A table that holds a mapping between the numerical FRESULT code and
it's name as a string.  This is used for looking up error codes for
printing to the console.
******************************************************************************/
tFresultString g_sFresultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_RW_ERROR),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_MKFS_ABORTED)
};
/* Private function prototypes----------------------------------------------------------------*/
extern void HSMMCSDFsMount(unsigned int driveNum, void *ptr);
extern void HSMMCSDFsProcessCmdLine(void);
/******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/
void HSMMCSDTxDmaConfig(void *ptr, unsigned int blkSize, unsigned int blks);
void HSMMCSDRxDmaConfig(void *ptr, unsigned int blkSize, unsigned int nblks);
/* Private functions--------------------------------------------------------------------------*/
/**
*\brief This function Check the command status.\n
*
* \param - ctrl - MMCSD controller info.\n
*
* \return command status.\n
*
*/

static unsigned int HSMMCSDCmdStatusGet(mmcsdCtrlInfo *ctrl)
{
    unsigned int status = 0;

    while ((cmdCompFlag == 0) && (cmdTimeout == 0));

    if (cmdCompFlag)
    {
        status = 1;
        cmdCompFlag = 0;
    }

    if (cmdTimeout)
    {
        status = 0;
        cmdTimeout = 0;
    }

    return status;
}


/**
*\brief This function gets the Xfer status.\n
*
* \param - ctrl - MMCSD controller info.\n
*
* \return Xfer status.\n
*
*/
static unsigned int HSMMCSDXferStatusGet(mmcsdCtrlInfo *ctrl)
{
    unsigned int status = 0;
    volatile unsigned int timeOut = 0xFFFF;

    while ((xferCompFlag == 0) && (dataTimeout == 0));

    if (xferCompFlag)
    {
        status = 1;
        xferCompFlag = 0;
    }

    if (dataTimeout)
    {
        status = 0;
        dataTimeout = 0;
    }

    /* Also, poll for the callback */
    if (HWREG(ctrl->memBase + MMCHS_CMD) & MMCHS_CMD_DP)
    {
        while(callbackOccured == 0 && ((timeOut--) != 0));
        callbackOccured = 0;

        if(timeOut == 0)
        {
            status = 0;
        }
    }

    ctrlInfo.dmaEnable = 0;

    return status;
}

/**
*\brief This function setup the controller and DMA for Xfer.\n
*
* \param - ctrl    - MMCSD controller info.\n
*
* \param - rwFlag  - Read/Write flag.\n
*
* \param - blkSize - Block Size.\n
*
* \param - nBlks   - Number of Blocks.\n
*
* \return none.\n
*
*/
static void HSMMCSDXferSetup(mmcsdCtrlInfo *ctrl, unsigned char rwFlag, void *ptr,
                             unsigned int blkSize, unsigned int nBlks)
{
    callbackOccured = 0;
    xferCompFlag = 0;

    if (rwFlag == 1)
    {
        HSMMCSDRxDmaConfig(ptr, blkSize, nBlks);
    }
    else
    {
        HSMMCSDTxDmaConfig(ptr, blkSize, nBlks);
    }

    ctrl->dmaEnable = 1;
    HSMMCSDBlkLenSet(ctrl->memBase, blkSize);
}
void HSMMCSDRxDmaConfig(void *ptr, unsigned int blkSize, unsigned int nblks)
{
    EDMA3CCPaRAMEntry paramSet;

    paramSet.srcAddr    = ctrlInfo.memBase + MMCHS_DATA;
    paramSet.destAddr   = (unsigned int)ptr;
    paramSet.srcBIdx    = 0;
    paramSet.srcCIdx    = 0;
    paramSet.destBIdx   = 4;
    paramSet.destCIdx   = (unsigned short)blkSize;
    paramSet.aCnt       = 0x4;
    paramSet.bCnt       = (unsigned short)blkSize/4;
    paramSet.cCnt       = (unsigned short)nblks;
    paramSet.bCntReload = 0x0;
    paramSet.linkAddr   = 0xffff;
    paramSet.opt        = 0;

    /* Set OPT */
    paramSet.opt |= ((MMCSD_RX_EDMA_CHAN << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);

    /* 1. Transmission complition interrupt enable */
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* 2. Read FIFO : SRC Constant addr mode */
    paramSet.opt |= (1 << 0);

    /* 3. SRC FIFO width is 32 bit */
    paramSet.opt |= (2 << 8);

    /* 4.  AB-Sync mode */
    paramSet.opt |= (1 << 2);

    /* configure PaRAM Set */
    EDMA3SetPaRAM(EDMA_INST_BASE, MMCSD_RX_EDMA_CHAN, &paramSet);

    /* Enable the transfer */
    EDMA3EnableTransfer(EDMA_INST_BASE, MMCSD_RX_EDMA_CHAN, EDMA3_TRIG_MODE_EVENT);
}

void HSMMCSDTxDmaConfig(void *ptr, unsigned int blkSize, unsigned int blks)
{
    EDMA3CCPaRAMEntry paramSet;

    paramSet.srcAddr    = (unsigned int)ptr;
    paramSet.destAddr   = ctrlInfo.memBase + MMCHS_DATA;
    paramSet.srcBIdx    = 4;
    paramSet.srcCIdx    = blkSize;
    paramSet.destBIdx   = 0;
    paramSet.destCIdx   = 0;
    paramSet.aCnt       = 0x4;
    paramSet.bCnt       = (unsigned short)blkSize/4;
    paramSet.cCnt       = (unsigned short)blks;
    paramSet.bCntReload = 0x0;
    paramSet.linkAddr   = 0xffff;
    paramSet.opt        = 0;

    /* Set OPT */
    paramSet.opt |= ((MMCSD_TX_EDMA_CHAN << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);

    /* 1. Transmission complition interrupt enable */
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* 2. Read FIFO : DST Constant addr mode */
    paramSet.opt |= (1 << 1);

    /* 3. DST FIFO width is 32 bit */
    paramSet.opt |= (2 << 8);

    /* 4.  AB-Sync mode */
    paramSet.opt |= (1 << 2);

    /* configure PaRAM Set */
    EDMA3SetPaRAM(EDMA_INST_BASE, MMCSD_TX_EDMA_CHAN, &paramSet);

    /* Enable the transfer */
    EDMA3EnableTransfer(EDMA_INST_BASE, MMCSD_TX_EDMA_CHAN, EDMA3_TRIG_MODE_EVENT);
}
/*
** This function is used as a callback from EDMA3 Completion Handler.
*/
static void callback(unsigned int tccNum, unsigned int status)
{
    callbackOccured = 1;
    EDMA3DisableTransfer(EDMA_INST_BASE, tccNum, EDMA3_TRIG_MODE_EVENT);
}

void Edma3CompletionIsr(void)
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int isIPR = 0;

    unsigned int indexl;
    unsigned int Cnt = 0;
    
    indexl = 1;

    isIPR = EDMA3GetIntrStatus(EDMA_INST_BASE);

    if(isIPR)
    {
        while ((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
        {
            indexl = 0u;
            pendingIrqs = EDMA3GetIntrStatus(EDMA_INST_BASE);

            while (pendingIrqs)
            {
                if((pendingIrqs & 1u) == TRUE)
                {
                    /**
                    * If the user has not given any callback function
                    * while requesting the TCC, its TCC specific bit
                    * in the IPR register will NOT be cleared.
                    */
                    /* here write to ICR to clear the corresponding IPR bits */

                    EDMA3ClrIntr(EDMA_INST_BASE, indexl);

                    if (cb_Fxn[indexl] != NULL)
                    {
                        (*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE);
                    }
                }
                ++indexl;
                pendingIrqs >>= 1u;
            }
            Cnt++;
        }
    }
}

void Edma3CCErrorIsr(void)
{
    unsigned int pendingIrqs;
    unsigned int evtqueNum = 0;  /* Event Queue Num */
    unsigned int isIPRH = 0;
    unsigned int isIPR = 0;
    volatile unsigned int Cnt = 0u;
    volatile unsigned int index;

    pendingIrqs = 0u;
    index = 1u;
    
    isIPR  = EDMA3GetIntrStatus(EDMA_INST_BASE);
    isIPRH = EDMA3IntrStatusHighGet(EDMA_INST_BASE);

    if((isIPR | isIPRH ) || (EDMA3QdmaGetErrIntrStatus(EDMA_INST_BASE) != 0)
        || (EDMA3GetCCErrStatus(EDMA_INST_BASE) != 0))
    {
        /* Loop for EDMA3CC_ERR_HANDLER_RETRY_COUNT number of time,
         * breaks when no pending interrupt is found
         */
        while ((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT)
                    && (index != 0u))
        {
            index = 0u;

            if(isIPR)
            {
                   pendingIrqs = EDMA3GetErrIntrStatus(EDMA_INST_BASE);
            }
            else
            {
                   pendingIrqs = EDMA3ErrIntrHighStatusGet(EDMA_INST_BASE);
            }

            while (pendingIrqs)
            {
                   /*Process all the pending interrupts*/
                   if(TRUE == (pendingIrqs & 1u))
                   {
                      /* Write to EMCR to clear the corresponding EMR bits.
                       */
                        /*Clear any SER*/

                        if(isIPR)
                        {
                             EDMA3ClrMissEvt(EDMA_INST_BASE, index);
                        }
                        else
                        {
                             EDMA3ClrMissEvt(EDMA_INST_BASE, index + 32);
                        }
                   }
                   ++index;
                   pendingIrqs >>= 1u;
            }
            index = 0u;
            pendingIrqs = EDMA3QdmaGetErrIntrStatus(EDMA_INST_BASE);
            while (pendingIrqs)
            {
                /*Process all the pending interrupts*/
                if(TRUE == (pendingIrqs & 1u))
                {
                    /* Here write to QEMCR to clear the corresponding QEMR bits*/
                    /*Clear any QSER*/
                    EDMA3QdmaClrMissEvt(EDMA_INST_BASE, index);
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;


            pendingIrqs = EDMA3GetCCErrStatus(EDMA_INST_BASE);
            if (pendingIrqs != 0u)
            {
            /* Process all the pending CC error interrupts. */
            /* Queue threshold error for different event queues.*/
            for (evtqueNum = 0u; evtqueNum < SOC_EDMA3_NUM_EVQUE; evtqueNum++)
                {
                if((pendingIrqs & (1u << evtqueNum)) != 0u)
                {
                        /* Clear the error interrupt. */
                        EDMA3ClrCCErr(EDMA_INST_BASE, (1u << evtqueNum));
                    }
                }

            /* Transfer completion code error. */
            if ((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
            {
                EDMA3ClrCCErr(EDMA_INST_BASE,
                                      (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT));
            }
                ++index;
            }
            Cnt++;
        }
    }
}

void HSMMCSDIsr(void)
{
    volatile unsigned int status = 0;
    
    status = HSMMCSDIntrStatusGet(ctrlInfo.memBase, 0xFFFFFFFF);
    
    HSMMCSDIntrStatusClear(ctrlInfo.memBase, status);

    if (status & HS_MMCSD_STAT_CMDCOMP)
    {
        cmdCompFlag = 1;
    }

    if (status & HS_MMCSD_STAT_ERR)
    {
        errFlag = status & 0xFFFF0000;

        if (status & HS_MMCSD_STAT_CMDTIMEOUT)
        {
            cmdTimeout = 1;
        }

        if (status & HS_MMCSD_STAT_DATATIMEOUT)
        {
            dataTimeout = 1;
        }
    }

    if (status & HS_MMCSD_STAT_TRNFCOMP)
    {
        xferCompFlag = 1;
    }    
}


/*
** This function configures the AINTC to receive EDMA3 interrupts.
*/
static void EDMA3AINTCConfigure(void)
{
    /* Register the ISR in the Interrupt Vector Table.*/
    BSP_IntVectReg(EDMA_COMPLTN_INT_NUM, EDMA3COMPLETION_IRQHandler);
    IntPrioritySet(EDMA_COMPLTN_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ );
    /* Register the ISR in the Interrupt Vector Table.*/
    BSP_IntVectReg(EDMA_ERROR_INT_NUM, EDMA3ERROR_IRQHandler);
    IntPrioritySet(EDMA_ERROR_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ );
    
    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(EDMA_COMPLTN_INT_NUM);
    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(EDMA_ERROR_INT_NUM);
    
    /* Register the ISR in the Interrupt Vector Table.*/
    BSP_IntVectReg(MMCSD_INT_NUM, HSMMCSD_IRQHandler);
    IntPrioritySet(MMCSD_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ );
    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(MMCSD_INT_NUM); 
    
}


/* 
** Powering up, initializing and registering interrupts for EDMA.
*/

static void EDMA3Initialize(void)
{
    /* Initialization of EDMA3 */
    EDMA3Init(EDMA_INST_BASE, EVT_QUEUE_NUM);

    /* Configuring the AINTC to receive EDMA3 interrupts. */ 
    EDMA3AINTCConfigure();
}

static void HSMMCSDEdmaInit(void)
{
    /* Initializing the EDMA. */
    EDMA3Initialize();

    /* Request DMA Channel and TCC for MMCSD Transmit*/
    EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                        MMCSD_TX_EDMA_CHAN, MMCSD_TX_EDMA_CHAN,
                        EVT_QUEUE_NUM);

    /* Registering Callback Function for TX*/
    cb_Fxn[MMCSD_TX_EDMA_CHAN] = &callback;

    /* Request DMA Channel and TCC for MMCSD Receive */
    EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                        MMCSD_RX_EDMA_CHAN, MMCSD_RX_EDMA_CHAN,
                        EVT_QUEUE_NUM);

    /* Registering Callback Function for RX*/
    cb_Fxn[MMCSD_RX_EDMA_CHAN] = &callback;
}
/**
*\brief This function initializes the controller struct.\n
*
* \param - none.\n
*
* \return - none.\n
*
*/
static void HSMMCSDControllerSetup(void)
{
    ctrlInfo.memBase = MMCSD_BASE;
    ctrlInfo.ctrlInit = HSMMCSDControllerInit;
    ctrlInfo.xferSetup = HSMMCSDXferSetup;
    ctrlInfo.cmdStatusGet = HSMMCSDCmdStatusGet;
    ctrlInfo.xferStatusGet = HSMMCSDXferStatusGet;
    ctrlInfo.cardPresent = HSMMCSDCardPresent;
    ctrlInfo.cmdSend = HSMMCSDCmdSend;
    ctrlInfo.busWidthConfig = HSMMCSDBusWidthConfig;
    ctrlInfo.busFreqConfig = HSMMCSDBusFreqConfig;
    ctrlInfo.intrMask = (HS_MMCSD_INTR_CMDCOMP | HS_MMCSD_INTR_CMDTIMEOUT |
                            HS_MMCSD_INTR_DATATIMEOUT | HS_MMCSD_INTR_TRNFCOMP);
    ctrlInfo.intrEnable = HSMMCSDIntEnable;
    ctrlInfo.busWidth = (SD_BUS_WIDTH_1BIT | SD_BUS_WIDTH_4BIT);
    ctrlInfo.highspeed = 1;
    ctrlInfo.ocr = MMCSD_OCR;
    ctrlInfo.card = &sdCard;
    ctrlInfo.ipClk = MMCSD_IN_FREQ;
    ctrlInfo.opClk = MMCSD_INIT_FREQ;
    ctrlInfo.cdPinNum = HSMMCSD_CARD_DETECT_PINNUM;
    sdCard.ctrl = &ctrlInfo;

    //cmdCompFlag = 0;
    //cmdTimeout = 0;
    
    callbackOccured = 0;
    xferCompFlag = 0;
    dataTimeout = 0;
    cmdCompFlag = 0;
    cmdTimeout = 0;  
    
}

/**
*\brief This function mounts the devices.\n
*
* \param - driveNum - Drive number.\n
*
* \param - prt      - Device pointer.\n
*
* \return none.\n
*
*/
void HSMMCSDFsMount(unsigned int driveNum, void *ptr)
{
    g_sPState = 0;
    g_sCState = 0;
    f_mount(driveNum, &g_sFatFs);
    fat_devices[driveNum].dev = ptr;
    fat_devices[driveNum].fs = &g_sFatFs;
    fat_devices[driveNum].initDone = 0;
}

/***********************************************************************************************
* Function		: HSMMCSDInit()
* Description	        : This function initializes the MMCSD controller and mounts the device.
* Input			: none
* Output		: none
* Note(s)		: 
* Contributor	        : 10/12/2013	wangyao
***********************************************************************************************/
void HSMMCSDInit(void)
{
    /* Basic controller initializations */
    HSMMCSDControllerSetup();
    /* First check, if card is insterted */
    while(1)
    {
        if (MMCSDCardPresent(&ctrlInfo) == 0)
        {
            while(1);//SD卡没有找到
        }
        else
        {
            break;
        }
    }
    /* Initialize the MMCSD controller */
    MMCSDCtrlInit(&ctrlInfo);
    MMCSDIntEnable(&ctrlInfo);
    if((HSMMCSDCardPresent(&ctrlInfo)) == 1)
        HSMMCSDFsMount(0, &sdCard);
}
/***********************************************************************************************
* Function		: BSP_MMCSDInit()
* Description	        : SD卡初始化
* Input			: none
* Output		: none
* Note(s)		: 
* Contributor	        : 10/12/2013	wangyao
***********************************************************************************************/
void BSP_MMCSDInit(void)
{
    /* Configure the EDMA clocks. */
    EDMAModuleClkConfig();
    /* Configure EDMA to service the HSMMCSD events. */
    HSMMCSDEdmaInit();
    /* Enable clock for MMCSD and Do the PINMUXing */
    HSMMCSDPinMuxSetup();
    HSMMCSDModuleClkConfig();   
    HSMMCSDInit();
}
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
