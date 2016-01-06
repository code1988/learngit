/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName		: app.h
* Author		: ��ҫ
* Date First Issued	: 2013-07-22
* Version		: V
* Description		: Ӧ�ñ�̹��õ�һЩ����
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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

//DSP��MT ARMͨ�ŵ�Э��
#define ID_DSP_BOOT_INIT 					0x000B		    // DSP��boot��ʼ�������Ŀǰ�������������ARM��
#define ID_DSP_BOOT_READ_END				0x0001			//DSP��BOOT����
#define ID_DSP_DSP_LEVEL_UP					0x3101			// ������������
#define ID_DSP_UPDATE_STATUS                0x3102          //DSP�������
#define ID_DSP_UPDATE_A8                    0x3104          //����A8�Լ��ĳ���

//������Ϣ
#define	ID_DSP_DSP_LEVEL_UP_FAIL			0x0001		   // DSP����ʧ��
#define	ID_DSP_FPGA_LEVEL_UP_FAIL			0x0002		   // FPGA����ʧ��
#define	ID_DSP_MODE_LEVEL_UP_FAIL			0x0003		   // �����ģ������ʧ��
#define	ID_DSP_DSP_LEVEL_UP_SUCCESS			0x0101		   // DSP�����ɹ�
#define	ID_DSP_FPGA_LEVEL_UP_SUCCESS		0x0102		   // FPGA�����ɹ�
#define	ID_DSP_MODE_LEVEL_UP_SUCCESS		0x0103		   // �����ģ�������ɹ�
#define	ID_DSP_DSP_LEVEL_UP_DISABLE			0x0201		   // DSP��֧�ֵ�������־
#define	ID_DSP_FPGA_LEVEL_UP_DISABLE		0x0202		   // FPGA��֧�ֵ�������־
#define	ID_DSP_MODE_LEVEL_UP_DISABLE		0x0203		   // �����ģ�岻֧�ֵ�������־

//ver id
#define ID_DSP_VER                          0x3201          //app�汾
#define ID_DSP_VER_GET 						0x0007			//DSP�汾ȡ��	
//check cs
#define ID_DSP_CHECK_CS_ACK                 0x3701          //CSУ��Ӧ��
#define ID_DSP_CHECK_CS                     0x3702          //CSУ��
//net para
#define ID_DSP_NET_PARA                     0x3301          //�������
//net send
#define ID_DSP_NET_SEND_PROTOCOL            0x3401          //����Э��,0-����Э��,1-����Э��,2-ά��Э��,3-����Э��
#define ID_DSP_NET_SEND_SWITCH              0x3402          //��������,0-������,1-������
#define ID_DSP_NET_SEND_MASK_TIME           0x3403          //������ʱ��
//img id
#define ID_DSP_CNT_MONEY_START              0x3501          //�㳮��ʼʱ��
#define ID_DSP_CNT_MONEY_END                0x3502          //�㳮����
#define ID_DSP_IMG                          0x3503          //ʵʱ��ͼ
#define ID_DSP_BASIC_INFO                   0x3504          //����������Ϣ
#define ID_DSP_SYNC_TIME                    0x3505          //ͬ���r�g

#define ID_DSP_BIG_IMG_START                0x3506          //��ͼ��ʼ
#define ID_DSP_BIG_IMG_DATA                 0x3507          //��ͼ����
#define ID_DSP_BIG_IMG_END                  0x3508          //��ͼ����

//sd id
#define ID_DSP_SD_RES_ALARM_THR             0x3601          //SD��ʣ�������澯ֵ,�ٷֱ�(1-99)
#define ID_DSP_SD_RES_ALARM_UP              0x3602          //SD��ʣ�������澯״̬
#define ID_DSP_SD_RES_SIZE                  0x3603          //SD������,��λΪ M��
#define ID_DSP_SD_DEL_DATA                  0x3604          //ɾ��SD����
#define ID_DSP_SD_OPT_PARA                  0x3605          //SD����������

//CSУ��
#define ID_DSP_CS_CHECK		                0x3701        //����CSУ������
#define ID_DSP_CS_CHECK_START				0x0004		  //����CSУ������
//UIA���͸�CAN�����ݸ���Э���ID��
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
extern OS_EVENT *LCDDisplay_prevent;		//����Һ����ʾ����
extern OS_EVENT *DispTaskEvent;                 //��������ʾ����
#if FZ1500
extern OS_EVENT *canEvent;                      //����ͨ������
#else
extern OS_EVENT *ComWithMCTaskEvent;        //����ͨ������
#endif
extern OS_EVENT *FlashEvent;                    //��Flash���в���

/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


#endif	//__APP_H_
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
