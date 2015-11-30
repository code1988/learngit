#ifndef _FOTAWIN_H_
#define _FOTAWIN_H_
#include "et850_data.h"
#include "monitor.h"
#include "fota_analyze.h"
#include "device.h"
#include "utypes.h"
#define WINDOW_WIDTH		480
#define WINDOW_HEIGHT		320




//获得主界面的句柄
extern HWND GetMainWinHwnd(void);
//获得主菜单句柄
extern HWND GetMenuWinHwnd(void);
//获得参数设置界面句柄
extern HWND parasetwin_handle_get(void);
//主菜单界面
extern int menuwin_create(HWND hWnd);


//批量设置界面
extern int batchset_window_create(HWND hWnd);
//等级设置界面
extern int levelsetwin_create(HWND hWnd);
//混版明细界面
extern int mixquery_window_create(HWND hWnd);
//冠字码查询界面
extern int guanzihaowin_create(HWND hWnd);
//参数显示界面
extern int picdistinguishwin_create(HWND hWnd);
//密码设置界面
extern int password_window_create(HWND hWnd);
//参数设置界面
extern int parasetwin_create(HWND hWnd);
//版本界面
extern int version_window_create(HWND hWnd);
//功能设置界面
extern int function_window_create(HWND hWnd);
//数据管理界面
extern int datamanage_window_create(HWND hWnd);
//黑名单设置界面
extern int blackname_window_create(HWND hWnd);
//网点号输入界面
extern int netnumber_window_create(HWND hWnd);
//时间设置界面
extern int timeset_window_create(HWND hWnd);
//机器IP设置界面
extern int myIP_window_create(HWND hWnd);
//老化界面
extern int agetest_window_create(HWND hWnd);

//服务器IP设置界面
extern int serverIPwin_create(HWND hWnd);
//机器子网掩码界面
extern int myMaskwin_create(HWND hWnd);
//机器网关界面
extern int myGatewin_create(HWND hWnd);
//机器端口界面
extern int myPortwin_create(HWND hWnd);
//服务器端口界面
extern int serverPortwin_create(HWND hWnd);
//机器MAC地址输入界面
extern int myMACwin_create(HWND hWnd);
//动态调试界面
extern int dynamicwin_create(HWND hWnd);
//静态调试界面
extern int staticwin_create(HWND hWnd);
//CIS校验界面
extern int CISadjust_create(HWND hWnd);
//计数模式界面
extern int countmodewin_create(HWND hWnd);
//上传图像时间设置界面
extern int uploadimg_window_create(HWND hWnd);
//CIS参数设置界面
extern int identifywin_create(HWND hWnd);
//调试界面
extern int adjustmenu_window_create(HWND hWnd);
//黑名单列表界面
extern int Blacknamelist_create(HWND hWnd);

#endif