/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.c
* Author			: 
* Date First Issued	: 130722    
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
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
#include "display.h"
#include "grlib.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//测试用函数
void PraperMianMenu(void);
//取得背景色
void FetchBackColor(tRectangle Coordinate);
//光标更新
void DisplayCursor(void);
//消除原有光标
void ClearCursor(void);
//显示ICON的函数
void printIcon(INT8U IMAGE,tRectangle Icon,tRectangle Coordinate);
//显示背景图片
void printBackImage(INT8U IMAGE);
//显示背景图片的预处理
void PraPerprintBackImage(INT8U IMAGE);
//基本的显示字符串的函数
void prints(INT8U size,INT16U x,INT16U y,INT8U *s);
//协议相关函数
void TXBYTE( INT8U i);
void TXWORD( INT16U i);
void frame_end();
//根据字体大小显示字符串的函数
void printsstring8(INT16U x,INT16U y,INT8U *s);
void printsstring32(INT16U x,INT16U y,INT8U *s);
void printsstring12(INT16U x,INT16U y,INT8U *s);
void printsstring24(INT16U x,INT16U y,INT8U *s);
//显示参数菜单的文字的函数(1个字符)
void print1charStrings16(tRectangle Coordinate,INT8U *s);
//显示参数菜单的文字的函数(2个字符)
void print2charStrings16(tRectangle Coordinate,INT8U *s);
//显示字符串
void printStrings16(tRectangle Coordinate,INT8U *s);
//显示出钞口查看和退钞口查看的面额
void printOutLetValueStrings16(tRectangle Coordinate,INT8U *s);
//显示冠字号码字符串
void printCrownWordStrings16(tRectangle Coordinate,INT8U *s);
//主菜单显示时间
void printMainMenuTimeStrings32(tRectangle Coordinate,INT8U *s);
//显示32号大小的字
void printStrings32(tRectangle Coordinate,INT8U *s);
//直接输出16*16的字符串
void printCharStrings16(tRectangle Coordinate,INT8U *s);
//马达SPEED
void JcSpeedprints(INT8U size,INT16U x,INT16U y,INT8U *s);
//马达PWM
void JcPwmprints(INT8U size,INT16U x,INT16U y,INT8U *s);
//显示版本信息
void printVersionStrings32(tRectangle Coordinate,INT8U *s);
//混点明细查看的字符串打印函数
void printMixNums16(tRectangle Coordinate,INT8U *s);
//清除区域内字符串的函数
void Clear(INT8U size,INT16U x,INT16U y);
//握手函数
void handshake(void);
//设置前景色和背景色
void setcolor(INT16U color1,INT16U color2);
//填充区域颜色
void fillw(INT8U code,INT16U x1,INT16U y1,INT16U x2,INT16U y2);

//普通菜单的显示函数
void DisplayCommonMenu(struct st_menu* pMenu, INT8U cursor);
//开机过程中菜单的显示函数
void DisplayStartMenu(struct st_menu* pMenu, INT8U cursor);
//密码菜单星号显示
void DisplayPasswordMenu(INT8U cursor);
//主菜单显示函数
void DisplayMianMenu(struct st_menu* pMenu, INT8U cursor);

//光标更新
void DisplayInfomationList(void);
//消除原有光标
void CleanInfomationList(void);
//显示鉴伪参数的项目名
void PraperAuthenticationDataSettings();
//参数菜单内容显示
void DisplayParaMenu(void);
//显示并修改显示内容
void DisplayEditParaMenu(void);
//升级中
void PraperLevelUpING(void);
//升级结束
void PraperLevelUpED(void);
//升级失败
void PraperLevelUpFail(void);
//显示参数修改的页码
void PraperParmSet(void);
//批量值越界
void PraperAmountOver(void);
//密码出错显示
void PraperPasswordError(void);
//按键更新
void DisplayKeyBoard(void);
//消除原有光标
void ClearKeyBoard(void);
//网发时间设置
void PraperNetPostTime(void);
//网发张数设置
void PraperNetPostPages(void);
//银行简称
void PraperBlankName(void);
//地区号
void PraperAreaCode(void);
//支行号
void PraperBranchNumber(void);
//网点号
void PraperNetworkNumber(void);
//IP
void PraperIPNumber(void);
//禁止网发1
void PraperBanTechnician1(void);
//禁止网发2
void PraperBanTechnician2(void);
//禁止网发3
void PraperBanTechnician3(void);
//Mask
void PraperMask(void);
//Gateway
void PraperGateWay(void);
//本地端口
void PraperLocalPort(void);
//远端端口
void PraperDistalPort(void);
//远端IP
void PraperDistalIP(void);
//黑名单
void PraperBlackList(void);
//黑名单页码显示
void PraperBlackListPage(void);
//显示用户信息第3页
void PraperInformation3Page(void);
//更新清分等级的选中标识
void DisplayClearLevelmark(void);
//清分等级和鉴伪等级选中勾消除
void ClearClearLevelmark(void);
//显示黑名单的选中状态
void DisplayCheckmark(void);
//消除黑名单的选中状态
void ClearCheckmark(void);
//速度调试菜单
void PraperSpeedSetMenu(void);

//帧协议
void Fota_UIA_UART_Frame_Start(INT8U ack,INT32U Len,INT8U *Pdata);
void Fota_UIA_UART_Frame_End(INT8U INMode,INT8U *Pdata);

//用户信息显示
void InformationDisp(void);
//黑名单显示
void blackListDisp(void);

//调试菜单用的函数

//马达调试主菜单
void PraperMcDebugMainMenu();
void printsstring16(INT16U x,INT16U y,INT8U *s);
void PraperOldTest();
void PraperMTDispTest();
void PraperSpeedSetMenuINMode();
void PraperSpeedSetMenuOutMode();
void PraperLevelUp();
void PraperMTADMenu();
void PraperUVADMenu();
void PraperInfoIRADMenu();
void PraperInfoMTADMenu();
void PraperInfoMGADMenu();
void PraperCheckCSMenu();
//打印进钞口出钞口AD采样调试表格
void PraperInLetADMenu(void);
//红外AD调试
void PraperIRADMenu(void);
//CS校验开始
void PraperCheckCSStart(void);
//CS校验失败
void PraperCheckCSFail(void);
//CS图像校验成功
void PraperCheckCSImageSuccess(void);
//CS校验第一阶段成功
void PraperCheckCSStage1Success(void);
//CS校验第二阶段开始
void PraperCheckCSStage2Start(void);
/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


