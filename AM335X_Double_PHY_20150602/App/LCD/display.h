/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: display.h
* Author			: ��ҫ
* Date First Issued	: 02/20/2014
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__DISPLAY_H_
#define	__DISPLAY_H_
/* Includes-----------------------------------------------------------------------------------*/
/*������ʾ�ĳ���-------------------------------------------------------*/
#define CHAR_X_WITH   8   //ÿ������ռ�ĸ�
#define CHAR_Y_WITH   16  //ÿ������ֵĳ���

#define MENU_X_START  10    //�˵���ʾx�ᶨλ���ĸ��ַ�λ��ʼ
#define MENU_Y_START  50    //�˵���ʾy�ᶨλ�����п�ʼ
#define MENU_Y_OFFSET 24    //ÿ���˵���ʾ�Ŀ��

/* Private define-----------------------------------------------------------------------------*/
#define MENU_TOPLAYER (&gsMenu0)
/* Private typedef----------------------------------------------------------------------------*/
/*����ṹ�ж���Menu_pValue�е���ֵ��ʵʱ���µ����������ʵʱ������Щֵ�Ƿ�仯
�б仯��ʱˢ��ҳ�� */
/*-----�˵��ṹ-----------------*/
typedef struct  st_menu
{
    struct st_menu* Menu_pFather;        //ָ�򸸲˵��ṹ    
    struct st_menu** Menu_paKidsMenu;     //�Ӳ˵��ṹ����    
	struct st_menu* Menu_UpMenu;     //��ҳҳ��ṹ����
    struct st_menu* Menu_DownMenu;   //��ҳҳ��ṹ����
    const char **Menu_pContent;              //��ǰ�˵����Ӳ˵�����    
    const INT8U Menu_Maxcursor;               //��ǰ�˵����Ӳ˵�����
    INT8U Menu_cursor;                  //������ڵ��Ӳ˵����
	INT8U Menu_Flag; /*�˵����� 0-�˵�  ; 1-��ҳ��; 2-����ҳ��(21:�������룻22���õ�����23����ʱ�䣻24���ý��)*/
	INT32S (*funpage)(INT16U*MeterNo,INT32U type);//��ǰ����ʾ��ҳ�溯��ָ���б�ĵ�ַ
	void (*function)(struct st_menu* pMenu, INT8U cursor);//,unsigned char type);	//�ڵ�ǰ�����iconʱ����enter��ִ�еĺ���
}ST_Menu;
/*------------�������ݽṹ-------*/

//�����ò˵���count��string��������һһ��Ӧ.
typedef struct setdec
{
	    INT8U x;//�����ʼλ��
        INT8U y;
		INT8U x_wide;//x����
		INT8U y_wide;//y�ֽڿ��
		INT8U offset;//ƫ���� ���ֽ�Ϊ��λ
		INT8U type;/*�������� 1-����ȷ��;2-��������;3-ʱ������*/
		INT8U count;//����<=���鳤�� 
		INT8U len;//���鳤��
		INT8U *String;//����
}SetDec;
extern SetDec gps_data;//��ǰ���õ����ݽṹ��
/*------���ṹ-----------------*/
typedef struct
{
    INT8U CursorIndex;     //�������icon���
    INT16U XCoor;           //����x����
    INT16U YCoor;           //����y����
}ST_Cursor;
extern ST_Cursor  gs_CursorInPage;   //��ǰҳ�еĹ��λ��
typedef struct ELECTRIFYMODE
{
INT8U Cursor; //��ǰ���λ��
INT8U Mode;	//0x33:������� 0x66:�ȵ��磨����ʮ����磩��0x55:�̶���ֵʱ�䵽��
INT8U WaitType;//ʲôģʽ��
}__ELECTRIFYMODE;
extern __ELECTRIFYMODE  ElectrifyMode;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
extern ST_Menu gsMenu0;
extern ST_Menu gsMenu0_1;
extern ST_Menu gsMenu0_2;
extern ST_Menu *gps_CurMenu;     //��ǰ�˵�
extern ST_Cursor  gs_CursorInPage;   //��ǰҳ�еĹ��λ��
extern INT8U gCursorInMenu;    //����ڵ�ǰ�˵��е����
extern INT8U gMainDispFlag;
extern INT8U menucount; //��ǰ�˵��в˵��������ڲ˵�����Ϊ0��1���κ�һ���˵��У����һ���˵�����Ϊ���ع��� 
/* Private function prototypes----------------------------------------------------------------*/
extern INT32S (*LcdPage)(INT16U *MeterNo,INT32U type);//ҳ����ʾ����ָ��
/* Private functions--------------------------------------------------------------------------*/
/********************************************************************
* Function Name : DisplayAll()
* Description   : ��ʾ�������,���ݲ˵����Բ�ͬ��ʾ�˵�,ҳ��,����ҳ��.
* Input         : -*pMenu,��ǰ���Բ˵��ṹ. 
*                 -cursor,���������
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayAll(struct st_menu* pMenu, INT8U cursor);
/***********************************************************************************************
* Function		: Displaymainpage
* Description	: ��ҳ����ʾ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Displaymainpage(void);
/***********************************************************************************************
* Function		: DisplayElectrifyRunning 
* Description	: ���ڳ�����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void DisplayElectrifyRunning(void);
#endif	//__DISPLAY_H_
/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/
