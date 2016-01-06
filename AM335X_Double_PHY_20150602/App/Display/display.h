/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: display.h
* Author			: 王耀
* Date First Issued	: 02/20/2014
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__DISPLAY_H_
#define	__DISPLAY_H_
/* Includes-----------------------------------------------------------------------------------*/
/*字体显示的长宽-------------------------------------------------------*/
#define DOUBLE        3
#define X_MIN         0     //x最小值（横坐标）
#define X_MAX         480     //x最大值（横坐标）
#define Y_MIN         0     //y最小值（纵坐标）
#define Y_MAX         272     //y最大值（纵坐标）

#define CHAR_X_WITH   8   //每个字所占的高
#define CHAR_Y_WITH   16  //每个半角字的长度

#define TABSTRING_X_WITH   3   //表格中每个字与表格的左竖线的间距
#define TABSTRING_Y_WITH   3  //表格中每个字与表格的上横线的间距

//用来控制输出第几行第几列，为了配合数组计算，所以行数列数都比实际少一
#define FIRSTLINE 0         //第一行
#define SECONDLINE 1          //第二行
#define THIRDLINE 2           //第三行
#define FOURTHLINE 3           //第四行
#define FIFTHLINE 4           //第五行
#define SIXTHLINE 5           //第六行
#define SEVENTHLINE 6           //第七行
#define EIGHTHLINE 7           //第八行
#define NINTHLINE 8           //第九行
#define FIRSTCOLUMN 0          //第一列
#define SECONDCOLUMN 1          //第二列
#define THIRDCOLUMN 2           //第三列
#define FOURTHCOLUMN 3           //第四列
#define FIFTHCOLUMN 4          //第五列
#define SIXTHCOLUMN 5           //第六列
#define SEVENTHCOLUMN 6           //第七列
#define EIGHTHCOLUMN 7           //第八列
#define NINTHCOLUMN 8           //第九列

#define FONTSIZE 24  //标题字体大小
#define MENU_X_START  10    //菜单显示x轴定位第四个字符位开始
#define MENU_Y_START  10    //菜单显示y轴定位第三行开始
#define MAINMENU_LINE_SPACING  30 //主菜单行间距(划线用)
#define MAINMENU_LINE_STRING_SPACING  28 //主菜单行间距(字符用)
#define MAINMENU_COLUMN_SPACING  210 //主菜单列间距
#define PWM_X_START  10    //主菜单出钞口的初始x坐标
#define PWM_Y_START  125    //主菜单出钞口的初始y坐标
#define DOUBFUL_X_START  10    //主菜单状态栏初始x坐标
#define DOUBFUL_Y_START  247    //主菜单状态栏初始y坐标
#define DOUBFUL_COLUMN_SPACING  70    //主菜单状态栏的列间距
#define LIST_X_START  135    //list信息的初始x坐标
#define LIST_Y_START  50    //list信息的初始y坐标
#define LISTPARAM_X_START  255    //list参数的初始x坐标
#define LISTPARAM_Y_START  50    //list参数的初始y坐标
#define MENU_Y_OFFSET 24    //每条菜单显示的跨度
#define SUMPWM 2  //进钞口总数
#define SUMFUNC 3  //功能图标
#define SUMDOUBFUL 2  //进钞口总数
#define LISTPARAM  8  //List菜单两边的文字显示项
#define UPSIGN    1   //按向上鍵传回的信号
#define DOWNSIGN    2   //按向下鍵传回的信号
#define CANCEl_KEY    3   //按cancel鍵回传的信号
#define OK_KEY    4   //按OK鍵回传的信号
#define ADD_KEY    5   //按OK鍵回传的信号
#define REDUCE_KEY    6   //按OK鍵回传的信号
#define BACK_KEY   8   //按back鍵回传的信号
/* Private define-----------------------------------------------------------------------------*/
#define MENU_TOPLAYER (&gsMenu0)
/* Private typedef----------------------------------------------------------------------------*/
/*下面结构中对于Menu_pValue中的数值会实时更新的情况，还需实时监视这些值是否变化
有变化则及时刷新页面 */
/*-----菜单结构-----------------*/
typedef struct  st_menu
{
    struct st_menu* Menu_pFather;        //指向父菜单结构    
    struct st_menu** Menu_paKidsMenu;     //子菜单结构数组    
    const INT8U Menu_Maxcursor;               //当前菜单的子菜单个数
    INT8U Menu_cursor;                  //光标所在的子菜单序号
	INT8U Menu_Flag; /*菜单属性 0-菜单  ; 1-子页面; 2-设置页面(21:设置密码；22设置电量；23设置时间；24设置金额)*/
	INT32S (*funpage)(INT16U*MeterNo,INT32U type);//当前需显示的页面函数指针列表的地址
	void (*function)(struct st_menu* pMenu, INT8U cursor);//,unsigned char type);	//在当前光标在icon时按下enter键执行的函数
    const int FrameID;                      //当前画面ID
    void (*function2)(INT8U *keyval);      //当前画面的按键处理
    void (*function3)(struct st_menu* pMenu);        //当前画面的需要参数从函数中获取然后传入
}ST_Menu;
/*------------设置数据结构-------*/

//在设置菜单中count与string数组的序号一一对应.
typedef struct setdec
{
	    INT8U x;//光标起始位置
        INT8U y;
		INT8U x_wide;//x轴跨度
		INT8U y_wide;//y字节宽度
		INT8U offset;//偏移量 以字节为单位
		INT8U type;/*设置类型 1-密码确认;2-电量输入;3-时间输入*/
		INT8U count;//计数<=数组长度 
		INT8U len;//数组长度
		INT8U *String;//数组
}SetDec;
extern SetDec gps_data;//当前设置的数据结构体
/*------光标结构-----------------*/
typedef struct
{
    INT8U CursorIndex;     //光标所在icon序号
    INT32U XCoor;           //光标的x坐标
    INT32U YCoor;           //光标的y坐标
}ST_Cursor;
extern ST_Cursor  gs_CursorInPage;   //当前页中的光标位置
typedef struct ELECTRIFYMODE
{
INT8U Cursor; //当前光标位置
INT8U Mode;	//0x33:立即充电 0x66:谷电充电（晚上十点后充电）：0x55:固定充值时间到了
INT8U WaitType;//什么模式下
}__ELECTRIFYMODE;
typedef struct
{
    unsigned char page;   //菜单所在级数
    unsigned char s1;   //菜单第1级下的子项号
    unsigned char s2;   //菜单第2级下的子项号
    unsigned char s3;   //菜单第3级下的子项号
    unsigned char s4;   //菜单第4级下的子项号 
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

//2级菜单
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

//操作员签到
extern ST_Menu gsMenu_14;
//操作员签退
extern ST_Menu gsMenu_15;
//交易号输入
extern ST_Menu gsMenu_16;
//信息查看1
extern ST_Menu gsMenu_17;
//信息查看2
extern ST_Menu gsMenu_18;
//网点号输入
extern ST_Menu gsMenu_19;
//时间设置
extern ST_Menu gsMenu_20;
//版本信息1
extern ST_Menu gsMenu_21;
//版本信息2
extern ST_Menu gsMenu_22;
//版本信息3
extern ST_Menu gsMenu_24;
//黑名单设置
extern ST_Menu gsMenu_23;
//机器号
extern ST_Menu gsMenu_25;
//MAC
extern ST_Menu gsMenu_26;
//开机出错
extern ST_Menu gsMenu_27;
//黑名单入力
extern ST_Menu gsMenu_28;
//开机菜单
extern ST_Menu gsMenu_30;
//参数调试菜单
extern ST_Menu gsMenu_40;
//密码菜单
extern ST_Menu gsMenu_41;
//IP
extern ST_Menu gsMenu_50;
//MASK
extern ST_Menu gsMenu_51;
//GW
extern ST_Menu gsMenu_52;
//本地端口
extern ST_Menu gsMenu_53;
//远端端口
extern ST_Menu gsMenu_54;
//远端IP
extern ST_Menu gsMenu_55;
//禁止网发时间
extern ST_Menu gsMenu_56;
//马达调试主菜单
extern ST_Menu gsMenu_79;
//速度调试菜单
extern ST_Menu gsMenu_80;
//老化调试菜单
extern ST_Menu gsMenu_81;
//红外调试菜单
extern ST_Menu gsMenu_82;
//常规传感器菜单
extern ST_Menu gsMenu_83;
//升级菜单
extern ST_Menu gsMenu_84;
//大小磁头
extern ST_Menu gsMenu_85;
//荧光
extern ST_Menu gsMenu_86;
//走钞红外信息
extern ST_Menu gsMenu_87;
//走钞MT信息
extern ST_Menu gsMenu_88;
//走钞MG信息
extern ST_Menu gsMenu_89;
//老化测试信息
extern ST_Menu gsMenu_90;
//MT数据查看
extern ST_Menu gsMenu_91;



extern ST_Menu *gps_CurMenu;     //当前菜单
extern ST_Cursor  gs_CursorInPage;   //当前页中的光标位置
extern INT8U gCursorInMenu;    //光标在当前菜单中的序号
extern INT8U gMainDispFlag;
extern INT8U menucount; //当前菜单中菜单个数，在菜单属性为0，1的任何一个菜单中，最后一个菜单都做为返回功能 
/* Private function prototypes----------------------------------------------------------------*/
extern INT32S (*LcdPage)(INT16U *MeterNo,INT32U type);//页面显示函数指针
/* Private functions--------------------------------------------------------------------------*/
/******************************************************************** 
* Function Name : DisplayAll()
* Description   : 显示函数入口,根据菜单属性不同显示菜单,页面,设置页面.
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayAll(struct st_menu* pMenu, INT8U cursor);

#endif	//__DISPLAY_H_
/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/
