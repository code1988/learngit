	 /*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: sysconfig.c
* Author			: 王耀
* Date First Issued	: 12/10/2010
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
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
static volatile unsigned int pageTable[4*1024];		// 一级页表基址，16k对齐
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
                        MMU_PGTYPE_SECTION, 		// 段大小1mb
                        START_ADDR_DDR, 			// DDR3起始地址0x80000000
                        NUM_SECTIONS_DDR,			// 段数量 512
                        MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA,
                                                     MMU_CACHE_WB_WA),
                        MMU_REGION_NON_SECURE, 
                        MMU_AP_PRV_RW_USR_RW,
                        (unsigned int*)pageTable	// 一级页表基址
                       };
    /*
    ** Define OCMC RAM region of AM335x. Same Attributes of DDR region given.
    */
    REGION regionOcmc = {
                         MMU_PGTYPE_SECTION, 
                         START_ADDR_OCMC, 			// OCMC起始地址0x40300000
                         NUM_SECTIONS_OCMC,			// 段数量 1
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
                        START_ADDR_DEV, 			// DEV起始地址0x44000000
                        NUM_SECTIONS_DEV,			// 段数量 960
                        MMU_MEMTYPE_DEVICE_SHAREABLE,
                        MMU_REGION_NON_SECURE,
                        MMU_AP_PRV_RW_USR_RW  | MMU_SECTION_EXEC_NEVER,
                        (unsigned int*)pageTable
                       };

    /* Initialize the page table and MMU */
    MMUInit((unsigned int*)pageTable);			// 将一级页表基址写入MMU，并初始化一级页表

    /* Map the defined regions */
    MMUMemRegionMap(&regionDdr);				// 配置DDR虚拟地址映射
    MMUMemRegionMap(&regionOcmc);				// 配置OCMC虚拟地址映射
    MMUMemRegionMap(&regionDev);				// 配置DEV虚拟地址映射

    /* Now Safe to enable MMU */
    MMUEnable((unsigned int*)pageTable);		// 将配置好的段描述符写入MMU，使能MMU
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
/************************(C)COPYRIGHT 2010 浙江方泰****END OF FILE****************************/


