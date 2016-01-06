/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_flash.h
* Author			: 王耀
* Date First Issued	: 05/01/2015
* Version			: V
* Description		: （1）此版本软件需要自己配置使用AT45DB页大小和页尺寸，本可以做成读取芯片
                      自动配置，不管任何型号的硬件AT45DB,目前考虑项目进度不做。
                     （2）由于这里使用的SPI的使能是标准的SPI使能脚，但考虑后续可能的扩展性，函数
                      名仍然采用多个的配置。同时由于目前只用了1个芯片AT45DB的写保护管脚目前没有
                      实际意义，硬件已经默认置高。但对该管脚的配置仍然在留用，以保证多个芯片时候
                      逻辑的正常
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2015		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
关于应用开发人员如何分布FLASH线性空间，举例如下：（同时目前规定按此规则进行）
#define    CSCHECK_LEN                 1		  //预留校验位一个字节，可能不用，但是先预留。
//FLASH线性排布参数方式
#define    FLASH_PARA_STARTADDR        (0)   //开始位置
#define    PARA_3001_FLASHADDR        (FLASH_PARA_STARTADDR)
#define    PARA_3001_FLASHLEN          2
#define    PARA_3002_FLASHADDR        (PARA_3001_FLASHADDR+PARA_3001_FLASHLEN+CSCHECK_LEN)
#define    PARA_3002_FLASHLEN          2
***********************************************************************************************/
#ifndef	__BSP_FLASH_H_
#define	__BSP_FLASH_H_

#include "os_cpu.h"


/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
#define  AT45DBCHIP1		0
#define  AT45DBCHIP2		1
// FLASH 片数
#define  FlashChipNum    1

// flash 属性定义；
#define MAXPAGE 		8192 //AT45DB321D //  AT45DB161D4096 //
#define MAXPAGELEN 		528//对应我们现在的芯片
// flash 操作类型；
#define	MAX_BLOCK_SIZE	(MAXPAGELEN * 8u)	    				// 块尺寸
#define	MAX_SECTOR_SIZE	(MAXPAGELEN * 256u)	    				// 扇区尺寸
#define	MAX_CHIP_SIZE	(MAXPAGE * MAXPAGELEN)					// 每片尺寸
#define	MAX_SIZE		(FlashChipNum * MAX_CHIP_SIZE)	        // 总尺寸

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: BSP_FlashInitAT
* Description	: 初始化SPI0-Flash,SPI0只给AT45DB flash使用
* Input			:
* Return		:
* Note(s)		: 也就是初始化SPI
* Contributor	: 150105  王耀
***********************************************************************************************/
void BSP_FlashInitAT(void);

/*******************************************************************************
* Function Name  : BSP_WriteDataToAT
* Description    : AT45写接口函数
* Input          : FlashAddr - INT32U
*                  DataAddr	 - INT8U *
*				   Len
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_WriteDataToAT(INT32U FlashAddr,INT8U *DataAddr,INT32U Len);

/*******************************************************************************
* Function Name  : BSP_ReadDataFromAT
* Description    : AT45读接口函数
* Input          : FlashAddr - INT32U
*				   Len
* Output         : DataAddr	 - INT8U *
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_ReadDataFromAT(INT32U FlashAddr,INT8U *DataAddr,INT32U Len);

/**********************************************************************************************
* Function Name	: BSP_EraseAT
* Description	: AT45擦除
* Input			: addr:地址(0 - MAX_SIZE-1)
				  len:数据长度,这里的长度是32位的
* Return		: TRUE; FALSE
* Note(s)		: 所有AT45的地址都连续的,用户不需考虑使用的是哪片。大范围的擦除是需要大量时间,
                  用户在使用时需要注意。2片AT45DB321,最大擦除时间在100s左右
* Contributor	: 2015-01-05 王耀
***********************************************************************************************/
INT8U BSP_EraseAT(INT32U addr,INT32U len);

/*******************************************************************************
* Function Name  : BSP_WriteByteToAtWithoutErase
* Description    : Write One Byte To at45db161 Without Erase
* Input          : FlashAddr - Address to write
*                  val       - data to write
* Output         : None
* Return         : 0 - fail, 1 - ok
*******************************************************************************/
INT8U BSP_WriteByteToAtWithoutErase(INT32U FlashAddr,INT8U val);

/*******************************************************************************
* Function Name  : BSP_GetTotalSizeOfAt
* Description    : Get Total Size Of At Flash
* Input          : None
* Output         : None
* Return         : Total Size
*******************************************************************************/
INT32U BSP_GetTotalSizeOfAt(void);

#endif	//__BSP_FLASH_H_
/************************(C)COPYRIGHT 2015 浙江方泰*****END OF FILE****************************/
