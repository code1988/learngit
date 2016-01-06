/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName		    : mmcsdmain.c
* Author		    :
* Date First Issued	: 130722
* Version		    : V
* Description		: SD��Ӧ�ó���
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History		    :   20150122 - ��Ӷ������湦��
*                       20150127 - �����������湦�ܣ���Ҫ���ڴ���list�ļ������һ����ʱ��־
*                       20150128 - ����list�ļ����ȳ���512(1��SD����)ʱ�����ݷ����ڴ��������
*                       20150129 - ����300�Ŵ洢dat�ļ�ʧ������;����TaskSDMaster����Ķ�ջ����,��������ܷ�����
                        20150311 - ����SD���洢dat�ļ�����������list�ļ���������
                        20140312 - ����list�ļ�65KB��������,���SD��������ȡ����
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "gpio_v2.h"
#include "def_config.h"
#include "dsp_net.h"
#include "net_server.h"
#include "DwinPotocol.h"
#include "main.h"

/* Private define-----------------------------------------------------------------------------*/
#define DSPLEVELUP 0x01
#define UIARMLEVELUP 0x02
#define FPGALEVELUP 0x03
#define RMBMODELEVELUP 0x04
#define MTARMLEVELUP 0x05
/* Private typedef----------------------------------------------------------------------------*/

/* This header is used by the ROM Code to indentify the size of bootloader
 * and the location to which it should be loaded
 */
typedef struct _ti_header_
{
    INT32U image_size;
    INT32U load_addr;
}ti_header;

/* Private macro------------------------------------------------------------------------------*/
#define TASK_STK_SIZE_SD_MINOR    0x400

/* Private variables--------------------------------------------------------------------------*/
#pragma pack(1)
// ���յ������ݸ�ʽ��һ��ֽ�ҵ�����
typedef struct{
    char Reserve[5];        // �����ֽ�
    char Guanzihao[10];     // ���ֺ�
    char Denomination;      // ��ֵ
    char Version;           // �汾
    char Result;            // ���
    char SectDraw[180];     // ��ͼ
}TypeStruct_RecvFormat;

// dat�ļ�ͷ��ʽ
typedef struct{
    char FileHead[5];       // �ļ���ʼ�ַ�
    char TradeID;           // ���״���
    char Reserve[11];       // ����
    INT16U PaperNum;        // ������
    char MachineID[24];     // ������
    char Time[14];          // ʱ��
    char Batch[6];          // ���Σ���ʱû�ã���0
    char Status;            // ״̬��־����ʱû�ã���0
    char ErrorNum;          // ʧ�ܴ�������ʱû�ã���0
}TypeStruct_DatHeadFromat;;
#pragma pack()

// �����ļ��ĵ�Ԫ����
typedef struct{
	char fname[21];			// �������ļ���
	INT8U fileflag;			// �ļ���־��0xAA:δ����־ 0xBB:�Ѷ���־	0xCC:�𻵱�־   0xDD:��ʱ��־����ʾ�ϴ��� 0x00:��
	INT8U UnitNums;         // ���滹�еĵ�Ԫ����
}TypeStruct_UnitAttribute;

// ��ǰʱ��ṹ��
typedef struct{
    char Year[4];
    char Month[2];
    char Day[2];
    char Hour[2];
    char Minute[2];
    char Second[2];
}TypeStruct_CurrTime;

/* Private function prototypes----------------------------------------------------------------*/
OS_EVENT *MMCSDEvent;                                   // SD���¼����ƿ�
void *MMCSDOSQ[4];					                    // ��Ϣ����
static OS_EVENT *SDMutex;                               // SD��������
OS_STK TaskSDMinorStk[TASK_STK_SIZE_SD_MINOR];          // SD���洢���������ջ


INT8U SDSTAT = 0x01;
INT8U Datalen = 0;

static INT8U Dat_SendBuff[1024*1024];                   // dat�ļ�������,����1MB��dat�ļ����ߴ�73KB
static INT32U LF_Basic_Index = 0;                       // list file ��׼������Ԫ
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment = 128
static TypeStruct_UnitAttribute ListFile[MAX_LIST_NUM]; // list file ������
#endif
/* Private functions--------------------------------------------------------------------------*/
//��SD����boot���ص�NAND��
static void BOOT_LOAD_FROM_SD(NandInfo_t *Nand_InitStructure);
static void BOOT_LOAD_FROM_SD_AUTO(NandInfo_t *Nand_InitStructure);
static void Fname_From_Time(char *pFname,TypeStruct_CurrTime *CurTime);
static INT8U Fname_Form_Comfirm(INT32U idx);
static INT8U List_File_Load(void);
static INT8U List_File_Add(char *pfname);
static char *List_File_Inquire(void);
static INT8U List_File_Update(char *pfname,INT8U fbflag);
static INT8U List_File_Save(void);
static INT8U Get_Vol_SD(INT8U *drv,INT32U *total,INT32U *free);
static INT8U Save_Dat_SD(_BSP_MESSAGE *DatMsg);
static INT8U Read_Dat_SD(char *pfname,INT8U index);
static int manage_dat_buf_ack_flag(void);

/***********************************************************************************************
* Function	: TaskMMCSD
* Description	: SD���������ͨ��SD����д�������̣����̴���nand��
* Input		:
* Return	:
* Note(s)	:
* Contributor	:130722   wangyao
***********************************************************************************************/
void TaskSDMaster(void *pdata)
{
    DBG_DIS("in TaskMMCSD 0:\r\n");

    pdata = pdata;
    INT8U err;
    INT32U Total_Vol,Free_Vol;
#ifdef SDCARD_HOTPLUG
    char const	*fname_app = "/app";
    char const	*fname_boot = "/boot.bin";
    FRESULT 	fresult;
#endif //SDCARD_HOTPLUG

    //Nand ��ʼ����ؽṹ�嶨��
    NandInfo_t 		    Nand_InitStructure;
    NandCtrlInfo_t          nandCtrlInfo;
    NandEccInfo_t           nandEccInfo;
    NandDmaInfo_t           nandDmaInfo;
    GPMCNANDTimingInfo_t    nandTimingInfo;

    //Nand��ʼ��
    Nand_InitStructure.hNandCtrlInfo 	= &nandCtrlInfo;
    Nand_InitStructure.hNandDmaInfo	= &nandDmaInfo;
    Nand_InitStructure.hNandEccInfo	= &nandEccInfo;
    nandCtrlInfo.hNandTimingInfo	= &nandTimingInfo;

    DBG_DIS("in TaskMMCSD 1:Nand config\r\n");

    if(BSP_NandConfig(&Nand_InitStructure) != NAND_STATUS_PASSED)
    {
        STOP_RUN("BSP_NandConfig(&Nand_InitStructure) != NAND_STATUS_PASSED \r\n");
    }

    //SD����ʼ��
#ifdef SDCARD_HOTPLUG
    _BSPGPIO_CONFIG GPIO_InitStructure;

    GPIO_InitStructure.PortNum = PORT3;
    GPIO_InitStructure.PinNum = 14;
    GPIO_InitStructure.Dir = GPIO_DIR_INPUT;
    GPIO_InitStructure.IntLine = GPIO_INT_LINE_1;
    GPIO_InitStructure.IntType = GPIO_INT_TYPE_BOTH_EDGE;
    GPIO_InitStructure.pEvent = MMCSDEvent;

    BSP_GPIOConfig(&GPIO_InitStructure);

#endif //SDCARD_HOTPLUG

    DBG_DIS("in TaskMMCSD 2_1:BSP_MMCSDInit start\r\n");

    BSP_MMCSDInit();
    DBG_DIS("in TaskMMCSD 2_2:BSP_MMCSDInit   end\r\n");

    // SD��������
    SDMutex = OSMutexCreate(PRI_SD_MUTEX,&err);

    // �Զ�����
    DBG_DIS("in TaskMMCSD 3_1:BOOT_LOAD_FROM_SD_AUTO start\r\n");
    BOOT_LOAD_FROM_SD_AUTO(&Nand_InitStructure);
    DBG_DIS("in TaskMMCSD 3_2:BOOT_LOAD_FROM_SD_AUTO   end\r\n");


    DBG_DIS("in TaskMMCSD 4_1:get list file start\r\n");
    // ��ȡlist�ļ�
    memset(ListFile,0,sizeof(TypeStruct_UnitAttribute)*MAX_LIST_NUM);
	if( List_File_Load() == FALSE)
        DBG_DIS("ERROR!!!ERROR!!!   :   LIST FILE LOAD ERROR\r\n");
    DBG_DIS("in TaskMMCSD 4_2:get list file   end\r\n");

    // ����SD����ȡ��������
    OSTaskCreateExt(TaskSDMinor,
                    (void *)0,
                    &TaskSDMinorStk[TASK_STK_SIZE_SD_MINOR-1],
                    PRI_SD_MINOR,
                    1,
                    &TaskSDMinorStk[0],
                    TASK_STK_SIZE_SD_MINOR,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
    
    while(1)
    {
        _BSP_MESSAGE *pMsg;						// ��Ϣָ��
        pMsg = OSQPend(MMCSDEvent,SYS_DELAY_100ms,&err);
        if(err == OS_NO_ERR)				    // �յ���Ϣ
        {
            switch(pMsg->MsgID)					// �ж���Ϣ
            {
                case APP_SAVE_DAT:              // ����dat�ļ���SD
                    err = Save_Dat_SD(pMsg);
                    break;
                case APP_SD_VOLUM:              // ��ȡSD������    
                    err = Get_Vol_SD("0",&Total_Vol,&Free_Vol);
                    break;
                case APP_COMFROM_UI:
				#if FZ1500
                    if(pMsg->DivNum == DSPLEVELUP)
                    {
                        DSPBOOT_LOAD_FROM_SD();
                    }
                    else if(pMsg->DivNum == UIARMLEVELUP)
                    {
                        //#ifdef SD_BOOT_LOAD
                        BOOT_LOAD_FROM_SD(&Nand_InitStructure);
                        //#endif //SD_BOOT_LOAD
                    }
                    else if(pMsg->DivNum == FPGALEVELUP)
                    {
                        FPGABOOT_LOAD_FROM_SD();
                    }
                    else if(pMsg->DivNum == RMBMODELEVELUP)
                    {
                        CNYBOOT_LOAD_FROM_SD();
                    }
                    else if(pMsg->DivNum == MTARMLEVELUP)
                    {
                        MTARM_LOAD_FROM_SD();
                    }
				#else
				 BOOT_LOAD_FROM_SD(&Nand_InitStructure);
				#endif	
					
                    break;
#ifdef SDCARD_HOTPLUG
                case BSP_SDINIT_COMFROM_GPIO:
                    //memcpy(&SDSTAT,pMsg->pData,pMsg->DataLen);
                    SDSTAT = !SDSTAT;
                    if(SDSTAT == 0x01)
                    {
                        OSTimeDlyHMSM(0,0,1,0);
                        BSP_MMCSDInit();
                        fresult = f_open(&g_sFileObject, fname_app, FA_READ);
                        if(fresult != FR_OK)
                        {
                            SDSTAT = 0x00;
                        }
                        else
                        {
                            SDSTAT = 0x01;
                        }

                        if(SDSTAT == 0x01)
                        {
                           BOOT_LOAD_FROM_SD(&Nand_InitStructure);
                        }
                    }
                    break;
#endif //SDCARD_HOTPLUG
                default:
                    break;
            }
        }
    }
}

/***********************************************************************************************
* Function	    : TaskSDMinor
* Description	: SD����ȡ��������
* Input		    :
* Output		:
* Note(s)	    : FZ2000��
* Contributor	: CODE
***********************************************************************************************/
void TaskSDMinor(void *pdata)
{

    OSTimeDlyHMSM(0,0,0,500);


	while(1)
	{
		// ÿ500ms���һ�������Ļ�����
		OSTimeDlyHMSM(0,0,0,500);
		manage_dat_buf_ack_flag();
	}

}




/****************************************************************************************************
**����:void BOOT_LOAD_FROM_SD_AUTO(NandInfo_t Nand_InitStructure)
**����:�Զ���SD�����س���NAND
* ���:��
* ����:��
**auth:hxj, date: 2015-1-20 18:33
*****************************************************************************************************/
static void BOOT_LOAD_FROM_SD_AUTO(NandInfo_t *Nand_InitStructure)
{
    char const		*fname_app = "/auto_app";
    FRESULT 		fresult;
    const INT32U  	NandAddr = APP_LOAD_ADDR;
    INT16U      	usBytesRead;

    INT32U      	offset = 0;

    DBG_DIS("pending boot and app images program...\r\n");

    //--------------------------------------------------------------------------------------
	//�򿪸�Ŀ¼���ļ���app��
    OSTimeDlyHMSM(0,0,5,0);
    fresult = f_open(&g_sFileObject, fname_app, FA_READ);
    DBG_DIS("\r\n fresult=%d\r\n",fresult);
    if(fresult != FR_OK)
    {
        DBG_DIS("not find %s in sd,return\r\n",fname_app);
        return;
    }


    DBG_DIS("begin to load boot and app...!!!\r\n");
    memset(g_cTmpBuf,0,g_sFileObject.fsize);
    do
    {
        fresult = f_read(&g_sFileObject, g_cTmpBuf+offset, NAND_PAGESIZE_2048BYTES,&usBytesRead);
        if(fresult != FR_OK)
        {
            STOP_RUN("SD : f_read() != FR_OK \r\n");
            return;
        } 
        offset += usBytesRead;
        if(usBytesRead != NAND_PAGESIZE_2048BYTES)
            if(offset != g_sFileObject.fsize)
            {
                STOP_RUN("SD : usBytesRead is error \r\n");   
                return;
            }
    }while(offset != g_sFileObject.fsize);


    if(BSP_NandWrite(Nand_InitStructure,NandAddr,g_cTmpBuf,offset) != E_PASS)
    {
        STOP_RUN("NAND : BSP_NandWrite() != E_PASS \r\n");
        return;
    }

    if(BSP_NandRead(Nand_InitStructure,NandAddr,g_cTmpBuf,offset) != E_PASS)
    {
        STOP_RUN("NAND : BSP_NandRead() != E_PASS \r\n");
        return;
    }

    f_close(&g_sFileObject);

    if(FR_OK != f_unlink(fname_app))
    {
        DBG_DIS("delete app[%s] err!\r\n",fname_app);
    }
    else
    {
        DBG_DIS("delete app[%s] ok!\r\n",fname_app);
    }

    MY_DELAY_X_S(3);

    DBG_DIS("|---------------------------------------------------------|\r\n");
    DBG_DIS("|                                                         |\r\n");
    DBG_DIS("|==============update auto_app successfully===============|\r\n");
    DBG_DIS("|                                                         |\r\n");
    DBG_DIS("|---------------------------------------------------------|\r\n");

}





/***********************************************************************************************
* Function	    : BOOT_LOAD_FROM_SD
* Description	: ��SD�����س���NAND
* Input		    :
* Output	    :
* Note(s)	    : app�̶�λ��nand��0x80000λ��
* Contributor	: 11/12/2014	�γ�
***********************************************************************************************/
static void BOOT_LOAD_FROM_SD(NandInfo_t *Nand_InitStructure)
{
    char const	    *fname_app = "/app";
    FRESULT 	    fresult;
    const INT32U    NandAddr = APP_LOAD_ADDR;
    INT16U          usBytesRead;
    
    INT32U          offset = 0;
    

    DBG_DIS("pending app images program...\r\n");

    //--------------------------------------------------------------------------------------
	//�򿪸�Ŀ¼���ļ���app��
    OSTimeDlyHMSM(0,0,1,0);
    fresult = f_open(&g_sFileObject, fname_app, FA_READ);
    DBG_DIS("\r\n fresult=%d\r\n",fresult);
    if(fresult != FR_OK)
    {
        DBG_DIS("not find app in sd,return\r\n");
        return;
    }

    DBG_DIS("begin to load app...!!!\r\n");
    memset(g_cTmpBuf,0,g_sFileObject.fsize);
    do
    {
        fresult = f_read(&g_sFileObject, g_cTmpBuf+offset, NAND_PAGESIZE_2048BYTES,&usBytesRead);
        if(fresult != FR_OK)
        {
            STOP_RUN("SD : f_read() != FR_OK \r\n");
            return;
        } 
        offset += usBytesRead;
        if(usBytesRead != NAND_PAGESIZE_2048BYTES)
            if(offset != g_sFileObject.fsize)
            {
                STOP_RUN("SD : usBytesRead is error \r\n");   
                return;
            }
    }while(offset != g_sFileObject.fsize);


    if(BSP_NandWrite(Nand_InitStructure,NandAddr,g_cTmpBuf,offset) != E_PASS)
    {
        STOP_RUN("NAND : BSP_NandWrite() != E_PASS \r\n");
        return;
    }

    if(BSP_NandRead(Nand_InitStructure,NandAddr,g_cTmpBuf,offset) != E_PASS)
    {
        STOP_RUN("NAND : BSP_NandRead() != E_PASS \r\n");
        return;
    }

    f_close(&g_sFileObject);

    MY_DELAY_X_S(3);

    DBG_DIS("|---------------------------------------------------------|\r\n");
    DBG_DIS("|                                                         |\r\n");
    DBG_DIS("|==============update app successfully====================|\r\n");
    DBG_DIS("|                                                         |\r\n");
    DBG_DIS("|---------------------------------------------------------|\r\n");
    g_update_a8_success_flag=1;
#if FZ1500
	PraperLevelUpED();
#endif
}




/***********************************************************************************************
* Function	    : Save_Dat_SD
* Description	: SD���洢dat�ļ�
* Input		    :
* Output		:
* Note(s)	    : FZ2000��      /20141229/123456.dat
* Contributor	: CODE
***********************************************************************************************/
INT8U Save_Dat_SD(_BSP_MESSAGE *DatMsg)
{
    char fname[21];

    INT32U uintlens;
    INT16U offset;
    FRESULT fresult;
	WORD usBytes;
    INT8U perr;
    uint16 all_momey_cnt=0;



	TypeStruct_DatHeadFromat *DatHead;
    TypeStruct_CurrTime *CurTime;

    // ��dat�ļ��ڻ�ȡʱ����Ϣ�������ļ���
    DatHead = (TypeStruct_DatHeadFromat *)DatMsg->pData;
    CurTime = (TypeStruct_CurrTime *)DatHead->Time;
    Fname_From_Time(fname,CurTime);
    all_momey_cnt=DatHead->PaperNum;

    DBG_DIS("in Save_Dat_SD 0:all_momey_cnt=%d\r\n",all_momey_cnt);

    DBG_DIS("|-----%s save to SD: STR------|\r\n",fname);
    OSMutexPend(SDMutex,0,&perr);                      // ��ȡSD��������

    // �������ļ�
	fresult = f_open(&g_sFileObject, fname, FA_CREATE_NEW |FA_WRITE|FA_READ);
    switch(fresult)
    {
        case FR_NO_PATH:                    // ���·��������,�½�Ŀ¼,�½��ļ�
            fname[9] = '\0';
            fresult = f_mkdir(fname);
            fname[9] = '/';
            if(fresult != FR_OK)
                return  FALSE;
            fresult = f_open(&g_sFileObject, fname, FA_CREATE_NEW |FA_WRITE|FA_READ);
            if(fresult != FR_OK)
                return FALSE;
        case FR_OK:                         // ����������ļ��ɹ�
            break;
        default:                            // ����������ļ�ʧ��
            return FALSE;
    }
    do
    {
        if(DatMsg->DataLen > 0xffff)
        {
            uintlens = 0xffff;
            DatMsg->DataLen -= 0xffff;
        }
        else
        {
            uintlens = DatMsg->DataLen;
            DatMsg->DataLen = 0;
        }
        offset = 0;
        do
        {
            // д��SD��
            fresult = f_write(&g_sFileObject,DatMsg->pData+offset,(INT16U)uintlens - offset,&usBytes);
            if(fresult != FR_OK || usBytes == 0)
            {
                DBG_DIS("ERROR!!!ERROR!!!   :   in save %s,uintlens = %d,usBytes = %d\r\n",fname,uintlens,usBytes);
                return FALSE;
            }

            offset += usBytes;
        }while(offset != (INT16U)uintlens);

        DatMsg->pData += uintlens;

    }while(DatMsg->DataLen);
    // �ر�dat�ļ�
    f_close(&g_sFileObject);

    OSMutexPost(SDMutex);                               // ����
    DBG_DIS("|-----%s save to SD: END------|\r\n",fname);

	//list�ļ�׷��
	List_File_Add(fname);

    DBG_DIS("in Save_Dat_SD 100:save %s ok,all_momey_cnt=%d\r\n",fname,all_momey_cnt);
    return TRUE;
}

/***********************************************************************************************
* Function	    : Read_Dat_SD
* Description	: ��ȡSD����ȡ��dat�ļ�
* Input		    :
* Output		:
* Note(s)	    : ��ʱ��˳���ȡ�����ظ�����ȡ��dat������ǣ���ʱֻ��1��dat
* Contributor	: CODE
***********************************************************************************************/
INT8U Read_Dat_SD(char *pfname,INT8U index)
{
    FRESULT fresult;
	WORD usBytes;
    INT8U perr;
    int ret;
    INT32U uintlens;
    INT32U offset = 0;

    DBG_DIS("in Read_Dat_SD 0:\r\n");
    
    DBG_DIS("|-----%s read from SD: STR------|\r\n",pfname);
    OSMutexPend(SDMutex,0,&perr);                      // ��ȡSD��������

    // ���ļ�
	fresult = f_open(&g_sFileObject,pfname,FA_READ);
    if(fresult != FR_OK)            	// ���ļ�ʧ��
    {
        DBG_DIS("in Read_Dat_SD 1:return SD_READ_DAT_FAIL\r\n");
        return FALSE;
    }

    do
    {
        if((g_sFileObject.fsize - offset) > 0xffff)
            uintlens = 0xffff;
        else
            uintlens = g_sFileObject.fsize - offset;

        usBytes = 0;
        do
        {
            uintlens -= (INT32U)usBytes;
    	    // ���ļ�
            fresult = f_read(&g_sFileObject,Dat_SendBuff+offset,(INT16U)uintlens,&usBytes);
        	if(fresult != FR_OK || usBytes == 0)
        	{
                DBG_DIS("in Read_Dat_SD 2:return SD_READ_DAT_FAIL\r\n");
                return FALSE;
            }
            offset += (INT32U)usBytes;
        }while(uintlens != (INT32U)usBytes);
    }while(offset != g_sFileObject.fsize);

    // �ر�dat�ļ�
    f_close(&g_sFileObject);

	OSMutexPost(SDMutex);                               // ����
    DBG_DIS("|-----%s read from SD: END------|\r\n",pfname);
    
	// �����ֱ�Ӷ�������buffer
    ret=save_dat_file_to_send_buf((char *)Dat_SendBuff,offset);
    if(0!=ret)
    {
        DBG_DIS("in Read_Dat_SD 99:save dat to buf err,not send this dat,ret=%d\r\n",ret);
        return FALSE;
    }


    DBG_DIS("in Read_Dat_SD 100:\r\n");
	return TRUE;			// DAT�ļ���ȡ�ɹ�
}

/***********************************************************************************************
* Function	    : Get_Vol_SD
* Description	: ��ȡSD������������������
* Input		    :
* Output		: *total    ������(��λKB)
                  *free     ��������(��λKB)
* Note(s)	    : ʵ���������Դ���totalֵ�����FAT32�ļ�ϵͳ
* Contributor	: CODE
***********************************************************************************************/
static INT8U Get_Vol_SD(INT8U *drv,INT32U *total,INT32U *free)
{
    FATFS *fs_sd;
    FRESULT fresult;
    INT32U fre_clust=0,fre_sect=0,totl_sect=0;

    //�õ�������Ϣ�����д�����
    fresult = f_getfree((const char*)drv, &fre_clust, &fs_sd);
    if(fresult == FR_OK)
    {
        totl_sect = (fs_sd->max_clust-2) * fs_sd->sects_clust;  // ��������
        fre_sect = fre_clust * fs_sd->sects_clust;              // ����������
        *total = totl_sect >> 1;
        *free = fre_sect >> 1; 
        
        return TRUE;
    }

    return FALSE;
}

/***********************************************************************************************
* Function	    : Fname_From_Time
* Description	: ��dat�ļ��л�ȡ�ļ���
* Input		    :
* Output		:
* Note(s)	    : �ļ�����ʽ: /20141229/123456.dat
* Contributor	: CODE
***********************************************************************************************/
static void Fname_From_Time(char *pFname,TypeStruct_CurrTime *CurTime)
{

    pFname[0] = '/';
    memcpy(pFname+1,CurTime->Year,4);
    memcpy(pFname+5,CurTime->Month,2);
    memcpy(pFname+7,CurTime->Day,2);
    pFname[9] = '/';
    memcpy(pFname+10,CurTime->Hour,2);
    memcpy(pFname+12,CurTime->Minute,2);
    memcpy(pFname+14,CurTime->Second,2);
    pFname[16] = '.';
    pFname[17] = 'D';
    pFname[18] = 'A';
    pFname[19] = 'T';
    pFname[20] = '\0';
}

/***********************************************************************************************
* Function	    : Fname_Form_Comfirm
* Description	: �ļ�����ʽȷ��,������ڴ��������޸�(�޸�������ʱδ��)
* Input		    : index     ��ȷ�ϵ��ļ���������
* Output		: TRUE      �ļ�����ʽ��ȷ(���ɹ��޸�)
                  FALSE     �ļ�����ʽ�������޷��޸�
* Note(s)	    : �ļ�����ʽ: /20141229/123456.dat
* Contributor	: CODE
***********************************************************************************************/
static INT8U Fname_Form_Comfirm(INT32U idx)
{
    int val_y,val_m,val_d,val_h,val_min,val_s;
    char *pFname = ListFile[idx].fname;
    
    if(pFname[0] != '/')
    {
        pFname[0] = '0';
    }
    
    val_y = (atoi(pFname+1)/10000);
    val_m = (atoi(pFname+5)/100);
    val_d = atoi(pFname+7);
    if((val_y > 2014) && (val_y < 2030))
    {
        if((val_m > 0) && (val_m < 13))
        {
            if((val_d > 0) && (val_m < 32))
            {
                if(pFname[9] != '/')
                {
                    pFname[9] = '0';
                }
                
                val_h = (atoi(pFname+10)/10000);
                val_min = (atoi(pFname+12)/100);
                val_s = atoi(pFname+14);
                
                if((val_h >= 0) && (val_h < 25))
                {
                    if((val_min >= 0) && (val_min < 61))
                    {
                        if((val_s >= 0) && (val_s < 61))
                        {                       
                            if(memcmp(pFname+16,".DAT",4))
                                memcpy(pFname+16,".DAT",4);
                            
                            return TRUE;
                        }
                    }
                }
            }
        }
        
    }

       

    return FALSE;
}

/***********************************************************************************************
* Function	    : manage_dat_buf_ack_flag
* Description	: ��������������
* Input		    :
* Output		:
* Note(s)	    :
* Contributor	: CODE
***********************************************************************************************/
int manage_dat_buf_ack_flag(void)
{
    INT8U i;
	char fname[21];
    char *pfname;

    if(0==g_init_tcp_ip_data_buf_over_flag) return 0;
    if(NULL == p_dat_file_buf_data) return 0;
    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(DATF_SEND_OK==p_dat_file_buf_data[i].dat_valid_flag)			//���DAT�Ѿ��ɹ�����
        {
			Fname_From_Time(fname,(TypeStruct_CurrTime *)p_dat_file_buf_data[i].time);
			List_File_Update(fname,0xBB);

            set_recv_dat_file_flag_by_index(i,DATF_EMPTY);

			// ��ѯlist�ļ������Ƿ�����δ������dat
			pfname = List_File_Inquire();

            // �������δ�����ģ����SD����ȡ��Ȼ������
			if(pfname != NULL)
				Read_Dat_SD(pfname,i);
        }
        else if(DATF_STOP_SEND==p_dat_file_buf_data[i].dat_valid_flag)  //���DAT������,���ٷ�
        {
			Fname_From_Time(fname,(TypeStruct_CurrTime *)p_dat_file_buf_data[i].time);
			List_File_Update(fname,0xCC);

			set_recv_dat_file_flag_by_index(i,DATF_EMPTY);

			// ��ѯlist�ļ������Ƿ�����δ������dat
			pfname = List_File_Inquire();
			if(pfname != NULL)
				Read_Dat_SD(pfname,i);
        }
        else if(DATF_EMPTY == p_dat_file_buf_data[i].dat_valid_flag)    // ���buffer��ԪΪ��
        {
            // ��ѯlist�ļ������Ƿ�����δ������dat
			pfname = List_File_Inquire();

            // �������δ�����ģ����SD����ȡ��Ȼ������
			if(pfname != NULL)
			{
                Read_Dat_SD(pfname,i);


            }

        }
    }

    return 0;

}

/***********************************************************************************************
* Function	    : List_File_Load
* Description	: ��SD������list�ļ�
* Input		    :
* Output		:
* Note(s)	    : ϵͳ�ϵ�ʱ���ã����ص�list�ļ��п������½��Ŀհ��ļ�
* Contributor	: CODE
***********************************************************************************************/
static INT8U List_File_Load(void)
{
	const char fname[10]="/list.txt";
    FRESULT fresult;
	WORD usBytes;
    INT8U perr;
    INT32U uintlens;
    INT32U offset = 0;

    OSMutexPend(SDMutex,0,&perr);                      // ��ȡSD��������

	// �򿪹����ļ����������򴴽�
	fresult = f_open(&g_sFileObject,fname, FA_OPEN_ALWAYS |FA_WRITE|FA_READ);
	if(fresult != FR_OK)
		return FALSE;

	// ��������ļ��Ѿ�����,��ȡ�����ļ�
	if(g_sFileObject.fsize > 0)
	{
	    do
        {
            if((g_sFileObject.fsize - offset) > 0xffff)
            uintlens = 0xffff;
            else
                uintlens = g_sFileObject.fsize - offset;

            usBytes = 0;
            do
            {
                uintlens -= (INT32U)usBytes;
        		// ��ȡ�����ļ�����
        		fresult = f_read(&g_sFileObject,(INT8U *)ListFile+offset,(INT16U)uintlens,&usBytes);
        		if(fresult != FR_OK || usBytes == 0)
        			return FALSE;
                offset += (INT32U)usBytes;
            }while(uintlens != (INT32U)usBytes);
        }while(offset != g_sFileObject.fsize);

        // ��ȡlist file��׼������
    	for(;LF_Basic_Index<MAX_LIST_NUM;LF_Basic_Index++)
        {
            if(ListFile[LF_Basic_Index].fileflag == 0x00)
                break;
            else if(ListFile[LF_Basic_Index].fileflag == 0xAA)
                break;
            else if(ListFile[LF_Basic_Index].fileflag == 0xBB)
                continue;
            else 
                DBG_DIS("ERROR!!!ERROR!!!   :   LIST FILE UNIT ERROR--->INDEX:%d FLAG:%s\r\n",LF_Basic_Index,ListFile[LF_Basic_Index].fileflag);           
        }
	}
    else
    {
        // д��һ����list��Ԫ
        fresult = f_write(&g_sFileObject,ListFile,sizeof(TypeStruct_UnitAttribute),&usBytes);
        if(fresult != FR_OK || usBytes != sizeof(TypeStruct_UnitAttribute))
            return FALSE;
    }

    // �ر��ļ�
	f_close(&g_sFileObject);

    OSMutexPost(SDMutex);                               // ����

	return TRUE;
}

/***********************************************************************************************
* Function	    : List_File_Add
* Description	: list�ļ���Ԫ׷��
* Input		    : *pfname   ��Ҫ׷�ӵ�dat�ļ���
* Output		:
* Note(s)	    : ֻҪlist�ļ��й�׷�ӣ�����Ҫ��дSD��
* Contributor	: CODE
***********************************************************************************************/
static INT8U List_File_Add(char *pfname)
{
	INT32U tmp_idx;

	for(tmp_idx=LF_Basic_Index;tmp_idx<MAX_LIST_NUM;tmp_idx++)
	{
		if(ListFile[tmp_idx].fileflag == 0x00)
		{
			ListFile[tmp_idx].fileflag = 0xAA;
			memcpy(ListFile[tmp_idx].fname,pfname,21);

            // ˢ��SD���е�list�ļ�
            if(List_File_Save() == FALSE)
                DBG_DIS("ERROR!!!ERROR!!!   :   LIST FILE SAVE ERROR\r\n");

			return TRUE;
		}
	}

	return FALSE;
}


/***********************************************************************************************
* Function	    : List_File_Inquire
* Description	: ����list�ļ�����ȡһ��δ�ϴ����ļ���
* Input		    :
* Output		: ��ȡ�ɹ������ļ�������ȡʧ�ܷ���NULL
* Note(s)	    : ��ʱ��־ֻ��������ڴ��е�list��������д��SD���е�list
* Contributor	: CODE
***********************************************************************************************/
static char *List_File_Inquire(void)
{
	INT32U tmp_idx;

	for(tmp_idx=LF_Basic_Index;tmp_idx<MAX_LIST_NUM;tmp_idx++)
	{
		if(ListFile[tmp_idx].fileflag == 0xAA)
        {
            // ��δ������־���Ϊ��ʱ��־
            ListFile[tmp_idx].fileflag = 0xDD;

            // �ļ�����ʽȷ��,�����޷��޸����ļ�������Ϊ��
            if(Fname_Form_Comfirm(tmp_idx) == TRUE) 
			    return ListFile[tmp_idx].fname;
            else
            {
                DBG_DIS("ERROR!!!ERROR!!! : Fname %s Form ERROR\r\n",ListFile[tmp_idx].fname);
                ListFile[tmp_idx].fileflag = 0xCC;
            }
        }
        // ���������0x00��־�����˳�����������NULL
		else if(ListFile[tmp_idx].fileflag == 0x00)
			return NULL;
	}

	// list�ļ��б���ʱ�����س���
	return NULL;
}

/***********************************************************************************************
* Function	    : List_File_Update
* Description	: ˢ��list�ļ�
* Input		    : *pfname   ��Ҫˢ�µ�dat�ļ���
*                 fbflag    ��Ҫˢ�³ɵı�־    0xBB - ��dat�����ɹ�
*                                               0xCC - ��dat����ʧ��
* Output		:
* Note(s)	    : �ڴ�����SD�����ű�Ҫˢ��
* Contributor	: CODE
***********************************************************************************************/
static INT8U List_File_Update(char *pfname,INT8U fbflag)
{
	INT32U tmp_idx;
	int rs;

	for(tmp_idx=LF_Basic_Index;tmp_idx<MAX_LIST_NUM;tmp_idx++)
	{
	    // list����ֻ�б�־Ϊ0xDD���ļ�����Ҫˢ��
		if(ListFile[tmp_idx].fileflag == 0xDD)
		{
		    // �Ƚ��ļ����Ƿ�ƥ��
			rs = memcmp(ListFile[tmp_idx].fname,pfname,21);
			if(!rs)
			{
				ListFile[tmp_idx].fileflag = fbflag;

                // ֻ��list�������ļ���ˢ�¹�������Ҫ��list��дSD��
                if(List_File_Save() == FALSE)
                    DBG_DIS("ERROR!!!ERROR!!!   :   LIST FILE LOAD ERROR\r\n");

				return TRUE;
			}
		}
        // ���������0x00��־�����ټ�������������ʧ�ܽ��
		else if(ListFile[tmp_idx].fileflag == 0x00)
			break;
	}

	return FALSE;
}

/***********************************************************************************************
* Function	    : List_File_Save
* Description	: ����list�ļ�
* Input		    :
* Output		:
* Note(s)	    : ����ʹ�õݹ��㷨
* Contributor	: CODE
***********************************************************************************************/
static INT8U List_File_Save(void)
{
	const char fname[10]="/list.txt";
    FRESULT fresult;
	WORD usBytes;
    INT8U perr;
    INT32U offset;

	INT32U tmp_idx;

    // ��ȡ�ڴ���list�ļ�����
    for(tmp_idx=LF_Basic_Index;tmp_idx<MAX_LIST_NUM;tmp_idx++)
    {
        // �����������־0xDD���ȸ�Ϊ0xAA,��д��SD�����ٸĻ�
        if(ListFile[tmp_idx].fileflag == 0xDD)
        {
            ListFile[tmp_idx].fileflag = 0xAA;
            perr = List_File_Save();
            ListFile[tmp_idx].fileflag = 0xDD;

            return perr;
        }
        else if(ListFile[tmp_idx].fileflag == 0x00)
            break;
    }

    DBG_DIS("|-----LIST FILE save: STR------|\r\n");
    OSMutexPend(SDMutex,0,&perr);                      // ��ȡSD��������

    offset = sizeof(TypeStruct_UnitAttribute)*LF_Basic_Index;
	// �򿪹����ļ����������򱨴�
	fresult = f_open(&g_sFileObject,fname, FA_OPEN_EXISTING |FA_WRITE|FA_READ);
	if(fresult != FR_OK)
		return FALSE;

    // �����ļ�ָ��
    fresult = f_lseek(&g_sFileObject,offset);
    if(fresult != FR_OK)
		return FALSE;

    // ���ڴ���list�ļ�����д��SD��
	fresult = f_write(&g_sFileObject,ListFile+LF_Basic_Index,sizeof(TypeStruct_UnitAttribute)*(tmp_idx-LF_Basic_Index),&usBytes);
	if(fresult != FR_OK || sizeof(TypeStruct_UnitAttribute)*(tmp_idx-LF_Basic_Index) != usBytes)
		return FALSE;

    // �رչ����ļ�
    f_close(&g_sFileObject);

    //tmp_idx = 0;

    OSMutexPost(SDMutex);                               // ����
    DBG_DIS("|-----LIST FILE save: END------|\r\n");

	return TRUE;
}


 
#if FZ1500



/***********************************************************************************************
* Function	: DSPBOOT_LOAD_FROM_SD
* Description	:��SD������DSP
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void DSPBOOT_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname[10]="/dsp";
    INT16U 	usBytesRead = 0;

    //�򿪸�Ŀ¼���ļ���1MB��
    fresult = f_open(&g_sFileObject, fname, FA_READ);

    if(fresult != FR_OK)
    while(1);

    //����ʼλ�ö��ļ���app����sizeof(g_cTmpBuf)�ֽ�
    do
    {
          fresult = f_read(&g_sFileObject,g_cTmpBuf+1+g_sFileObject.fptr,0xD000,&usBytesRead);
          if(fresult != FR_OK)
                    while(1);
    }while(usBytesRead);


    *g_cTmpBuf = UPDATE_FOR_DSP;	// DSP

    pMsg.MsgID = SEND_REQ | SUB_ID_UPDATE;				
    pMsg.DivNum = DSP2;
    pMsg.pData = g_cTmpBuf;
    pMsg.DataLen = g_sFileObject.fsize;
    SYSPost(W5500BEvent,&pMsg);

}

/***********************************************************************************************
* Function	: FPGABOOT_LOAD_FROM_SD
* Description	:��SD������DSP
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void FPGABOOT_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname[10]="/fpga";
    INT16U 	usBytesRead = 0;

    //�򿪸�Ŀ¼���ļ���1MB��
    fresult = f_open(&g_sFileObject, fname, FA_READ);

    if(fresult != FR_OK)
    while(1);

    //����ʼλ�ö��ļ���app����sizeof(g_cTmpBuf)�ֽ�
    do
    {
          fresult = f_read(&g_sFileObject,g_cTmpBuf+1+g_sFileObject.fptr,0xD000,&usBytesRead);
          if(fresult != FR_OK)
                    while(1);
    }while(usBytesRead);


    *g_cTmpBuf = UPDATE_FOR_FPGA;	//FPGA

    pMsg.MsgID = SEND_REQ | SUB_ID_UPDATE;				
    pMsg.DivNum = DSP2;
    pMsg.pData = g_cTmpBuf;
    pMsg.DataLen = g_sFileObject.fsize;
    SYSPost(W5500BEvent,&pMsg);

}

/***********************************************************************************************
* Function	: BOOT_LOAD_FROM_SD
* Description	:��SD������DSP
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void CNYBOOT_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname[10]="/cny";
    INT16U 	usBytesRead = 0;

    //�򿪸�Ŀ¼���ļ���1MB��
    fresult = f_open(&g_sFileObject, fname, FA_READ);

    if(fresult != FR_OK)
    while(1);

    //����ʼλ�ö��ļ���app����sizeof(g_cTmpBuf)�ֽ�
    do
    {
          fresult = f_read(&g_sFileObject,g_cTmpBuf+1+g_sFileObject.fptr,0xD000,&usBytesRead);
          if(fresult != FR_OK)
                    while(1);
    }while(usBytesRead);


    *g_cTmpBuf = UPDATE_FOR_CNY;	// CNY

    pMsg.MsgID = SEND_REQ | SUB_ID_UPDATE;				
    pMsg.DivNum = DSP2;
    pMsg.pData = g_cTmpBuf;
    pMsg.DataLen = g_sFileObject.fsize;
    SYSPost(W5500BEvent,&pMsg);
}

/***********************************************************************************************
* Function	: MTARM_LOAD_FROM_SD
* Description	:��SD������MTARM
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void MTARM_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname_app[10]="/mt";
    INT16U 	usBytesRead = 0;

    //�򿪸�Ŀ¼���ļ���1MB��
    fresult = f_open(&g_sFileObject, fname_app, FA_READ);
    DBG_DIS("\r\n fresult=%d\r\n",fresult);
    if(fresult != FR_OK)
    {
        DBG_DIS("not find %s in sd,return\r\n",fname_app);
        return;
    }


    DBG_DIS("begin to load boot and app...!!!\r\n");
    do
    {
        fresult = f_read(&g_sFileObject,g_cTmpBuf+g_sFileObject.fptr,0xD000,&usBytesRead);
        if(fresult != FR_OK)
        {
            DBG_DIS("f_read is not ok,return\r\n");
            return;
         }
    }while(usBytesRead);

    pMsg.MsgID = APP_MT_LEVEL_UP;
    pMsg.pData = g_cTmpBuf;
    pMsg.DataLen = g_sFileObject.fsize;
    SYSPost(canEvent,&pMsg);

    DBG_DIS("|---------------------------------------------------------|\r\n");
    DBG_DIS("|                                                         |\r\n");
    DBG_DIS("|==============update mt arm successfully===============|\r\n");
    DBG_DIS("|                                                         |\r\n");
    DBG_DIS("|---------------------------------------------------------|\r\n");
}

#endif


/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
