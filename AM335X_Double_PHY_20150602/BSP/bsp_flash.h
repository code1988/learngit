/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bsp_flash.h
* Author			: ��ҫ
* Date First Issued	: 05/01/2015
* Version			: V
* Description		: ��1���˰汾�����Ҫ�Լ�����ʹ��AT45DBҳ��С��ҳ�ߴ磬���������ɶ�ȡоƬ
                      �Զ����ã������κ��ͺŵ�Ӳ��AT45DB,Ŀǰ������Ŀ���Ȳ�����
                     ��2����������ʹ�õ�SPI��ʹ���Ǳ�׼��SPIʹ�ܽţ������Ǻ������ܵ���չ�ԣ�����
                      ����Ȼ���ö�������á�ͬʱ����Ŀǰֻ����1��оƬAT45DB��д�����ܽ�Ŀǰû��
                      ʵ�����壬Ӳ���Ѿ�Ĭ���øߡ����Ըùܽŵ�������Ȼ�����ã��Ա�֤���оƬʱ��
                      �߼�������
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2015		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
����Ӧ�ÿ�����Ա��ηֲ�FLASH���Կռ䣬�������£���ͬʱĿǰ�涨���˹�����У�
#define    CSCHECK_LEN                 1		  //Ԥ��У��λһ���ֽڣ����ܲ��ã�������Ԥ����
//FLASH�����Ų�������ʽ
#define    FLASH_PARA_STARTADDR        (0)   //��ʼλ��
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
// FLASH Ƭ��
#define  FlashChipNum    1

// flash ���Զ��壻
#define MAXPAGE 		8192 //AT45DB321D //  AT45DB161D4096 //
#define MAXPAGELEN 		528//��Ӧ�������ڵ�оƬ
// flash �������ͣ�
#define	MAX_BLOCK_SIZE	(MAXPAGELEN * 8u)	    				// ��ߴ�
#define	MAX_SECTOR_SIZE	(MAXPAGELEN * 256u)	    				// �����ߴ�
#define	MAX_CHIP_SIZE	(MAXPAGE * MAXPAGELEN)					// ÿƬ�ߴ�
#define	MAX_SIZE		(FlashChipNum * MAX_CHIP_SIZE)	        // �ܳߴ�

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: BSP_FlashInitAT
* Description	: ��ʼ��SPI0-Flash,SPI0ֻ��AT45DB flashʹ��
* Input			:
* Return		:
* Note(s)		: Ҳ���ǳ�ʼ��SPI
* Contributor	: 150105  ��ҫ
***********************************************************************************************/
void BSP_FlashInitAT(void);

/*******************************************************************************
* Function Name  : BSP_WriteDataToAT
* Description    : AT45д�ӿں���
* Input          : FlashAddr - INT32U
*                  DataAddr	 - INT8U *
*				   Len
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_WriteDataToAT(INT32U FlashAddr,INT8U *DataAddr,INT32U Len);

/*******************************************************************************
* Function Name  : BSP_ReadDataFromAT
* Description    : AT45���ӿں���
* Input          : FlashAddr - INT32U
*				   Len
* Output         : DataAddr	 - INT8U *
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_ReadDataFromAT(INT32U FlashAddr,INT8U *DataAddr,INT32U Len);

/**********************************************************************************************
* Function Name	: BSP_EraseAT
* Description	: AT45����
* Input			: addr:��ַ(0 - MAX_SIZE-1)
				  len:���ݳ���,����ĳ�����32λ��
* Return		: TRUE; FALSE
* Note(s)		: ����AT45�ĵ�ַ��������,�û����迼��ʹ�õ�����Ƭ����Χ�Ĳ�������Ҫ����ʱ��,
                  �û���ʹ��ʱ��Ҫע�⡣2ƬAT45DB321,������ʱ����100s����
* Contributor	: 2015-01-05 ��ҫ
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
/************************(C)COPYRIGHT 2015 �㽭��̩*****END OF FILE****************************/
