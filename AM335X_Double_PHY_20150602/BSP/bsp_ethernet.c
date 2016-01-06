/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_ethernet.c
**Ӳ          ��: am335x
**��    ��    ��: wangyao
**��  ��  ��  ��: 131029
**��  ��  ��  ��: V0.1
**��          ��: ethernet��Ӳ����ʼ��������
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "os_cpu.h"
#include "dmtimer.h"
#include "lib_def.h"
#include "interrupt.h"
#include "soc_AM335x.h"
#include "interrupt.h"
#include "evmAM335x.h"
#include "am335x_irq.h"
#include "bsp_int.h"
//#include "lwiplib.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
void  (*ETH_PostSem)(void) = NULL;
/***********************************************************************************************
* Function		: BSP_CPSWIntrSetUp 
* Description	        : ����������жϳ�ʼ��
* Input			: none
* Output		: none
* Note(s)		: 1) This function SHOULD be called before any other BSP function is called.
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
static void BSP_CPSWIntrSetUp(void)
{
    /* Enable IRQ for ARM (in CPSR)*/
    //IntMasterIRQEnable();�ж�ʹ�ܳ�ʼ�����ڶ�ʱ��2ϵͳ����������Ҫʹ�ܣ����Է���ϵͳStick��
    //IntAINTCInit();
    /*ʹ����OS��ϵ��жϳ�ʼ�������ȼ����ú���*/
    /* Register the Receive ISR for Core 0 */
    BSP_IntVectReg(SYS_INT_3PGSWRXINT0, CPSWCore0Rx_IRQHandler);
    /* Register the Transmit ISR for Core 0 */
    BSP_IntVectReg(SYS_INT_3PGSWTXINT0, CPSWCore0Tx_IRQHandler);
    
    /* Set the priority */
    IntPrioritySet(SYS_INT_3PGSWTXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);
    IntPrioritySet(SYS_INT_3PGSWRXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enable the system interrupt */
    IntSystemEnable(SYS_INT_3PGSWTXINT0);
    IntSystemEnable(SYS_INT_3PGSWRXINT0);
    
}
/***********************************************************************************************
* Function		: BSP_EtherInit 
* Description	        : ����ڵĳ�ʼ��
* Input			: none
* Output		: none
* Note(s)		: 1) �ú�������Ҫ������LWIP�ĳ�ʼ���У����ǵ��������ȶ�����BSP����
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void  BSP_EtherInit(void)
{
    CPSWPinMuxSetup();          // �ܽ�Ƭѡ
    CPSWClkEnable();            // ʱ�ӳ�ʼ��  
    BSP_CPSWIntrSetUp();        // �жϹ����ʼ��
    EVMPortRGMIIModeSelect();   // PHY�ӿ�ģʽѡ��
}

/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/