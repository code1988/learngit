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
#include "bsp.h"
#include "app.h"
#include "BaseDisp.h"
#include "DwinPotocol.h"
#include "CoodinateInit.h"
#include "DisplayMain.h"
#include "grlib.h"
#include "RecevCanPotocol.h"
/* Private define-----------------------------------------------------------------------------*/
#define SENDTODISPLEN (SendToDisp[2]+3)
#define __UART2_ENABLE

#define CLEAR 0x5A
#define MATRIX 0x59

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
INT8U count = 0;
INT8U SendToUART2[100] = {0};

INT8U Setdata[30];//���ò��������ȫ�ֱ���
INT8U Sconfirm = 0;//list�˵�����ok,back,cancel�Ժ������軭�˵�
ST_Menu *gps_CurMenu;     //��ǰ�˵�
INT32S (*LcdPage)(INT16U *MeterNo,INT32U type);//ҳ����ʾ����ָ��
SetDec gps_data = {64,1,16,8,1,0,0,0,Setdata};//����ҳ���ýṹ��
INT8U gCursorInMenu;    //����ڵ�ǰ�˵��е����
ST_Cursor  gs_CursorInPage;   //��ǰҳ�еĹ��λ��
//�����޸Ĳ˵�����ָ���Ĳ���λ��
INT8U Paramhorizontal = 0;
INT8U Paramvertical = 1;
INT8U EditParamPage = 0;
INT8U CheckParamPage = 0;
//��������
INT8U KeyBoardhorizontal = 0;
INT8U KeyBoardvertical = 0;
//�û���Ϣ��ʾ�ڼ���LIST
INT8U InfomationPage = 0;
INT8U InfomationList = 0;

INT8U AuthenticationDataPages = 0;
INT8U CursorSize = 2;
extern _BSP_MESSAGE Nandsend_message; 

//�������
extern tRectangle BaseMainMenuNumCoordinate[3][10];
extern tRectangle BaseInletTitle[2];
extern tRectangle BaseModeIcon[9];
extern tRectangle BaseNetIcon[2];
extern tRectangle BaseOutLetCoordinate[4][3];
extern tRectangle BaseTotalPage[4];
extern tRectangle BaseTotalSum[7];
extern tRectangle BaseTotalSumCoordinate;
extern tRectangle BaseNetCoordinate;
extern tRectangle BaseMainMenuModeCoordinate[2];
extern tRectangle BaseOutLetParam[7][10];
extern tRectangle BaseCheckParam[10][10];
extern tRectangle BaseEditParam[10][12];
extern tRectangle BaseEditParamColor[4];
extern tRectangle BaseKeyBoardSettingsCoordinate[8][12];
extern tRectangle BaseBlackListCoordinate[7];
extern tRectangle BaseBlackListNumberCoordinate[7];
extern tRectangle BaseInfomationCoordinate[3][7];
extern tRectangle BaseCleanLevelSettingsSelectCoordinate[6];
extern tRectangle BaseAuthenticationSettingsSelectCoordinate[7];
//��α����
extern INT8U AuthenticationDataSettings[3][90];
//���״̬
extern INT8U MTStatus;
//�˵���־
extern INT8U MainMenuStatus;
//��������������
extern INT8U BlackListhorizontal;
//��ֵȼ���������ֵ��������λ��꣩
extern INT8U ClearLevelhorizontal;
//��α�ȼ���������ֵ��������λ��꣩
extern INT8U AuthenticationLevelhorizontal;
//��ǰ�˵�
extern ST_Menu *gps_CurMenu;     
//���͸�CAN����Ϣ
extern _BSP_MESSAGE Dsend_message; 
//����ֵ
extern INT16U TemBatchNum;
//�û���Ϣ�鿴����
extern INT8U NetPostTime[6];
extern INT8U NetPostPage[2];
extern INT8U BlankName[16];
extern INT8U AreaNum[16];
extern INT8U BranchNum[16];
extern INT8U LatticePoint[11];
extern INT8U MachineNo[16];
extern INT8U IP[13];
extern INT8U Mac[13];
extern INT8U MachineNo[16];
extern INT8U BanTechnician[3][16];
extern INT8U Time[6];
extern INT8U Mask[4];
extern INT8U GateWay[4];
extern INT8U LocalPort[4];
extern INT8U DiskPort[4];
extern INT8U DiskIP[13];
extern INT8U BlackLists[2][7][13];
extern INT8U BlackListPage;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: printIcon
* Description	: ��ʾͼ���õ��ĺ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printIcon(INT8U IMAGE,tRectangle Icon,tRectangle Coordinate)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x71); //0x71 ����ͼ�굽����ͼƬ��
    TXBYTE(IMAGE); //0x64ͼ��1���ϵ�ͼ�꣬0x65��ͼ���2�����ͼ��
    //����ͼ�������
    TXBYTE(Icon.sXMin>>8);
    TXBYTE(Icon.sXMin&0xFF);
    TXBYTE(Icon.sYMin>>8);
    TXBYTE(Icon.sYMin&0xFF);
    TXBYTE(Icon.sXMax>>8);
    TXBYTE(Icon.sXMax&0xFF);
    TXBYTE(Icon.sYMax>>8);
    TXBYTE(Icon.sYMax&0xFF);
    
    //����ͼ�������
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printCharStrings16
* Description	: ֱ�����16*16���ַ�
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printCharStrings16(tRectangle Coordinate,INT8U *s)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while(*s) //�����ַ�������
    {
        TXBYTE(*s);
        s++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printStrings16
* Description	: ��ʾ�����˵������ֵĺ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[4];
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    bin_to_char(*s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printOutLetValueStrings16
* Description	: ��ʾ�����ڲ鿴���˳��ڲ鿴�����
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printOutLetValueStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[4];
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    JcSpeed_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: print2charStrings16
* Description	: ��ʾ�����˵������ֵĺ���(2���ַ�)
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void print2charStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[3];
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    bin_to_2char(*s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: print2charStrings16
* Description	: ��ʾ�����˵������ֵĺ���(2���ַ�)
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void print1charStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[2];
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    bin_to_1char(*s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printCrownWordStrings16
* Description	: ���ֺ�����ʾ
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printCrownWordStrings16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while(i<10)
    {
        TXBYTE(*(s++));
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printCrownWordStrings16
* Description	: ���ֺ�����ʾ
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printMainMenuTimeStrings32(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x6F); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while(*s != 0)
    {
        TXBYTE(*(s++));
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printMixNums16
* Description	: ��ʾ�����ϸ�˵��������ͽ��
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printMixNums16(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    INT8U String[4];
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x6F); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    Mix_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printStrings16
* Description	: ��ʾ�����˵������ֵĺ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printStrings32(tRectangle Coordinate,INT8U *s)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    TXBYTE(*s);
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}


/***********************************************************************************************
* Function		: printStrings16
* Description	: ��ʾ�汾��Ϣ�ĺ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printVersionStrings32(tRectangle Coordinate,INT8U *s)
{
    INT8U i = 0;
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x54); //0x54 ��ʾ16*16����
    //������ʾ���ֵ�����
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    while((s[i])&&(s[i] != 0xFF))
    {
        TXBYTE(s[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: FetchBackColor
* Description	: ȡ�ñ���ɫ
* Input		: Coordinate ȡ����ɫ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void FetchBackColor(tRectangle Coordinate)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x42); //0x54 ��ʾ16*16����
    //�������������
    TXBYTE(Coordinate.sXMin>>8);
    TXBYTE(Coordinate.sXMin&0xFF);
    TXBYTE(Coordinate.sYMin>>8);
    TXBYTE(Coordinate.sYMin&0xFF);
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: PrintCursor
* Description	: ��ʾ���
* Input		: Coordinate ȡ����ɫ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void PrintCursor(ST_Cursor Coordinate)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x44); //0x54 ��ʾ16*16����
    TXBYTE(0x01); 
    //���͹�������
    Coordinate.YCoor = Coordinate.YCoor -1;
    TXBYTE(Coordinate.XCoor>>8);
    TXBYTE(Coordinate.XCoor&0xFF);
    TXBYTE(Coordinate.YCoor>>8);
    TXBYTE(Coordinate.YCoor&0xFF);
    switch(CursorSize)
    {
        case 0x01:
            TXBYTE(0x07); //��ʾ�Ĺ�곤��
            break;
        case 0x02:
            TXBYTE(0x0F); //��ʾ�Ĺ�곤��
            break;
        case 0x03:
            TXBYTE(0x17); //��ʾ�Ĺ�곤��
            break;
    }
    TXBYTE(0x0F); //��ʾ�Ĺ��߶� 
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: CloseCursor
* Description	: �رչ����ʾ
* Input		: Coordinate ȡ�õ�����
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void CloseCursor(ST_Cursor Coordinate)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x44); //0x54 ��ʾ16*16����
    TXBYTE(0x00); 
    //���͹�������
    TXBYTE(Coordinate.XCoor>>8);
    TXBYTE(Coordinate.XCoor&0xFF);
    TXBYTE(Coordinate.YCoor>>8);
    TXBYTE(Coordinate.YCoor&0xFF);
	TXBYTE(0x16); //��ʾ�Ĺ�곤��
    TXBYTE(0x0F); //��ʾ�Ĺ��߶�
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printBackImage
* Description	: ��ʾ����ͼƬ(���ﻭ��ID��ͼƬ���������ϵ�˳�򱣳�һ��)
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 12/11/2014	songchao
***********************************************************************************************/
void PraPerprintBackImage(INT8U IMAGE)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x70); //0x7 ��ʾ�ڼ���ͼƬ
    TXBYTE(IMAGE); //0x64ͼ��1���ϵ�ͼ�꣬0x65��ͼ���2�����ͼ��
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: printBackImage
* Description	: ��ʾ����ͼƬ(���ﻭ��ID��ͼƬ���������ϵ�˳�򱣳�һ��)
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 12/11/2014	songchao
***********************************************************************************************/
void printBackImage(INT8U IMAGE)
{
    INT8U MTmode[2] = {0};
    if(IMAGE == 0)
    {
          PraPerprintBackImage(IMAGE);
          RefreshMainMenu();
          PraperMianMenu(); 
          MainMenuStatus = INMAINMENU;
          MTmode[0] = MTStatus;
          MTmode[1] = MainMenuStatus;
          Dsend_message.MsgID = APP_COMFROM_UI;
          Dsend_message.DivNum = UIACANMOTORSET;
          Dsend_message.DataLen = 2;
          Dsend_message.pData = MTmode;
          OSQPost (canEvent,&Dsend_message);
          OSTimeDlyHMSM(0,0,0,10);
    }
    else
    {
          PraPerprintBackImage(IMAGE);
    }
}

/***********************************************************************************************
* Function		: printsstring8
* Description	: ��ʾ8*8��С���ַ���
* Input			:   x       ������
                    y       ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring8(INT16U x,INT16U y,INT8U *s)
{
    prints(0x53,x,y,s);
} 

/***********************************************************************************************
* Function		: printsstring32
* Description	: ��ʾ32*32��С���ַ���
* Input			:   x       ������
                    y       ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring32(INT16U x,INT16U y,INT8U *s)
{
    prints(0x55,x,y,s);
}

/***********************************************************************************************
* Function	: printsJcSpeedstring32
* Description	: ��ʾ32*32��С���ַ���
* Input		:   x       ������
                    y       ������
* Output	: 
* Note(s)	: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsJcSpeedstring32(INT16U x,INT16U y,INT8U *s)
{
    JcSpeedprints(0x55,x,y,s);
}

/***********************************************************************************************
* Function		: printsJcPwmstring32
* Description	: ��ʾ32*32��С���ַ���
* Input			:   x       ������
                    y       ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsJcPwmstring32(INT16U x,INT16U y,INT8U *s)
{
    JcPwmprints(0x55,x,y,s);
}

/***********************************************************************************************
* Function		: printsstring12
* Description	: ��ʾ12*12��С���ַ���
* Input			:   x       ������
                    y       ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring12(INT16U x,INT16U y,INT8U *s)
{
    prints(0x6E,x,y,s);
}

/***********************************************************************************************
* Function		: printsstring24
* Description	: ��ʾ24*24��С���ַ���
* Input			:   x       ������
                    y       ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring24(INT16U x,INT16U y,INT8U *s)
{
    prints(0x6F,x,y,s);
}

/***********************************************************************************************
* Function		: prints
* Description	: ��ʾ�ַ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void prints(INT8U size,INT16U x,INT16U y,INT8U *s)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(size); //0x53=8����ASKII��0x54=16�����ַ�����0x55=32����
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x����
    TXWORD(y); //y����
    while(*s) //�����ַ�������
    {
        TXBYTE(*s);
        s++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

//����ӳ���
void TXBYTE( INT8U i) //�򴮿ڷ���һ���ֽ�
{
    SendToUART2[count++] = i;
    //BSP_UARTWrite(UART2,&i,1);
}

void TXWORD( INT16U i) //�򴮿ڷ���һ����
{
    TXBYTE(((i>>8)&0xFF));
    TXBYTE((i&0xFF)); 
}
void frame_end() //����֡������ cc 33 c3 3c
{
    TXBYTE(0xcc);
    TXBYTE(0x33);
    TXBYTE(0xc3);
    TXBYTE(0x3c);
}

/***********************************************************************************************
* Function		: JcSpeedprints
* Description	: ��ʾ�ַ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void JcSpeedprints(INT8U size,INT16U x,INT16U y,INT8U *s)
{
    INT8U String[4] = {0x00};
    INT8U i = 0;
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(size); //0x53=8����ASKII��0x54=16�����ַ�����0x55=32����
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x����
    TXWORD(y); //y����
    JcSpeed_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: JcPwmprints
* Description	: ��ʾ�ַ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void JcPwmprints(INT8U size,INT16U x,INT16U y,INT8U *s)
{
    INT8U String[4];
    INT8U i = 0;
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(size); //0x53=8����ASKII��0x54=16�����ַ�����0x55=32����
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x����
    TXWORD(y); //y����
    JcPWM_bin_to_char(s,String);
    while(String[i])
    {
        TXBYTE(String[i]);
        i++;
    }
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_5ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: Clear
* Description	: ����̶�������ַ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void Clear(INT8U size,INT16U x,INT16U y)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x5A); //0x53=8����ASKII��0x54=16�����ַ�����0x55=32����
    if(x==0)
    {
        x=1;
    }
    TXWORD(x); //x����
    TXWORD(y); //y����
    TXWORD(x+size); //x����
    TXWORD(y+size); //x����
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_1ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: handshake
* Description	: ����̶�������ַ���
* Input			: size �����С
                  x     ������
                  y     ������
                  s     ��ʾ���ַ���������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void handshake(void)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x00); //����Э��
    frame_end(); //����֡������
#ifdef __UART2_ENABLE
    BSP_UARTWrite(UART2,SendToUART2,count);
#else
    BSP_UARTWrite(UART0,SendToUART2,count);
#endif //__UART2_ENABLE
    count = 0;
    OSTimeDly(SYS_DELAY_1ms); //ȷ��������ϣ�������
}

/***********************************************************************************************
* Function		: setcolor
* Description	: ����ǰ��ɫ�ͱ���ɫ
* Input			: color1 ǰ��ɫ
                  color2 ����ɫ
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void setcolor(INT16U color1,INT16U color2)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(0x40); //������ɫ
    TXWORD(color1); //x����
    TXWORD(color2); //y����
    frame_end(); //����֡������
}

/***********************************************************************************************
* Function		: fillw
* Description	: �������
* Input			: color1 ǰ��ɫ
                  color2 ����ɫ
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void fillw(INT8U code,INT16U x1,INT16U y1,INT16U x2,INT16U y2)
{
    TXBYTE(0xAA); //֡ͷ0xAA
    TXBYTE(code); //���þ��ο�
    TXWORD(x1); //x����
    TXWORD(y1); //y����
    TXWORD(x2); //x����
    TXWORD(y2); //y����
    frame_end(); //����֡������
}

/********************************************************************
* Function Name : DisplayMianMenu() 
* Description   : ���˵���ʾ����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayMianMenu(struct st_menu* pMenu, INT8U cursor)
{
    gps_CurMenu = pMenu;//��ǰ�˵�ָ�븳ֵ

    if(pMenu->Menu_Flag==0)//��ʾ��ҳ
    {	
        MainMenuStatus = INMAINMENU;
        printBackImage(pMenu->FrameID);
        //��ʾ���˵�
        PraperMianMenu();      
    }
}

/********************************************************************
* Function Name : DisplayCommonMenu() 
* Description   : ��ͨ�˵���ʾ����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayCommonMenu(struct st_menu* pMenu, INT8U cursor)
{    
    gps_CurMenu = pMenu;//��ǰ�˵�ָ�븳ֵ

    printBackImage(pMenu->FrameID);
}

/********************************************************************
* Function Name : PraperAuthenticationDataSettings() 
* Description   : ��α������ʾ��������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperAuthenticationDataSettings()
{   
    INT8U horizontal = 0;
    INT8U vertical = 0;  
    INT8U AuthenticationItem1[3][10][8] = {{{"����"},{"�ױ�"},{"�汾"},{"��ֵ"},{"����"},{"�¾�"},{"D06"},{"D07"},{"D08"},{"D09"}},
                                          {{"D10"},{"D11"},{"D12"},{"D13"},{"D14"},{"D15"},{"D16"},{"D17"},{"D18"},{"D19"}},
                                          {{"D20"},{"D21"},{"D22"},{"D23"},{"D24"},{"D25"},{"D26"},{"D27"},{"D28"},{"D29"}},};
   
    INT8U AuthenticationItem2[3][10][8] = {{{"D30"},{"D31"},{"D32"},{"D33"},{"D34"},{"D35"},{"D36"},{"D37"},{"D38"},{"D39"}},
                                          {{"D40"},{"D41"},{"D42"},{"D43"},{"D44"},{"D45"},{"D46"},{"D47"},{"D48"},{"D49"}},
                                          {{"D50"},{"D51"},{"D52"},{"D53"},{"D54"},{"D55"},{"D56"},{"D57"},{"D58"},{"D59"}},};

    INT8U AuthenticationItem3[3][10][8] = {{{"D60"},{"D61"},{"D62"},{"D63"},{"D64"},{"D65"},{"D66"},{"D67"},{"D68"},{"D69"}},
                                          {{"D70"},{"D71"},{"D72"},{"D73"},{"D74"},{"D75"},{"D76"},{"D77"},{"D78"},{"D79"}},
                                          {{"D80"},{"D81"},{"D82"},{"D83"},{" "},{" "},{" "},{" "},{" "},{" "}},};

    
    //����ɫ
    setcolor(0x0000,0x0000);
    
    switch(AuthenticationDataPages)
    {
        case 0x00:
            for(vertical = 0;vertical < 3;vertical++)
            {
                for(horizontal = 0;horizontal < 10;horizontal++)
                {
                    printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem1[vertical][horizontal][0]);
                }
            }
            PraperParmSet();
            break;
        case 0x01:
            for(vertical = 0;vertical < 3;vertical++)
            {
                for(horizontal = 0;horizontal < 10;horizontal++)
                {
                    printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem2[vertical][horizontal][0]);
                }
            }
            PraperParmSet();
            break;
        case 0x02:
            for(vertical = 0;vertical < 2;vertical++)
            {
                for(horizontal = 0;horizontal < 10;horizontal++)
                {
                    printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem3[vertical][horizontal][0]);
                }
            }
            for(horizontal = 0;horizontal < 4;horizontal++)
            {
                printCharStrings16(BaseEditParam[horizontal][(vertical+1)*4-4],&AuthenticationItem3[vertical][horizontal][0]);
            }
            PraperParmSet();
            break;
        default:
          break;
    }  
}

/********************************************************************
* Function Name : PraperSpeedSetMenu() 
* Description   : �ٶȵ��Բ˵�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperSpeedSetMenu()
{       
    //8��������ʾ
    setcolor(0x0000,0xFFFF);
    printsstring32(3,5,"����");
    printsstring32(3,68,"����");
  
    printsstring32(400,5,"+1");
    printsstring32(400,68,"-1");
    
    printsstring32(400,136,"����");
    printsstring32(400,204,"����");
    
    
    //���
    printsstring32(80,32,"����");
    printsstring32(80,76,"����");
    
    printsstring32(294,2,"PWM");
    printsstring32(180,2,"SPEED");
    
    
    printsstring32(60,152,"����");    
    printsstring32(60,196,"����");
    
    //��Ӧ��ת����Ϣ
    printsstring16(150,125,"750");
    printsstring16(210,125,"850");
    printsstring16(270,125,"950");
    printsstring16(320,125,"1050");    
}

/********************************************************************
* Function Name : PraperMcDebugMainMenu() 
* Description   : �ٶȵ��Բ˵�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMcDebugMainMenu()
{   
  
  	//����������ɫ
	setcolor(0x0000,0xFFFF);
    
    //����ɫ    
    printsstring16(3,5,"����ٶ�");
    printsstring16(3,68,"�����ӳ�AD");
  
    printsstring16(3,138,"����AD��");
    printsstring16(3,208,"��С��ͷAD");
    
    printsstring16(400,5,"ӫ��AD");
    printsstring16(400,68,"�߳���Ϣ");
    
    printsstring16(400,138,"CSУ��");  

    printsstring16(400,204,"����");
  
}

/********************************************************************
* Function Name : PraperInLetADMenu() 
* Description   : ��������AD��������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInLetADMenu()
{   
    //���
    printsstring32(3,32,"����");
    printsstring32(3,76,"����");
    printsstring32(3,120,"�ɱ�H");
    printsstring32(3,164,"�ɱ�A");
    printsstring32(3,208,"����");

    
    
    printsstring32(100,2,"PWM");
    printsstring32(200,2,"ԭʼֵ");
    printsstring32(200,2,"�ڵ�ֵ"); 
   
}

/********************************************************************
* Function Name : PraperIRADMenu() 
* Description   : ����D��������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperIRADMenu()
{       
    //���
    printsstring32(3,32,"L1");
    printsstring32(3,72,"R1");
    printsstring32(3,112,"L2");
    printsstring32(3,152,"R2");
    printsstring32(3,192,"L3");
    printsstring32(3,232,"R3");
    
    
    printsstring32(240,32,"L4");
    printsstring32(240,72,"R4");
    printsstring32(240,112,"L5");
    printsstring32(240,152,"R5");  
    
    printsstring16(80,2,"ԭʼֵ");
    printsstring16(170,2,"����ֵ");
    printsstring16(320,2,"ԭʼֵ"); 
    printsstring16(410,2,"����ֵ"); 
}

/********************************************************************
* Function Name : PraperMTADMenu() 
* Description   : ��С��ͷ��������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMTADMenu()
{     
    //���
    printsstring16(3,32,"MT1");
    printsstring16(3,52,"MT2");
    printsstring16(3,72,"MG1");
    printsstring16(3,92,"MG2");
    printsstring16(3,112,"MG3");
    printsstring16(3,132,"MG4");
    printsstring16(3,152,"UV1");
    printsstring16(3,172,"UV2");
    
    printsstring16(100,2,"ԭʼֵ");
    printsstring16(170,2,"����ֵ");
}

/********************************************************************
* Function Name : PraperUVADMenu() 
* Description   :ӫ�����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperUVADMenu()
{    
    //���
    printsstring16(3,32,"UV1");
    printsstring16(3,72,"UV2");
    
    printsstring16(100,2,"ԭʼֵ");
    printsstring16(170,2,"����ֵ");
}

/********************************************************************
* Function Name : PraperInfoIRADMenu() 
* Description   :�߳�IR����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInfoIRADMenu()
{       
    //���
    printsstring16(3,40,"1");
    printsstring16(3,65,"2"); 
    printsstring16(3,90,"3");
    printsstring16(3,115,"4");  
    printsstring16(3,140,"5");
    printsstring16(3,165,"6");
 
    
    printsstring16(52,2,"E1"); 
    printsstring16(101,2,"E2");
    printsstring16(150,2,"E3");  
    printsstring16(200,2,"E4");
    printsstring16(250,2,"E5");
    printsstring16(300,2,"E6");
    printsstring16(350,2,"E7");
}

/********************************************************************
* Function Name : PraperCheckCSStart() 
* Description   :��ʼCSУ��
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSStart()
{     
    //���
    printsstring32(50,100,"CSУ�鿪ʼ�����ֽ");
}

/********************************************************************
* Function Name : PraperCheckCSMenu() 
* Description   :CSУ��
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSMenu()
{      
    //���
    printsstring32(50,100,"����CSУ��ģʽ����ȴ�");
}

/********************************************************************
* Function Name : PraperCheckCSFail() 
* Description   :CSУ��ʧ��
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSFail()
{  
    //���
    printsstring32(50,100,"CSУ��ʧ�ܣ������·�ֽ");
}

/********************************************************************
* Function Name : PraperParmSet() 
* Description   :��α�����޸�
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperParmSet()
{       
    setcolor(0xFFFF,0x0000);
	
    switch(AuthenticationDataPages)
    {
        case 0x00:
            //��ʾ����
            printsstring32(100,2,"��α������һҳ");
            break;
        case 0x01:
            //��ʾ����
            printsstring32(100,2,"��α�����ڶ�ҳ");
            break;
        case 0x02:
            //��ʾ����
            printsstring32(100,2,"��α��������ҳ");
            break;
        default:
            //��ʾ����
            printsstring32(100,2,"��α������һҳ");
            break;
    }
}

/********************************************************************
* Function Name : PraperAmountOver() 
* Description   :����ֵԽ��
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperAmountOver()
{  	
    if(TemBatchNum>899)
    {
        printsstring32(100,2,"�����������ֵ̫����");
    }
    else if(TemBatchNum<101)
    {
        printsstring32(100,2,"�����������ֵ̫С��");
    }
}

/********************************************************************
* Function Name : PraperCheckCSImageSuccess() 
* Description   :CSУ��ͼ��ɹ�
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSImageSuccess()
{      
    //����ַ���ӡ
    setcolor(0xFFFF,0x0000);    
    
    //���
    printsstring32(50,100,"CSУ��ͼ��ɹ�,����������");
}

/********************************************************************
* Function Name : PraperCheckCSStage1Success() 
* Description   :CSУ���һ�׶γɹ�
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSStage1Success()
{      
    //���
    printsstring32(50,100,"CSУ���һ�׶γɹ����������ֽ,�ٰ���������");
}

/********************************************************************
* Function Name : PraperCheckCSStage2Start() 
* Description   :CSУ��ڶ��׶ο�ʼ
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void PraperCheckCSStage2Start()
{      
    //���
    printsstring32(50,100,"CSУ��ڶ��׶ο�ʼ");
}

/********************************************************************
* Function Name : PraperInfoMTADMenu() 
* Description   :�߳�MT����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInfoMTADMenu()
{     
    //���
    printsstring16(240,2,"MT");
    
    printsstring16(40,25,"�������");
    printsstring16(240,25,"�����ֵ");
}

/********************************************************************
* Function Name : PraperInfoMGADMenu() 
* Description   :�߳�MG����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInfoMGADMenu()
{      
    //���
    printsstring16(3,32,"MT1");
    printsstring16(3,52,"MT2");
    printsstring16(3,72,"MG1");
    printsstring16(3,92,"MG2");
    printsstring16(3,112,"MG3");
    printsstring16(3,132,"MG4");
    printsstring16(3,152,"UV1");
    printsstring16(3,172,"UV2");
    
    printsstring16(80,2,"1");
    printsstring16(130,2,"2");
    printsstring16(180,2,"3");
    printsstring16(230,2,"4");
    printsstring16(280,2,"5");
    printsstring16(330,2,"6");
    printsstring16(380,2,"7");
}

/********************************************************************
* Function Name : PraperSpeedSetMenu() 
* Description   : �ٶȵ��Բ˵�����ģʽ
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperSpeedSetMenuINMode()
{  
    //����ģʽ��ʾ
    setcolor(0xF800,0xFFE0);
    printsstring32(3,5,"����");
    
}

/********************************************************************
* Function Name : PraperSpeedSetMenu() 
* Description   : �ٶȵ��Բ˵�����ģʽ
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperSpeedSetMenuOutMode()
{  
    //����ģʽ��ʾ
    setcolor(0xF800,0xFFE0);
    printsstring32(3,68,"����");
}

/********************************************************************
* Function Name : DisplayDebugMenu() 
* Description   : ���Բ˵���ʾ
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperOldTest()
{      
    //����������ɫ
	setcolor(0x0000,0xFFFF);
    
    //���
    printsstring32(70,72,"����");
    printsstring32(70,110,"���");
    printsstring32(70,148,"�����");
    printsstring32(70,186,"С���");
    printsstring32(70,224,"LCD");

    //�����
    printsstring32(190,36,"PWM");
    printsstring32(280,36,"SPEED"); 
}

/********************************************************************
* Function Name : PraperMTDispTest() 
* Description   : MT�������ݵ���ʾ
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMTDispTest()
{  
    setcolor(0x0000,0xFFFF);

    printsstring32(130,2,"MT���ݲ鿴");
     //���
    printsstring16(3,82,"1");
    printsstring16(3,112,"2");
    printsstring16(3,142,"3");
    printsstring16(3,172,"4");
    printsstring16(3,202,"5");
    printsstring16(3,232,"6");

    printsstring16(80,52,"1");
    printsstring16(130,52,"2");
    printsstring16(180,52,"3");
    printsstring16(230,52,"4");
    printsstring16(280,52,"5");
    printsstring16(330,52,"6");
    printsstring16(380,52,"7");      
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : �����˵�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUp()
{  
    setcolor(0x0000,0xFFFF);
    
    printsstring32(3,5,"UI ARM");
    printsstring32(3,68,"DSP");
    
    printsstring32(3,136,"ģ��");
    printsstring32(3,204,"FPGA");
    
    printsstring32(350,5,"MT ARM");
    printsstring32(350,68,"  ");
    
    printsstring32(350,136,"");
    printsstring32(350,204,"����");
      
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : �����˵�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUpING()
{  
    //����ɫ
    setcolor(0x0000,0xFFFF);
    fillw(MATRIX,0,0,479,271);
    fillw(CLEAR,3,2,478,270);
    
    printsstring32(100,130,"�����У���ȴ�");
    printsstring32(100,170,"�������ǰ�벻Ҫ�ػ�");      
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : �����˵�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUpED()
{  
    //����ɫ
    printsstring32(100,130,"�����ɹ�������");
}

/********************************************************************
* Function Name : PraperLevelUp() 
* Description   : �����˵�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLevelUpFail()
{  
    //����ɫ
    printsstring32(100,130,"����ʧ������������������");
}


/********************************************************************
* Function Name : PraperPasswordError() 
* Description   : �������˵�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperPasswordError()
{      
    setcolor(0xFFFF,0x0000);
    printsstring32(160,120,"�������");
}

/********************************************************************
* Function Name : PraperNetPostTime() 
* Description   : ����ʱ������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperNetPostTime()
{  
    printsstring32(180,150,"����ʱ��");
}

/********************************************************************
* Function Name : PraperNetPostPages() 
* Description   : ������������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperNetPostPages()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"��������");
}

/********************************************************************
* Function Name : PraperBlankName() 
* Description   : ���м��
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBlankName()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"���м��");
}

/********************************************************************
* Function Name : PraperAreaCode() 
* Description   : ������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperAreaCode()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"������");
}

/********************************************************************
* Function Name : PraperBranchNumber() 
* Description   : ֧�к�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBranchNumber()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"֧�к�");
}

/********************************************************************
* Function Name : PraperNetworkNumber() 
* Description   : �����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperNetworkNumber()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"�����");
}

/********************************************************************
* Function Name : PraperIPNumber() 
* Description   : IP
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperIPNumber()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"IP");
}

/********************************************************************
* Function Name : PraperBanTechnician1() 
* Description   : ��ֹ����1
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBanTechnician1()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"��ֹ����1");
}

/********************************************************************
* Function Name : PraperBanTechnician2() 
* Description   : ��ֹ����2
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBanTechnician2()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"��ֹ����2");
}

/********************************************************************
* Function Name : PraperBanTechnician3() 
* Description   : ��ֹ����2
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBanTechnician3()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"��ֹ����3");
}

/********************************************************************
* Function Name : PraperMask() 
* Description   : Mask
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMask()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"Mask");
}

/********************************************************************
* Function Name : PraperGateWay() 
* Description   : GateWay
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperGateWay()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"GateWay");
}

/********************************************************************
* Function Name : PraperLocalPort() 
* Description   : ���ض˿�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperLocalPort()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"���ض˿�");
}

/********************************************************************
* Function Name : PraperDistalPort() 
* Description   : Զ�˶˿�
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperDistalPort()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"Զ�˶˿�");
}

/********************************************************************
* Function Name : PraperBlackList() 
* Description   : ������
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBlackList()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"������");
}

/********************************************************************
* Function Name : PraperBlackListPage() 
* Description   : ������ҳ��
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperBlackListPage()
{  
    setcolor(0xFFFF,0x0000);
	switch(BlackListPage)
	{
		case 0x00:
  			printsstring16(370,6,"��һҳ");
			break;
		case 0x01:
			printsstring16(370,6,"�ڶ�ҳ");
			break;
		default:
			printsstring16(370,6,"��һҳ");
			break;
	}
}


/********************************************************************
* Function Name : PraperDistalIP() 
* Description   : Զ��IP
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperDistalIP()
{  
    //����ɫ
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,180,150,307,182);
    
    setcolor(0xFFFF,0x0000);
    printsstring32(180,150,"Զ��IP");
}

/********************************************************************
* Function Name : PraperInformation3Page() 
* Description   : ��ʾ�û���Ϣ�ĵ���ҳ
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void PraperInformation3Page()
{    
    setcolor(0xFFFF,0x0000);
    fillw(CLEAR,88,27,184,244);
    printsstring16(117,36,"MASK ");
    
    printsstring16(117,64,"GW ");
    
    printsstring16(117,96,"���ض˿�");
    
    printsstring16(117,125,"Զ�˶˿�");
    
    printsstring16(117,155,"Զ��IP");

}

/********************************************************************
* Function Name : DisplayCommonMenu() 
* Description   : ��ͨ�˵���ʾ����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayStartMenu(struct st_menu* pMenu, INT8U cursor)
{
    printBackImage(30);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(31);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(32);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(33);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(34);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(35);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(36);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(37);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(38);
    OSTimeDlyHMSM(0,0,0,500);
    printBackImage(39);
    OSTimeDlyHMSM(0,0,2,0);
}

/********************************************************************
* Function Name : DisplayPasswordMenu() 
* Description   : ������ʾ����
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayPasswordMenu(INT8U cursor)
{
        setcolor(0xFFFF,0x0000);
        switch(cursor)
        {
            case 0x01:
                printsstring32(160,128,"*");
                break;
            case 0x02:
                printsstring32(200,128,"*");
                break;
            case 0x03:
                printsstring32(240,128,"*");
                break;
            case 0x04:
                printsstring32(280,128,"*");
                break;
            default:
                break;
        }
}
/********************************************************************
* Function Name : DisplayParaMenu() 
* Description   : ��ʾ����
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayParaMenu(void)
{
    INT8U Param[12] = {0x10,0x11,0x12,0x13,0x21,0x22,0x23,0x24,0x31,0x32,0x33,0x34};
    
    INT8U horizontal = 0;
    INT8U vertical = 0;
    for(horizontal = 0;horizontal < 10;horizontal++)
    {
        for(vertical = 0;vertical < 10;vertical++)
        {
            FetchBackColor(EDITPARAMCOLOR01);
            printStrings16(BaseCheckParam[horizontal][vertical],&Param[vertical]);
        }
    }
}

/********************************************************************
* Function Name : DisplayEditParaMenu() 
* Description   : ��ʾ���޸Ĳ���
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayEditParaMenu(void)

{
    INT8U horizontal = 0;
    INT8U vertical = 0;
    INT8U i = 0;
    DisplayCursor();
      
    //����ɫ
    setcolor(0x0000,0x0000);    

    if(AuthenticationDataPages == 2)
    {
        for(vertical = 0;vertical < 2;vertical++)
        {
            for(horizontal = 0;horizontal < 10;horizontal++)
            {
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3];
                if(i>99)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3] = 99;
                    i = 99;
                }
                print2charStrings16(BaseEditParam[horizontal][vertical*4+1],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+1];
                printStrings16(BaseEditParam[horizontal][vertical*4+2],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2];
                if(i>1)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2] = 0;
                    i = 0;
                }
                print1charStrings16(BaseEditParam[horizontal][vertical*4+3],&i);
            }
        }
        for(horizontal = 0;horizontal < 4;horizontal++)
        {
            i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3];
            if(i>99)
            {
                AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3] = 99;
                i = 99;
            }
            print2charStrings16(BaseEditParam[horizontal][vertical*4+1],&i);
            i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+1];
            printStrings16(BaseEditParam[horizontal][vertical*4+2],&i);
            i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2];
            if(i>1)
            {
                AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2] = 0;
                i = 0;
            }
            print1charStrings16(BaseEditParam[horizontal][vertical*4+3],&i);
        }
    }
    else
    {
        for(vertical = 0;vertical < 3;vertical++)
        {
            for(horizontal = 0;horizontal < 10;horizontal++)
            {
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3];
                if(i>99)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3] = 99;
                    i = 99;
                }
                print2charStrings16(BaseEditParam[horizontal][vertical*4+1],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+1];
                printStrings16(BaseEditParam[horizontal][vertical*4+2],&i);
                i = AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2];
                if(i>1)
                {
                    AuthenticationDataSettings[AuthenticationDataPages][vertical*30+horizontal*3+2] = 0;
                    i = 0;
                }
                print1charStrings16(BaseEditParam[horizontal][vertical*4+3],&i);
            }
        }
    }
}

/********************************************************************
* Function Name : DisplayCursor() 
* Description   : ������
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayCursor(void)
{
    gs_CursorInPage.XCoor = BaseEditParam[Paramhorizontal][Paramvertical].sXMin;
    gs_CursorInPage.YCoor = BaseEditParam[Paramhorizontal][Paramvertical].sYMin;
    PrintCursor(gs_CursorInPage);
}

/********************************************************************
* Function Name : ClearCursor() 
* Description   : ����ԭ�й��
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearCursor(void)
{
    gs_CursorInPage.XCoor = BaseEditParam[Paramhorizontal][Paramvertical].sXMin;
    gs_CursorInPage.YCoor = BaseEditParam[Paramhorizontal][Paramvertical].sYMin;
    CloseCursor(gs_CursorInPage);
}

/********************************************************************
* Function Name : DisplayClearLevelmark() 
* Description   : ��ֵȼ��ͼ�α�ȼ�ѡ�й�����
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayClearLevelmark()
{
    if(gps_CurMenu->FrameID == 6)
    {
        //��ֵȼ�
        tRectangle Checkmark,DispMark;
        DispMark.sXMin = BaseBlackListCoordinate[1].sXMin;
        DispMark.sXMax = BaseBlackListCoordinate[1].sXMax-5;
        DispMark.sYMin = BaseBlackListCoordinate[1].sYMin;
        DispMark.sYMax = BaseBlackListCoordinate[1].sYMax-4;
        
        Checkmark.sXMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMax;
        printIcon(BLACKLISTMARK,DispMark,Checkmark);
    
    }
    else if(gps_CurMenu->FrameID == 7)
    {
        //��α�ȼ�
        tRectangle Checkmark,DispMark;
        DispMark.sXMin = BaseBlackListCoordinate[1].sXMin+2;
        DispMark.sXMax = BaseBlackListCoordinate[1].sXMax-6;
        DispMark.sYMin = BaseBlackListCoordinate[1].sYMin;
        DispMark.sYMax = BaseBlackListCoordinate[1].sYMax-4;
        
        Checkmark.sXMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMax;
        printIcon(BLACKLISTMARK,DispMark,Checkmark);
    }

}

/********************************************************************
* Function Name : ClearClearLevelmark() 
* Description   : ��ֵȼ��ͼ�α�ȼ�ѡ�й�����
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearClearLevelmark()
{
    tRectangle Checkmark;
    if(gps_CurMenu->FrameID == 6)
    {
        //��ֵȼ�
        Checkmark.sXMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseCleanLevelSettingsSelectCoordinate[ClearLevelhorizontal].sYMax;
        printIcon(BLACKLISTBACK,Checkmark,Checkmark);
    }
    else if(gps_CurMenu->FrameID == 7)
    {
        //��ֵȼ�
        Checkmark.sXMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMin;
        Checkmark.sXMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sXMax;
        Checkmark.sYMin = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMin;
        Checkmark.sYMax = BaseAuthenticationSettingsSelectCoordinate[AuthenticationLevelhorizontal].sYMax;
        printIcon(BLACKLISTBACK,Checkmark,Checkmark);
    
    }
}

/********************************************************************
* Function Name : DisplayCheckmark() 
* Description   : �������˵�ѡ�й�����
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayCheckmark(void)
{
    tRectangle Checkmark,DispMark;
    DispMark.sXMin = BaseBlackListCoordinate[1].sXMin;
    DispMark.sXMax = BaseBlackListCoordinate[1].sXMax;
    DispMark.sYMin = BaseBlackListCoordinate[1].sYMin;
    DispMark.sYMax = BaseBlackListCoordinate[1].sYMax;
    
    Checkmark.sXMin = BaseBlackListCoordinate[BlackListhorizontal].sXMin;
    Checkmark.sXMax = BaseBlackListCoordinate[BlackListhorizontal].sXMax;
    Checkmark.sYMin = BaseBlackListCoordinate[BlackListhorizontal].sYMin;
    Checkmark.sYMax = BaseBlackListCoordinate[BlackListhorizontal].sYMax;
    printIcon(BLACKLISTMARK,DispMark,Checkmark);
}

/********************************************************************
* Function Name : ClearCheckmark() 
* Description   : �������˵�ѡ�й�����
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearCheckmark(void)
{
    tRectangle Checkmark;
    Checkmark.sXMin = BaseBlackListCoordinate[BlackListhorizontal].sXMin;
    Checkmark.sXMax = BaseBlackListCoordinate[BlackListhorizontal].sXMax;
    Checkmark.sYMin = BaseBlackListCoordinate[BlackListhorizontal].sYMin;
    Checkmark.sYMax = BaseBlackListCoordinate[BlackListhorizontal].sYMax;
    printIcon(BLACKLISTBACK,Checkmark,Checkmark);
}

/********************************************************************
* Function Name : DisplayKeyBoard() 
* Description   : ����ѡ��״̬����
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayKeyBoard(void)
{
    tRectangle KeyBoard;
    KeyBoard.sXMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMin;
    KeyBoard.sXMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMax;
    KeyBoard.sYMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMin;
    KeyBoard.sYMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMax;
    printIcon(KEYBOARDACRTIVE,KeyBoard,KeyBoard);
}

/********************************************************************
* Function Name : ClearKeyBoard() 
* Description   : ����ѡ��״̬����
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void ClearKeyBoard(void)
{
    tRectangle KeyBoard;
    KeyBoard.sXMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMin;
    KeyBoard.sXMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sXMax;
    KeyBoard.sYMin = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMin;
    KeyBoard.sYMax = BaseKeyBoardSettingsCoordinate[KeyBoardhorizontal][KeyBoardvertical].sYMax;
    printIcon(KEYBOARDIMAGE,KeyBoard,KeyBoard);
}


/***********************************************************************************************
* Function		: printsstring16
* Description	: ��ʾ16*16��С���ַ���
* Input			:   x       ������
                    y       ������
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/
void printsstring16(INT16U x,INT16U y,INT8U *s)
{
    prints(0x54,x,y,s);
}


/********************************************************************
* Function Name : DisplayInfomationList() 
* Description   : ��ʾ��ǰѡ�еڼ���
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayInfomationList(void)
{
    gs_CursorInPage.XCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sXMin;
    gs_CursorInPage.YCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sYMin;
    PrintCursor(gs_CursorInPage);
    InformationDisp();
}

/********************************************************************
* Function Name : DisplayInfomationList() 
* Description   : ��ʾ��ǰѡ�еڼ���
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void CleanInfomationList(void)
{
    gs_CursorInPage.XCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sXMin;
    gs_CursorInPage.YCoor = BaseInfomationCoordinate[InfomationPage][InfomationList].sYMin;
    CloseCursor(gs_CursorInPage);
}

/********************************************************************
* Function Name : InformationDisp() 
* Description   : �û���Ϣ��ʾ
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void InformationDisp(void)
{   
    setcolor(0xFFFF,0x0000);
    
    switch(InfomationPage)
    {
        case 0x00:
            //����ʱ��
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][0],NetPostTime);
            //��������
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][1],NetPostPage);
            //���м��
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][2],BlankName);
            //������
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][3],AreaNum);
            //֧�к�
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][4],BranchNum);
            //�����
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][5],LatticePoint);
            //IP
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][6],IP);
            break;
        case 0x01:
            //MAC
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][0],Mac);
            //�������к�
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][1],MachineNo);
            //��ֹ����1
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][2],BanTechnician[0]);
            //��ֹ����2
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][3],BanTechnician[1]);
            //��ֹ����3
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][4],BanTechnician[2]);
            //ʱ��
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][5],Time);
            break;
        case 0x02:
            //MASK
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][0],Mask);
            //GW
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][1],GateWay);
            //���ض˿�
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][2],LocalPort);
            //Զ�˶˿�
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][3],DiskPort);
            //Զ��IP
            printVersionStrings32(BaseInfomationCoordinate[InfomationPage][4],DiskIP);
            break;   
        default:
            break; 
    }
}

/********************************************************************
* Function Name : blackListDisp() 
* Description   : ��������ʾ
* Input         : 
* Output        : None.
* Return        : None.
********************************************************************/
void blackListDisp(void)
{   
    INT8U i = 0;
    
    setcolor(0xFFFF,0x0000);
    
    for(i=0;i<7;i++)
    {
        //����������
        printVersionStrings32(BaseBlackListNumberCoordinate[i],BlackLists[BlackListPage][i]);
    }
}

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/
