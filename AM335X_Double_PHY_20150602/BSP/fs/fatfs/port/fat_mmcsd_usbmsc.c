/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: fat_mmcsd_usbmsc.c
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: FatFS要同时支持SD卡和USBMSC。这里需要将磁盘管理驱动结合起来，而USB又分2
                      路，虽然实际使用中是1路但由于USBLIB支持了2路需要在细节把控上注意。
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		: 20150305 - 修改get_fattime函数，使文件属性中正确显示文件修改时间 by code
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "diskio.h"
#include "hw_types.h"
#include "usblib.h"
#include "usbmsc.h"
#include "usbhost.h"
#include "usbhmsc.h"
#include "ff.h"
#include "mmcsd_proto.h"
#include "hs_mmcsdlib.h"
#include "Net_server.h"
/* Private define-----------------------------------------------------------------------------*/
#define DRIVE_NUM_MMCSD     0
#define DRIVE_NUM_MAX       2
//#define DISC_MMCSD      0
//#define DISC_USB0       1
//#define DISC_USB1       2
/* Private typedef----------------------------------------------------------------------------*/
typedef enum
{
    DISC_MMCSD = 0,
    DISC_USB0,
    DISC_USB1
}_DISC_TYPE;
typedef struct _fatDevice
{
    /* Pointer to underlying device/controller */
    void *dev;
    /* File system pointer */
    FATFS *fs;
	/* state */
	unsigned int initDone;

}fatDevice;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
extern tUSBHMSCInstance g_USBHMSCDevice[];//实际有2个元素，USB0  USB1
static volatile DSTATUS USBStat = STA_NOINIT;    /* Disk status */
fatDevice fat_devices[DRIVE_NUM_MAX];
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: disk_initialize
* Description	: disk初始化，这里定义MMCSD:0;USB0:1;USB1:2
* Input			: BYTE bValue 对应：MMCSD:0;USB0:1;USB1:2
* Return		: 
* Note(s)		:
* Contributor	: 130722   wangyao
***********************************************************************************************/
DSTATUS disk_initialize(BYTE bValue)          
{ 
    switch((_DISC_TYPE)bValue)
    {
        case DISC_MMCSD:
        {
            unsigned int status;
            
            if (DRIVE_NUM_MAX <= bValue)
            {
                return STA_NODISK;
            }
            if ((DRIVE_NUM_MMCSD == bValue) && (fat_devices[bValue].initDone != 1))
            {
                mmcsdCardInfo *card = (mmcsdCardInfo *) fat_devices[bValue].dev;
                /* SD Card init */
                status = MMCSDCardInit(card->ctrl);
                if (status == 0)
                {
                    //UARTPuts("\r\nCard Init Failed \r\n", -1);
                    return STA_NOINIT;
                }
                else
                {     
                    /* Set bus width */
                    if (card->cardType == MMCSD_CARD_SD)
                    {
                        MMCSDBusWidthSet(card->ctrl);
                    }
                    /* Transfer speed */
                    MMCSDTranSpeedSet(card->ctrl);
                }
                fat_devices[bValue].initDone = 1;
            }
            return 0;           
        }
        case DISC_USB0:
        {
            unsigned int ulMSCInstance;
                           
            ulMSCInstance = (unsigned int)&g_USBHMSCDevice[0];
            /* Set the not initialized flag again. If all goes well and the disk is */
            /* present, this will be cleared at the end of the function.            */
            USBStat |= STA_NOINIT;
            /* Find out if drive is ready yet. */
            if (USBHMSCDriveReady(ulMSCInstance)) 
                return(FR_NOT_READY);
            /* Clear the not init flag. */
            USBStat &= ~STA_NOINIT;
            return 0;            
        }
        default:
            break;
    }
    return 0;
}



/***********************************************************************************************
* Function		: disk_status
* Description	: disk状态
* Input			: 
* Return		: 
* Note(s)		
* Contributor	: 130722   wangyao
***********************************************************************************************/
DSTATUS disk_status ( BYTE drv)               
{
    switch((_DISC_TYPE)drv)
    {
        case DISC_MMCSD:
            return 0;
        case DISC_USB0:
            return USBStat;
        default:
            break;
    }
    return 0;
}


/***********************************************************************************************
* Function		: disk_read
* Description	: 磁盘读操作
* Input			: 
* Return		: 
* Note(s)		
* Contributor	: 130722   wangyao
***********************************************************************************************/
DRESULT disk_read (
    BYTE drv,               /* Physical drive number (0) */
    BYTE* buff,             /* Pointer to the data buffer to store read data */
    DWORD sector,           /* Physical drive nmuber (0) */
    BYTE count)             /* Sector count (1..255) */
{
	switch((_DISC_TYPE)drv)
    {
        case DISC_MMCSD:
            if (drv == DRIVE_NUM_MMCSD)
            {
                mmcsdCardInfo *card = fat_devices[drv].dev;
                /* READ BLOCK */
                if (MMCSDReadCmdSend(card->ctrl, buff, sector, count) == 1)
                {
                    return RES_OK;
                }
            }
            return RES_ERROR;
        case DISC_USB0:
        {
            unsigned int ulMSCInstance;
            ulMSCInstance = (unsigned int)&g_USBHMSCDevice[0];//g_USBHMSCDevice[drv];
            if(USBStat & STA_NOINIT)
            {
                return(RES_NOTRDY);
            }
            /* READ BLOCK */
            if (USBHMSCBlockRead(ulMSCInstance, sector, buff, count) == 0)
                return RES_OK;
            return RES_ERROR;
        }
        default:
            break;
    }
    return RES_ERROR;  
}

/***********************************************************************************************
* Function		: disk_write
* Description	: 磁盘写操作
* Input			: 
* Return		: 
* Note(s)		
* Contributor	: 130722   wangyao
***********************************************************************************************/
#if _READONLY == 0
DRESULT disk_write (
    BYTE ucDrive,           /* Physical drive number (0) */
    const BYTE* buff,       /* Pointer to the data to be written */
    DWORD sector,           /* Start sector number (LBA) */
    BYTE count)             /* Sector count (1..255) */
{
    switch((_DISC_TYPE)ucDrive)
    {
        case DISC_MMCSD:
        {
            if (ucDrive == DRIVE_NUM_MMCSD)
            {
                mmcsdCardInfo *card = fat_devices[ucDrive].dev;
                /* WRITE BLOCK */
                if(MMCSDWriteCmdSend(card->ctrl,(BYTE*) buff, sector, count) == 1)
                {
                    return RES_OK;
                }
            }
            return RES_ERROR;
        }
        case DISC_USB0:
        {
            unsigned int ulMSCInstance;
            ulMSCInstance = (unsigned int)&g_USBHMSCDevice[0];//g_USBHMSCDevice[ucDrive];           
            if (!ucDrive || !count) return RES_PARERR;
            if (USBStat & STA_NOINIT) return RES_NOTRDY;
            if (USBStat & STA_PROTECT) return RES_WRPRT;
            /* WRITE BLOCK */
            if(USBHMSCBlockWrite(ulMSCInstance, sector, (unsigned char *)buff,count) == 0)
                return RES_OK;
            return RES_ERROR;   
        }
        default:
            break;
    }
    return	RES_ERROR;
}
#endif /* _READONLY */

/***********************************************************************************************
* Function		: disk_ioctl
* Description	:
* Input			: 
* Return		: 
* Note(s)		:
* Contributor	: 130722   wangyao
***********************************************************************************************/
DRESULT disk_ioctl (
    BYTE drv,               /* Physical drive number (0) */
    BYTE ctrl,              /* Control code */
    void *buff)             /* Buffer to send/receive control data */
{
    switch((_DISC_TYPE)drv)
    {
        case DISC_MMCSD:
            return RES_OK;
        case DISC_USB0:
        {
            if(USBStat & STA_NOINIT)
            {
                return(RES_NOTRDY);
            }
            switch(ctrl)
            {
                case CTRL_SYNC:
                    return(RES_OK);
                default:
                    return(RES_PARERR);
            }    
        }
        default:
            break;
    }
    return RES_ERROR; 
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */
//獲取时间函数，如果系统设计了RTC,最好直接从RTC获取。
DWORD get_fattime (void)
{
    char curr_time[15];
    DWORD curr_year,curr_month,curr_day,curr_hour,curr_min,curr_sec;
    
    if(get_os_sys_date(curr_time))   
    {
        // 时间缺省值
        curr_year = 2007;
        curr_month = 6;
        curr_day = 5;
        curr_hour = 11;
        curr_min = 38;
        curr_sec = 0;
    }
    else
    {
        // 真实时间
        curr_year = (curr_time[0]-0x30)*1000 + (curr_time[1]-0x30)*1000 + (curr_time[2]-0x30)*10 + (curr_time[3]-0x30);  
        curr_month = (curr_time[4]-0x30)*10 + (curr_time[5]-0x30);
        curr_day = (curr_time[6]-0x30)*10 + (curr_time[7]-0x30);
        curr_hour = (curr_time[8]-0x30)*10 + (curr_time[9]-0x30);
        curr_min = (curr_time[10]-0x30)*10 + (curr_time[11]-0x30);
        curr_sec = (curr_time[12]-0x30)*10 + (curr_time[13]-0x30);
    }

    return    ((curr_year-1980) << 25)      // Year 
                | (curr_month << 21)        // Month 
                | (curr_day << 16)          // Day 
                | (curr_hour << 11)         // Hour 
                | (curr_min << 5)           // Min 
                | (curr_sec >> 1);          // Sec 
}


/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/