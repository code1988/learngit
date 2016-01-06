/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_int.c
**创    建    人: wangyao
**创  建  日  期: 2013-09-25
**最  新  版  本: V0.1
**描          述: 中断向量表管理程序
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人:
**日          期:
**版          本:
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/

/* Includes-----------------------------------------------------------------------------------*/
#define  BSP_INT_MODULE
#include  <cpu.h>
#include  <cpu_core.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <bsp_int.h>
#include "interrupt.h"
#include "def_config.h"


/* Private define-----------------------------------------------------------------------------*/
#define  AM35XX_REG_INTCPS_BASE_ADDR                    0x48200000
#define  REG_INTCPS_SYSCONFIG                          (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x010))
#define  REG_INTCPS_SYSSTATUS                          (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x014))
#define  REG_INTCPS_SIR_IRQ                            (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x040))
#define  REG_INTCPS_SIR_FIQ                            (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x044))
#define  REG_INTCPS_CONTROL                            (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x048))
#define  REG_INTCPS_PROTECTION                         (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x04C))
#define  REG_INTCPS_IDLE                               (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x050))
#define  REG_INTCPS_IRQ_PRIORITY                       (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x060))
#define  REG_INTCPS_FIQ_PRIORITY                       (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x064))
#define  REG_INTCPS_THRESHOLD                          (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x068))
#define  REG_INTCPS_ITR(x)                             (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x080 + (0x020 * x)))
#define  REG_INTCPS_MIR(x)                             (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x084 + (0x020 * x)))
#define  REG_INTCPS_MIR_CLEAR(x)                       (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x088 + (0x020 * x)))
#define  REG_INTCPS_MIR_SET(x)                         (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x08C + (0x020 * x)))
#define  REG_INTCPS_ISR_SET(x)                         (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x090 + (0x020 * x)))
#define  REG_INTCPS_ISR_CLEAR(x)                       (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x094 + (0x020 * x)))
#define  REG_INTCPS_PENDING_IRQ(x)                     (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x098 + (0x020 * x)))
#define  REG_INTCPS_PENDING_FIQ(x)                     (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x09C + (0x020 * x)))
#define  REG_INTCPS_ILR(x)                             (*(CPU_REG32 *)(AM35XX_REG_INTCPS_BASE_ADDR + 0x100 + (0x004 * x)))

#define  INTCPS_SYSCONFIG_SOFTRESET						DEF_BIT_01
#define  INTCPS_SYSSTATUS_RESETDONE						DEF_BIT_00

#define  INTCPS_CONTROL_NEWFIQAGR						DEF_BIT_01
#define  INTCPS_CONTROL_NEWIRQAGR						DEF_BIT_00

#define  ACTIVE_IRQ_MASK                                0x7F
#define  ACTIVE_FIQ_MASK                                0x7F

#define  INTCPR_ILR_IRQ									DEF_BIT_NONE
#define  INTCPR_ILR_FIQ									DEF_BIT_00

#define  BSP_INT_ID_MAX        128u  //中断的最大个数
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
static  void  OS_BSP_ExceptHandler (CPU_INT08U  except_type);
/* Private functions--------------------------------------------------------------------------*/
//static CPU_FNCT_PTR BSP_IntVectTbl[BSP_INT_ID_MAX];
static  void  BSP_IntHandlerDummy (void);

/*
*********************************************************************************************************
*                                              BSP_IntInit()
*
* Description : Initialize interrupts:
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/
#include "soc_AM335x.h"
void  BSP_IntInit (void)
{
    CPU_INT32U  int_id;


    REG_INTCPS_SYSCONFIG = INTCPS_SYSCONFIG_SOFTRESET;

    while ((REG_INTCPS_SYSSTATUS & INTCPS_SYSSTATUS_RESETDONE) != INTCPS_SYSSTATUS_RESETDONE)
    {
    	;
    }
    /* Disable all pending interrupts */
    REG_INTCPS_ISR_CLEAR(0) = DEF_BIT_FIELD(32u, 0u);
    REG_INTCPS_ISR_CLEAR(1) = DEF_BIT_FIELD(32u, 0u);
    REG_INTCPS_ISR_CLEAR(2) = DEF_BIT_FIELD(32u, 0u);
    REG_INTCPS_ISR_CLEAR(3) = DEF_BIT_FIELD(32u, 0u);

//    /* Reset the ARM interrupt controller */
//    HWREG(SOC_AINTC_REGS + INTC_SYSCONFIG) = INTC_SYSCONFIG_SOFTRESET;
//
//    /* Wait for the reset to complete */
//    while((HWREG(SOC_AINTC_REGS + INTC_SYSSTATUS)
//          & INTC_SYSSTATUS_RESETDONE) != INTC_SYSSTATUS_RESETDONE);
//
//    /* Enable any interrupt generation by setting priority threshold */
//    HWREG(SOC_AINTC_REGS + INTC_THRESHOLD) =
//                                       INTC_THRESHOLD_PRIORITYTHRESHOLD;

    for (int_id = 0; int_id < BSP_INT_ID_MAX; int_id++)
    {
        BSP_IntVectReg((CPU_INT08U  )int_id,BSP_IntHandlerDummy);//初始化全部用BSP_IntHandlerDummy填充
    }

    REG_INTCPS_CONTROL = (INTCPS_CONTROL_NEWFIQAGR | INTCPS_CONTROL_NEWIRQAGR);
}


/*
*********************************************************************************************************
*                                          BSP_IntClr()
*
* Description : This function clears an interrupt
*
* Argument(s) : int_id        The interrupt ID
*                             BSP_INT_ID_ICWDTA0
*                             BSP_INT_ID_ICWDTA1
*                             BSP_INT_ID_ICOSTM0
*									.
*									.
*							  BSP_INT_ID_ICHDMA7
* Returns     : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntClr (CPU_INT08U  int_id)
{
    CPU_INT08U  reg_nbr;


    if (int_id > BSP_INT_ID_MAX) {
        return;
    }

    reg_nbr = int_id / 32;

    REG_INTCPS_ISR_CLEAR(reg_nbr) = DEF_BIT(int_id % 32);
}


/*
*********************************************************************************************************
*                                             BSP_IntDis()
*
* Description : This function disables an interrupt.
*
* Argument(s) : int_id        The interrupt id
*                             BSP_INT_ID_ICWDTA0
*                             BSP_INT_ID_ICWDTA1
*                             BSP_INT_ID_ICOSTM0
*									.
*									.
*							  BSP_INT_ID_ICHDMA7
* Returns     : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntDis (CPU_INT08U  int_id)
{
    CPU_INT08U  reg_nbr;

    /* -------------- ARGUMENTS CHECKING ---------------- */
    if (int_id > BSP_INT_ID_MAX) {
        return;
    }

    reg_nbr = int_id / 32;

    REG_INTCPS_MIR_SET(reg_nbr) = DEF_BIT(int_id % 32);
}


/*
*********************************************************************************************************
*                                           BSP_IntDisAll()
*
* Description : Disable ALL interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntDisAll (void)
{
                                                                /* Disable ALL interrupts                            */
    REG_INTCPS_MIR_SET(0) = DEF_BIT_FIELD(32u, 0u);
    REG_INTCPS_MIR_SET(1) = DEF_BIT_FIELD(32u, 0u);
    REG_INTCPS_MIR_SET(2) = DEF_BIT_FIELD(32u, 0u);
    REG_INTCPS_MIR_SET(3) = DEF_BIT_FIELD(32u, 0u);
}


/*
*********************************************************************************************************
*                                          BSP_IntEn()
*
* Description : This function enables an interrupt.
*
* Argument(s) : int_id        The interrupt id
*                             BSP_INT_ID_ICWDTA0
*                             BSP_INT_ID_ICWDTA1
*                             BSP_INT_ID_ICOSTM0
*									.
*									.
*							  BSP_INT_ID_ICHDMA7
* Returns     : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntEn (CPU_INT08U  int_id)
{
    CPU_INT08U  reg_nbr;


    if (int_id > BSP_INT_ID_MAX) {
        return;
    }

    reg_nbr = int_id / 32;

    REG_INTCPS_MIR_CLEAR(reg_nbr) = DEF_BIT(int_id % 32);
}


/*
*********************************************************************************************************
*                                            BSP_IntVectReg()
*
* Description : Assign ISR handler.
*
* Argument(s) : int_id      Interrupt for which vector will be set.
*
*				prio		Interrupt priority level. 0(Highest) - 15(Lowest)
*
*               isr         Handler to assign
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
//这里移植库文件里的fnRAMVectors，以统一使用中断向量表，同时可以方便参考例子程序功能开发
void  BSP_IntVectReg (CPU_INT08U int_id, void (*fnHandler)(void) )//CPU_FNCT_PTR  isr)
{
    CPU_SR_ALLOC();


    if (int_id > BSP_INT_ID_MAX)
    {
        return;
    }
    if (int_id < BSP_INT_ID_MAX)
    {
        CPU_CRITICAL_ENTER();
        /* Assign ISR */
        fnRAMVectors[int_id] = fnHandler;
        CPU_CRITICAL_EXIT();
    }
}


/*
*********************************************************************************************************
*                                        BSP_IntHandlerDummy()
*
* Description : Dummy interrupt handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_IntHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void  BSP_IntHandlerDummy (void)
{

}

#include "os_cpu.h"
/*
*********************************************************************************************************
*                                        BSP_IntHandler()
*
* Description : General Maskable Interrupt handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_IntHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_IntHandler (CPU_INT32U  src_nbr)
{
    CPU_INT32U    int_nbr;
    //CPU_FNCT_PTR  isr;
    void (*isr)(void);//中断函数指针

    switch (src_nbr)
    {
        case CPU_ARM_EXCEPT_IRQ:
             int_nbr = REG_INTCPS_SIR_IRQ;//读当前中断号
             isr = fnRAMVectors[int_nbr & ACTIVE_IRQ_MASK];
             //isr = BSP_IntVectTbl[int_nbr & ACTIVE_IRQ_MASK];
             if (isr != (void*)0)
             {
                 isr();//执行中断函数
             }
             BSP_IntClr(int_nbr);                               /* Clear interrupt									   	*/
             REG_INTCPS_CONTROL = INTCPS_CONTROL_NEWIRQAGR;
             break;

        case CPU_ARM_EXCEPT_FIQ:
             int_nbr = REG_INTCPS_SIR_FIQ;
             isr = fnRAMVectors[int_nbr & ACTIVE_FIQ_MASK];
             //isr = BSP_IntVectTbl[int_nbr & ACTIVE_FIQ_MASK];
             if (isr != (void*)0)
             {
                 isr();
             }
             BSP_IntClr(int_nbr);                       		/* Clear interrupt									   	*/
              REG_INTCPS_CONTROL = INTCPS_CONTROL_NEWFIQAGR;
             break;
        default:
             break;
    }
}
/*
*********************************************************************************************************
*                                          ARM INTERRUPT/EXCEPTION HANDLER
*
* Description : Handle ARM exceptions.
*
* Argument(s) : src_id     ARM exception source identifier:
*
*                                  OS_CPU_ARM_EXCEPT_RESET             0x00
*                                  OS_CPU_ARM_EXCEPT_UNDEF_INSTR       0x01
*                                  OS_CPU_ARM_EXCEPT_SWI               0x02
*                                  OS_CPU_ARM_EXCEPT_ABORT_PREFETCH    0x03
*                                  OS_CPU_ARM_EXCEPT_ABORT_DATA        0x04
*                                  OS_CPU_ARM_EXCEPT_RSVD              0x05
*                                  OS_CPU_ARM_EXCEPT_IRQ               0x06
*                                  OS_CPU_ARM_EXCEPT_FIQ               0x07
*
* Return(s)   : none.
*
* Caller(s)   : OS_CPU_ARM_ExceptHndlr(), which is declared in os_cpu_a.s.
*
* Note(s)     : (1) Only OS_CPU_ARM_EXCEPT_FIQ and OS_CPU_ARM_EXCEPT_IRQ exceptions handler are implemented.
*                   For the rest of the exception a infinite loop is implemented for debuging pruposes. This behavior
*                   should be replaced with another behavior (reboot, etc).
*********************************************************************************************************
*/
void  OS_CPU_IntHandler (CPU_INT32U  src_id)
{
    switch (src_id)
    {
        case OS_CPU_ARM_EXCEPT_IRQ:
        case OS_CPU_ARM_EXCEPT_FIQ:
             BSP_IntHandler((CPU_INT32U)src_id);
             break;
        case OS_CPU_ARM_EXCEPT_RST:
        case OS_CPU_ARM_EXCEPT_UND:
        case OS_CPU_ARM_EXCEPT_SWI:
        case OS_CPU_ARM_EXCEPT_ABORT_DATA:
        case OS_CPU_ARM_EXCEPT_ABORT_PREFETCH:
        case OS_CPU_ARM_EXCEPT_RSVD:
        default:
             OS_BSP_ExceptHandler((CPU_INT08U)src_id);
             break;
    }
}
/*
*********************************************************************************************************
*                                    OS_CSP_BSP_ExceptHandler()
*
* Description : Handles ARM exceptions.
*
* Argument(s) : Exception type.
*
*                   CPU_ARM_EXCEPT_RST              Reset exception.
*                   CPU_ARM_EXCEPT_UND              Undefined instruction.
*                   CPU_ARM_EXCEPT_SWI              Software interrupt.
*                   CPU_ARM_EXCEPT_ABORT_PREFETCH   Prefetch Abort.
*                   CPU_ARM_EXCEPT_ABORT_DATA       Data Abort.
*
* Return(s)   : none.
*
* Caller(s)   : OS_CPU_IntHandler().
*
* Note(s)     : (1) This exception handler is implemented with an infinite loop for
*                   debugging porpuses only.
*********************************************************************************************************
*/
static  void  OS_BSP_ExceptHandler (CPU_INT08U  except_type)
{
    //while(1);
    switch (except_type) {
        case CPU_ARM_EXCEPT_RST:
        case CPU_ARM_EXCEPT_UND:
        case CPU_ARM_EXCEPT_SWI:
        case CPU_ARM_EXCEPT_ABORT_PREFETCH:
        case CPU_ARM_EXCEPT_ABORT_DATA:
        {
            if(CPU_ARM_EXCEPT_RST==except_type)
            {
                DBG_OS("sys dead:Reset exception\r\n");
            }
            else if(CPU_ARM_EXCEPT_UND==except_type)
            {
                DBG_OS("sys dead:Undefined instruction\r\n");
            }
            else if(CPU_ARM_EXCEPT_SWI==except_type)
            {
                DBG_OS("sys dead:Software interrupt\r\n");
            }
            else if(CPU_ARM_EXCEPT_ABORT_PREFETCH==except_type)
            {
                DBG_OS("sys dead:Prefetch Abort\r\n");
            }
            else if(CPU_ARM_EXCEPT_ABORT_DATA==except_type)
            {
                DBG_OS("sys dead:Data Abort\r\n");
            }

            while (DEF_TRUE)
            {
                ;
            }

        }
        break;


    }
}










/************************(C)COPYRIGHT 2010 浙江方泰*****END OF FILE****************************/