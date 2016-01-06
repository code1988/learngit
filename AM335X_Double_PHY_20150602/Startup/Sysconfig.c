	 /*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: sysconfig.c
* Author			: ��ҫ
* Date First Issued	: 12/10/2010
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "ucos_ii.h"
#include <string.h>
#include "sysConfig.h"

/* Private define-----------------------------------------------------------------------------*/
#define START_ADDR_DDR                     (0x80000000)
#define START_ADDR_DEV                     (0x44000000)
#define START_ADDR_OCMC                    (0x40300000)
#define NUM_SECTIONS_DDR                   (512)
#define NUM_SECTIONS_DEV                   (960)
#define NUM_SECTIONS_OCMC                  (1)

/* page tables start must be aligned in 16K boundary */
#ifdef __TMS470__
#pragma DATA_ALIGN(pageTable, 16384);
static volatile unsigned int pageTable[4*1024];
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=16*1024
static volatile unsigned int pageTable[4*1024];		// һ��ҳ���ַ��16k����
#else
static volatile unsigned int pageTable[4*1024] __attribute__((aligned(16*1024)));
#endif
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: MMUConfigAndEnable 
* Description	: Function to setup MMU. This function Maps three regions (1. DDR
*                 2. OCMC and 3. Device memory) and enables MMU.
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void MMUConfigAndEnable(void)
{
    /*
    ** Define DDR memory region of AM335x. DDR can be configured as Normal
    ** memory with R/W access in user/privileged modes. The cache attributes
    ** specified here are,
    ** Inner - Write through, No Write Allocate
    ** Outer - Write Back, Write Allocate
    */
    REGION regionDdr = {
                        MMU_PGTYPE_SECTION, 		// �δ�С1mb
                        START_ADDR_DDR, 			// DDR3��ʼ��ַ0x80000000
                        NUM_SECTIONS_DDR,			// ������ 512
                        MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA,
                                                     MMU_CACHE_WB_WA),
                        MMU_REGION_NON_SECURE, 
                        MMU_AP_PRV_RW_USR_RW,
                        (unsigned int*)pageTable	// һ��ҳ���ַ
                       };
    /*
    ** Define OCMC RAM region of AM335x. Same Attributes of DDR region given.
    */
    REGION regionOcmc = {
                         MMU_PGTYPE_SECTION, 
                         START_ADDR_OCMC, 			// OCMC��ʼ��ַ0x40300000
                         NUM_SECTIONS_OCMC,			// ������ 1
                         MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA,
                                                      MMU_CACHE_WB_WA),
                         MMU_REGION_NON_SECURE, 
                         MMU_AP_PRV_RW_USR_RW,
                         (unsigned int*)pageTable
                        };

    /*
    ** Define Device Memory Region. The region between OCMC and DDR is
    ** configured as device memory, with R/W access in user/privileged modes.
    ** Also, the region is marked 'Execute Never'.
    */
    REGION regionDev = {
                        MMU_PGTYPE_SECTION, 
                        START_ADDR_DEV, 			// DEV��ʼ��ַ0x44000000
                        NUM_SECTIONS_DEV,			// ������ 960
                        MMU_MEMTYPE_DEVICE_SHAREABLE,
                        MMU_REGION_NON_SECURE,
                        MMU_AP_PRV_RW_USR_RW  | MMU_SECTION_EXEC_NEVER,
                        (unsigned int*)pageTable
                       };

    /* Initialize the page table and MMU */
    MMUInit((unsigned int*)pageTable);			// ��һ��ҳ���ַд��MMU������ʼ��һ��ҳ��

    /* Map the defined regions */
    MMUMemRegionMap(&regionDdr);				// ����DDR�����ַӳ��
    MMUMemRegionMap(&regionOcmc);				// ����OCMC�����ַӳ��
    MMUMemRegionMap(&regionDev);				// ����DEV�����ַӳ��

    /* Now Safe to enable MMU */
    MMUEnable((unsigned int*)pageTable);		// �����úõĶ�������д��MMU��ʹ��MMU
}
/***********************************************************************************************
* Function		: DelayUS 
* Description	        : Inserts a delay time.
* Input			: nCount: specifies the delay time length in us.
* Output		: 
* Note(s)		: 
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void DelayUS(volatile INT32U nCount)
{
	volatile INT32U Tmpi;

	/* system clock = 40MHZ,1us = 40 instruction circle */
	nCount = nCount * 4;

  	for(; nCount != 0; nCount--)
	{   /* 10 instruction circle delay */
		Tmpi = nCount + 1;	  //do as NOP
		Tmpi = nCount + 2;	  //do as NOP
		Tmpi = nCount + 3;	  //do as NOP
		Tmpi = nCount + 4;	  //do as NOP
			Tmpi = nCount + 5;	  //do as NOP
		/* another three instruction for "nCount--"+1 "CMP"+1 and "BNE"+3 */
	}
}
/***********************************************************************************************
* Function		: Delay200NS
* Description	        : delay time 200 NS.
* Input			: Nothing
* Output		: 
* Note(s)		: 
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void Delay200NS(void)
{
	volatile INT32U Tmpi;
	volatile INT32U Tmpj;
	
	/* system clock = 40MHZ, 200NS = 8 instruction circles */
	
	/* "BL W" , 3 instruction circles                      */

	Tmpi = 100;	      //do as NOP
	/* "BX x" , 3 instruction circles                      */	
}
/***********************************************************************************************
* Function		: WWdg_Init
* Description	        : Window Watch dog config.
                          1)We must make sure that all the code in Critical region less
                            than ONE WDGTB tick(~6.5ms,as below).

                          2)Other case, We must make sure OSTaskIdleHook may be run within
                           (0x7F - 0x3F) WDGTB ticks(~419ms,as below).
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void WWdg_Init(void)
{

}
/************************(C)COPYRIGHT 2010 �㽭��̩****END OF FILE****************************/


