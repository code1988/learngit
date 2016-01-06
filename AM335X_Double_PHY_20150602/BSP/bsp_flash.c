/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_flash.c
* Author			: 王耀
* Date First Issued	: 05/01/2015
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2015		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
//#include "sysconfig.h"
#include "bsp.h"
/* Private define-----------------------------------------------------------------------------*/
#define	AT45SPIWrite			BSP_SPIWrite	// SPI写函数重定义
#define	AT45SPIRead				BSP_SPIRead		// SPI读函数重定义
// at45db161D 与 at45db161B的区别: 出厂一样。
// 但at45db161D 可以设定 512每页。
// 增加Sector Erase, Security Register功能。

// flash 状态字 mask；
#define DB161_STATUS	0xd7
// flash 操作类型；
#define WRITEAT 		0
#define READAT 			1

//AT45DB161B指令集
/* Main memory page read */
//#define DB161_PAGE_READ                  0x52
#define DB161_PAGE_READ                  		0xd2
/* Buffer 1 read */
#define DB161_BUF1_READ                   		0x54
/* Buffer 2 read */
#define DB161_BUF2_READ                   		0x56
/* Main memory page to buffer 1 transfert */
#define DB161_PAGE_2_BUF1_TRF       			0x53
/* Main memory page to buffer 2 transfert */
#define DB161_PAGE_2_BUF2_TRF      				0x55
/* Main memory page to buffer 1 compare */
#define DB161_PAGE_2_BUF1_CMP      				0x60
/* Main memory page to buffer 2 compare */
#define DB161_PAGE_2_BUF2_CMP       			0x61
/* Buffer 1 write */
#define DB161_BUF1_WRITE                  		0x84
/* Buffer 2 write */
#define DB161_BUF2_WRITE                  		0x87
/* Buffer 1 to main memory page program with built-In erase */
#define DB161_BUF1_PAGE_ERASE_PGM      			0x83
/* Buffer 2 to main memory page program with built-In erase */
#define DB161_BUF2_PAGE_ERASE_PGM      			0x86
/* Buffer 1 to main memory page program without built-In erase */
#define DB161_BUF1_PAGE_PGM               		0x88
/* Buffer 2 to main memory page program without built-In erase */
#define DB161_BUF2_PAGE_PGM               		0x89
/* Page Erase */
#define DB161_PAGE_ERASE                  		0x81
/* Block Erase */
#define DB161_BLOCK_ERASE                 		0x50
/* Main memory page through buffer 1 */
#define DB161_PAGE_PGM_BUF1               		0x82
/* Main memory page through buffer 2 */
#define DB161_PAGE_PGM_BUF2               		0x85
/* Auto page rewrite throught buffer 1 */
#define DB161_AUTO_PAGE_PGM_BUF1        		0x58
/* Auto page rewrite throught buffer 2 */
#define DB161_AUTO_PAGE_PGM_BUF2        		0x59
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
void BSP_FlashInitAT(void)
{      
    _BSPSPI_CONFIG SPI_InitStructure;

    SPI_InitStructure.num			= BSPAT_SPINUM;
    SPI_InitStructure.pEvent 		= NULL;
    SPI_InitStructure.TxRespond 	= BSPSPI_RESPOND_NORMAL;	    // 轮询模式
    SPI_InitStructure.channel 		= BSPSPI_CHN0;			        // 0通道
    SPI_InitStructure.pTxBuffer 	= 0;			
    SPI_InitStructure.pRxBuffer 	= 0;
    SPI_InitStructure.MaxTxBuffer	= 0;
    SPI_InitStructure.MaxRxBuffer 	= 0;
    SPI_InitStructure.Mode 			= BSPSPI_WORKMODE_MASTER;       // 主模式
    SPI_InitStructure.RxOvertime 	= SYS_DELAY_10ms;
    BSP_SPIConfig(&SPI_InitStructure);
}

/*******************************************************************************
* Function Name  : At45db161_open
* Description    : 选定FLASH，这里由于使用的是标准CS，所以直接调用标准SPI的操作
                    对于入参的考虑虽然目前没有用，为以后可以扩展外部存储做预留，保持不变。
                  函数名使用161 实际使用的是32,不要被函数名混淆
* Input          : FlashNo - FLASH 片号
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_open(INT8U FlashNo)
{
	//选择要操作的AT45,首先操作片选信号
	//片选信号低有效
    BSP_SPICSEnable(BSPAT_SPINUM);

}

/*******************************************************************************
* Function Name  : At45db161_close
* Description    : 终止 FLASH 访问
* Input          : FlashNo - FLASH 片号
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_close(INT8U FlashNo)
{
    BSP_SPICSDisEnable(BSPAT_SPINUM);
}

/*******************************************************************************
* Function Name  : At45db161_DisWP
* Description    : FLASH 关闭写保护，由于目前设备只用了一个flash,WP管脚硬件已经默认
                    但考虑到软件的裸机周全性，空函数预留不做改变
* Input          : FlashNo - FLASH 片号
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_DisWP(INT8U FlashNo)
{
	//写保护信号低有效
}

/*******************************************************************************
* Function Name  : At45db161_EnWP
* Description    : FLASH 打开写保护
* Input          : FlashNo - FLASH 片号
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_EnWP(INT8U FlashNo)
{
	//写保护信号低有效
}

/*******************************************************************************
* Function Name  : CloseFlashAT
* Description    : Close All Flash
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CloseFlashAT(void)
{
	At45db161_close(AT45DBCHIP1);
	At45db161_close(AT45DBCHIP2);
	DelayUS(2);
}

/*******************************************************************************
* Function Name  : ReadFlash_Status
* Description    : 读取FLASH的状态
* Input          : FlashNo - FLASH 片号
				   Status  - 存放地址
* Output         : None
* Return         : TRUE / FALSE.
*******************************************************************************/
INT8U ReadFlash_Status(INT8U FlashNo,INT8U *status)
{
	INT8U spi_com_buf = DB161_STATUS ;
    INT8U err;
    
    BSP_SPICSEnable(BSPAT_SPINUM); //必须要有，表明一个开关周期
	if(AT45SPIWrite(BSPAT_SPINUM,&spi_com_buf,1) == TRUE)	// SPI写
		err=AT45SPIRead(BSPAT_SPINUM,status,1);	// SPI读
	else
		err = FALSE;
    BSP_SPICSDisEnable(BSPAT_SPINUM);
	return err;
}

/*******************************************************************************
* Function Name  : WaitFlashFree
* Description    : 等待FLASH空闲
* Input          : FlashNo - FLASH 片号
* Output         : None
* Return         : None
*******************************************************************************/
INT8U  WaitFlashFree(INT8U FlashNo)
{
	INT8U status = 0x00;

	while(1)
	{
		//读取FLASH的状态字节
		if ( ReadFlash_Status(FlashNo,&status) == FALSE )
			return FALSE;
		//空闲退出
		if( (status & 0x80) != 0)
			break;		
	}

	return TRUE;
}

/*******************************************************************************
* Function Name  : FlashPageToBuf
* Description    : Flash Page 内容读到 Flash Buf
* Input          : FlashNo - FLASH 片号
*                  command - 命令字
*                  PageNo  - 页地址
* Output         : None
* Return         : TRUE/FALSE
*******************************************************************************/
INT8U FlashPageToBuf(INT8U FlashNo,INT8U command,INT16U PageNo)
{
	INT8U spi_com_buf[4];

	At45db161_open(FlashNo);
	 
	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 5);
		spi_com_buf[2] = (INT8U)((PageNo << 3) & 0xff);
		spi_com_buf[3] = 0x00;	
	}
	else
	{//at45db321d, 
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 6);
		spi_com_buf[2] = (INT8U)((PageNo << 2) & 0xff);
		spi_com_buf[3] = 0x00;
	}
			
	if( AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4) == FALSE )
	{
		At45db161_close(FlashNo);
		return FALSE;
	}
	
	At45db161_close(FlashNo);
	
	if( WaitFlashFree(FlashNo) == FALSE )
		return FALSE;

	return TRUE;
}

/*******************************************************************************
* Function Name  : WaitCmpResult
* Description    : 等待比较结果
* Input          : FlashNo - FLASH 片号
* Output         : None
* Return         : TRUE/FALSE
*******************************************************************************/
INT8U WaitCmpResult(INT8U FlashNo)
{
	INT8U status = 0x00;
	
	while(1)
	{
		//读取FLASH的状态字节
		if( ReadFlash_Status(FlashNo,&status) == FALSE )
			return(FALSE);
		//空闲退出
		if( (status & 0x80) != 0)
			break;
	}

	if((status & 0x40) != 0 ) 
		return(FALSE);
	else
		return(TRUE);
}

/*******************************************************************************
* Function Name  : FlashCompare
* Description    : 比较写入的内容是否一致
* Input          : FlashNo - FLASH 片号
*                  command - 命令字
*                  PageNo  - 页地址
* Output         : None
* Return         : TRUE/FALSE
*******************************************************************************/
INT8U FlashCompare(INT8U FlashNo,INT8U command,INT16U PageNo)
{
	INT8U spi_com_buf[4];
	INT8U ret = TRUE;
	
	At45db161_open(FlashNo);
	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 5);
		spi_com_buf[2] = (INT8U)((PageNo << 3) & 0xff);
		spi_com_buf[3] = 0x00;
	}
	else
	{ 
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 6);
		spi_com_buf[2] = (INT8U)((PageNo << 2) & 0xff);
		spi_com_buf[3] = 0x00;
	}
		
	ret = AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4);
	At45db161_close(FlashNo);
	
	return ret;
}

 /*******************************************************************************
* Function Name  : Page_Pgm_Data
* Description    : 写入一页的内容
* Input          : FlashNo - FLASH 片号
*                  command - 命令字
*                  PageNo  - 页地址
*				   InPageNo- 页内地址
*                  data    - 待写数据
*                  len     - 待写数据长度      
* Output         : None
* Return         : TRUE/FALSE
*******************************************************************************/
INT8U Page_Pgm_Data(INT8U FlashNo,INT8U command,INT16U PageNo,INT16U InPageNo,INT8U data,INT32U len)
{
	INT8U spi_com_buf[4];
	INT8U ret;
	INT8U command1 = DB161_PAGE_2_BUF1_TRF;

	At45db161_DisWP(FlashNo);
	
	if( command == DB161_PAGE_PGM_BUF1 )  command1 = DB161_PAGE_2_BUF1_TRF;
	else if( command == DB161_PAGE_PGM_BUF2 )  command1 = DB161_PAGE_2_BUF2_TRF;

	if ( FlashPageToBuf(FlashNo,command1,PageNo) == FALSE )
	{
		At45db161_DisWP(FlashNo);
		return FALSE;
	}
	//如上	
	At45db161_open(FlashNo);

	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 5);
		spi_com_buf[2] = (INT8U)(((PageNo << 3) & 0xff) |( InPageNo >> 8 ));
		spi_com_buf[3] = (INT8U)( InPageNo & 0xff );
	}
	else
	{//at45db321d,
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 6);
		spi_com_buf[2] = (INT8U)(((PageNo << 2) & 0xff) |( InPageNo >> 8 ));
		spi_com_buf[3] = (INT8U)( InPageNo & 0xff );
	}
		
	if( AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4) == FALSE )
	{
		At45db161_close(FlashNo);
		At45db161_DisWP(FlashNo);
		return FALSE;
	}
	for(; len > 0; len--)
	{
		if( AT45SPIWrite(BSPAT_SPINUM,&data,1) == FALSE )
		{
			At45db161_close(FlashNo);
			At45db161_DisWP(FlashNo);
			return FALSE;
		}
	}

	At45db161_close(FlashNo);
	
	if( WaitFlashFree(FlashNo) == FALSE )
	{
		At45db161_DisWP(FlashNo);
		return FALSE;
	}
	
	if( command == DB161_PAGE_PGM_BUF1 )  command1 = DB161_PAGE_2_BUF1_CMP;
	else if( command == DB161_PAGE_PGM_BUF2 )  command1 = DB161_PAGE_2_BUF2_CMP;

	if ( FlashCompare(FlashNo,command1,PageNo) == FALSE )
	{
		At45db161_DisWP(FlashNo);
		return FALSE;
	}
	
	ret = WaitCmpResult(FlashNo);
	
	At45db161_EnWP(FlashNo);
	return(ret);
}

/*******************************************************************************
* Function Name  : Page_Pgm_Buf
* Description    : 写入一页的内容
* Input          : FlashNo - FLASH 片号
*                  command - 命令字
*                  PageNo  - 页地址
*				   InPageNo- 页内地址
*                  addr    - 待写数据存放首地址
*                  len     - 待写数据长度      
* Output         : None
* Return         : TRUE/FALSE
*******************************************************************************/
INT8U Page_Pgm_Buf(INT8U FlashNo,INT8U command,INT16U PageNo,INT16U InPageNo,INT8U *addr,INT32U len)
{
	INT8U spi_com_buf[4];
	INT8U ret;
	INT8U command1 = DB161_PAGE_2_BUF1_TRF;

	At45db161_DisWP(FlashNo);
	
	if( command == DB161_PAGE_PGM_BUF1 )  command1 = DB161_PAGE_2_BUF1_TRF;
	else if( command == DB161_PAGE_PGM_BUF2 )  command1 = DB161_PAGE_2_BUF2_TRF;

	if ( FlashPageToBuf(FlashNo,command1,PageNo) == FALSE )
	{
		At45db161_DisWP(FlashNo);
		return FALSE;
	}
	//如上	
	At45db161_open(FlashNo);

	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 5);
		spi_com_buf[2] = (INT8U)(((PageNo << 3) & 0xff) |( InPageNo >> 8 ));
		spi_com_buf[3] = (INT8U)( InPageNo & 0xff );
	}
	else
	{//at45db321d
		spi_com_buf[0] = command;
		spi_com_buf[1] = (INT8U)(PageNo >> 6);
		spi_com_buf[2] = (INT8U)(((PageNo << 2) & 0xff) |( InPageNo >> 8 ));
		spi_com_buf[3] = (INT8U)( InPageNo & 0xff );
	}
		
	if( AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4) == FALSE )
	{
		At45db161_close(FlashNo);
		At45db161_DisWP(FlashNo);
		return FALSE;
	}

	if( AT45SPIWrite(BSPAT_SPINUM,addr,len) == FALSE )
	{
		At45db161_close(FlashNo);
		At45db161_DisWP(FlashNo);
		return FALSE;
	}

	At45db161_close(FlashNo);
	
	if( WaitFlashFree(FlashNo) == FALSE )
	{
		At45db161_DisWP(FlashNo);
		return FALSE;
	}
	
	if( command == DB161_PAGE_PGM_BUF1 )  command1 = DB161_PAGE_2_BUF1_CMP;
	else if( command == DB161_PAGE_PGM_BUF2 )  command1 = DB161_PAGE_2_BUF2_CMP;

	if ( FlashCompare(FlashNo,command1,PageNo) == FALSE )
	{
		At45db161_DisWP(FlashNo);
		return FALSE;
	}
	
	ret = WaitCmpResult(FlashNo);
	
	At45db161_EnWP(FlashNo);
	return(ret);
}

/*******************************************************************************
* Function Name  : ReadFlash_Page
* Description    : 读取一页的内容
* Input          : FlashNo - FLASH 片号
*                  PageNo  - 页地址
*				   InPageNo- 页内地址
*                  len     - 读取数据长度      
* Output         : addr    - 读数据存放首地址
* Return         : TRUE/FALSE
*******************************************************************************/
//opcode:0x52
//F Byte:r,r,PA11-PA6
//S Byte:PA5-PA0,BA9-BA8
//T Byte:BA7-BA0
//x 4Byte:0x00
//读整页528个字节
//BA9-BA0: 0x000
//PA11-PA0: PageNo
INT8U ReadFlash_Page(INT8U FlashNo,INT16U PageNo,INT16U InPageNo,INT8U *addr,INT32U len)
{
	INT8U spi_com_buf[8] = {DB161_PAGE_READ, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF};
	
	//发送读取命令
	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[1] = (INT8U)(PageNo >> 5);
		spi_com_buf[2] = (INT8U)(((PageNo << 3) & 0xff) |( InPageNo >> 8 ));
		spi_com_buf[3] = (INT8U)( InPageNo & 0xff );
	}
	else
	{//at45db321d
		spi_com_buf[1] = (INT8U)(PageNo >> 6);
		spi_com_buf[2] = (INT8U)(((PageNo << 2) & 0xff) |( InPageNo >> 8 ));
		spi_com_buf[3] = (INT8U)( InPageNo & 0xff );
	}

	//选通FLASH
	At45db161_open(FlashNo);
		
	if( AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,8) == FALSE )
	{
		At45db161_close(FlashNo);
		return FALSE;
	}

	//接收数据
	if( AT45SPIRead(BSPAT_SPINUM,addr,len) == FALSE )
	{
		At45db161_close(FlashNo);
		return FALSE;
	}

	//关闭FLASH操作 
	At45db161_close(FlashNo);

	return(TRUE);
}

/*******************************************************************************
* Function Name  : ATOperation
* Description    : AT45读写操作函数
* Input          : Opflag - 操作类型：读 / 写
                   FlashNo   - FLASH 片号
*          		   FlashAddr - INT32U
*                  DataAddr	 - INT8U *	写操作
*				   Len        
* Output         : DataAddr	 - INT8U *	读操作
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U ATOperation(INT8U Opflag,INT8U FlashNo,INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
	//Flash 页地址，0~4095
	INT16U FPageAddr;
	//Flash 页内地址，0~527
	INT16U FInPageAddr;

	INT16U LenPageTemp;
	INT8U  ret;
	
	//低16位存放Flash页内地址
	FInPageAddr = (INT16U) ( FlashAddr & 0xffff );
	//高16位存放Flash 页地址
	FPageAddr = (INT16U) ( FlashAddr>>16 );

	if( FPageAddr > ( MAXPAGE - 1) )
		return FALSE;

	if( FInPageAddr > ( MAXPAGELEN -1) )
		return FALSE;
	//数据溢出判断?

	//当前页还剩下的空间；
	LenPageTemp = MAXPAGELEN - FInPageAddr;
	//一页能处理完的情况
	if( Len <=  LenPageTemp )
	{
		if( Opflag == WRITEAT )
			ret = Page_Pgm_Buf( FlashNo,DB161_PAGE_PGM_BUF1,FPageAddr,FInPageAddr,DataAddr,Len );
		else
			ret = ReadFlash_Page( FlashNo,FPageAddr,FInPageAddr,DataAddr,Len );
	}
	//需分多页处理
	else
	{
		//先保存当前页数据
		if( Opflag == WRITEAT )
			ret = Page_Pgm_Buf( FlashNo,DB161_PAGE_PGM_BUF1,FPageAddr,FInPageAddr,DataAddr,LenPageTemp );
		else
			ret = ReadFlash_Page( FlashNo,FPageAddr,FInPageAddr,DataAddr,LenPageTemp );
		
		Len -= LenPageTemp;
		DataAddr += LenPageTemp;
		
		//连续多页保存数据
		//从零地址开始保存
		while(1)
		{
			//增加页地址
			FPageAddr++;
			if(FPageAddr >= MAXPAGE)
			{/* 跨FLASH片读写的情况 */
				FlashNo++;
				FPageAddr = 0;
			}

			//处理最后一页数据
			if( Len <=  MAXPAGELEN )
			{
				if( Opflag == WRITEAT )
					ret = Page_Pgm_Buf( FlashNo,DB161_PAGE_PGM_BUF1,FPageAddr,0x0,DataAddr,Len );
				else
					ret = ReadFlash_Page( FlashNo,FPageAddr,0x0,DataAddr,Len );
				break;
			}
			//处理中间页数据
			else
			{
				if( Opflag == WRITEAT )
					ret = Page_Pgm_Buf( FlashNo,DB161_PAGE_PGM_BUF2,FPageAddr,0x0,DataAddr,MAXPAGELEN );
				else
					ret = ReadFlash_Page( FlashNo,FPageAddr,0x0,DataAddr,MAXPAGELEN );
			}
			
			Len -= MAXPAGELEN;
			DataAddr += MAXPAGELEN;
			DelayUS(3);	
		}
	}
	
	return ret;
}

/*******************************************************************************
* Function Name  : WriteDataToAT
* Description    : AT45写接口函数
* Input          : FlashNo   - FLASH 片号
*          		   FlashAddr - INT32U
*                  DataAddr	 - INT8U *
*				   Len        
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U WriteDataToAT(INT8U FlashNo,INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{   
    INT8U i;
    
	//assert_param(Len <= 2048);	/* 长度限制，限制非调度区时间 */
	
//	OSSchedLock();

	i= ATOperation(WRITEAT,FlashNo,FlashAddr,DataAddr,Len);

//	OSSchedUnlock();
	return i;
}

/*******************************************************************************
* Function Name  : ReadDataFromAT
* Description    : AT45读接口函数
* Input          : FlashNo   - FLASH 片号
*          		   FlashAddr - INT32U
*				   Len        
* Output         : DataAddr	 - INT8U *
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U ReadDataFromAT(INT8U FlashNo,INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{   
  INT8U i;

  //assert_param(Len <= 2048);	/* 长度限制，限制非调度区时间 */
	
//  OSSchedLock();

  i= ATOperation(READAT,FlashNo,FlashAddr,DataAddr,Len) ;

//  OSSchedUnlock();
  return i;
}

 // 页擦除,大约15 ~ 35ms,PageNo是页地址
INT8U AT45PageErase(INT8U FlashNo,INT16U PageNo)
{
	INT8U err;
	INT8U spi_com_buf[4];

	OSTimeDlyHMSM(0,0,0,30); /* 擦除耗时长， 维持系统占空比 */

//	OSSchedLock();
	if(WaitFlashFree(FlashNo) == FALSE)
	{	
	    OSSchedUnlock();
		return FALSE;
	}
	At45db161_DisWP(FlashNo);

	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = 0x81;				// 命令字1
		spi_com_buf[1] = (PageNo>>5);	    // 高8位
		spi_com_buf[2] = PageNo*8;	        // 低5位
		spi_com_buf[3] = 0x00;				// 命令字4
	}
	else
	{//at45db321d
		spi_com_buf[0] = 0x81;				// 命令字1
		spi_com_buf[1] = ((PageNo>>6) & 0x7f);	    // 高7位
		spi_com_buf[2] = PageNo*4;	        // 低6位
		spi_com_buf[3] = 0x00;				// 命令字4
	}
	
	//选通FLASH
	At45db161_open(FlashNo);	
	if(AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4) == FALSE)
	{
		At45db161_close(FlashNo);
		OSSchedUnlock();
		return FALSE;
	}

	//关闭FLASH操作 
	At45db161_close(FlashNo);
	DelayUS(20*1000);
	err = WaitFlashFree(FlashNo);
//	OSSchedUnlock();
	
	return err;
}

// 块擦除,大约45 ~ 100ms,PageNo是页地址
INT8U AT45BlockErase(INT8U FlashNo,INT16U PageNo)
{
	INT8U err;
	INT8U spi_com_buf[4];

	OSTimeDlyHMSM(0,0,0,60);	/* 擦除耗时长， 维持系统占空比 */

	/* 系统忙的情况, 减低擦除速度, 提高实时性 */


//	OSSchedLock();
	if(WaitFlashFree(FlashNo) == FALSE)
	{	
	    OSSchedUnlock();
		return FALSE;
	}
	At45db161_DisWP(FlashNo);

	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = 0x50;				    // 命令字1
		spi_com_buf[1] = (PageNo>>5);	        // 高8位
		spi_com_buf[2] = ((PageNo<<3) & 0xc0);	// 低2位
		spi_com_buf[3] = 0x00;				    // 命令字
	}
	else
	{//at45db321d
		spi_com_buf[0] = 0x50;				    // 命令字1
		spi_com_buf[1] = ((PageNo>>6) & 0x7f);	// 高7位
		spi_com_buf[2] = ((PageNo<<2) & 0xe0);	// 低3位
		spi_com_buf[3] = 0x00;				    // 命令字4
	}
	
	//选通FLASH
	At45db161_open(FlashNo);	
	if(AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4) == FALSE)
	{
		At45db161_close(FlashNo);
		OSSchedUnlock();
		return FALSE;
	}

	//关闭FLASH操作 
	At45db161_close(FlashNo);
	DelayUS(50*1000);
	err = WaitFlashFree(FlashNo);
//	OSSchedUnlock();
	
	return err;
}
/**********************************************************************************************
* Function Name	: BSP_EraseAT
* Description	: AT45擦除
* Input			: addr:地址(0 - MAX_SIZE-1)
				  len:数据长度,这里的长度是32位的
* Return		: TRUE; FALSE
* Note(s)		: 所有AT45的地址都连续的,用户不需考虑使用的是哪片。大范围的擦除是需要大量时间,
                  用户在使用时需要注意。1片AT45DB321,最大擦除时间在120s左右
* Contributor	: 2015-01-05 王耀
* Update        : 禁用扇区擦除，块擦除添加OSTimeDly（）延时维持系统占空比。
***********************************************************************************************/
INT8U BSP_EraseAT(INT32U addr,INT32U len)
{
	INT8U chip,err=TRUE;
	INT16U PageNo,InPageNo;
	INT32U dolen = 0;
	
	if(len == 0 || addr >= MAX_SIZE || addr + len > MAX_SIZE)
		return FALSE;
	
	// 计算地址信息
	chip = addr / MAX_CHIP_SIZE;			// 片地址
	addr = addr % MAX_CHIP_SIZE;			// 片内地址
	
	while(len)
	{		
/*		// 判断是否可以扇区擦除
		if((addr % MAX_SECTOR_SIZE==0) && (len >= MAX_SECTOR_SIZE))
		{
			if(AT45SectorErase(chip, addr / MAXPAGELEN)==FALSE)
				err = FALSE;
			dolen = MAX_SECTOR_SIZE;
		}
		// 判断是否可以块擦除
		else*/ if((addr % MAX_BLOCK_SIZE==0) && (len >= MAX_BLOCK_SIZE))
		{
			if(AT45BlockErase(chip, addr / MAXPAGELEN)==FALSE)
				err = FALSE;
			dolen = MAX_BLOCK_SIZE;
		}
		// 判断是否可以页擦除
		else if((addr % MAXPAGELEN==0) && (len >= MAXPAGELEN))
		{
			if(AT45PageErase(chip, addr / MAXPAGELEN)==FALSE)
				err = FALSE;
			dolen = MAXPAGELEN;
		}
		// 不完整页,只能老老实实的写0xff了
		else
		{
			PageNo = (INT16U)(addr / MAXPAGELEN);	// 页地址
			InPageNo = (INT16U)(addr % MAXPAGELEN);	// 页内地址
			if(len >= (MAXPAGELEN - InPageNo))
				dolen = MAXPAGELEN - InPageNo;	// 一直写到本页结束
			else
				dolen = len;								// 写完数据
			
//			OSSchedLock();
			if(Page_Pgm_Data(chip,DB161_PAGE_PGM_BUF1,PageNo,InPageNo,0XFF,dolen) == FALSE)
				err = FALSE;
//			OSSchedUnlock();
		}

		if(err == FALSE)
		{
			break;
		}

		addr += dolen;
		if(addr >= MAX_CHIP_SIZE)
		{
			addr -= MAX_CHIP_SIZE;
			chip++;
		}
		len = len - dolen;
	}

	return err;
}

/*******************************************************************************
* Function Name  : BSP_WriteByteToAtWithoutErase
* Description    : Write One Byte To at45db161 Without Erase
* Input          : FlashAddr - Address to write
*                  val       - data to write
* Output         : None
* Return         : 0 - fail, 1 - ok
*******************************************************************************/
INT8U BSP_WriteByteToAtWithoutErase(INT32U FlashAddr,INT8U val)
{
	return BSP_WriteDataToAT(FlashAddr,&val,1);
}

/*******************************************************************************
* Function Name  : BSP_WriteDataToAT
* Description    : AT45写接口函数
* Input          : FlashAddr - INT32U
*                  DataAddr	 - INT8U *
*				   Len        
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_WriteDataToAT(INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{   
	INT8U   chip;
	INT32U  PageAddr;
	
	if((Len == 0)|| (FlashAddr >= MAX_SIZE) || (FlashAddr + Len) > MAX_SIZE || (DataAddr == NULL))
		return FALSE;
	// 计算地址信息
	chip = FlashAddr / MAX_CHIP_SIZE;			// 片地址
	FlashAddr = FlashAddr % MAX_CHIP_SIZE;		// 片内地址
	PageAddr = ((FlashAddr/MAXPAGELEN)<<16) | (FlashAddr%MAXPAGELEN); // 线性地址转换到FLASH地址
	return(WriteDataToAT(chip,PageAddr,DataAddr,Len));
}

/*******************************************************************************
* Function Name  : BSP_ReadDataFromAT
* Description    : AT45读接口函数
* Input          : FlashAddr - INT32U
*				   Len        
* Output         : DataAddr	 - INT8U *
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_ReadDataFromAT(INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{   
    INT8U   chip;
	INT32U  PageAddr;
	if((Len == 0)|| (FlashAddr >= MAX_SIZE) || (FlashAddr + Len) > MAX_SIZE || (DataAddr == NULL))
		return FALSE;

	// 计算地址信息
	chip = FlashAddr / MAX_CHIP_SIZE;			// 片地址
	FlashAddr = FlashAddr % MAX_CHIP_SIZE;		// 片内地址
	PageAddr = ((FlashAddr/MAXPAGELEN)<<16) | (FlashAddr%MAXPAGELEN); // 线性地址转换到FLASH地址
	return(ReadDataFromAT(chip,PageAddr,DataAddr,Len));
}

/*******************************************************************************
* Function Name  : BSP_GetTotalSizeOfAt
* Description    : Get Total Size Of At Flash
* Input          : None
* Output         : None
* Return         : Total Size
*******************************************************************************/
INT32U BSP_GetTotalSizeOfAt(void)
{
	return MAX_SIZE;
}
/************************(C)COPYRIGHT 2015 浙江方泰****END OF FILE****************************/


