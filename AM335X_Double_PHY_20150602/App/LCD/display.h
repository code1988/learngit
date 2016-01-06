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
#define CHAR_X_WITH   8   //每个字所占的高
#define CHAR_Y_WITH   16  //每个半角字的长度

#define MENU_X_START  10    //菜单显示x轴定位第四个字符位开始
#define MENU_Y_START  50    //菜单显示y轴定位第三行开始
#define MENU_Y_OFFSET 24    //每条菜单显示的跨度

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
	struct st_menu* Menu_UpMenu;     //上页页面结构数组
    struct st_menu* Menu_DownMenu;   //下页页面结构数组
    const char **Menu_pContent;              //当前菜单的子菜单名称    
    const INT8U Menu_Maxcursor;               //当前菜单的子菜单个数
    INT8U Menu_cursor;                  //光标所在的子菜单序号
	INT8U Menu_Flag; /*菜单属性 0-菜单  ; 1-子页面; 2-设置页面(21:设置密码；22设置电量；23设置时间；24设置金额)*/
	INT32S (*funpage)(INT16U*MeterNo,INT32U type);//当前需显示的页面函数指针列表的地址
	void (*function)(struct st_menu* pMenu, INT8U cursor);//,unsigned char type);	//在当前光标在icon时按下enter键执行的函数
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
    INT16U XCoor;           //光标的x坐标
    INT16U YCoor;           //光标的y坐标
}ST_Cursor;
extern ST_Cursor  gs_CursorInPage;   //当前页中的光标位置
typedef struct ELECTRIFYMODE
{
INT8U Cursor; //当前光标位置
INT8U Mode;	//0x33:立即充电 0x66:谷电充电（晚上十点后充电）：0x55:固定充值时间到了
INT8U WaitType;//什么模式下
}__ELECTRIFYMODE;
extern __ELECTRIFYMODE  ElectrifyMode;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
extern ST_Menu gsMenu0;
extern ST_Menu gsMenu0_1;
extern ST_Menu gsMenu0_2;
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
/***********************************************************************************************
* Function		: Displaymainpage
* Description	: 主页面显示
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Displaymainpage(void);
/***********************************************************************************************
* Function		: DisplayElectrifyRunning 
* Description	: 正在充电界面
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void DisplayElectrifyRunning(void);
#endif	//__DISPLAY_H_
/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/
