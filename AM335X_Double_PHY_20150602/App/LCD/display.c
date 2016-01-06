/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: display.c
* Author			: ��ҫ
* Date First Issued	: 02/20/2014
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "guisprint.h"
#include "display.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
INT8U Setdata[30];//���ò��������ȫ�ֱ���
INT16U MeasNum=0;//����������
SetDec gps_data={64,1,16,8,1,0,0,0,Setdata};//����ҳ���ýṹ��
ST_Menu *gps_CurMenu;     //��ǰ�˵�
INT8U  gCursorInMenu;    //����ڵ�ǰ�˵��е����
ST_Cursor  gs_CursorInPage;   //��ǰҳ�еĹ��λ��
//���¼�ͷ����ǰ2�����ݱ�ʾͼ�εĳ���,����Ϊͼ������
//static const INT8U downdata[]={0x08,0x08,0x10,0x20,0x40,0xff,0x40,0x20,0x10,0x00};
//static const INT8U updata[]={0x08,0x08,0x08,0x04,0x02,0xff,0x02,0x04,0x08,0x00};
INT32S (*LcdPage)(INT16U *MeterNo,INT32U type);//ҳ����ʾ����ָ��
/* Private function prototypes -----------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/********************************************************************
* Function Name : DisplayAll()
* Description   : ��ʾ�������,���ݲ˵����Բ�ͬ��ʾ�˵�,ҳ��,����ҳ��.
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayAll(struct st_menu* pMenu, INT8U cursor)
{    
	INT8U i=0;
//	INT32S index=0;
    
    LcdPage=NULL;//����ָ���ʼ��
    gps_CurMenu=pMenu;//��ǰ�˵�ָ�븳ֵ
	LcdPage=*gps_CurMenu->funpage;//����ָ�븳ֵ
    gps_data.count=0;//��������

    if((pMenu->Menu_Flag==0)||(pMenu->Menu_Flag==1))//��ʾ��ҳ,������ҳ����˵�
	{	
		gCursorInMenu = cursor;//����ڵ�ǰ�˵��е����
    	if(cursor > (pMenu->Menu_Maxcursor-1)) //�����곬����Χ
    		return;
        BSP_GUIFill(0,0,LCD_WIDTH-1,LCD_HEIGHT-1,ClrDarkBlue);
    	gs_CursorInPage.YCoor = MENU_Y_START;//�������λ��
    	gs_CursorInPage.XCoor = MENU_X_START;
    	//��ʾ�ڸ�ҳ
        GrContextBackgroundSet(&sContext,ClrDarkBlue);
        GrContextFontSet(&sContext, &g_sFontCH24);
        GrContextForegroundSet(&sContext, ClrWhite);
		for(i=0;i<gps_CurMenu->Menu_Maxcursor;i+=2)
		{
			GrStringDraw(&sContext,pMenu->Menu_pContent[i],4,MENU_X_START,MENU_Y_START+30*i,0);
            if((i+1)<=(gps_CurMenu->Menu_Maxcursor-1))
            {
                GrStringDraw(&sContext,pMenu->Menu_pContent[i+1],4,MENU_X_START+370,MENU_Y_START+30*i,0);
            }
		}
		BSP_UpdataLCD(); 
	}
    else
    {
        return;   
    }
	

}

/************************(C)COPYRIGHT 2010 �㽭����****END OF FILE****************************/
