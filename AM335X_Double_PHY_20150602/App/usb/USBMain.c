/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: USBMain.c
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "usbmain.h"
//结合文件系统
#include "ff.h"
#define USB_FF_ENABLE
/* Private define-----------------------------------------------------------------------------*/
#define FatFS_Drive_Index   1
#define USB_FILEPATH_SIZE       80       //文件路径缓冲大小
#define USB_MSC_MAX_WRITE_SIZE (16*1024) // limitation of FATFS
//USB SPEED test
#define USBBUFF_LENGTH     32768u
#define WRITEWITHUSB 1
#define WRITEWITHUSBINSERIAL 1
#define READWITHUSB 1
#define CREATFILEUSB 1
#define UNLINKFILEUSB 1

INT16U Count = 0;
INT32U circle = 0;
FRESULT	fresult;
INT8U TargetRxBuff[4194304] = {0};
INT16U	usBytesRead,usBytesWrite = 0;
INT8U SDRxBuff[USBBUFF_LENGTH] = {0};
INT32U USBtime = 0;
INT32U showtime = 0;
INT32U i = 0;
INT8U USBRxBuff[USBBUFF_LENGTH] = {0};
INT8U num = 0;
INT8U usbwriteflag = 1;
extern volatile INT32U USBTicks;
INT16U Writrcount;
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
//FAT fs state
static FATFS MSCFatFs;
static DIR MSCDirObject;
static FILINFO MSCFileInfo;
static FIL MSCFileObject;
static FIL MSCFileObjectWrite;
/* Private variables--------------------------------------------------------------------------*/
OS_EVENT *USBEvent;
//fat_mmcsd_usbmsc限定了这里必须要这样初始化，这里的1表示USB0,否则文件系统无法同时支持SD,USB
INT8U USBFilePathBuff[USB_FILEPATH_SIZE]="1:/fota";
INT8U USBFileBuf[USB_MSC_MAX_WRITE_SIZE] = "/";

INT8U USBTestFileBuf[USB_MSC_MAX_WRITE_SIZE] = "1:/test.txt";

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : ReadFilesInfoinMSC
* Description	: 读取U盘里的6个文件，发送给液晶显示信息这是一个测试程序
* Input		    : 
* Return	    : 
* Note(s)	    : 
* Contributor	: 130722   wangyao
***********************************************************************************************/
void ReadFilesInfoinMSC(void)
{
    unsigned long ulTotalSize;
    unsigned long ulFileCount;
    unsigned long ulDirCount;
 
    FRESULT fresult;
    static _BSP_MESSAGE send_message;
    
    fresult = f_opendir(&MSCDirObject, (const char *)USBFilePathBuff);
   //fresult = f_open(&g_sFileObject, fname, FA_READ);
    /*
    ** If there was some problem opening the file, then return an error.
    */
    if(fresult != FR_OK)
    {
        send_message.MsgID = (_BSP_MSGID)MSC_FILE_OPEN_FAILURE;
        OSQPost (DispTaskEvent,&send_message); //给液晶显示
        OSTimeDly(SYS_DELAY_5s); //过5s后读文件
    }
    while(1)
    {

            fresult = f_readdir(&MSCDirObject, &MSCFileInfo);
            if(fresult != FR_OK)
            {
                send_message.MsgID = (_BSP_MSGID)MSC_FILE_READ_FAILURE;
                OSQPost (DispTaskEvent,&send_message); //给液晶显示
                OSTimeDly(SYS_DELAY_500ms); //过5s后读文件
                break;
            }
            if(!MSCFileInfo.fname[0])
            {
                break;
            }
            if(MSCFileInfo.fattrib & AM_DIR)
            {
                ulDirCount++;
            }
            else
            {
                ulFileCount++;
                ulTotalSize += MSCFileInfo.fsize;
            }
            send_message.MsgID = (_BSP_MSGID)MSC_FILE_READ_SUCESS;//读成功，把文件大小信息传递过去
            send_message.DataLen = sizeof(MSCFileInfo);
            send_message.pData = (INT8U *)&MSCFileInfo;
            OSQPost (DispTaskEvent,&send_message); //给液晶显示
            OSTimeDly(SYS_DELAY_500ms); //确保发送完毕，必须有 
            if(ulFileCount == 6)
                break;
                
    }   
}
char const   *fname = "1:/fota/3.txt";  
char const   *testfname = "1:/TestInfo.txt"; 
/***********************************************************************************************
* Function	    : TaskUSB
* Description	: USB任务
* Input		    : 
* Return	    : 
* Note(s)	    : 
* Contributor	: 130722   wangyao
***********************************************************************************************/
void TaskUSB(void *pdata)
{
    INT32U  NandAddr = 0;
    INT8U err,ready = 0;
    _BSP_MESSAGE  *pSM;    
    _BSPUSB_CONFIG usbconfig;
    
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
    
    if(BSP_NandConfig(&Nand_InitStructure) != NAND_STATUS_PASSED)
        while(1);
    
    USBEvent = OSMboxCreate(0);//创建USB测试邮箱	
	usbconfig.pEvent = USBEvent;
    usbconfig.Mode = BSPUSB_WORKMODE_HOST;
    BSP_USBInit(0,&usbconfig);
    OSTimeDlyHMSM(0,0,0,50);
    //文件系统初始化
    f_mount(FatFS_Drive_Index, &MSCFatFs);
    InitialiseUSBRxBuff();
    OSTimeDlyHMSM(0,0,1,0);
    while(1)
    {		
        pSM=OSMboxPend(USBEvent,SYS_DELAY_500ms,&err);	
        if(OS_ERR_NONE == err)
        {
            switch(pSM->MsgID)// 判断消息类型(只能是_BSP_MSGID中定义的类型)
            {
                case STATE_NO_DEVICE:      //无设备
                    break;
                case STATE_DEVICE_READY:   //MSC设备准备就绪
                    ready = 1;          
                    break;
                case STATE_UNKNOWN_DEVICE: //无法识别的设备接入
                    break;
                case STATE_POWER_FAULT:    //电源出错
                    break;
                case STATE_BABBLE_INT:     //异常中断发生
                    break;
                case STATE_TIMEDOUT:       //设备超时
                    break;
                case STATE_PULLOUT:        //设备拔除
                    break;
                case STATE_UNKNOWN_PULLOUT: //不认识的设备拔除  
                    break;
                default:
                    break;
                //...其他消息
            }
        }
        if(ready&&usbwriteflag)
        {
         usbwriteflag = 0;
         USBtime = 0;
         BOOT_LOAD_FROM_USB(Nand_InitStructure,NandAddr,usBytesRead);
#ifdef USBTEST
         f_unlink(testfname);
#ifdef WRITEWITHUSB
         fresult = f_mkdir((const char *)USBFilePathBuff);
         f_unlink(fname);              
         for(Writrcount = 0 ; Writrcount<1024;Writrcount++)
         {
            WriteWithUSB();//不停地打开关闭文件指针方式写数据
         }
         WriteTestInfo(1);
         //OSTimeDlyHMSM(0,0,1,0);
#endif // WRITEWITHUSB  
          
#ifdef WRITEWITHUSBINSERIAL
         USBtime = 0;
         f_unlink(fname);
         fresult = f_open(&MSCFileObject, fname, FA_CREATE_NEW|FA_WRITE|FA_READ);

         if(fresult != FR_OK)
         {
             fresult = f_open(&MSCFileObject, fname, FA_WRITE|FA_READ);
             if(fresult != FR_OK)
                 while(1);   
         }
         
         for(Writrcount = 0 ; Writrcount<1024;Writrcount++)
         {
            WriteWithUSBInSerial();//打开一次以后一直写入文件
         }
           fresult = f_close(&MSCFileObject);
          if(fresult != FR_OK)
          while(1);
    
         WriteTestInfo(2);
         //OSTimeDlyHMSM(0,0,1,0); 
#endif // WRITEWITHUSBINSERIAL   
         
#ifdef READWITHUSB
         USBtime = 0;
         for(Writrcount = 0 ; Writrcount<1024;Writrcount++)
         {
            ReadWithUSB();//读取数据
         }  
         WriteTestInfo(3);
         //OSTimeDlyHMSM(0,0,1,0); 
#endif // WRITEWITHUSBINSERIAL
         
#ifdef CREATFILEUSB
         USBtime = 0;
         CreatfileUSB();//创建1000个文件
         WriteTestInfo(4);
         OSTimeDlyHMSM(0,0,1,0);     
#endif // CREATFILEUSB
         
#ifdef UNLINKFILEUSB
         USBtime = 0;
         unlinkfileUSB();//删除1000个文件
         WriteTestInfo(5);
        // OSTimeDlyHMSM(0,0,1,0);     
#endif // UNLINKFILEUSB
#endif //USBTEST         
          OSTimeDlyHMSM(0,0,1,0); 
        }
  }
}

/***********************************************************************************************
* Function		: WriteWithUSB
* Description   	: 连续往U盘中写入数据测试
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	        : 09/10/2014	songchao
***********************************************************************************************/
void WriteWithUSB()
{          
    fresult = f_open(&MSCFileObject, fname, FA_CREATE_NEW|FA_WRITE|FA_READ);

    if(fresult != FR_OK)
    {
        fresult = f_open(&MSCFileObject, fname, FA_WRITE|FA_READ);
        if(fresult != FR_OK)
            while(1);   
    }
         
    f_lseek(&MSCFileObject, MSCFileObject.fsize);
    if(fresult != FR_OK)
        while(1);
            
    fresult = f_write(&MSCFileObject,USBRxBuff,USBBUFF_LENGTH,&usBytesWrite);
    if(fresult != FR_OK)
        while(1); 
    
    //如果发现实际存储的大小小于存储的大小，则删除原文件，并重新建立一个文件
    if(usBytesWrite < USBBUFF_LENGTH)
    {
        f_unlink(fname);
        fresult = f_open(&g_sFileObject, fname, FA_CREATE_NEW);
        if(fresult != FR_OK)
        while(1);  
        
        fresult = f_write(&MSCFileObject,USBRxBuff,USBBUFF_LENGTH,&usBytesWrite);
        if(fresult != FR_OK)
            while(1); 
    }
            
    fresult = f_close(&MSCFileObject);
    if(fresult != FR_OK)
        while(1);

    USBtime += USBTicks;
    USBTicks = 0;
    
}

/***********************************************************************************************
* Function		: WriteTestInfo
* Description   	: 连续往U盘中写入数据测试
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	        : 09/10/2014	songchao
***********************************************************************************************/
void WriteTestInfo(INT8U Test)
{     
    INT64U BlockSize = USBBUFF_LENGTH;
    INT8U TestBlock[50] = "TestBlock size is";
    INT8U WriteWithUSB[50] = "WriteWithUSB time is ";
    INT8U WriteWithUSBInSerial[50] = "WriteWithUSBInSerial time is ";
    INT8U ReadWithUSB[50] = "ReadWithUSB time is ";  
    INT8U CreatfileUSB[50] = "1000 files creat time is ";  
    INT8U unlinkfileUSB[50] = "1000 files unlink time is ";
    INT8U TimeBuff[10] = "";
    INT8U BlankBuff[50] = "\n";
    INT8U Block4kBuff[5] = "4k";
    INT8U Block8kBuff[5] = "8k";
    INT8U Block16kBuff[5] = "16k";
    INT8U Block32kBuff[5] = "32k";
    INT8U Block64kBuff[5] = "64k";
     
    fresult = f_open(&MSCFileObject, testfname, FA_CREATE_NEW|FA_WRITE|FA_READ);

    if(fresult != FR_OK)
    {
        fresult = f_open(&MSCFileObject, testfname, FA_WRITE|FA_READ);
        if(fresult != FR_OK)
            while(1);   
    }
         
    f_lseek(&MSCFileObject, MSCFileObject.fsize);
    if(fresult != FR_OK)
        while(1);
    
    fresult = f_write(&MSCFileObject,TestBlock,50,&usBytesWrite);
    if(fresult != FR_OK)
        while(1); 
    
    switch(USBBUFF_LENGTH)
    {
          case 4096:
              fresult = f_write(&MSCFileObject,Block4kBuff,5,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1); 
              break;
          case 8192:
              fresult = f_write(&MSCFileObject,Block8kBuff,5,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1); 
              break;
          case 16384:
              fresult = f_write(&MSCFileObject,Block16kBuff,5,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1); 
              break;
          case 32768:
              fresult = f_write(&MSCFileObject,Block32kBuff,5,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1); 
              break;
          case 65536:
              fresult = f_write(&MSCFileObject,Block64kBuff,5,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1); 
              break;
          default:
              break;
    }
    
    fresult = f_write(&MSCFileObject,BlankBuff,50,&usBytesWrite);
    if(fresult != FR_OK)
        while(1);
    
    ChangeINTtoChar(TimeBuff);
              
    switch(Test)
    {
          case 0x01:
              fresult = f_write(&MSCFileObject,WriteWithUSB,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,TimeBuff,10,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,BlankBuff,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              break;
          case 0x02:
              fresult = f_write(&MSCFileObject,WriteWithUSBInSerial,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,TimeBuff,10,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,BlankBuff,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              break;
          case 0x03:
              fresult = f_write(&MSCFileObject,ReadWithUSB,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,TimeBuff,10,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,BlankBuff,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              break;
          case 0x04:
              fresult = f_write(&MSCFileObject,CreatfileUSB,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,TimeBuff,10,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,BlankBuff,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              break;
          case 0x05:
              fresult = f_write(&MSCFileObject,unlinkfileUSB,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,TimeBuff,10,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              fresult = f_write(&MSCFileObject,BlankBuff,50,&usBytesWrite);
              if(fresult != FR_OK)
                  while(1);
              break;
    }

    fresult = f_close(&MSCFileObject);
    if(fresult != FR_OK)
        while(1);
    
}

/***********************************************************************************************
* Function		: WriteWithUSB
* Description   	: 连续往U盘中写入数据测试
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	        : 09/10/2014	songchao
***********************************************************************************************/
void WriteWithUSBInSerial()
{          
    f_lseek(&MSCFileObject, (USBBUFF_LENGTH*Writrcount));
    if(fresult != FR_OK)
        while(1);
            
    fresult = f_write(&MSCFileObject,USBRxBuff,USBBUFF_LENGTH,&usBytesWrite);
    if(fresult != FR_OK)
        while(1); 
         
    fresult = f_sync(&MSCFileObject);
    if(fresult != FR_OK)
        while(1);
    
    USBtime += USBTicks;
    USBTicks = 0;
    
}

/***********************************************************************************************
* Function		: ReadWithUSB
* Description	        : 往U盘中写入数据
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	: 09/10/2014	songchao
***********************************************************************************************/
void ReadWithUSB()
{    
    USBTicks = 0;
    
    fresult = f_open(&MSCFileObject, fname, FA_READ);                   
    if(fresult != FR_OK)
        while(1);    
    
    fresult = f_lseek(&MSCFileObject, USBBUFF_LENGTH*Writrcount); 
    if(fresult != FR_OK)
        while(1);
    
    //读DISPBUFF_LENGTH字节
    fresult = f_read(&MSCFileObject,TargetRxBuff,USBBUFF_LENGTH,&usBytesRead);
    if(fresult != FR_OK)
        while(1);
    
    fresult = f_close(&MSCFileObject);
    if(fresult != FR_OK)
        while(1);
    
    USBtime += USBTicks;
    USBTicks = 0;
       
}

/***********************************************************************************************
* Function		: InitialiseUSBRxBuff
* Description	        : 初始化U盘存储的信息
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	        : 09/10/2014	songchao
***********************************************************************************************/
void InitialiseUSBRxBuff()
{
    while(i < USBBUFF_LENGTH)
    {
        USBRxBuff[i] = num;
        if(num < 10)
        {
           num++;
        }
        else
        {
           num = 0;
        }
        i++;
    }
    
    for(i =0;i<200;i++)
    {
        if(TargetRxBuff[i] == 0xFF)TargetRxBuff[i]=0;
        TargetRxBuff[i]++;
    }
}

/***********************************************************************************************
* Function		: CreatfileUSB
* Description	        : 测试生成文件的速度
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	        : 09/10/2014	songchao
***********************************************************************************************/
void CreatfileUSB(void)
{
    char creatfname[20] = "1:/1000.dat"; 
    USBTicks = 0;  
    for(Count = 0;Count<1000;Count++)
    {
        USBTicks = 0;
        creatfname[4] = Count/100+48;
        creatfname[5] = (Count-Count/100*100)/10+48;
        creatfname[6] = Count%10+48;
        fresult = f_open(&MSCFileObject, creatfname, FA_CREATE_ALWAYS);
        
        if(fresult != FR_OK)
        {
            fresult = f_open(&MSCFileObject, creatfname, FA_WRITE);
            if(fresult != FR_OK)
              while(1);   
        }
        
        fresult = f_close(&MSCFileObject);

        USBtime += USBTicks;
    }
}

/***********************************************************************************************
* Function		: unlinkfileUSB
* Description	        : 测试删除的速度
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	        : 09/10/2014	songchao
***********************************************************************************************/
void unlinkfileUSB(void)
{
    char unlinkfname[20] = "1:/1000.dat"; 
    USBTicks = 0;
    
    for(Count = 0;Count<1000;Count++)
    {
        USBTicks = 0;
        unlinkfname[4] = Count/100+48;
        unlinkfname[5] = (Count-Count/100*100)/10+48;
        unlinkfname[6] = Count%10+48;

        f_unlink(unlinkfname);
        USBtime += USBTicks;
    }
}

/***********************************************************************************************
* Function		: ChangeINTtoChar
* Description	        : 把时间转化为Char格式输出
* Input			:  
* Output		: 
* Note(s)		: 
* Contributor	        : 09/10/2014	songchao
***********************************************************************************************/
void ChangeINTtoChar(INT8U *Ptr)
{
    INT32U totalnum = 0;
    totalnum = USBtime;
    if(totalnum/100000000)
    {
        Ptr[8] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[7] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[6] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';    
        Ptr[9] = 0x00;
    }
    else if(totalnum/10000000)
    {
        Ptr[7] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[6] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';    
        Ptr[8] = 0x00;
    }
    else if(totalnum/1000000)
    {
        Ptr[6] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';    
        Ptr[7] = 0x00;
    }
    else if(totalnum/100000)
    {
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';    
        Ptr[6] = 0x00;
    }
    else if(totalnum/10000)
    {
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';    
        Ptr[5] = 0x00;
    }
    else if(totalnum/1000)
    {
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';    
        Ptr[4] = 0x00;
    }
    else if(totalnum/100)
    {
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';    
        Ptr[3] = 0x00;
    }
    else if(totalnum/10)
    {
        Ptr[0] = totalnum/10 + '0';
        totalnum = totalnum%10;
        Ptr[1] = totalnum + '0';
        Ptr[2] = 0x00;
    }
    else
    {
        Ptr[0] = totalnum + '0';
        Ptr[1] = 0x00;
    }
}

/***********************************************************************************************
* Function	: BOOT_LOAD_FROM_USB
* Description	:从SD卡下载程序到NAND
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 11/12/2014	宋超
***********************************************************************************************/
void BOOT_LOAD_FROM_USB(NandInfo_t Nand_InitStructure,INT32U NandAddr,INT16U usBytesRead)
{
    char const	*fname_app = "1:/app";
    char const	*fname_boot = "1:/boot.bin";
    FRESULT 	fresult;
    
    BSP_UARTWrite(UART2,"pending boot and app images program...\n",39);

    //--------------------------------------------------------------------------------------
	//打开根目录下文件“app”

    fresult = f_open(&MSCFileObject, fname_app, FA_READ);
    if(fresult != FR_OK)
        while(1);   
    
    BSP_UARTWrite(UART2,"begin to load boot and app...\n",30);

    NandAddr = APP_LOAD_ADDR;
    do
    {	
            memset(g_cTmpBuf,0,NAND_PAGESIZE_2048BYTES);		
            if (f_read(&MSCFileObject, g_cTmpBuf, NAND_PAGESIZE_2048BYTES,&usBytesRead) != FR_OK)
                    while(1);
            if(BSP_NandWrite(&Nand_InitStructure,NandAddr,g_cTmpBuf,usBytesRead) != E_PASS)
                    while(1);
    
    memset(g_cTmpBuf,0,NAND_PAGESIZE_2048BYTES);
    if(BSP_NandRead(&Nand_InitStructure,NandAddr,g_cTmpBuf,usBytesRead) != E_PASS)
        while(1);
            NandAddr += usBytesRead;
    }while(MSCFileObject.fptr < MSCFileObject.fsize);
                    
    f_close(&g_sFileObject);
    //--------------------------------------------------------------------------------------------
    //打开根目录下文件“boot.bin”
    fresult = f_open(&MSCFileObject, fname_boot, FA_READ);    
    if(fresult != FR_OK)
    while(1);

    NandAddr = BOOT_LOAD_ADDR;

    do
    {	
            memset(g_cTmpBuf,0,NAND_PAGESIZE_2048BYTES);		
            if (f_read(&MSCFileObject, g_cTmpBuf, NAND_PAGESIZE_2048BYTES,&usBytesRead) != FR_OK)
                    while(1);
            if(BSP_NandWrite(&Nand_InitStructure,NandAddr,g_cTmpBuf,usBytesRead) != E_PASS)
                    while(1);
    
            memset(g_cTmpBuf,0,NAND_PAGESIZE_2048BYTES);
    if(BSP_NandRead(&Nand_InitStructure,NandAddr,g_cTmpBuf,usBytesRead) != E_PASS)
        while(1);
            NandAddr += usBytesRead;
    }while(MSCFileObject.fptr < MSCFileObject.fsize);

    f_close(&MSCFileObject);
    
    BSP_UARTWrite(UART2,"loader successfully!\n",21);
}

/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/