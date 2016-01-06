/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		: app.h
* Author		: 王耀
* Date First Issued	: 2013-07-22
* Version		: V
* Description		: 应用编程共用的一些定义
*----------------------------------------历史版本信息-------------------------------------------
* History		:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__APP_H_
#define	__APP_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "dsp_comm.h"
#include "mmcsdmain.h"
#include "DwinPotocol.h"

/* Private define-----------------------------------------------------------------------------*/
#define MMCSD_FILE_OPEN_FAILURE     0xA1
#define MMCSD_FILE_OPEN_SUCESS      0xA2
#define MMCSD_FILE_READ_FAILURE     0xA3
#define MMCSD_FILE_READ_SUCESS      0xA4

#define USB_ENUM_SUCESSE            0xA5
#define MSC_FILE_OPEN_FAILURE       0xA6
#define MSC_FILE_OPEN_SUCESS        0xA7
#define MSC_FILE_READ_FAILURE       0xA8
#define MSC_FILE_READ_SUCESS        0xA9

#define APP_COMFROM_MC              0xAA
#define APP_COMFROM_UI              0xAB
#define APP_COMFROM_KEY             0xAF

#define APP_DISP_COMFROM_MC         0xAC
#define APP_DISP_COMFROM_UI         0xAD
#define APP_DISP_COMFROM_KEY        0xAE
#define APP_DISP_COMFROM_FLASH	    0xB1
#define APP_DISP_COMFROM_DSP	    0xB2

#define APP_MT_LEVEL_UP	            0xB3
#define APP_DISP_DSP_VER	        0xB4
#define APP_DISP_DSP_CHECK_CS       0xB5

//DSP和MT ARM通信的协议
#define ID_DSP_BOOT_INIT 					0x000B		    // DSP的boot初始化结果，目前该命令做在马达ARM里
#define ID_DSP_BOOT_READ_END				0x0001			//DSP读BOOT结束
#define ID_DSP_DSP_LEVEL_UP					0x3101			// 发送升级命令
#define ID_DSP_UPDATE_STATUS                0x3102          //DSP升级结果
#define ID_DSP_UPDATE_A8                    0x3104          //升级A8自己的程序

//升级信息
#define	ID_DSP_DSP_LEVEL_UP_FAIL			0x0001		   // DSP升级失败
#define	ID_DSP_FPGA_LEVEL_UP_FAIL			0x0002		   // FPGA升级失败
#define	ID_DSP_MODE_LEVEL_UP_FAIL			0x0003		   // 人民币模板升级失败
#define	ID_DSP_DSP_LEVEL_UP_SUCCESS			0x0101		   // DSP升级成功
#define	ID_DSP_FPGA_LEVEL_UP_SUCCESS		0x0102		   // FPGA升级成功
#define	ID_DSP_MODE_LEVEL_UP_SUCCESS		0x0103		   // 人民币模板升级成功
#define	ID_DSP_DSP_LEVEL_UP_DISABLE			0x0201		   // DSP不支持的升级标志
#define	ID_DSP_FPGA_LEVEL_UP_DISABLE		0x0202		   // FPGA不支持的升级标志
#define	ID_DSP_MODE_LEVEL_UP_DISABLE		0x0203		   // 人民币模板不支持的升级标志

//ver id
#define ID_DSP_VER                          0x3201          //app版本
#define ID_DSP_VER_GET 						0x0007			//DSP版本取得	
//check cs
#define ID_DSP_CHECK_CS_ACK                 0x3701          //CS校验应答
#define ID_DSP_CHECK_CS                     0x3702          //CS校验
//net para
#define ID_DSP_NET_PARA                     0x3301          //网络参数
//net send
#define ID_DSP_NET_SEND_PROTOCOL            0x3401          //网络协议,0-工行协议,1-光荣协议,2-维融协议,3-中信协议
#define ID_DSP_NET_SEND_SWITCH              0x3402          //网发开关,0-网发关,1-网发开
#define ID_DSP_NET_SEND_MASK_TIME           0x3403          //不网发时段
//img id
#define ID_DSP_CNT_MONEY_START              0x3501          //点钞起始时间
#define ID_DSP_CNT_MONEY_END                0x3502          //点钞结束
#define ID_DSP_IMG                          0x3503          //实时抠图
#define ID_DSP_BASIC_INFO                   0x3504          //开机基本信息
#define ID_DSP_SYNC_TIME                    0x3505          //同步時間

#define ID_DSP_BIG_IMG_START                0x3506          //大图开始
#define ID_DSP_BIG_IMG_DATA                 0x3507          //大图数据
#define ID_DSP_BIG_IMG_END                  0x3508          //大图结束

//sd id
#define ID_DSP_SD_RES_ALARM_THR             0x3601          //SD卡剩余容量告警值,百分比(1-99)
#define ID_DSP_SD_RES_ALARM_UP              0x3602          //SD卡剩余容量告警状态
#define ID_DSP_SD_RES_SIZE                  0x3603          //SD余容量,单位为 M，
#define ID_DSP_SD_DEL_DATA                  0x3604          //删除SD数据
#define ID_DSP_SD_OPT_PARA                  0x3605          //SD卡操作参数

//CS校验
#define ID_DSP_CS_CHECK		                0x3701        //发送CS校验命令
#define ID_DSP_CS_CHECK_START				0x0004		  //发送CS校验命令
//UIA发送给CAN的数据根据协议得ID号
#define UIACANID 0x10
#define UIACANMODE 0x01
#define UIACANBATCH 0x02
#define UIACANMOTORSET 0x03
#define UIACANAUTHENTICATION 0x04
#define UIACANAUTHENTICLEVEL1 0x05
#define UIACANAUTHENTICLEVEL2 0x06
#define UIACANCLEARLEVEL 0x07
#define UIACANMIXEDSEARCH 0x08
#define UIACANOUTLETSEARCH 0x09
#define UIACANINLERSSEARCH 0x0A
#define UIACANMAINMENU 0x0B
#define UIACANERRORCODE 0x0C
#define UIACANEDITION 0x0D
#define UIACANJCSPEED 0x0E
#define UIACANLEVELUPSTART 0xF0
#define UIACANLEVELUPREADY 0xF1
#define UIACANLEVELUPDATA 0xF2

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
extern OS_EVENT *LCDDisplay_prevent;		//定义液晶显示邮箱
extern OS_EVENT *DispTaskEvent;                 //迪文屏显示任务
#if FZ1500
extern OS_EVENT *canEvent;                      //马达板通信任务
#else
extern OS_EVENT *ComWithMCTaskEvent;        //马达板通信任务
#endif
extern OS_EVENT *FlashEvent;                    //对Flash进行操作

/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


#endif	//__APP_H_
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
