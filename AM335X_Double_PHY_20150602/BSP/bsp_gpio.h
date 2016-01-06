/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_gpio.c
**Ӳ          ��: am335x
**��    ��    ��: code
**��  ��  ��  ��: 141115
**��  ��  ��  ��: V0.1
**��          ��:

**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��:
**��          ��:
**��          ��:
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_GPIO_H_
#define __BSP_GPIO_H_

/* Private macro------------------------------------------------------------------------------*/

// GPIO���ýṹ��
typedef struct{
    INT8U PortNum;              // �˿ں�
    INT32U PinNum;              // �ܽź�
    INT32U Dir;                 // I/O����
    INT32U IntLine;             // �ж��ߺ�
    INT32U IntType;             // �жϴ�������
    OS_EVENT *pEvent;			// �¼�ָ��,����������Ϣ�¼�
}_BSPGPIO_CONFIG;


#define PORT0 0x00
#define PORT1 0x01
#define PORT2 0x02
#define PORT3 0x03

#define GPIO_DIR_I 0x01
#define GPIO_DIR_O 0x00

#define GPIO_INTA   0x00
#define GPIO_INTB   0x01

#define GPIO_INT_TYPE_NO    0x01
#define GPIO_INT_TYPE_L     0x04
#define GPIO_INT_TYPE_H     0x08
#define GPIO_INT_TYPE_BOTH  0x0C

//#define SDCARD_HOT_START
#ifdef SDCARD_HOT_START
#define BSP_SDINIT_COMFROM_GPIO 0xC1
#endif //SDCARD_HOT_START

void BSP_GPIOINTnIRQHandler(unsigned int portnum,unsigned int pinnum,unsigned int intLine);
INT8U BSP_GPIOConfig(_BSPGPIO_CONFIG *pConfig);
INT8U BSP_GPIOInit(INT8U num);



#endif
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/

