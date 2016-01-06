/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		    : mmcsdmain.c
* Author		    :
* Date First Issued	: 130722
* Version		    : V
* Description		: SD卡应用程序
*----------------------------------------历史版本信息-------------------------------------------
* History		    :   20150122 - 添加断网续存功能
*                       20150127 - 修正断网续存功能，主要在内存区list文件中添加一个临时标志
*                       20150128 - 修正list文件长度超过512(1个SD扇区)时，数据放入内存出错问题
*                       20150129 - 修正300张存储dat文件失败问题;扩增TaskSDMaster任务的堆栈长度,解决程序跑飞问题
                        20150311 - 扩增SD卡存储dat文件数量，修正list文件对齐问题
                        20140312 - 修正list文件65KB上限问题,添加SD卡容量获取功能
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
// 接收到的数据格式，一张纸币的数据
typedef struct{
    char Reserve[5];        // 保留字节
    char Guanzihao[10];     // 冠字号
    char Denomination;      // 币值
    char Version;           // 版本
    char Result;            // 真假
    char SectDraw[180];     // 抠图
}TypeStruct_RecvFormat;

// dat文件头格式
typedef struct{
    char FileHead[5];       // 文件起始字符
    char TradeID;           // 交易代码
    char Reserve[11];       // 保留
    INT16U PaperNum;        // 总张数
    char MachineID[24];     // 机器号
    char Time[14];          // 时间
    char Batch[6];          // 批次，暂时没用，填0
    char Status;            // 状态标志，暂时没用，填0
    char ErrorNum;          // 失败次数，暂时没用，填0
}TypeStruct_DatHeadFromat;;
#pragma pack()

// 管理文件的单元属性
typedef struct{
	char fname[21];			// 待发送文件名
	INT8U fileflag;			// 文件标志，0xAA:未读标志 0xBB:已读标志	0xCC:损坏标志   0xDD:临时标志，表示上传中 0x00:空
	INT8U UnitNums;         // 后面还有的单元个数
}TypeStruct_UnitAttribute;

// 当前时间结构体
typedef struct{
    char Year[4];
    char Month[2];
    char Day[2];
    char Hour[2];
    char Minute[2];
    char Second[2];
}TypeStruct_CurrTime;

/* Private function prototypes----------------------------------------------------------------*/
OS_EVENT *MMCSDEvent;                                   // SD卡事件控制块
void *MMCSDOSQ[4];					                    // 消息队列
static OS_EVENT *SDMutex;                               // SD卡互斥体
OS_STK TaskSDMinorStk[TASK_STK_SIZE_SD_MINOR];          // SD卡存储辅助任务堆栈


INT8U SDSTAT = 0x01;
INT8U Datalen = 0;

static INT8U Dat_SendBuff[1024*1024];                   // dat文件缓存区,容量1MB，dat文件最大尺寸73KB
static INT32U LF_Basic_Index = 0;                       // list file 基准索引单元
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment = 128
static TypeStruct_UnitAttribute ListFile[MAX_LIST_NUM]; // list file 缓存区
#endif
/* Private functions--------------------------------------------------------------------------*/
//把SD卡上boot下载到NAND中
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
* Description	: SD卡处理程序（通过SD卡烧写整个工程，工程存入nand）
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

    //Nand 初始化相关结构体定义
    NandInfo_t 		    Nand_InitStructure;
    NandCtrlInfo_t          nandCtrlInfo;
    NandEccInfo_t           nandEccInfo;
    NandDmaInfo_t           nandDmaInfo;
    GPMCNANDTimingInfo_t    nandTimingInfo;

    //Nand初始化
    Nand_InitStructure.hNandCtrlInfo 	= &nandCtrlInfo;
    Nand_InitStructure.hNandDmaInfo	= &nandDmaInfo;
    Nand_InitStructure.hNandEccInfo	= &nandEccInfo;
    nandCtrlInfo.hNandTimingInfo	= &nandTimingInfo;

    DBG_DIS("in TaskMMCSD 1:Nand config\r\n");

    if(BSP_NandConfig(&Nand_InitStructure) != NAND_STATUS_PASSED)
    {
        STOP_RUN("BSP_NandConfig(&Nand_InitStructure) != NAND_STATUS_PASSED \r\n");
    }

    //SD卡初始化
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

    // SD卡互斥锁
    SDMutex = OSMutexCreate(PRI_SD_MUTEX,&err);

    // 自动升级
    DBG_DIS("in TaskMMCSD 3_1:BOOT_LOAD_FROM_SD_AUTO start\r\n");
    BOOT_LOAD_FROM_SD_AUTO(&Nand_InitStructure);
    DBG_DIS("in TaskMMCSD 3_2:BOOT_LOAD_FROM_SD_AUTO   end\r\n");


    DBG_DIS("in TaskMMCSD 4_1:get list file start\r\n");
    // 获取list文件
    memset(ListFile,0,sizeof(TypeStruct_UnitAttribute)*MAX_LIST_NUM);
	if( List_File_Load() == FALSE)
        DBG_DIS("ERROR!!!ERROR!!!   :   LIST FILE LOAD ERROR\r\n");
    DBG_DIS("in TaskMMCSD 4_2:get list file   end\r\n");

    // 创建SD卡存取辅助任务
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
        _BSP_MESSAGE *pMsg;						// 消息指针
        pMsg = OSQPend(MMCSDEvent,SYS_DELAY_100ms,&err);
        if(err == OS_NO_ERR)				    // 收到消息
        {
            switch(pMsg->MsgID)					// 判断消息
            {
                case APP_SAVE_DAT:              // 保存dat文件至SD
                    err = Save_Dat_SD(pMsg);
                    break;
                case APP_SD_VOLUM:              // 获取SD卡容量    
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
* Description	: SD卡存取辅助任务
* Input		    :
* Output		:
* Note(s)	    : FZ2000用
* Contributor	: CODE
***********************************************************************************************/
void TaskSDMinor(void *pdata)
{

    OSTimeDlyHMSM(0,0,0,500);


	while(1)
	{
		// 每500ms检查一下网发的缓冲区
		OSTimeDlyHMSM(0,0,0,500);
		manage_dat_buf_ack_flag();
	}

}




/****************************************************************************************************
**名称:void BOOT_LOAD_FROM_SD_AUTO(NandInfo_t Nand_InitStructure)
**功能:自动从SD卡下载程序到NAND
* 入口:无
* 出口:无
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
	//打开根目录下文件“app”
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
* Description	: 从SD卡下载程序到NAND
* Input		    :
* Output	    :
* Note(s)	    : app固定位于nand中0x80000位置
* Contributor	: 11/12/2014	宋超
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
	//打开根目录下文件“app”
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
* Description	: SD卡存储dat文件
* Input		    :
* Output		:
* Note(s)	    : FZ2000用      /20141229/123456.dat
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

    // 从dat文件内获取时间信息，生成文件名
    DatHead = (TypeStruct_DatHeadFromat *)DatMsg->pData;
    CurTime = (TypeStruct_CurrTime *)DatHead->Time;
    Fname_From_Time(fname,CurTime);
    all_momey_cnt=DatHead->PaperNum;

    DBG_DIS("in Save_Dat_SD 0:all_momey_cnt=%d\r\n",all_momey_cnt);

    DBG_DIS("|-----%s save to SD: STR------|\r\n",fname);
    OSMutexPend(SDMutex,0,&perr);                      // 获取SD卡互斥锁

    // 创建新文件
	fresult = f_open(&g_sFileObject, fname, FA_CREATE_NEW |FA_WRITE|FA_READ);
    switch(fresult)
    {
        case FR_NO_PATH:                    // 如果路径不存在,新建目录,新建文件
            fname[9] = '\0';
            fresult = f_mkdir(fname);
            fname[9] = '/';
            if(fresult != FR_OK)
                return  FALSE;
            fresult = f_open(&g_sFileObject, fname, FA_CREATE_NEW |FA_WRITE|FA_READ);
            if(fresult != FR_OK)
                return FALSE;
        case FR_OK:                         // 如果创建新文件成功
            break;
        default:                            // 如果创建新文件失败
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
            // 写入SD卡
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
    // 关闭dat文件
    f_close(&g_sFileObject);

    OSMutexPost(SDMutex);                               // 解锁
    DBG_DIS("|-----%s save to SD: END------|\r\n",fname);

	//list文件追加
	List_File_Add(fname);

    DBG_DIS("in Save_Dat_SD 100:save %s ok,all_momey_cnt=%d\r\n",fname,all_momey_cnt);
    return TRUE;
}

/***********************************************************************************************
* Function	    : Read_Dat_SD
* Description	: 读取SD卡存取的dat文件
* Input		    :
* Output		:
* Note(s)	    : 按时间顺序读取，不重复，读取后dat需做标记，暂时只读1个dat
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
    OSMutexPend(SDMutex,0,&perr);                      // 获取SD卡互斥锁

    // 打开文件
	fresult = f_open(&g_sFileObject,pfname,FA_READ);
    if(fresult != FR_OK)            	// 打开文件失败
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
    	    // 读文件
            fresult = f_read(&g_sFileObject,Dat_SendBuff+offset,(INT16U)uintlens,&usBytes);
        	if(fresult != FR_OK || usBytes == 0)
        	{
                DBG_DIS("in Read_Dat_SD 2:return SD_READ_DAT_FAIL\r\n");
                return FALSE;
            }
            offset += (INT32U)usBytes;
        }while(uintlens != (INT32U)usBytes);
    }while(offset != g_sFileObject.fsize);

    // 关闭dat文件
    f_close(&g_sFileObject);

	OSMutexPost(SDMutex);                               // 解锁
    DBG_DIS("|-----%s read from SD: END------|\r\n",pfname);
    
	// 读完后直接丢给网发buffer
    ret=save_dat_file_to_send_buf((char *)Dat_SendBuff,offset);
    if(0!=ret)
    {
        DBG_DIS("in Read_Dat_SD 99:save dat to buf err,not send this dat,ret=%d\r\n",ret);
        return FALSE;
    }


    DBG_DIS("in Read_Dat_SD 100:\r\n");
	return TRUE;			// DAT文件读取成功
}

/***********************************************************************************************
* Function	    : Get_Vol_SD
* Description	: 获取SD卡总容量、空余容量
* Input		    :
* Output		: *total    总容量(单位KB)
                  *free     空余容量(单位KB)
* Note(s)	    : 实际总容量略大于total值，详见FAT32文件系统
* Contributor	: CODE
***********************************************************************************************/
static INT8U Get_Vol_SD(INT8U *drv,INT32U *total,INT32U *free)
{
    FATFS *fs_sd;
    FRESULT fresult;
    INT32U fre_clust=0,fre_sect=0,totl_sect=0;

    //得到磁盘信息及空闲簇数量
    fresult = f_getfree((const char*)drv, &fre_clust, &fs_sd);
    if(fresult == FR_OK)
    {
        totl_sect = (fs_sd->max_clust-2) * fs_sd->sects_clust;  // 总扇区数
        fre_sect = fre_clust * fs_sd->sects_clust;              // 空闲扇区数
        *total = totl_sect >> 1;
        *free = fre_sect >> 1; 
        
        return TRUE;
    }

    return FALSE;
}

/***********************************************************************************************
* Function	    : Fname_From_Time
* Description	: 从dat文件中获取文件名
* Input		    :
* Output		:
* Note(s)	    : 文件名格式: /20141229/123456.dat
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
* Description	: 文件名格式确认,如果存在错误则尝试修复(修复功能暂时未加)
* Input		    : index     待确认的文件名索引号
* Output		: TRUE      文件名格式正确(含成功修复)
                  FALSE     文件名格式错误且无法修复
* Note(s)	    : 文件名格式: /20141229/123456.dat
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
* Description	: 管理网发缓冲区
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
        if(DATF_SEND_OK==p_dat_file_buf_data[i].dat_valid_flag)			//这个DAT已经成功发送
        {
			Fname_From_Time(fname,(TypeStruct_CurrTime *)p_dat_file_buf_data[i].time);
			List_File_Update(fname,0xBB);

            set_recv_dat_file_flag_by_index(i,DATF_EMPTY);

			// 查询list文件，看是否有尚未网发的dat
			pfname = List_File_Inquire();

            // 如果有尚未网发的，则从SD卡读取，然后网发
			if(pfname != NULL)
				Read_Dat_SD(pfname,i);
        }
        else if(DATF_STOP_SEND==p_dat_file_buf_data[i].dat_valid_flag)  //这个DAT有问题,不再发
        {
			Fname_From_Time(fname,(TypeStruct_CurrTime *)p_dat_file_buf_data[i].time);
			List_File_Update(fname,0xCC);

			set_recv_dat_file_flag_by_index(i,DATF_EMPTY);

			// 查询list文件，看是否有尚未网发的dat
			pfname = List_File_Inquire();
			if(pfname != NULL)
				Read_Dat_SD(pfname,i);
        }
        else if(DATF_EMPTY == p_dat_file_buf_data[i].dat_valid_flag)    // 这个buffer单元为空
        {
            // 查询list文件，看是否有尚未网发的dat
			pfname = List_File_Inquire();

            // 如果有尚未网发的，则从SD卡读取，然后网发
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
* Description	: 从SD卡加载list文件
* Input		    :
* Output		:
* Note(s)	    : 系统上电时调用，加载的list文件有可能是新建的空白文件
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

    OSMutexPend(SDMutex,0,&perr);                      // 获取SD卡互斥锁

	// 打开管理文件，不存在则创建
	fresult = f_open(&g_sFileObject,fname, FA_OPEN_ALWAYS |FA_WRITE|FA_READ);
	if(fresult != FR_OK)
		return FALSE;

	// 如果管理文件已经存在,读取管理文件
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
        		// 读取管理文件内容
        		fresult = f_read(&g_sFileObject,(INT8U *)ListFile+offset,(INT16U)uintlens,&usBytes);
        		if(fresult != FR_OK || usBytes == 0)
        			return FALSE;
                offset += (INT32U)usBytes;
            }while(uintlens != (INT32U)usBytes);
        }while(offset != g_sFileObject.fsize);

        // 获取list file基准索引号
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
        // 写入一个空list单元
        fresult = f_write(&g_sFileObject,ListFile,sizeof(TypeStruct_UnitAttribute),&usBytes);
        if(fresult != FR_OK || usBytes != sizeof(TypeStruct_UnitAttribute))
            return FALSE;
    }

    // 关闭文件
	f_close(&g_sFileObject);

    OSMutexPost(SDMutex);                               // 解锁

	return TRUE;
}

/***********************************************************************************************
* Function	    : List_File_Add
* Description	: list文件单元追加
* Input		    : *pfname   需要追加的dat文件名
* Output		:
* Note(s)	    : 只要list文件有过追加，就需要回写SD卡
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

            // 刷新SD卡中的list文件
            if(List_File_Save() == FALSE)
                DBG_DIS("ERROR!!!ERROR!!!   :   LIST FILE SAVE ERROR\r\n");

			return TRUE;
		}
	}

	return FALSE;
}


/***********************************************************************************************
* Function	    : List_File_Inquire
* Description	: 遍历list文件，获取一个未上传的文件名
* Input		    :
* Output		: 获取成功返回文件名，获取失败返回NULL
* Note(s)	    : 临时标志只会出现在内存中的list，而不会写入SD卡中的list
* Contributor	: CODE
***********************************************************************************************/
static char *List_File_Inquire(void)
{
	INT32U tmp_idx;

	for(tmp_idx=LF_Basic_Index;tmp_idx<MAX_LIST_NUM;tmp_idx++)
	{
		if(ListFile[tmp_idx].fileflag == 0xAA)
        {
            // 将未网发标志变更为临时标志
            ListFile[tmp_idx].fileflag = 0xDD;

            // 文件名格式确认,对于无法修复的文件名则标记为损坏
            if(Fname_Form_Comfirm(tmp_idx) == TRUE) 
			    return ListFile[tmp_idx].fname;
            else
            {
                DBG_DIS("ERROR!!!ERROR!!! : Fname %s Form ERROR\r\n",ListFile[tmp_idx].fname);
                ListFile[tmp_idx].fileflag = 0xCC;
            }
        }
        // 如果遍历到0x00标志，则退出遍历，返回NULL
		else if(ListFile[tmp_idx].fileflag == 0x00)
			return NULL;
	}

	// list文件列表满时，返回出错
	return NULL;
}

/***********************************************************************************************
* Function	    : List_File_Update
* Description	: 刷新list文件
* Input		    : *pfname   需要刷新的dat文件名
*                 fbflag    需要刷新成的标志    0xBB - 此dat网发成功
*                                               0xCC - 此dat网发失败
* Output		:
* Note(s)	    : 内存区和SD卡两张表都要刷新
* Contributor	: CODE
***********************************************************************************************/
static INT8U List_File_Update(char *pfname,INT8U fbflag)
{
	INT32U tmp_idx;
	int rs;

	for(tmp_idx=LF_Basic_Index;tmp_idx<MAX_LIST_NUM;tmp_idx++)
	{
	    // list表中只有标志为0xDD的文件才需要刷新
		if(ListFile[tmp_idx].fileflag == 0xDD)
		{
		    // 比较文件名是否匹配
			rs = memcmp(ListFile[tmp_idx].fname,pfname,21);
			if(!rs)
			{
				ListFile[tmp_idx].fileflag = fbflag;

                // 只有list表中有文件被刷新过，才需要将list回写SD卡
                if(List_File_Save() == FALSE)
                    DBG_DIS("ERROR!!!ERROR!!!   :   LIST FILE LOAD ERROR\r\n");

				return TRUE;
			}
		}
        // 如果遍历到0x00标志，则不再继续遍历，返回失败结果
		else if(ListFile[tmp_idx].fileflag == 0x00)
			break;
	}

	return FALSE;
}

/***********************************************************************************************
* Function	    : List_File_Save
* Description	: 保存list文件
* Input		    :
* Output		:
* Note(s)	    : 这里使用递归算法
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

    // 获取内存区list文件长度
    for(tmp_idx=LF_Basic_Index;tmp_idx<MAX_LIST_NUM;tmp_idx++)
    {
        // 如果遍历到标志0xDD，先改为0xAA,等写入SD卡后再改回
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
    OSMutexPend(SDMutex,0,&perr);                      // 获取SD卡互斥锁

    offset = sizeof(TypeStruct_UnitAttribute)*LF_Basic_Index;
	// 打开管理文件，不存在则报错
	fresult = f_open(&g_sFileObject,fname, FA_OPEN_EXISTING |FA_WRITE|FA_READ);
	if(fresult != FR_OK)
		return FALSE;

    // 调整文件指针
    fresult = f_lseek(&g_sFileObject,offset);
    if(fresult != FR_OK)
		return FALSE;

    // 将内存区list文件部分写入SD卡
	fresult = f_write(&g_sFileObject,ListFile+LF_Basic_Index,sizeof(TypeStruct_UnitAttribute)*(tmp_idx-LF_Basic_Index),&usBytes);
	if(fresult != FR_OK || sizeof(TypeStruct_UnitAttribute)*(tmp_idx-LF_Basic_Index) != usBytes)
		return FALSE;

    // 关闭管理文件
    f_close(&g_sFileObject);

    //tmp_idx = 0;

    OSMutexPost(SDMutex);                               // 解锁
    DBG_DIS("|-----LIST FILE save: END------|\r\n");

	return TRUE;
}


 
#if FZ1500



/***********************************************************************************************
* Function	: DSPBOOT_LOAD_FROM_SD
* Description	:从SD卡升级DSP
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	宋超
***********************************************************************************************/
void DSPBOOT_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname[10]="/dsp";
    INT16U 	usBytesRead = 0;

    //打开根目录下文件“1MB”
    fresult = f_open(&g_sFileObject, fname, FA_READ);

    if(fresult != FR_OK)
    while(1);

    //从起始位置读文件“app”的sizeof(g_cTmpBuf)字节
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
* Description	:从SD卡升级DSP
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	宋超
***********************************************************************************************/
void FPGABOOT_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname[10]="/fpga";
    INT16U 	usBytesRead = 0;

    //打开根目录下文件“1MB”
    fresult = f_open(&g_sFileObject, fname, FA_READ);

    if(fresult != FR_OK)
    while(1);

    //从起始位置读文件“app”的sizeof(g_cTmpBuf)字节
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
* Description	:从SD卡升级DSP
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	宋超
***********************************************************************************************/
void CNYBOOT_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname[10]="/cny";
    INT16U 	usBytesRead = 0;

    //打开根目录下文件“1MB”
    fresult = f_open(&g_sFileObject, fname, FA_READ);

    if(fresult != FR_OK)
    while(1);

    //从起始位置读文件“app”的sizeof(g_cTmpBuf)字节
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
* Description	:从SD卡升级MTARM
* Input		:
* Output	:
* Note(s)	:
* Contributor	: 15/1/2015	宋超
***********************************************************************************************/
void MTARM_LOAD_FROM_SD(void)
{
    static _BSP_MESSAGE pMsg;
    FRESULT fresult;
    const char fname_app[10]="/mt";
    INT16U 	usBytesRead = 0;

    //打开根目录下文件“1MB”
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


/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
