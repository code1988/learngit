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
#include "bsp_edma.h"

/* Private variables--------------------------------------------------------------------------*/
volatile int IrqRaised;
void (*EDMAAppCallbackFxn[EDMA3_NUM_TCC])(unsigned int status);

#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = SOC_CACHELINE_SIZE_MAX
volatile char SrcBuff[EDMAAPP_MAX_BUFFER_SIZE];
volatile char DstBuff[EDMAAPP_MAX_BUFFER_SIZE];
#endif
/* Private functions--------------------------------------------------------------------------*/
/* Callback function */
void EDMAAppCallback(unsigned int status)
{
    if(EDMA3_XFER_COMPLETE == status)
    {
        /* Transfer completed successfully */
        IrqRaised = EDMAAPP_IRQ_STATUS_XFER_COMP;
    }
    else if(EDMA3_CC_DMA_EVT_MISS == status)
    {
        /* Transfer resulted in DMA event miss error. */
        IrqRaised = EDMAAPP_IRQ_STATUS_DMA_EVT_MISS;
    }
    else if(EDMA3_CC_QDMA_EVT_MISS == status)
    {
        /* Transfer resulted in QDMA event miss error. */
        IrqRaised = EDMAAPP_IRQ_STATUS_QDMA_EVT_MISS;
    }
}

// 注册EDMA中断
void _EDMAAppRegisterEdma3Interrupts()
{
    /* Enable IRQ in CPSR. */
    IntMasterIRQEnable();

    /* Register Interrupts Here */

    /******************** Completion Interrupt ********************************/

    // 向量表中注册 Edma3ComplHandler0 
    IntRegister(SYS_INT_EDMACOMPINT , _EDMAAppEdma3ccComplIsr);

    // 设置中断优先级
    IntPrioritySet(SYS_INT_EDMACOMPINT, 0u, AINTC_HOSTINT_ROUTE_IRQ);

    // 在AINTC中使能SYS_INT_EDMACOMPINT中断
    IntSystemEnable(SYS_INT_EDMACOMPINT);

    /********************** CC Error Interrupt ********************************/

    // 向量表中注册 _EDMAAppEdma3ccErrIsr
    IntRegister(SYS_INT_EDMAERRINT , _EDMAAppEdma3ccErrIsr);

    // 设置中断优先级
    IntPrioritySet(SYS_INT_EDMAERRINT, 0u, AINTC_HOSTINT_ROUTE_IRQ);

    // 在AINTC中使能SYS_INT_EDMAERRINT中断
    IntSystemEnable(SYS_INT_EDMAERRINT);
}

/*
** ISR for successful transfer completion.
**
** Note: This function first disables its own interrupt to make it non-entrant.
*/
void _EDMAAppEdma3ccComplIsr()
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int isIntrPending = 0u;
    volatile unsigned int isHighIntrPending = 0u;
    unsigned int index;
    unsigned int count = 0u;

    index = 1u;
    isIntrPending  = EDMA3GetIntrStatus(EDMAAPP_EDMACC_BASE_ADDRESS);
    isHighIntrPending = EDMA3IntrStatusHighGet(EDMAAPP_EDMACC_BASE_ADDRESS);

    if(isIntrPending | isHighIntrPending)
    {
        while ((count < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (index != 0u))
        {
            index = 0u;

            if(isIntrPending)
            {
                pendingIrqs = EDMA3GetIntrStatus(EDMAAPP_EDMACC_BASE_ADDRESS);
            }
            else
            {
                pendingIrqs =
                            EDMA3IntrStatusHighGet(EDMAAPP_EDMACC_BASE_ADDRESS);
            }
            while(pendingIrqs)
            {
                if(TRUE == (pendingIrqs & 1u))
                {
                    /*
                    **
                    ** If the user has not given any Callback function
                    ** while requesting the TCC, its TCC specific bit
                    ** in the IPR register will NOT be cleared.
                    */
                    if(isIntrPending)
                    {
                        /*
                        ** Here write to ICR to clear the corresponding
                        ** IPR bits
                        */
                        EDMA3ClrIntr(EDMAAPP_EDMACC_BASE_ADDRESS, index);
                        (*EDMAAppCallbackFxn[index])(EDMA3_XFER_COMPLETE);
                    }
                    else
                    {
                        /*
                        ** Here write to ICR to clear the corresponding
                        ** IPR bits
                        */
                        EDMA3ClrIntr(EDMAAPP_EDMACC_BASE_ADDRESS, index + 32u);
                        (*EDMAAppCallbackFxn[index + 32u])(EDMA3_XFER_COMPLETE);
                    }

                }
                ++index;
                pendingIrqs >>= 1u;
            }
            count++;
        }
    }
}

/*
** Interrupt ISR for Channel controller error.
**
** Note: This function first disables its own interrupt to make it non-entrant.
*/
void _EDMAAppEdma3ccErrIsr()
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int evtQueNum = 0u; /* Event Queue Num */
    volatile unsigned int isHighIntrPending = 0u;
    volatile unsigned int isIntrPending = 0u;
    volatile unsigned int count = 0u;
    volatile unsigned int index;

    pendingIrqs = 0u;
    index = 1u;

    isIntrPending  = EDMA3GetIntrStatus(EDMAAPP_EDMACC_BASE_ADDRESS);
    isHighIntrPending = EDMA3IntrStatusHighGet(EDMAAPP_EDMACC_BASE_ADDRESS);

    if((isIntrPending | isHighIntrPending ) ||
       (EDMA3QdmaGetErrIntrStatus(EDMAAPP_EDMACC_BASE_ADDRESS) != 0u)
        || (EDMA3GetCCErrStatus(EDMAAPP_EDMACC_BASE_ADDRESS) != 0u))
    {
        /* Loop for EDMA3CC_ERR_HANDLER_RETRY_COUNT number of time,
        ** breaks when no pending interrupt is found
        */
        while ((count < EDMA3CC_ERR_HANDLER_RETRY_COUNT) && (index != 0u))
        {
            index = 0u;

            if(isIntrPending)
            {
                pendingIrqs =
                             EDMA3GetErrIntrStatus(EDMAAPP_EDMACC_BASE_ADDRESS);
            }
            else
            {
                pendingIrqs =
                         EDMA3ErrIntrHighStatusGet(EDMAAPP_EDMACC_BASE_ADDRESS);
            }

            while(pendingIrqs)
            {
                /* Process all the pending interrupts */
                if(TRUE == (pendingIrqs & 1u))
                {
                    /*
                    ** Write to EMCR to clear the corresponding EMR bits.
                    ** Clear any SER
                    */
                    if(isIntrPending)
                    {
                        EDMA3ClrMissEvt(EDMAAPP_EDMACC_BASE_ADDRESS, index);
                    }
                    else
                    {
                        EDMA3ClrMissEvt(EDMAAPP_EDMACC_BASE_ADDRESS,
                                        index + 32u);
                    }
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;
            pendingIrqs =
                         EDMA3QdmaGetErrIntrStatus(EDMAAPP_EDMACC_BASE_ADDRESS);
            while(pendingIrqs)
            {
                /* Process all the pending interrupts */
                if(TRUE == (pendingIrqs & 1u))
                {
                    /*
                    ** Here write to QEMCR to clear the corresponding QEMR bits
                    ** Clear any QSER
                    */
                    EDMA3QdmaClrMissEvt(EDMAAPP_EDMACC_BASE_ADDRESS, index);
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;

            pendingIrqs = EDMA3GetCCErrStatus(EDMAAPP_EDMACC_BASE_ADDRESS);
            if(pendingIrqs != 0u)
            {
                /*
                ** Process all the pending CC error interrupts.
                ** Queue threshold error for different event queues.
                */
                for(evtQueNum = 0u; evtQueNum < EDMAAPP_EDMACC_BASE_ADDRESS;
                    evtQueNum++)
                {
                    if((pendingIrqs & (1u << evtQueNum)) != 0u)
                    {
                        /* Clear the error interrupt. */
                        EDMA3ClrCCErr(EDMAAPP_EDMACC_BASE_ADDRESS,
                                      (1u << evtQueNum));
                    }
                }

                /* Transfer completion code error. */
                if((pendingIrqs & (1u << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
                {
                    EDMA3ClrCCErr(EDMAAPP_EDMACC_BASE_ADDRESS,
                                  (1u << EDMA3CC_CCERR_TCCERR_SHIFT));
                }
                ++index;
            }
            count++;
        }
    }
}


/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/