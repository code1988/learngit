/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: main.c
* Author			:
* Date First Issued	: 130722
* Version			: V
* Description		:
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2013	        : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "main.h"
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>
#include  <ucos_ii.h>
#include "bsp.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
//#include "beaglebone.h"
#include "gpio.h"
#include "gpio_v2.h"
//#include "netconf.h"//����
#include "app.h"
#include "httpserver-netconn.h"

#include "def_config.h"
#include "uart_arm.h"
#include "net_server.h"
#include "compile_time.h"
#include "dsp_net.h"
#include "arm_com.h"
#include "version.h"





/* Private define-----------------------------------------------------------------------------*/
//#define ETHERNET_APP_ENABLE

#define OS_INIT_TASK_STACK_SIZE		0x200//��ʼ�������ջ��С
OS_STK	InitTaskStk[OS_INIT_TASK_STACK_SIZE];	//��ʼ�������ջ

// ����ռ����
#define	TASK_STK_SIZE_START		0x400
#define	TASK_STK_SIZE_TEST		0x400
#define	TASK_STK_SIZE_LED		0x30
#define	TASK_STK_SIZE_WDG		0x100
#define TASK_STK_SIZE_MMCSD     0x400
#define TASK_STK_SIZE_FLASH      0x200
#define TASK_STK_SIZE_LCD       0x200
#define TASK_STK_SIZE_USB       0x200
#define TASK_STK_SIZE_COMWITHMC 0x200
#define TASK_STK_SIZE_DISPLAY   0x800
#define TASK_STK_SIZE_COMWITHDSP1 0x400
#define TASK_STK_SIZE_COMWITHDSP2 0x400
#define TASK_STK_SIZE_COMWITH2410 0x400
#define TASK_STK_SIZE_SD_MASTER     0x400
#define TASK_STK_SIZE_PHY_DSP       0x400  

#define TASK_STK_SIZE_NET_SERVER  0x4000
#define TASK_STK_SIZE_NET_SERVER_TIME 0x400
#define TASK_STK_SIZE_DSP_NET_SERVER_TIME 0x400
#define TASK_STK_SIZE_ARM_UART      0x400




#define GPIO_INSTANCE_ADDRESS           (SOC_GPIO_3_REGS)
#define GPIO_INSTANCE_PIN_NUMBER        (20)
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//used by os to store critical-region times
INT32U   OSInterrputSum;
//flag for window watch dog
volatile INT32U   WDG_clr_flag;

OS_STK TaskStartStk[TASK_STK_SIZE_START];		    // ����ϵͳ�������ջ
#if __DEBUG_TASK_TEST_ENABLE
OS_STK TaskTestStk[TASK_STK_SIZE_TEST];			    // ���������ջ
#endif	//__DEBUG_TASK_TEST_ENABLE
OS_STK TaskLEDStk[TASK_STK_SIZE_LED];			    // LED�����ջ
OS_STK TaskWDGStk[TASK_STK_SIZE_WDG];			    // WDG�����ջ
OS_STK TaskMMCSDStk[TASK_STK_SIZE_MMCSD];           // MMCSD�����ջ
OS_STK TaskFlashStk[TASK_STK_SIZE_FLASH];           //FLASH�����ջ
OS_STK TaskLCDStk[TASK_STK_SIZE_LCD];               // LCD�����ջ
OS_STK TaskUSBStk[TASK_STK_SIZE_USB];               // USB�����ջ
OS_STK TaskComWithMCStk[TASK_STK_SIZE_COMWITHMC];   // ����ͨ�������ջ
OS_STK TaskDisplayStk[TASK_STK_SIZE_DISPLAY];       // �����������ջ
OS_STK TaskComWithDSP1Stk[TASK_STK_SIZE_COMWITHDSP1];	// DSPͨѶ�����ջ
OS_STK TaskComWithDSP2Stk[TASK_STK_SIZE_COMWITHDSP2];	// DSPͨѶ�����ջ
OS_STK TaskComWith2410Stk[TASK_STK_SIZE_COMWITH2410];	// 2410ͨѶ�����ջ
OS_STK TaskSDMasterStk[TASK_STK_SIZE_SD_MASTER];       	// SD���洢�������ջ
OS_STK Task_PHY_DSPStk[TASK_STK_SIZE_PHY_DSP];       // A8_PHY_DSPͨѶ�����ջ



OS_STK task_net_server_stk[TASK_STK_SIZE_NET_SERVER];	// NET���������ջ
OS_STK task_net_server_stk_time[TASK_STK_SIZE_NET_SERVER_TIME];	// NET��ʱ���������ջ
OS_STK task_dsp_net_server_stk_time[TASK_STK_SIZE_DSP_NET_SERVER_TIME];	// NET��ʱ���������ջ
OS_STK task_arm_uart[TASK_STK_SIZE_ARM_UART];	// arm uart�����ջ







/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: TaskTest(void *pdata)
* Description	: ��������
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
#if __DEBUG_TASK_TEST_ENABLE
void TaskTest(void *pdata)
{
    DBG_DIS("in TaskTest 0:\r\n");


    // edma
    volatile unsigned int index = 0u;
    volatile unsigned int count = 0u;
    EDMA3CCPaRAMEntry paramSet;
    unsigned char data = 0u;
    volatile unsigned int retVal = 0u;
    unsigned int isTestPassed = false;
    unsigned int numEnabled = 0u;
    unsigned int aCount = EDMAAPP_MAX_ACOUNT;
    unsigned int bCount = EDMAAPP_MAX_BCOUNT;
    unsigned int cCount = EDMAAPP_MAX_CCOUNT;



    // sd������dsp
    char const			    *fname = "/1MB";
	INT16U					usBytesRead = 0;
	FRESULT 				fresult;

    INT8U err;
    _BSP_MESSAGE DSPMsg;

    OSTimeDlyHMSM(0,0,1,0);

	while(1)
	{
	    if(0)//FLASH�������ԣ������Բ��ú����Ϲرա��������ͣ��дFLASH
		{
			INT8U TEMP[100]={0};
			INT8U i;

			for(i=0;i<100;i++)
				TEMP[i]=i+1;
			BSP_WriteDataToAT(0x800,TEMP,100);
			OSTimeDly(SYS_DELAY_1s);
			for(i=0;i<100;i++)
				TEMP[i]=0;
			BSP_ReadDataFromAT(0x803,TEMP,97);//�缸���ֽڶ���
			for(i=0;i<100;i++)
				TEMP[i]=0;
			OSTimeDly(SYS_DELAY_1s);
		}

        //w5500����
        if(0)
        {
        	INT8U Detination_IP[4]	={192,168,11,20};					// Ŀ��IP
			INT8U Source_IP[4]		={192,168,11,42};					// ԴIP
			INT8U Gateway_IP[4]		={192,168,11,1};					// ����
			INT8U Subnet_Mask[4]	={255,255,255,0};					// ��������
			INT8U MAC_Addr[6]		={0x48,0x53,0x00,0x57,0x55,0x00};	// MAC

        	_BSPW5500_CONFIG W5500_InitStructure;
            _BSPGPIO_CONFIG GPIO_InitStructure;

            memcpy(W5500_InitStructure.GW_IP,Gateway_IP,4);
            memcpy(W5500_InitStructure.MAC,MAC_Addr,6);
            memcpy(W5500_InitStructure.S_IP,Source_IP,4);
            memcpy(W5500_InitStructure.Sub_Mask,Subnet_Mask,4);
            memcpy(W5500_InitStructure.D_IP,Detination_IP,4);
            W5500_InitStructure.D_Port = DESPORT_NUM;
            W5500_InitStructure.S_Port = 7566;
            W5500_InitStructure.s_num = SOCKET0;

			// W5500��ʼ��
			BSP_W5500Config(W5500A,&W5500_InitStructure);

            // �ж��߳�ʼ��
            GPIO_InitStructure.PortNum = PORT3;
            GPIO_InitStructure.PinNum = 19;
            GPIO_InitStructure.Dir = GPIO_DIR_INPUT;
            GPIO_InitStructure.IntLine = GPIO_INT_LINE_1;
            GPIO_InitStructure.IntType = GPIO_INT_TYPE_LEVEL_LOW;
            BSP_GPIOConfig(&GPIO_InitStructure);

			while(1)
			{
				TCPS_LoopBack(W5500A);	// TCP������
		//		TCPC_LoopBack(W5500A);	// TCP�ͻ���
		//		UDP_LoopBack(W5500A);	// UDP
            }

        }
        //DMA����
        if(0)
        {


            // EDMA��ģ��ʱ�ӳ�ʼ��
            EDMAModuleClkConfig();

            // ��ʼ��EDMA(DMA/QDMA�����˳�ʼ��)
            EDMA3Init(SOC_EDMA30CC_0_REGS, EDMAAPP_DMA_EVTQ);

            // ע��EDMA�ж�
            _EDMAAppRegisterEdma3Interrupts();

            // ��ʼ��Դ��ַ����
            for (count = 0u; count < (aCount * bCount * cCount); count++)
                SrcBuff[count] = data++;

            // ����һ��DMAͨ��
            retVal = EDMA3RequestChannel(EDMAAPP_EDMACC_BASE_ADDRESS,
                                         EDMAAPP_DMA_CH_TYPE, EDMAAPP_DMA_CH_NUM,
                                         EDMAAPP_DMA_TCC_NUM, EDMAAPP_DMA_EVTQ);

            // ע��ָ��ͨ���Ļص�����
            EDMAAppCallbackFxn[EDMAAPP_DMA_TCC_NUM] = &EDMAAppCallback;

            if(TRUE == retVal)
            {
                /* Fill the PaRAM Set with transfer specific information */
                paramSet.srcAddr = (unsigned int)(SrcBuff);
                paramSet.destAddr = (unsigned int)(DstBuff);

                paramSet.aCnt = (unsigned short)aCount;
                paramSet.bCnt = (unsigned short)bCount;
                paramSet.cCnt = (unsigned short)cCount;

                /* Setting up the SRC/DES Index */
                paramSet.srcBIdx = (short)aCount;
                paramSet.destBIdx = (short)aCount;

                if(EDMA3_SYNC_A == EDMAAPP_DMA_SYNC_TYPE)
                {
                    /* A Sync Transfer Mode */
                    paramSet.srcCIdx = (short)aCount;
                    paramSet.destCIdx = (short)aCount;
                }
                else
                {
                    /* AB Sync Transfer Mode */
                    paramSet.srcCIdx = ((short)aCount * (short)bCount);
                    paramSet.destCIdx = ((short)aCount * (short)bCount);
                }

                /* Configure the paramset with NULL link */
                paramSet.linkAddr = (unsigned short)0xFFFFu;

                paramSet.bCntReload = (unsigned short)0u;
                paramSet.opt = 0u;

                /* Src & Dest are in INCR modes */
                paramSet.opt &= ~(EDMA3CC_OPT_SAM | EDMA3CC_OPT_DAM);

                /* Program the TCC */
                paramSet.opt |= ((EDMAAPP_DMA_TCC_NUM << EDMA3CC_OPT_TCC_SHIFT)
                                 & EDMA3CC_OPT_TCC);

                /* Enable Intermediate & Final transfer completion interrupt */
                paramSet.opt |= (1u << EDMA3CC_OPT_ITCINTEN_SHIFT);
                paramSet.opt |= (1u << EDMA3CC_OPT_TCINTEN_SHIFT);

                if(EDMA3_SYNC_A == EDMAAPP_DMA_SYNC_TYPE)
                {
                    paramSet.opt &= ~EDMA3CC_OPT_SYNCDIM;
                }
                else
                {
                    /* AB Sync Transfer Mode */
                    paramSet.opt |= (1u << EDMA3CC_OPT_SYNCDIM_SHIFT);
                }

                /* Now, write the PaRAM Set. */
                EDMA3SetPaRAM(EDMAAPP_EDMACC_BASE_ADDRESS, EDMAAPP_DMA_CH_NUM,
                              &paramSet);

                EDMA3GetPaRAM(EDMAAPP_EDMACC_BASE_ADDRESS, EDMAAPP_DMA_CH_NUM,
                              &paramSet);
            }

            /*
            ** Since the transfer is going to happen in Manual mode of EDMA3
            ** operation, we have to 'Enable the Transfer' multiple times.
            ** Number of times depends upon the Mode (A/AB Sync)
            ** and the different counts.
            */
            if(TRUE == retVal)
            {
                /* Need to activate next param */
                if(EDMA3_SYNC_A == EDMAAPP_DMA_SYNC_TYPE)
                {
                    numEnabled = bCount * cCount;
                }
                else
                {
                    /* AB Sync Transfer Mode */
                    numEnabled = cCount;
                }

                for(index = 0u; index < numEnabled; index++)
                {
                    IrqRaised = EDMAAPP_IRQ_STATUS_XFER_INPROG;

                    // clean a section of D-CACHE
                    CacheDataCleanBuff((INT32U)SrcBuff,12);


                    /*
                    ** Now enable the transfer as many times as calculated above.
                    */
                    retVal = EDMA3EnableTransfer(EDMAAPP_EDMACC_BASE_ADDRESS,
                                                 EDMAAPP_DMA_CH_NUM,
                                                 EDMAAPP_DMA_TRIG_MODE);

                    /* Wait for the Completion ISR. */
                    while(EDMAAPP_IRQ_STATUS_XFER_INPROG == IrqRaised)
                    {
                        /*
                        ** Wait for the Completion ISR on Master Channel.
                        ** You can insert your code here to do something
                        ** meaningful.
                        */
                    }

                    /* Check the status of the completed transfer */
                    if(IrqRaised < (int)EDMAAPP_IRQ_STATUS_XFER_INPROG)
                    {

                        /* Clear the error bits first */
                        EDMA3ClearErrorBits(EDMAAPP_EDMACC_BASE_ADDRESS,
                                            EDMAAPP_DMA_CH_NUM, EDMAAPP_DMA_EVTQ);
                        break;
                    }
                }
            }

            // invalidate a section of D-CACHE
            CacheDataInvalidateBuff((INT32U)DstBuff,12);

            /* Match the Source and Destination Buffers. */
            if(TRUE == retVal)
            {
                for(index = 0u; index < (aCount * bCount * cCount); index++)
                {
                    if(SrcBuff[index] != DstBuff[index])
                    {
                        isTestPassed = false;
                        break;
                    }
                }

                if(index == (aCount * bCount * cCount))
                {
                    isTestPassed = true;
                }

                /* Free the previously allocated channel. */
                retVal = EDMA3FreeChannel(EDMAAPP_EDMACC_BASE_ADDRESS,
                                          EDMAAPP_DMA_CH_TYPE, EDMAAPP_DMA_CH_NUM,
                                          EDMAAPP_DMA_TRIG_MODE, EDMAAPP_DMA_TCC_NUM,
                                          EDMAAPP_DMA_EVTQ);

                /* Unregister Callback Function */
                EDMAAppCallbackFxn[EDMAAPP_DMA_TCC_NUM] = NULL;

                if(TRUE != retVal)
                    while(1);
            }

            if(true == isTestPassed)
                while(1);

        }
        //SPI����
        if(0)
        {
			INT8U err;

            _BSPSPI_CONFIG SPI_InitStructure;
            OS_EVENT *SPI0Event;
            void *spi0OSQ[4];										// SPI0 ��Ϣ����
//			INT8U SPI0TxBuff[100] ={9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,
//            						9,8,7,6,5,4,3,2,1,0,};			// SPI0 ���ͻ���
            INT8U SPI0TxBuff[5] = {0x9f,0x00,0x00,0x00,0x00};

            INT8U SPI0RxBuff[5] = {0};							    // SPI0 ���ջ���


			//----------------- SPI0 ��ģʽ ��ʼ�� ---------------------------------------------
            SPI0Event = OSQCreate(spi0OSQ,4);						// ����SPI��������
			SPI_InitStructure.num			= BSPW550A_SPINUM;
			SPI_InitStructure.pEvent 		= SPI0Event;
            SPI_InitStructure.TxRespond 	= BSPSPI_RESPOND_INT;	// �ж�ģʽ
            SPI_InitStructure.channel 		= BSPSPI_CHN0;			// 0ͨ��
            SPI_InitStructure.pTxBuffer 	= SPI0TxBuff;
            SPI_InitStructure.pRxBuffer 	= SPI0RxBuff;
            SPI_InitStructure.MaxTxBuffer	= sizeof(SPI0TxBuff);
            SPI_InitStructure.MaxRxBuffer 	= sizeof(SPI0RxBuff);
            SPI_InitStructure.Mode 			= BSPSPI_WORKMODE_MASTER;// ��ģʽ
            SPI_InitStructure.RxOvertime 	= SYS_DELAY_10ms;
            BSP_SPIConfig(&SPI_InitStructure);
            //BSP_SPITransBytes(SPI0,SPI0TxBuff,SPI0RxBuff,sizeof(SPI0TxBuff));
            while(1)
            {
                _BSP_MESSAGE  *pSM;
                pSM=OSQPend(SPI0Event,SYS_DELAY_250ms,&err);
                if(OS_ERR_NONE == err)
                {
                    switch(pSM->MsgID)								// �ж���Ϣ����(ֻ����_BSP_MSGID�ж��������)
                    {
                        case BSP_MSGID_SPI_RXOVER:					// �������
                        	BSP_SPIClear(BSPW550A_SPINUM);
                            //BSP_SPITransBytes(SPI0,SPI0TxBuff,SPI0RxBuff,sizeof(SPI0TxBuff));
                            break;
                        case BSP_MSGID_SPI_TXOVER:					// �������
                        	OSTimeDlyHMSM(0,0,1,0);
                            break;
                        case BSP_MSGID_SPI_RXERR:
                            OSTimeDlyHMSM(0,0,1,0);
                            break;
                        default:
                            break;
                        //...������Ϣ
                    }
                }
            }
        }
        // UART����
		if(0)
		{
#if 0 /*hxj amend,date 2014-12-16 9:42*/
		    #define	XXX_RX_NUM	1024
            INT16U i;
			INT8U txBuff[XXX_RX_NUM] = "This is wangyao UART Demo Test Code.\n";
            INT8U rxBuff[XXX_RX_NUM];
            INT8U Rxbuff[XXX_RX_NUM];
            INT8U *p = Rxbuff;
			_BSPUART_CONFIG UART_InitStructure;                         // �ñ����������޸ģ���Ϊ��Bsp_UART.c�ļ��б���ͬ����Ӱ���Ķ��ж�
			void *uartOSQ[4];					                        // ��Ϣ����
			OS_EVENT *uartEvent;				                        // ʹ�õ��¼�

			memset(Rxbuff,0,XXX_RX_NUM);

			uartEvent = OSQCreate(uartOSQ,4);
			UART_InitStructure.Baudrate = 115200;		                // ������
			UART_InitStructure.Parity = BSPUART_PARITY_NO;			    // У��λ
			UART_InitStructure.StopBits = BSPUART_STOPBITS_1;		    // ֹͣλ
			UART_InitStructure.WordLength = BSPUART_WORDLENGTH_8D;	    // ����λ��
			UART_InitStructure.Work = BSPUART_WORK_FULLDUPLEX;		    // ����ģʽ
			UART_InitStructure.TxRespond = BSPUART_RESPOND_INT;	        // �ж�ģʽ
			UART_InitStructure.pEvent = uartEvent;	                    // ��Ϣ�¼�
			UART_InitStructure.MaxTxBuffer = XXX_RX_NUM;				// ���ͻ�������
			UART_InitStructure.MaxRxBuffer = XXX_RX_NUM;				// ���ջ�������
			UART_InitStructure.pTxBuffer = txBuff;					    // ���ͻ���ָ��
			UART_InitStructure.pRxBuffer = rxBuff;					    // ���ջ���ָ��
			UART_InitStructure.TxSpacetime = 0;							// ����֡���
			UART_InitStructure.RxOvertime = 10;			    			// ����֡���

			BSP_UARTConfig(UART0,&UART_InitStructure);
			BSP_UARTWrite(UART0,txBuff,40);

			while(1)
			{
				_BSP_MESSAGE *pMsg;										// ��Ϣָ��
				pMsg = OSQPend(uartEvent,SYS_DELAY_100ms,(INT8U *)&i);	// ��ϢԴ:������֪ͨ���������ʱ֪ͨ;���մ���
				if(((INT8U)i) == OS_NO_ERR)								// �յ���Ϣ
				{
					switch(pMsg->MsgID)								// �ж���Ϣ
					{
						case BSP_MSGID_UART_TXOVER:						// RS232�������
							NOP();
                            break;
						case BSP_MSGID_UART_RXOVER:						// RS232�������
							if(pMsg->DataLen > 600)
                            {
                                pMsg->DataLen=0;
                                break;
                            }
                            memcpy(p,pMsg->pData,pMsg->DataLen);
                            p += pMsg->DataLen;
                            OSTimeDlyHMSM(0,0,1,0);
                            BSP_UARTWrite(UART0,pMsg->pData,pMsg->DataLen);
							BSP_UARTRxClear(UART0);
							break;
						default:
							break;
					}
				}
			}

#else
		    #define	XXX_RX_NUM	1024


            static int init_uart0=0;
            INT8U *txBuff = "UART0\r\n";


            if(0==init_uart0)
            {
                init_uart0=1;

    			_BSPUART_CONFIG UART_InitStructure;                         // �ñ����������޸ģ���Ϊ��Bsp_UART.c�ļ��б���ͬ����Ӱ���Ķ��ж�

    			UART_InitStructure.Baudrate = 115200;		                // ������
    			UART_InitStructure.Parity = BSPUART_PARITY_NO;			    // У��λ
    			UART_InitStructure.StopBits = BSPUART_STOPBITS_1;		    // ֹͣλ
    			UART_InitStructure.WordLength = BSPUART_WORDLENGTH_8D;	    // ����λ��
    			UART_InitStructure.Work = BSPUART_WORK_FULLDUPLEX;		    // ����ģʽ
    			UART_InitStructure.TxRespond = BSPUART_RESPOND_NORMAL;	    // �ж�ģʽ     //BSPUART_RESPOND_INT
    			UART_InitStructure.pEvent = NULL;	                        // ��Ϣ�¼�
    			UART_InitStructure.MaxTxBuffer = XXX_RX_NUM;				// ���ͻ�������
    			UART_InitStructure.MaxRxBuffer = 0;				            // ���ջ�������
    			UART_InitStructure.pTxBuffer = txBuff;					    // ���ͻ���ָ��
    			UART_InitStructure.pRxBuffer = NULL;					    // ���ջ���ָ��
    			UART_InitStructure.TxSpacetime = 0;							// ����֡���
    			UART_InitStructure.RxOvertime = 10;			    			// ����֡���

    			BSP_UARTConfig(UART0,&UART_InitStructure);
            }

            //send data
			//BSP_UARTWrite(UART0,txBuff,strlen(txBuff));
            //BSP_UARTWrite(UART0,txBuff,strlen(txBuff));
            BSP_UARTWrite(UART0,txBuff,7);
            OSTimeDlyHMSM(0,0,2,0); //sleep 2
            //printf("hello printf\n");
            OSTimeDlyHMSM(0,0,2,0); //sleep 2


#endif






		}

        if(0)
        {
            INT8U err;
			INT32U txBuff[2],rxBuff[2];
			_BSPDCAN_CONFIG CAN_Config;
			void *canOSQ[4];					// ��Ϣ����
			OS_EVENT *canEvent;				    // ʹ�õ��¼�

			canEvent = OSQCreate(canOSQ,4);
            CAN_Config.pEvent = canEvent;	// ��Ϣ�¼�
			CAN_Config.num = 0;		        // �˿ں�
			CAN_Config.pTxBuffer = txBuff;	// ���ͻ���ָ��
			CAN_Config.pRxBuffer = rxBuff;	// ���ջ���ָ��
            CAN_Config.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_RX);// ����֡����Ϊ����
            BSP_DCANInit(0,&CAN_Config);
            while(1)
			{
				_BSP_MESSAGE *pMsg;						// ��Ϣָ��
				pMsg = OSQPend(canEvent,SYS_DELAY_100ms,&err);
				if(err == OS_NO_ERR)				    // �յ���Ϣ
				{
					switch(pMsg->MsgID)					// �ж���Ϣ
					{
						case BSP_MSGID_DCAN_STDTXOVER:
							NOP();
                            break;
						case BSP_MSGID_DCAN_STDRXOVER:
							if(pMsg->DataLen > 8)
                            {
                                pMsg->DataLen=0;
                                break;
                            }
                            memcpy(txBuff,pMsg->pData,pMsg->DataLen);
                            CAN_Config.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_TX);// ��׼����֡����Ϊ����
                            CAN_Config.entry.id = BSP_CANTX_MSG_STD_ID;//��׼֡
                            CAN_Config.entry.dlc = pMsg->DataLen;
                            CAN_Config.entry.data = txBuff;
                            BSP_DCANWrite(CAN_Config);
							break;
                        case BSP_MSGID_DCAN_EXTRXOVER:
                            memcpy(txBuff,pMsg->pData,pMsg->DataLen);
                            CAN_Config.entry.flag = (BSP_CANDATA_EXIFRAME | BSP_CANMSG_DIR_TX);// ��������֡����Ϊ����
                            CAN_Config.entry.id = BSP_CANTX_MSG_EXTD_ID;//����֡
                            CAN_Config.entry.dlc = pMsg->DataLen;
                            CAN_Config.entry.data = txBuff;
                            BSP_DCANWrite(CAN_Config);
							break;
						default:
							break;
					}
				}
			}
        }

	}
}
#endif //__DEBUG_TASK_TEST_ENABLE

/***********************************************************************************************
* Function		: TaskLED
* Description	: LEDָʾ����ʾ��������������
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void TaskLED(void *pdata)
{

    /* Enabling functional clocks for GPIO1 instance. */
    GPIO1ModuleClkConfig();

    /* Selecting GPIO1[23] pin for use. */
    GPIO1Pin23PinMuxSetup();

    /* Enabling the GPIO module. */
    GPIOModuleEnable(GPIO_INSTANCE_ADDRESS);

    /* Resetting the GPIO module. */
    GPIOModuleReset(GPIO_INSTANCE_ADDRESS);

    /* Setting the GPIO pin as an output pin. */
    GPIODirModeSet(GPIO_INSTANCE_ADDRESS,
                   GPIO_INSTANCE_PIN_NUMBER,
                   GPIO_DIR_OUTPUT);

    OSTimeDlyHMSM(0,0,0,100);
    while(1)
    {
        /* Driving a logic HIGH on the GPIO pin. */
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
                     GPIO_INSTANCE_PIN_NUMBER,
                     GPIO_PIN_HIGH);
        OSTimeDlyHMSM(0,0,0,100);

        /* Driving a logic LOW on the GPIO pin. */
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
                     GPIO_INSTANCE_PIN_NUMBER,
                     GPIO_PIN_LOW);
        OSTimeDlyHMSM(0,0,0,100);
    }
}

/***********************************************************************************************
* Function		: init_task_core
* Description	: ����Ӧ�ó����������ӳ���
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 25/09/2013	wangyao
***********************************************************************************************/
static void init_task_core(void *pdata)
{


    pdata = pdata;   //��ֹ����������

    MMUConfigAndEnable();
    CacheEnable(CACHE_ALL);

    BSP_Init();

    OS_CPU_TickInit();//ϵͳʱ�ӽ��ĵĳ�ʼ��
    
    // ����APP���¼���ʼ������
#if DBG_DSP_RECV_TWO_EN
    W5500AEvent = OSQCreate(W5500OAOSQ,4);
#endif
    W5500BEvent = OSQCreate(W5500OBOSQ,4);

    MMCSDEvent = OSQCreate(MMCSDOSQ,4);
    
#if 1 /*hxj amend,date 2014-12-26 16:14*/
    init_uart0(115200);
    DBG_NORMAL("\r\nuart0 init ok \r\n");

    char ver_info[20]={0};
    getVersion(ver_info);

    DBG_NORMAL("AM_VER:%s\r\n",ver_info);
    char time_st[30];
    get_compile_time_my_format (time_st );
    DBG_NORMAL("AM_app:%s\r\n",time_st);
#endif



    /* Initialize BSP functions                             */
    //Mem_Init();         /* Initialize memory managment module                   */
   // Math_Init();        /* Initialize mathematical module                       */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();       /* Determine CPU capacity                               */
#endif

    //�ȴ�����Ӳ��оƬ��������
    OSTimeDlyHMSM(0,0,0,500);


    //��ȡ����
    get_save_para();


#if 0
    //������ͨ�ŵ�����
    OSTaskCreateExt(TaskComWithMC,
                    (void *)0,
                    &TaskComWithMCStk[TASK_STK_SIZE_COMWITHMC-1],
                    PRI_TaskComWithMC,
                    1,
                    &TaskComWithMCStk[0],
                    TASK_STK_SIZE_COMWITHMC,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

    //��������ʾ����
	OSTaskCreateExt(TaskDisplay,
                    (void *)0,
                    &TaskDisplayStk[TASK_STK_SIZE_DISPLAY-1],
                    PRI_TaskDisplay,
                    1,
                    &TaskDisplayStk[0],
                    TASK_STK_SIZE_DISPLAY,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

    // ��������Nand
    OSTaskCreateExt(TaskFlash,
                    (void *)0,
                    &TaskFlashStk[TASK_STK_SIZE_FLASH-1],
                    PRI_FLASH,
                    1,
                    &TaskFlashStk[0],
                    TASK_STK_SIZE_FLASH,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(TaskUSB,
                    (void *)0,
                    &TaskUSBStk[TASK_STK_SIZE_USB-1],
                    PRI_USB,
                    1,
                    &TaskUSBStk[0],
                    TASK_STK_SIZE_USB,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif

#if 0 // ����2410
    OSTaskCreateExt(TaskComWith2410,
                    (void *)0,
                    &TaskComWith2410Stk[TASK_STK_SIZE_COMWITH2410 - 1],
                    PRI_2410,
                    1,
                    &TaskComWith2410Stk[0],
                    TASK_STK_SIZE_COMWITH2410,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

#endif

#if DBG_DSP_RECV_TWO_EN
    // DSP1ͨѶ����
    OSTaskCreateExt(TaskComWithDSP1,
                    NULL,
                    &TaskComWithDSP1Stk[TASK_STK_SIZE_COMWITHDSP1-1],
                    PRI_TaskComWithDSP1,
                    1,
                    &TaskComWithDSP1Stk[0],
                    TASK_STK_SIZE_COMWITHDSP1,
                    NULL,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif
#if 0
    // DSP2ͨѶ����
	OSTaskCreateExt(TaskComWithDSP2,
					NULL,
					&TaskComWithDSP2Stk[TASK_STK_SIZE_COMWITHDSP2-1],
					PRI_TaskComWithDSP2,
					1,
					&TaskComWithDSP2Stk[0],
					TASK_STK_SIZE_COMWITHDSP2,
					NULL,
					OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif

    // A8_PHY_DSPͨѶ����
	OSTaskCreateExt(Task_PHY_DSP,
					NULL,
					&Task_PHY_DSPStk[TASK_STK_SIZE_PHY_DSP-1],
					PRI_Task_PHY_DSP,
					1,
					&Task_PHY_DSPStk[0],
					TASK_STK_SIZE_PHY_DSP,
					NULL,
					OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

	//dsp ������ݴ�������
    OSTaskCreateExt(task_dsp_net_server_time,
                    (void *)0,
                    &task_dsp_net_server_stk_time[TASK_STK_SIZE_DSP_NET_SERVER_TIME-1],
                    PRI_DSP_NET_SERVER_TIME,
                    1,
                    &task_dsp_net_server_stk_time[0],
                    TASK_STK_SIZE_DSP_NET_SERVER_TIME,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);



    // ����SD����ȡ������,�ͳ�����������
    OSTaskCreateExt(TaskSDMaster,
                    (void *)0,
                    &TaskSDMasterStk[TASK_STK_SIZE_SD_MASTER-1],
                    PRI_SD_MASTER,
                    1,
                    &TaskSDMasterStk[0],
                    TASK_STK_SIZE_SD_MASTER,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);


#if __DEBUG_TASK_TEST_ENABLE
    // ��������,Test
    OSTaskCreateExt(TaskTest,
                    (void *)0,
                    &TaskTestStk[TASK_STK_SIZE_TEST-1],
                    PRI_TEST,
                    1,
                    &TaskTestStk[0],
                    TASK_STK_SIZE_TEST,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif	//__DEBUG_TASK_TEST_ENABLE




#if 0 /*hxj amend,date 2014-12-29 14:34*/
    OSTaskCreateExt(task_uart_server,
                    (void *)0,
                    &task_arm_uart[TASK_STK_SIZE_ARM_UART-1],
                    PRI_ARM_UART,
                    1,
                    &task_arm_uart[0],
                    TASK_STK_SIZE_ARM_UART,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif




#if NET_PC_FUN_EN

    OSTaskCreateExt(task_net_server_time,
                    (void *)0,
                    &task_net_server_stk_time[TASK_STK_SIZE_NET_SERVER_TIME-1],
                    PRI_NET_SERVER_TIME,
                    1,
                    &task_net_server_stk_time[0],
                    TASK_STK_SIZE_NET_SERVER_TIME,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);


    OSTaskCreateExt(task_net_server,
                    (void *)0,
                    &task_net_server_stk[TASK_STK_SIZE_NET_SERVER-1],
                    PRI_NET_SERVER,
                    1,
                    &task_net_server_stk[0],
                    TASK_STK_SIZE_NET_SERVER,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif






    while(1)
    {
        //OSTimeDlyHMSM(0,0,1,0);
		OSTaskDel(OS_PRIO_SELF);
    }

}
/***********************************************************************************************
* Function		: main
* Description	:
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 25/09/2013	wangyao
***********************************************************************************************/
int main(void)
{
    CPU_Init();  /* Initialize the uC/CPU services */

    CPU_IntDis();

    OSInit();	//��ʼ��OS

    OSTaskCreateExt(init_task_core,
                    (void *)0,
                    (OS_STK *)&InitTaskStk[OS_INIT_TASK_STACK_SIZE - 1],
                    OS_TASK_INIT_PRIO,
                    OS_TASK_INIT_PRIO,
                    (OS_STK *)&InitTaskStk[0],
                    OS_INIT_TASK_STACK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);



    OSStart();	//���������񻷾�

    return(0);
}
/***********************************************************************************************
* Function		: assert_failed
* Description	:
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
#ifdef  DEBUG_PARAM
void assert_failed(u8* file, u32 line)
{
  //User can add his own implementation to report the file name and line number,
  //ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)

  //Infinite loop
  if (1)
  {
  	NOP();
  }
}
#endif


/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/


