/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bsp_flash.c
* Author			: ��ҫ
* Date First Issued	: 05/01/2015
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2015		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
//#include "sysconfig.h"
#include "bsp.h"
/* Private define-----------------------------------------------------------------------------*/
#define	AT45SPIWrite			BSP_SPIWrite	// SPIд�����ض���
#define	AT45SPIRead				BSP_SPIRead		// SPI�������ض���
// at45db161D �� at45db161B������: ����һ����
// ��at45db161D �����趨 512ÿҳ��
// ����Sector Erase, Security Register���ܡ�

// flash ״̬�� mask��
#define DB161_STATUS	0xd7
// flash �������ͣ�
#define WRITEAT 		0
#define READAT 			1

//AT45DB161Bָ�
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
* Description	: ��ʼ��SPI0-Flash,SPI0ֻ��AT45DB flashʹ��
* Input			: 
* Return		: 
* Note(s)		: Ҳ���ǳ�ʼ��SPI
* Contributor	: 150105  ��ҫ
***********************************************************************************************/
void BSP_FlashInitAT(void)
{      
    _BSPSPI_CONFIG SPI_InitStructure;

    SPI_InitStructure.num			= BSPAT_SPINUM;
    SPI_InitStructure.pEvent 		= NULL;
    SPI_InitStructure.TxRespond 	= BSPSPI_RESPOND_NORMAL;	    // ��ѯģʽ
    SPI_InitStructure.channel 		= BSPSPI_CHN0;			        // 0ͨ��
    SPI_InitStructure.pTxBuffer 	= 0;			
    SPI_InitStructure.pRxBuffer 	= 0;
    SPI_InitStructure.MaxTxBuffer	= 0;
    SPI_InitStructure.MaxRxBuffer 	= 0;
    SPI_InitStructure.Mode 			= BSPSPI_WORKMODE_MASTER;       // ��ģʽ
    SPI_InitStructure.RxOvertime 	= SYS_DELAY_10ms;
    BSP_SPIConfig(&SPI_InitStructure);
}

/*******************************************************************************
* Function Name  : At45db161_open
* Description    : ѡ��FLASH����������ʹ�õ��Ǳ�׼CS������ֱ�ӵ��ñ�׼SPI�Ĳ���
                    ������εĿ�����ȻĿǰû���ã�Ϊ�Ժ������չ�ⲿ�洢��Ԥ�������ֲ��䡣
                  ������ʹ��161 ʵ��ʹ�õ���32,��Ҫ������������
* Input          : FlashNo - FLASH Ƭ��
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_open(INT8U FlashNo)
{
	//ѡ��Ҫ������AT45,���Ȳ���Ƭѡ�ź�
	//Ƭѡ�źŵ���Ч
    BSP_SPICSEnable(BSPAT_SPINUM);

}

/*******************************************************************************
* Function Name  : At45db161_close
* Description    : ��ֹ FLASH ����
* Input          : FlashNo - FLASH Ƭ��
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_close(INT8U FlashNo)
{
    BSP_SPICSDisEnable(BSPAT_SPINUM);
}

/*******************************************************************************
* Function Name  : At45db161_DisWP
* Description    : FLASH �ر�д����������Ŀǰ�豸ֻ����һ��flash,WP�ܽ�Ӳ���Ѿ�Ĭ��
                    �����ǵ�����������ȫ�ԣ��պ���Ԥ�������ı�
* Input          : FlashNo - FLASH Ƭ��
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_DisWP(INT8U FlashNo)
{
	//д�����źŵ���Ч
}

/*******************************************************************************
* Function Name  : At45db161_EnWP
* Description    : FLASH ��д����
* Input          : FlashNo - FLASH Ƭ��
* Output         : None
* Return         : None
*******************************************************************************/
void At45db161_EnWP(INT8U FlashNo)
{
	//д�����źŵ���Ч
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
* Description    : ��ȡFLASH��״̬
* Input          : FlashNo - FLASH Ƭ��
				   Status  - ��ŵ�ַ
* Output         : None
* Return         : TRUE / FALSE.
*******************************************************************************/
INT8U ReadFlash_Status(INT8U FlashNo,INT8U *status)
{
	INT8U spi_com_buf = DB161_STATUS ;
    INT8U err;
    
    BSP_SPICSEnable(BSPAT_SPINUM); //����Ҫ�У�����һ����������
	if(AT45SPIWrite(BSPAT_SPINUM,&spi_com_buf,1) == TRUE)	// SPIд
		err=AT45SPIRead(BSPAT_SPINUM,status,1);	// SPI��
	else
		err = FALSE;
    BSP_SPICSDisEnable(BSPAT_SPINUM);
	return err;
}

/*******************************************************************************
* Function Name  : WaitFlashFree
* Description    : �ȴ�FLASH����
* Input          : FlashNo - FLASH Ƭ��
* Output         : None
* Return         : None
*******************************************************************************/
INT8U  WaitFlashFree(INT8U FlashNo)
{
	INT8U status = 0x00;

	while(1)
	{
		//��ȡFLASH��״̬�ֽ�
		if ( ReadFlash_Status(FlashNo,&status) == FALSE )
			return FALSE;
		//�����˳�
		if( (status & 0x80) != 0)
			break;		
	}

	return TRUE;
}

/*******************************************************************************
* Function Name  : FlashPageToBuf
* Description    : Flash Page ���ݶ��� Flash Buf
* Input          : FlashNo - FLASH Ƭ��
*                  command - ������
*                  PageNo  - ҳ��ַ
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
* Description    : �ȴ��ȽϽ��
* Input          : FlashNo - FLASH Ƭ��
* Output         : None
* Return         : TRUE/FALSE
*******************************************************************************/
INT8U WaitCmpResult(INT8U FlashNo)
{
	INT8U status = 0x00;
	
	while(1)
	{
		//��ȡFLASH��״̬�ֽ�
		if( ReadFlash_Status(FlashNo,&status) == FALSE )
			return(FALSE);
		//�����˳�
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
* Description    : �Ƚ�д��������Ƿ�һ��
* Input          : FlashNo - FLASH Ƭ��
*                  command - ������
*                  PageNo  - ҳ��ַ
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
* Description    : д��һҳ������
* Input          : FlashNo - FLASH Ƭ��
*                  command - ������
*                  PageNo  - ҳ��ַ
*				   InPageNo- ҳ�ڵ�ַ
*                  data    - ��д����
*                  len     - ��д���ݳ���      
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
	//����	
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
* Description    : д��һҳ������
* Input          : FlashNo - FLASH Ƭ��
*                  command - ������
*                  PageNo  - ҳ��ַ
*				   InPageNo- ҳ�ڵ�ַ
*                  addr    - ��д���ݴ���׵�ַ
*                  len     - ��д���ݳ���      
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
	//����	
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
* Description    : ��ȡһҳ������
* Input          : FlashNo - FLASH Ƭ��
*                  PageNo  - ҳ��ַ
*				   InPageNo- ҳ�ڵ�ַ
*                  len     - ��ȡ���ݳ���      
* Output         : addr    - �����ݴ���׵�ַ
* Return         : TRUE/FALSE
*******************************************************************************/
//opcode:0x52
//F Byte:r,r,PA11-PA6
//S Byte:PA5-PA0,BA9-BA8
//T Byte:BA7-BA0
//x 4Byte:0x00
//����ҳ528���ֽ�
//BA9-BA0: 0x000
//PA11-PA0: PageNo
INT8U ReadFlash_Page(INT8U FlashNo,INT16U PageNo,INT16U InPageNo,INT8U *addr,INT32U len)
{
	INT8U spi_com_buf[8] = {DB161_PAGE_READ, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF};
	
	//���Ͷ�ȡ����
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

	//ѡͨFLASH
	At45db161_open(FlashNo);
		
	if( AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,8) == FALSE )
	{
		At45db161_close(FlashNo);
		return FALSE;
	}

	//��������
	if( AT45SPIRead(BSPAT_SPINUM,addr,len) == FALSE )
	{
		At45db161_close(FlashNo);
		return FALSE;
	}

	//�ر�FLASH���� 
	At45db161_close(FlashNo);

	return(TRUE);
}

/*******************************************************************************
* Function Name  : ATOperation
* Description    : AT45��д��������
* Input          : Opflag - �������ͣ��� / д
                   FlashNo   - FLASH Ƭ��
*          		   FlashAddr - INT32U
*                  DataAddr	 - INT8U *	д����
*				   Len        
* Output         : DataAddr	 - INT8U *	������
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U ATOperation(INT8U Opflag,INT8U FlashNo,INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
	//Flash ҳ��ַ��0~4095
	INT16U FPageAddr;
	//Flash ҳ�ڵ�ַ��0~527
	INT16U FInPageAddr;

	INT16U LenPageTemp;
	INT8U  ret;
	
	//��16λ���Flashҳ�ڵ�ַ
	FInPageAddr = (INT16U) ( FlashAddr & 0xffff );
	//��16λ���Flash ҳ��ַ
	FPageAddr = (INT16U) ( FlashAddr>>16 );

	if( FPageAddr > ( MAXPAGE - 1) )
		return FALSE;

	if( FInPageAddr > ( MAXPAGELEN -1) )
		return FALSE;
	//��������ж�?

	//��ǰҳ��ʣ�µĿռ䣻
	LenPageTemp = MAXPAGELEN - FInPageAddr;
	//һҳ�ܴ���������
	if( Len <=  LenPageTemp )
	{
		if( Opflag == WRITEAT )
			ret = Page_Pgm_Buf( FlashNo,DB161_PAGE_PGM_BUF1,FPageAddr,FInPageAddr,DataAddr,Len );
		else
			ret = ReadFlash_Page( FlashNo,FPageAddr,FInPageAddr,DataAddr,Len );
	}
	//��ֶ�ҳ����
	else
	{
		//�ȱ��浱ǰҳ����
		if( Opflag == WRITEAT )
			ret = Page_Pgm_Buf( FlashNo,DB161_PAGE_PGM_BUF1,FPageAddr,FInPageAddr,DataAddr,LenPageTemp );
		else
			ret = ReadFlash_Page( FlashNo,FPageAddr,FInPageAddr,DataAddr,LenPageTemp );
		
		Len -= LenPageTemp;
		DataAddr += LenPageTemp;
		
		//������ҳ��������
		//�����ַ��ʼ����
		while(1)
		{
			//����ҳ��ַ
			FPageAddr++;
			if(FPageAddr >= MAXPAGE)
			{/* ��FLASHƬ��д����� */
				FlashNo++;
				FPageAddr = 0;
			}

			//�������һҳ����
			if( Len <=  MAXPAGELEN )
			{
				if( Opflag == WRITEAT )
					ret = Page_Pgm_Buf( FlashNo,DB161_PAGE_PGM_BUF1,FPageAddr,0x0,DataAddr,Len );
				else
					ret = ReadFlash_Page( FlashNo,FPageAddr,0x0,DataAddr,Len );
				break;
			}
			//�����м�ҳ����
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
* Description    : AT45д�ӿں���
* Input          : FlashNo   - FLASH Ƭ��
*          		   FlashAddr - INT32U
*                  DataAddr	 - INT8U *
*				   Len        
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U WriteDataToAT(INT8U FlashNo,INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{   
    INT8U i;
    
	//assert_param(Len <= 2048);	/* �������ƣ����Ʒǵ�����ʱ�� */
	
//	OSSchedLock();

	i= ATOperation(WRITEAT,FlashNo,FlashAddr,DataAddr,Len);

//	OSSchedUnlock();
	return i;
}

/*******************************************************************************
* Function Name  : ReadDataFromAT
* Description    : AT45���ӿں���
* Input          : FlashNo   - FLASH Ƭ��
*          		   FlashAddr - INT32U
*				   Len        
* Output         : DataAddr	 - INT8U *
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U ReadDataFromAT(INT8U FlashNo,INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{   
  INT8U i;

  //assert_param(Len <= 2048);	/* �������ƣ����Ʒǵ�����ʱ�� */
	
//  OSSchedLock();

  i= ATOperation(READAT,FlashNo,FlashAddr,DataAddr,Len) ;

//  OSSchedUnlock();
  return i;
}

 // ҳ����,��Լ15 ~ 35ms,PageNo��ҳ��ַ
INT8U AT45PageErase(INT8U FlashNo,INT16U PageNo)
{
	INT8U err;
	INT8U spi_com_buf[4];

	OSTimeDlyHMSM(0,0,0,30); /* ������ʱ���� ά��ϵͳռ�ձ� */

//	OSSchedLock();
	if(WaitFlashFree(FlashNo) == FALSE)
	{	
	    OSSchedUnlock();
		return FALSE;
	}
	At45db161_DisWP(FlashNo);

	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = 0x81;				// ������1
		spi_com_buf[1] = (PageNo>>5);	    // ��8λ
		spi_com_buf[2] = PageNo*8;	        // ��5λ
		spi_com_buf[3] = 0x00;				// ������4
	}
	else
	{//at45db321d
		spi_com_buf[0] = 0x81;				// ������1
		spi_com_buf[1] = ((PageNo>>6) & 0x7f);	    // ��7λ
		spi_com_buf[2] = PageNo*4;	        // ��6λ
		spi_com_buf[3] = 0x00;				// ������4
	}
	
	//ѡͨFLASH
	At45db161_open(FlashNo);	
	if(AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4) == FALSE)
	{
		At45db161_close(FlashNo);
		OSSchedUnlock();
		return FALSE;
	}

	//�ر�FLASH���� 
	At45db161_close(FlashNo);
	DelayUS(20*1000);
	err = WaitFlashFree(FlashNo);
//	OSSchedUnlock();
	
	return err;
}

// �����,��Լ45 ~ 100ms,PageNo��ҳ��ַ
INT8U AT45BlockErase(INT8U FlashNo,INT16U PageNo)
{
	INT8U err;
	INT8U spi_com_buf[4];

	OSTimeDlyHMSM(0,0,0,60);	/* ������ʱ���� ά��ϵͳռ�ձ� */

	/* ϵͳæ�����, ���Ͳ����ٶ�, ���ʵʱ�� */


//	OSSchedLock();
	if(WaitFlashFree(FlashNo) == FALSE)
	{	
	    OSSchedUnlock();
		return FALSE;
	}
	At45db161_DisWP(FlashNo);

	if(MAX_CHIP_SIZE == 1024*8192 || MAX_CHIP_SIZE == 1056*8192)
	{//at45db642d
		spi_com_buf[0] = 0x50;				    // ������1
		spi_com_buf[1] = (PageNo>>5);	        // ��8λ
		spi_com_buf[2] = ((PageNo<<3) & 0xc0);	// ��2λ
		spi_com_buf[3] = 0x00;				    // ������
	}
	else
	{//at45db321d
		spi_com_buf[0] = 0x50;				    // ������1
		spi_com_buf[1] = ((PageNo>>6) & 0x7f);	// ��7λ
		spi_com_buf[2] = ((PageNo<<2) & 0xe0);	// ��3λ
		spi_com_buf[3] = 0x00;				    // ������4
	}
	
	//ѡͨFLASH
	At45db161_open(FlashNo);	
	if(AT45SPIWrite(BSPAT_SPINUM,spi_com_buf,4) == FALSE)
	{
		At45db161_close(FlashNo);
		OSSchedUnlock();
		return FALSE;
	}

	//�ر�FLASH���� 
	At45db161_close(FlashNo);
	DelayUS(50*1000);
	err = WaitFlashFree(FlashNo);
//	OSSchedUnlock();
	
	return err;
}
/**********************************************************************************************
* Function Name	: BSP_EraseAT
* Description	: AT45����
* Input			: addr:��ַ(0 - MAX_SIZE-1)
				  len:���ݳ���,����ĳ�����32λ��
* Return		: TRUE; FALSE
* Note(s)		: ����AT45�ĵ�ַ��������,�û����迼��ʹ�õ�����Ƭ����Χ�Ĳ�������Ҫ����ʱ��,
                  �û���ʹ��ʱ��Ҫע�⡣1ƬAT45DB321,������ʱ����120s����
* Contributor	: 2015-01-05 ��ҫ
* Update        : ����������������������OSTimeDly������ʱά��ϵͳռ�ձȡ�
***********************************************************************************************/
INT8U BSP_EraseAT(INT32U addr,INT32U len)
{
	INT8U chip,err=TRUE;
	INT16U PageNo,InPageNo;
	INT32U dolen = 0;
	
	if(len == 0 || addr >= MAX_SIZE || addr + len > MAX_SIZE)
		return FALSE;
	
	// �����ַ��Ϣ
	chip = addr / MAX_CHIP_SIZE;			// Ƭ��ַ
	addr = addr % MAX_CHIP_SIZE;			// Ƭ�ڵ�ַ
	
	while(len)
	{		
/*		// �ж��Ƿ������������
		if((addr % MAX_SECTOR_SIZE==0) && (len >= MAX_SECTOR_SIZE))
		{
			if(AT45SectorErase(chip, addr / MAXPAGELEN)==FALSE)
				err = FALSE;
			dolen = MAX_SECTOR_SIZE;
		}
		// �ж��Ƿ���Կ����
		else*/ if((addr % MAX_BLOCK_SIZE==0) && (len >= MAX_BLOCK_SIZE))
		{
			if(AT45BlockErase(chip, addr / MAXPAGELEN)==FALSE)
				err = FALSE;
			dolen = MAX_BLOCK_SIZE;
		}
		// �ж��Ƿ����ҳ����
		else if((addr % MAXPAGELEN==0) && (len >= MAXPAGELEN))
		{
			if(AT45PageErase(chip, addr / MAXPAGELEN)==FALSE)
				err = FALSE;
			dolen = MAXPAGELEN;
		}
		// ������ҳ,ֻ������ʵʵ��д0xff��
		else
		{
			PageNo = (INT16U)(addr / MAXPAGELEN);	// ҳ��ַ
			InPageNo = (INT16U)(addr % MAXPAGELEN);	// ҳ�ڵ�ַ
			if(len >= (MAXPAGELEN - InPageNo))
				dolen = MAXPAGELEN - InPageNo;	// һֱд����ҳ����
			else
				dolen = len;								// д������
			
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
* Description    : AT45д�ӿں���
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
	// �����ַ��Ϣ
	chip = FlashAddr / MAX_CHIP_SIZE;			// Ƭ��ַ
	FlashAddr = FlashAddr % MAX_CHIP_SIZE;		// Ƭ�ڵ�ַ
	PageAddr = ((FlashAddr/MAXPAGELEN)<<16) | (FlashAddr%MAXPAGELEN); // ���Ե�ַת����FLASH��ַ
	return(WriteDataToAT(chip,PageAddr,DataAddr,Len));
}

/*******************************************************************************
* Function Name  : BSP_ReadDataFromAT
* Description    : AT45���ӿں���
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

	// �����ַ��Ϣ
	chip = FlashAddr / MAX_CHIP_SIZE;			// Ƭ��ַ
	FlashAddr = FlashAddr % MAX_CHIP_SIZE;		// Ƭ�ڵ�ַ
	PageAddr = ((FlashAddr/MAXPAGELEN)<<16) | (FlashAddr%MAXPAGELEN); // ���Ե�ַת����FLASH��ַ
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
/************************(C)COPYRIGHT 2015 �㽭��̩****END OF FILE****************************/


