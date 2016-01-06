/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		    : bsp_LCD.c
* Author		    : 王耀
* Date First Issued	: 12/10/2010
* Version		    : V
* Description		: 尝试做的版本，非正式版本。液晶驱动文件。供外部用户使用。
*----------------------------------------历史版本信息-------------------------------------------
* History		    :
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "os_cpu.h"
#include "include.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "gpio.h"
#include "gpio_v2.h"
#include "bsp_lcd.h"
//#include "image.h"
#include "raster.h"
#include "cpu.h"
#include "bsp_int.h"
#include "bsp_lcd.h"
#define GUI_FONT24_SBC_USED //启用汉字全角字库
#include "font2424.h"
#include "am335x_irq.h"
/* Private define-----------------------------------------------------------------------------*/
#define LCDC_INSTANCE SOC_LCDC_0_REGS

#define X_OFFSET    (80+150)
#define Y_OFFSET    (16+100)

#define PALETTE_OFFSET	4
#define FRAME_BUFFER_0	0
#define FRAME_BUFFER_1	1

#define PIXEL_24_BPP_PACKED		(0x0)
#define PIXEL_24_BPP_UNPACKED	(0x1)


#define	GUI_FONT_SBC  _FONT_SBC //汉字字库

const tFont g_sFontCH24=
{
	FONT_CH_STYLE,
	24,
	24,
    24,
	{4},
	GUI_FONT_SBC,//全角字库	
};
/* Private typedef----------------------------------------------------------------------------*/
// The graphics library display structure.
tDisplay g_s35_800x480x24Display;//显示的关键结构体，定义了图像的最基本的函数

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
// Memory that is used as the local frame buffer.
unsigned char g_pucBuffer[GrOffScreen24BPPSize(LCD_WIDTH, LCD_HEIGHT, PIXEL_24_BPP_UNPACKED)];
// 32 byte Palette.
unsigned int palette_32b[PALETTE_SIZE/sizeof(unsigned int)] = 
            {0x4000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u};
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
static void LCDAINTCConfigure(void)
{
    /* Register the ISR in the Interrupt Vector Table.*/
    BSP_IntVectReg(SYS_INT_LCDCINT, LCD_IRQHandler);
    IntPrioritySet(SYS_INT_LCDCINT, 0, AINTC_HOSTINT_ROUTE_IRQ );
    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(SYS_INT_LCDCINT);
}
/*
** configures arm interrupt controller to generate raster interrupt 
*/
static void SetupIntc(void)
{
    //IntMasterIRQEnable();
    //IntAINTCInit();
    LCDAINTCConfigure();
    LCDBackLightEnable();
    //UPDNPinControl();	
    //TouchIntRegister();
}

/***********************************************************************************************
* Function		: LCDIRQHandler
* Description	: LCD中断处理函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void LCDIRQHandler(void)
{
    unsigned int  status;

    
    status = RasterIntStatus(SOC_LCDC_0_REGS,RASTER_END_OF_FRAME0_INT_STAT |
                                             RASTER_END_OF_FRAME1_INT_STAT );

    status = RasterClearGetIntStatus(SOC_LCDC_0_REGS, status);   

    if (status & RASTER_END_OF_FRAME0_INT_STAT)
    {
        /* configuring the base ceiling */
        RasterDMAFBConfig(SOC_LCDC_0_REGS, 
                          (unsigned int)(g_pucBuffer+4),
                          (unsigned int)(g_pucBuffer+4) + sizeof(g_pucBuffer) - 2 - 4,
                          0);
    }

    if(status & RASTER_END_OF_FRAME1_INT_STAT)
    {

        RasterDMAFBConfig(SOC_LCDC_0_REGS,
                          (unsigned int)(g_pucBuffer+4),
                          (unsigned int)(g_pucBuffer+4) + sizeof(g_pucBuffer) - 2 - 4,
                          1);
    }   
}

/***********************************************************************************************
* Function		: BSP_GUIInit
* Description	: GUI初始化,带LCD初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void BSP_GUIInit(tContext *pContext)
{
    unsigned long i = 0;
    unsigned char *dest;
    unsigned char *src;

    SetupIntc();
    /* Enable clock for LCD Module */ 
    LCDModuleClkConfig();
    /*Pin multiplexing registers to enable LCD raster pin and a standard configuration is provided as part of the
    function LCDPinMuxSetup() in platform directory
    */
    LCDPinMuxSetup();
      /* 
    **Clock for DMA,LIDD and for Core(which encompasses
    ** Raster Active Matrix and Passive Matrix logic) 
    ** enabled.
    */
    RasterClocksEnable(SOC_LCDC_0_REGS);

    /* Disable raster */
    RasterDisable(SOC_LCDC_0_REGS);
    
    /* Configure the pclk */
    RasterClkConfig(SOC_LCDC_0_REGS, 23040000, 192000000);

    /* Configuring DMA of LCD controller */ 
    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_DOUBLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_8,
                    RASTER_BIG_ENDIAN_DISABLE);

    /* Configuring modes(ex:tft or stn,color or monochrome etc) for raster controller */
    RasterModeConfig(SOC_LCDC_0_REGS, RASTER_DISPLAY_MODE_TFT_UNPACKED,
                     RASTER_PALETTE_DATA, RASTER_COLOR, RASTER_RIGHT_ALIGNED);


     /* Configuring the polarity of timing parameters of raster controller */
    RasterTiming2Configure(SOC_LCDC_0_REGS, RASTER_FRAME_CLOCK_LOW |
                                            RASTER_LINE_CLOCK_LOW  |
                                            RASTER_PIXEL_CLOCK_HIGH|
                                            RASTER_SYNC_EDGE_RISING|
                                           RASTER_SYNC_CTRL_ACTIVE|
                                           RASTER_AC_BIAS_HIGH , 0, 255);

    /* Configuring horizontal timing parameter */
    RasterHparamConfig(SOC_LCDC_0_REGS,LCD_WIDTH, 48, 40, 40);//800480

    /* Configuring vertical timing parameters */
    RasterVparamConfig(SOC_LCDC_0_REGS,LCD_HEIGHT , 3, 13, 29);//272


    RasterFIFODMADelayConfig(SOC_LCDC_0_REGS, 128);//128

    
    /* configuring the base ceiling */
    RasterDMAFBConfig(SOC_LCDC_0_REGS, 
                      (unsigned int)(g_pucBuffer+PALETTE_OFFSET),
                      (unsigned int)(g_pucBuffer+PALETTE_OFFSET) + sizeof(g_pucBuffer) - 2 -
					  PALETTE_OFFSET, FRAME_BUFFER_0);
    RasterDMAFBConfig(SOC_LCDC_0_REGS, 
                      (unsigned int)(g_pucBuffer+PALETTE_OFFSET),
                      (unsigned int)(g_pucBuffer+PALETTE_OFFSET) + sizeof(g_pucBuffer) - 2 - 
					  PALETTE_OFFSET, FRAME_BUFFER_1);
    src = (unsigned char *) palette_32b;
    dest = (unsigned char *) (g_pucBuffer+PALETTE_OFFSET);
    // Copy palette info into buffer
    for( i = PALETTE_OFFSET; i < (PALETTE_SIZE+PALETTE_OFFSET); i++)
    {
            *dest++ = *src++;
    }

    GrOffScreen24BPPInit(&g_s35_800x480x24Display, g_pucBuffer, LCD_WIDTH, LCD_HEIGHT);   
    // Initialize a drawing context.
    GrContextInit(pContext, &g_s35_800x480x24Display);
    /* enable End of frame interrupt */
    //RasterEndOfFrameIntEnable(SOC_LCDC_0_REGS);
    /* Enable End of frame0/frame1 interrupt */
    RasterIntEnable(SOC_LCDC_0_REGS, RASTER_END_OF_FRAME0_INT |
                                     RASTER_END_OF_FRAME1_INT);
    /* enable raster */
    RasterEnable(SOC_LCDC_0_REGS);
     

}
/***********************************************************************************************
* Function	    : BSP_UpdataLCD
* Description	: 刷新屏幕
* Input		    : 
* Output	    : 
* Note(s)	    : 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void BSP_UpdataLCD(void)
{
    CacheDataCleanBuff((unsigned int)g_pucBuffer , ((LCD_WIDTH*LCD_HEIGHT*4) + 32));	 //刷新屏幕必須要有  
}
/************************(C)COPYRIGHT 2013 浙江方泰****END OF FILE****************************/
