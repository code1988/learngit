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
#define DOUBLE        3
#define X_MIN         0     //x��Сֵ�������꣩
#define X_MAX         480     //x���ֵ�������꣩
#define Y_MIN         0     //y��Сֵ�������꣩
#define Y_MAX         272     //y���ֵ�������꣩

#define CHAR_X_WITH   8   //ÿ������ռ�ĸ�
#define CHAR_Y_WITH   16  //ÿ������ֵĳ���

#define TABSTRING_X_WITH   3   //�����ÿ��������������ߵļ��
#define TABSTRING_Y_WITH   3  //�����ÿ����������Ϻ��ߵļ��

//������������ڼ��еڼ��У�Ϊ�����������㣬����������������ʵ����һ
#define FIRSTLINE 0         //��һ��
#define SECONDLINE 1          //�ڶ���
#define THIRDLINE 2           //������
#define FOURTHLINE 3           //������
#define FIFTHLINE 4           //������
#define SIXTHLINE 5           //������
#define SEVENTHLINE 6           //������
#define EIGHTHLINE 7           //�ڰ���
#define NINTHLINE 8           //�ھ���
#define FIRSTCOLUMN 0          //��һ��
#define SECONDCOLUMN 1          //�ڶ���
#define THIRDCOLUMN 2           //������
#define FOURTHCOLUMN 3           //������
#define FIFTHCOLUMN 4          //������
#define SIXTHCOLUMN 5           //������
#define SEVENTHCOLUMN 6           //������
#define EIGHTHCOLUMN 7           //�ڰ���
#define NINTHCOLUMN 8           //�ھ���

#define FONTSIZE 24  //���������С
#define MENU_X_START  10    //�˵���ʾx�ᶨλ���ĸ��ַ�λ��ʼ
#define MENU_Y_START  10    //�˵���ʾy�ᶨλ�����п�ʼ
#define MAINMENU_LINE_SPACING  30 //���˵��м��(������)
#define MAINMENU_LINE_STRING_SPACING  28 //���˵��м��(�ַ���)
#define MAINMENU_COLUMN_SPACING  210 //���˵��м��
#define PWM_X_START  10    //���˵������ڵĳ�ʼx����
#define PWM_Y_START  125    //���˵������ڵĳ�ʼy����
#define DOUBFUL_X_START  10    //���˵�״̬����ʼx����
#define DOUBFUL_Y_START  247    //���˵�״̬����ʼy����
#define DOUBFUL_COLUMN_SPACING  70    //���˵�״̬�����м��
#define LIST_X_START  135    //list��Ϣ�ĳ�ʼx����
#define LIST_Y_START  50    //list��Ϣ�ĳ�ʼy����
#define LISTPARAM_X_START  255    //list�����ĳ�ʼx����
#define LISTPARAM_Y_START  50    //list�����ĳ�ʼy����
#define MENU_Y_OFFSET 24    //ÿ���˵���ʾ�Ŀ��
#define SUMPWM 2  //����������
#define SUMFUNC 3  //����ͼ��
#define SUMDOUBFUL 2  //����������
#define LISTPARAM  8  //List�˵����ߵ�������ʾ��
#define UPSIGN    1   //�������I���ص��ź�
#define DOWNSIGN    2   //�������I���ص��ź�
#define CANCEl_KEY    3   //��cancel�I�ش����ź�
#define OK_KEY    4   //��OK�I�ش����ź�
#define ADD_KEY    5   //��OK�I�ش����ź�
#define REDUCE_KEY    6   //��OK�I�ش����ź�
#define BACK_KEY   8   //��back�I�ش����ź�
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
    const INT8U Menu_Maxcursor;               //��ǰ�˵����Ӳ˵�����
    INT8U Menu_cursor;                  //������ڵ��Ӳ˵����
	INT8U Menu_Flag; /*�˵����� 0-�˵�  ; 1-��ҳ��; 2-����ҳ��(21:�������룻22���õ�����23����ʱ�䣻24���ý��)*/
	INT32S (*funpage)(INT16U*MeterNo,INT32U type);//��ǰ����ʾ��ҳ�溯��ָ���б�ĵ�ַ
	void (*function)(struct st_menu* pMenu, INT8U cursor);//,unsigned char type);	//�ڵ�ǰ�����iconʱ����enter��ִ�еĺ���
    const int FrameID;                      //��ǰ����ID
    void (*function2)(INT8U *keyval);      //��ǰ����İ�������
    void (*function3)(struct st_menu* pMenu);        //��ǰ�������Ҫ�����Ӻ����л�ȡȻ����
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
    INT32U XCoor;           //����x����
    INT32U YCoor;           //����y����
}ST_Cursor;
extern ST_Cursor  gs_CursorInPage;   //��ǰҳ�еĹ��λ��
typedef struct ELECTRIFYMODE
{
INT8U Cursor; //��ǰ���λ��
INT8U Mode;	//0x33:������� 0x66:�ȵ��磨����ʮ����磩��0x55:�̶���ֵʱ�䵽��
INT8U WaitType;//ʲôģʽ��
}__ELECTRIFYMODE;
typedef struct
{
    unsigned char page;   //�˵����ڼ���
    unsigned char s1;   //�˵���1���µ������
    unsigned char s2;   //�˵���2���µ������
    unsigned char s3;   //�˵���3���µ������
    unsigned char s4;   //�˵���4���µ������ 
}Menu_List;
typedef struct
{
    INT16U xstart;
    INT16U ystart;
    INT16U xend;
    INT16U yend;
}Set_Coordinate;
extern __ELECTRIFYMODE  ElectrifyMode;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
extern ST_Menu gsMenu0;
extern ST_Menu gsMenu0_1;
extern ST_Menu gsMenu0_2;
extern ST_Menu gsMenu0_3;
extern ST_Menu gsMenu0_4;
extern ST_Menu gsMenu0_5;
extern ST_Menu gsMenu0_6;
extern ST_Menu gsMenu0_7;
extern ST_Menu gsMenu0_8;
extern ST_Menu gsMenu0_9;
extern ST_Menu gsMenu0_10;
extern ST_Menu gsMenu0_11;
extern ST_Menu gsMenu0_12;

//2���˵�
extern ST_Menu gsMenu0_2_1;
extern ST_Menu gsMenu0_2_2;
extern ST_Menu gsMenu0_2_3;
extern ST_Menu gsMenu0_2_4;
extern ST_Menu gsMenu0_2_5;

extern ST_Menu gsMenu0_3_1;
extern ST_Menu gsMenu0_3_2;
extern ST_Menu gsMenu0_3_3;
extern ST_Menu gsMenu0_3_4;
extern ST_Menu gsMenu0_3_5;
extern ST_Menu gsMenu0_3_6;
extern ST_Menu gsMenu0_3_7;
extern ST_Menu gsMenu0_3_8;

extern ST_Menu gsMenu0_4_1;
extern ST_Menu gsMenu0_4_2;
extern ST_Menu gsMenu0_4_3;
extern ST_Menu gsMenu0_4_4;

//����Աǩ��
extern ST_Menu gsMenu_14;
//����Աǩ��
extern ST_Menu gsMenu_15;
//���׺�����
extern ST_Menu gsMenu_16;
//��Ϣ�鿴1
extern ST_Menu gsMenu_17;
//��Ϣ�鿴2
extern ST_Menu gsMenu_18;
//���������
extern ST_Menu gsMenu_19;
//ʱ������
extern ST_Menu gsMenu_20;
//�汾��Ϣ1
extern ST_Menu gsMenu_21;
//�汾��Ϣ2
extern ST_Menu gsMenu_22;
//�汾��Ϣ3
extern ST_Menu gsMenu_24;
//����������
extern ST_Menu gsMenu_23;
//������
extern ST_Menu gsMenu_25;
//MAC
extern ST_Menu gsMenu_26;
//��������
extern ST_Menu gsMenu_27;
//����������
extern ST_Menu gsMenu_28;
//�����˵�
extern ST_Menu gsMenu_30;
//�������Բ˵�
extern ST_Menu gsMenu_40;
//����˵�
extern ST_Menu gsMenu_41;
//IP
extern ST_Menu gsMenu_50;
//MASK
extern ST_Menu gsMenu_51;
//GW
extern ST_Menu gsMenu_52;
//���ض˿�
extern ST_Menu gsMenu_53;
//Զ�˶˿�
extern ST_Menu gsMenu_54;
//Զ��IP
extern ST_Menu gsMenu_55;
//��ֹ����ʱ��
extern ST_Menu gsMenu_56;
//���������˵�
extern ST_Menu gsMenu_79;
//�ٶȵ��Բ˵�
extern ST_Menu gsMenu_80;
//�ϻ����Բ˵�
extern ST_Menu gsMenu_81;
//������Բ˵�
extern ST_Menu gsMenu_82;
//���洫�����˵�
extern ST_Menu gsMenu_83;
//�����˵�
extern ST_Menu gsMenu_84;
//��С��ͷ
extern ST_Menu gsMenu_85;
//ӫ��
extern ST_Menu gsMenu_86;
//�߳�������Ϣ
extern ST_Menu gsMenu_87;
//�߳�MT��Ϣ
extern ST_Menu gsMenu_88;
//�߳�MG��Ϣ
extern ST_Menu gsMenu_89;
//�ϻ�������Ϣ
extern ST_Menu gsMenu_90;
//MT���ݲ鿴
extern ST_Menu gsMenu_91;



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

#endif	//__DISPLAY_H_
/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/
