/*************************************************************************
 *	
 *
**************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include "fotawin.h"
#include "fota_analyze.h"
//#include "device.h"

//#define debug
static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static HDC		hModMemDC = NULL;
static HBITMAP	hModBitmap,hOldModBitmap;

static HDC		hBgNumMemDC = NULL;
static HBITMAP	hBgNumBitmap,hOldBgNumBitmap;

static HDC 		hLtNumMemDC = NULL;
static HBITMAP	hLtNumBitmap,hOldLtNumBitmap;

static HDC 		hKyNumMemDC = NULL;
static HBITMAP	hKyNumBitmap,hOldKyNumBitmap;


static HDC		hSDMemDC = NULL;
static HBITMAP	hSDBitmap,hOldSDBitmap;

static HDC		hNetMemDC = NULL;
static HBITMAP	hNetBitmap,hOldNetBitmap;

static HDC		h4MemDC = NULL;
static HBITMAP	h4Bitmap,hOld4Bitmap;

static HDC		hIncmptMemDC = NULL;
static HBITMAP	hIncmptBitmap,hOldIncmptBitmap;

static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;



static s16_t preset_value = 0;		// 批量值


static s8_t modNo = 0;		// 模式切换0-智能 1-混点 2-计数3-分版
static u8_t	addSW = 0;		// 累加开关
static u8_t valueorpre = 0;  //最下面一栏显示内容

static u8_t presetfull= 0;
static u8_t motorstate = 0;

static u8_t sdflag1 = 1,sdflag2 = 1,fullflag = 0,warningstopflag = 0,nosdstartwarning = 1;

static u8_t	Switch[2][4] = {{1,0,0,0},	// 须长按的开关:网发、第四套、残币、SD、
								{0,0,0,0}};	// 开关选中状态0-未选中，1-选中	

static MONEYDISP_S monenyInfo;	// 纸币信息


static u32_t timer_10ms 	= 0;	// 10ms定时器
static u32_t timer_1s	= 0;	// 1s定时器
#define ID_TIMER_10MS 	500	
#define ID_TIMER_1S		501

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

static status_t Timer_1S_Handle(HWND hWnd);
static status_t Timer_10Ms_Handle(HWND hWnd);
static status_t Disp_SD_Check(HWND hwnd,HDC hDC);
static u8_t Error_Warning(HWND hWnd,u32_t *value);
static status_t Disp_Warning(HWND hWnd,s8_t *buf,s8_t flag);

static status_t Disp_Motor_Run(HDC hDC);
static status_t Disp_Mode_Change(HWND hWnd);
static status_t Disp_Add_Status(HDC hDC,u32_t *value,u8_t error_id,u8_t valueorpre);
static status_t Disp_Batch_Status(HDC hDC,u8_t valueorpre);
static status_t Disp_Last_Status(HDC hDC,u8_t valueorpre);
static status_t Disp_Moneny_Status(HDC hDC,u32_t *value,u8_t error_id,u8_t valueorpre);

static int ET850Init(void)
{
	s32_t		ip, mask, gate, type;
	s32_t		ret;
	ret = system("ifconfig lo 127.0.0.1");
	net1_interface_get(&type, &ip, &mask, &gate);
	ifaddr_set_f("eth0", ip, mask, gate);
	net2_interface_get(&type, &ip, &mask, &gate);
	ifaddr_set_f("eth1", ip, mask, gate);
	return 0;
}

int WinAppInit(void)
{
	ET850Init();
	mcu_param_init();
	fotatcp_server_init( );
	monitor_initialize();
	mcudsp_comm_open();
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PSTR szCmdLine, int iCmdShow)
{
	MSG msg;
	WNDCLASS	wndclass;

	wndclass.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc    = (WNDPROC)MainProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = 0;
	wndclass.hIcon          = 0;
	wndclass.hCursor        = 0;
	wndclass.hbrBackground  = NULL;
	wndclass.lpszMenuName   = NULL;
	wndclass.lpszClassName  = (LPCSTR)"DISPLAYWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"DISPLAYWIN",
							  (LPCSTR)"",
							  WS_VISIBLE,
							  0,
							  0,
							  WINDOW_WIDTH,
							  WINDOW_HEIGHT,
							  NULL,
							  NULL,
							  NULL,
							  NULL);


	while (GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg) {
		case WM_CREATE:
			OnCreate(hWnd, wParam, lParam);
			break;
		case WM_PAINT:
			OnPaint(hWnd, wParam, lParam);
			break;
		case WM_KEYDOWN:
			printf("keydown\n");
			OnKeyDown(hWnd, wParam, lParam);
			break;
		case WM_KEYUP:
			OnKeyUp(hWnd, wParam, lParam);
		case WM_TIMER:
			OnTimer(hWnd, wParam, lParam);
			break;
		case WM_DESTROY:
			OnDestroy(hWnd, wParam, lParam);
			break;
		case WM_COMMAND:
			if(wParam == 0xF0F0)
			{
				function_type_set(modNo + 1);
				
			}
			break;
		default:
			return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return (0);
}

static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDC			hDC;
	RECT		rect;
	HBRUSH		hBrush;
	u8_t funcback = 2;

	hDC = GetDC(hWnd);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));

	// 主界面图
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/mainmenuwin.bmp", 0);

	// 模式、累加、面额图
	hModMemDC = CreateCompatibleDC(hDC);
	hModBitmap = CreateCompatibleBitmap(hModMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldModBitmap = SelectObject(hModMemDC, hModBitmap); 
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hModMemDC, &rect, hBrush);
	GdDrawImageFromFile(hModMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/mainmenu1win.bmp", 0);
	

	// 大数字黑底图
	hBgNumMemDC = CreateCompatibleDC(hDC);
	hBgNumBitmap = CreateCompatibleBitmap(hBgNumMemDC, WINDOW_WIDTH, 100);
	hOldBgNumBitmap = SelectObject(hBgNumMemDC, hBgNumBitmap); 
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = 100;
	FillRect(hBgNumMemDC, &rect, hBrush);
	GdDrawImageFromFile(hBgNumMemDC->psd, 0, 0, WINDOW_WIDTH, 100, "/bmp/fota/zongzhangshu.bmp", 0);

	// 小数字黑底图
	hLtNumMemDC = CreateCompatibleDC(hDC);
	hLtNumBitmap = CreateCompatibleBitmap(hLtNumMemDC, 410, 57);
	hOldLtNumBitmap = SelectObject(hLtNumMemDC, hLtNumBitmap); 
	rect.left = 0; rect.right = 410; rect.top = 0; rect.bottom = 57;
	FillRect(hLtNumMemDC, &rect, hBrush);
	GdDrawImageFromFile(hLtNumMemDC->psd, 0, 0, 410, 57, "/bmp/fota/zongjinewin.bmp", 0);

	// 可疑币黑底图
	hKyNumMemDC = CreateCompatibleDC(hDC);
	hKyNumBitmap = CreateCompatibleBitmap(hKyNumMemDC, 320, 44);
	hOldKyNumBitmap = SelectObject(hKyNumMemDC, hKyNumBitmap); 
	rect.left = 0; rect.right = 320; rect.top = 0; rect.bottom = 44;
	FillRect(hKyNumMemDC, &rect, hBrush);
	GdDrawImageFromFile(hKyNumMemDC->psd, 0, 0, 320, 44, "/bmp/fota/keyibiwin.bmp", 0);


	// SD标志
	hSDMemDC = CreateCompatibleDC(hDC);
	hSDBitmap = CreateCompatibleBitmap(hSDMemDC, 34, 35);
	hOldSDBitmap = SelectObject(hSDMemDC, hSDBitmap); 
	rect.left = 0; rect.right = 34; rect.top = 0; rect.bottom = 35;
	FillRect(hSDMemDC, &rect, hBrush);
	GdDrawImageFromFile(hSDMemDC->psd, 0, 0, 34, 35, "/bmp/fota/SDwin.bmp", 0);

	// e标志
	hNetMemDC = CreateCompatibleDC(hDC);
	hNetBitmap = CreateCompatibleBitmap(hNetMemDC, 34, 35);
	hOldNetBitmap = SelectObject(hNetMemDC, hNetBitmap); 
	rect.left = 0; rect.right = 34; rect.top = 0; rect.bottom = 35;
	FillRect(hNetMemDC, &rect, hBrush);
	GdDrawImageFromFile(hNetMemDC->psd, 0, 0, 34, 35, "/bmp/fota/ewin.bmp", 0);

	// 4标志
	h4MemDC = CreateCompatibleDC(hDC);
	h4Bitmap = CreateCompatibleBitmap(h4MemDC, 34, 35);
	hOld4Bitmap = SelectObject(h4MemDC, h4Bitmap); 
	rect.left = 0; rect.right = 34; rect.top = 0; rect.bottom = 35;
	FillRect(h4MemDC, &rect, hBrush);
	GdDrawImageFromFile(h4MemDC->psd, 0, 0, 34, 35, "/bmp/fota/4win.bmp", 0);

	// 残币标志
	hIncmptMemDC = CreateCompatibleDC(hDC);
	hIncmptBitmap = CreateCompatibleBitmap(hIncmptMemDC, 34, 35);
	hOldIncmptBitmap = SelectObject(hIncmptMemDC, hIncmptBitmap); 
	rect.left = 0; rect.right = 34; rect.top = 0; rect.bottom = 35;
	FillRect(hIncmptMemDC, &rect, hBrush);
	GdDrawImageFromFile(hIncmptMemDC->psd, 0, 0, 34, 35, "/bmp/fota/canbiwin.bmp", 0);


	//报警标志
	hwarningMemDC = CreateCompatibleDC(hDC);
	hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 400, 45);
	hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
	rect.left = 0; rect.right = 400; rect.top = 0; rect.bottom = 45;
	FillRect(hwarningMemDC, &rect, hBrush);
	GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 400, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);

	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	// 开启10ms定时器
		timer_10ms = SetTimer(hWnd,ID_TIMER_10MS,10,NULL);

	// 获取功能设置里面的批量模式
	funcback = upload_param_load("批量模式", 2);			
	if(funcback==0)
	{
		preset_value_set(200);
	}
			
	else if(funcback==1)
	{
		preset_value_set(100);

	}
	else if(funcback==2)
	{
		preset_value_set(-1);

	}
	// 获取功能设置里面的备用1，是关于兼容第四套的功能
	funcback = upload_param_load("备用1111", 1);
	if(funcback==1 )
	{
		Switch[0][1] = 1;
		fota_money4_support_set(1);
		fota_paramsave(1);

	}
	else
	{
		Switch[0][1] = 0;
		fota_money4_support_set(0);
		fota_paramsave(1);

	}
	//备用2 为2 时不网发，其余情况网发	
	Switch[0][0] = upload_param_load("网发开关", 1);
		
	// 获取挑残功能

	Switch[0][2] = fota_damaged_support_get();

	

	//获取批量值
	preset_value =	preset_value_get();
	
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{


	if (timer_10ms > 0)
	{
		KillTimer(hWnd, ID_TIMER_10MS);
		timer_10ms = 0;
	}
	if (timer_1s > 0)
	{
		KillTimer(hWnd, ID_TIMER_1S);
		timer_1s = 0;
	}

	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	
	SelectObject(hModMemDC, hOldModBitmap);
	DeleteObject(hModBitmap);
	DeleteDC(hModMemDC);

	SelectObject(hBgNumMemDC, hOldBgNumBitmap);
	DeleteObject(hBgNumBitmap);
	DeleteDC(hBgNumMemDC);

	SelectObject(hLtNumMemDC, hOldLtNumBitmap);
	DeleteObject(hLtNumBitmap);
	DeleteDC(hLtNumMemDC);

	SelectObject(hKyNumMemDC, hOldKyNumBitmap);
	DeleteObject(hKyNumBitmap);
	DeleteDC(hKyNumMemDC);

	SelectObject(hSDMemDC, hOldSDBitmap);
	DeleteObject(hSDBitmap);
	DeleteDC(hSDMemDC);

	SelectObject(hNetMemDC, hOldNetBitmap);
	DeleteObject(hNetBitmap);
	DeleteDC(hNetMemDC);

	SelectObject(h4MemDC, hOld4Bitmap);
	DeleteObject(h4Bitmap);
	DeleteDC(h4MemDC);

	SelectObject(hIncmptMemDC, hOldIncmptBitmap);
	DeleteObject(hIncmptBitmap);
	DeleteDC(hIncmptMemDC);

	SelectObject(hwarningMemDC, hOldwarningBitmap);
	DeleteObject(hwarningBitmap);
	DeleteDC(hwarningMemDC);
			

	hMainWnd = NULL;
	
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC,hMemDC;
	HBITMAP			hMemBitmap, hOldMemBitmap;
	PAINTSTRUCT		ps;
	char 			buf[10];
	int 			len;
	u8_t 			i;

	//获取纸币信息
	moneydisp_info_get(&monenyInfo);

	//SD卡显示相关
	
	sdflag1 = 1;
	sdflag2 = 1;

	//兼容第四套状态
	Switch[0][1] = fota_money4_support_get();
	//备用2 为2 时不网发，其余情况网发	
	Switch[0][0] = upload_param_load("网发开关", 1);
	printf("Switch[0][0] = %d\n",Switch[0][0]);
	
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hMemBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldMemBitmap = SelectObject(hMemDC, hMemBitmap); 
	
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	
	// 模式、累加、面额
	switch(modNo)
	{
		case 0:
			BitBlt(hMemDC,28 ,275 , 75, 43, hModMemDC, 36, 255, SRCCOPY);
			break;
		case 1:
			BitBlt(hMemDC,28 ,275 , 75, 43, hModMemDC, 36, 255 - 52, SRCCOPY);
			break;
		case 3:
			BitBlt(hMemDC,28 ,275 , 75, 43, hModMemDC, 36, 255 - 52*2, SRCCOPY);
			break;
		default:
			break;
	}
	 
	if(monenyInfo.errnum<0)  //yi
	{
		monenyInfo.errnum = 0;
		printf("monenyInfo.errnum error\n");
	}
	if(monenyInfo.curpages< monenyInfo.errnum) //zhen
	{
		monenyInfo.curpages= monenyInfo.errnum ;
		printf("realpage error\n");
	}
	if(monenyInfo.total_sum<0)  //zong jin e 
	{

		monenyInfo.total_sum = 0;
		printf("monenyInfo.total_sum error\n");

	}
	if(monenyInfo.curpages < 0)
	{
		monenyInfo.curpages = 0;
		printf("monenyInfo.curpages error\n");


	}	
	// 疑张
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.errnum);
	len = strlen(buf);
	if(len>5)
		len = 5;
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,110 + i*32,10 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);			
	}

	// 真张
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.curpages- monenyInfo.errnum);
	len = strlen(buf);
	if(len>5)
		len = 5;
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,320 + i*32 ,10 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);			
	}

	// 总金额
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.total_sum);
	len = strlen(buf);
	if(len>6)
		len = 6;
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,440 - (len - i)*41 ,200 , 41, 57, hLtNumMemDC, (buf[i] - '0')*41, 0, SRCCOPY);
	}

	// 总张数
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.curpages);
	len = strlen(buf);
	if(len>5)
		len = 5;
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,440 - (len - i)*48,75 , 48, 100, hBgNumMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
	}

	if((preset_value==0) && (addSW==1)) //累加 ，只显示面额值
	{
		valueorpre = 0;

		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",monenyInfo.enomination);
		len = strlen(buf);
		if(len>3)
			len = 3;
		for(i=0;i<len;i++)
		{
			BitBlt(hMemDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY); 		
		}
		BitBlt(hMemDC,190 ,275 , 75, 43, hModMemDC, 281, 250, SRCCOPY);  //mianzhi
		BitBlt(hMemDC,110 ,275 , 75, 43, hModMemDC, 158, 253, SRCCOPY);
		printf("111monenyInfo.enomination = %d\n",monenyInfo.enomination);
		 
	}
	else if(preset_value>0&&(addSW==0)&&(modNo != 1))  //非累加，有批量只显示批量值
	{
			

			memset(buf,0,sizeof(buf));
			sprintf(buf,"%d",preset_value);
			len = strlen(buf);
			if(len>5)
				len = 5;
			for(i=0;i<len;i++)
			{
				BitBlt(hMemDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY); 		
			}
			for(;i<5;i++)
			{
				BitBlt(hMemDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
			}
			BitBlt(hMemDC,190 ,275 , 75, 43, hModMemDC, 281, 199, SRCCOPY);  //piliang
		   
	}
	else if(((preset_value==-1)&&(addSW==0))||(modNo != 1) )
	{  
		if((motorstate == 0))//非累加无批量显示上次prepages
		{
				

			 BitBlt(hMemDC,110 ,275 , 75, 43, hBgMemDC, 110 ,275 , SRCCOPY);
			 BitBlt(hMemDC,190 ,275 , 75, 43, hModMemDC, 281, 150, SRCCOPY);//上次
			 memset(buf,0,sizeof(buf));
			 sprintf(buf,"%d",monenyInfo.prepages);
			 len = strlen(buf);
			 if(len>5)
				len = 5;
			 for(i=0;i<len;i++)
			 {
				BitBlt(hMemDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY); 	
			 }
			 for(;i<5;i++)
			 {
				BitBlt(hMemDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
			 }												
			  
				
		}
		else if((motorstate == 3))
		{
	
			BitBlt(hMemDC,110 ,275 , 75, 43, hBgMemDC, 110 ,275 , SRCCOPY);
			BitBlt(hMemDC,190 ,275 , 75, 43, hModMemDC, 281, 250, SRCCOPY);
			
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%d",monenyInfo.enomination);
			len = strlen(buf);
			if(len>3)
				len = 3;
			for(i=0;i<len;i++)
			{
				BitBlt(hMemDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY); 			
			}
			for(;i<5;i++)
			{
				BitBlt(hMemDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
			}
						
		
		}


	}
	printf("111monenyInfo.errnum = %d,monenyInfo.curpages = %d,monenyInfo.total_sum = %d\n",monenyInfo.errnum,monenyInfo.curpages,monenyInfo.total_sum);
	
	// 网发
	if(Switch[0][0]!=2)
		BitBlt(hMemDC,15,143 , 34, 35, hNetMemDC,0, 0, SRCCOPY);	

	// 兼容第四套
	if(Switch[0][1]&&(modNo == 0 ||modNo == 2 ))
		BitBlt(hMemDC,15 + 36,143 , 34, 35, h4MemDC,0, 0, SRCCOPY);	

	//挑残
	if(Switch[0][2])
		BitBlt(hMemDC,15 + 36*2,143 , 34, 35, hIncmptMemDC,0, 0, SRCCOPY);

	// SD
	Disp_SD_Check(hWnd,hMemDC);
	
	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOldMemBitmap);
	DeleteObject(hMemBitmap);
	DeleteDC(hMemDC);
	
	EndPaint(hWnd, &ps);
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) 
	{
		case ID_TIMER_10MS:
			Timer_10Ms_Handle(hWnd);	
			break;
		case ID_TIMER_1S:
			if(timer_1s > 0)
			{
				KillTimer(hWnd, ID_TIMER_1S);
				timer_1s = 0;
			}
			Timer_1S_Handle(hWnd);
			break;
		default:
			break;
	}
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

	
	switch(wParam) {
		case VK_F1:
			menuwin_create(hWnd);
			break;
		case VK_F2:  //按币种键 开始发大图
			break;
		case VK_F3:
			if((preset_value!=0)&&modNo != 1) //非累加 非混版方可设批量
				batchset_window_create(hWnd);
			break;
		case VK_F4:
			if(!timer_1s)
			{
				timer_1s = SetTimer(hWnd,ID_TIMER_1S,1000,NULL);
				Switch[1][2] = 1;
			}
			break;
		case VK_F5:
			if(!timer_1s)
			{
				timer_1s = SetTimer(hWnd,ID_TIMER_1S,1000,NULL);
				Switch[1][0] = 1;
				printf("modNochange1\n");
				
			}	
			break;
		case VK_F6:
			if((preset_value==0) )
			{
printf("apreset_value = %d,addSW = %d\n",preset_value,addSW);
				preset_value_set(-1);
				addSW = 0;
			}
				
			else if((preset_value!=0))
			{
printf("bpreset_value = %d,addSW = %d\n",preset_value,addSW);

				preset_value_set(0);
				addSW= 1;

			}
		//	InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:
			if(!timer_1s&&(modNo == 0 ||modNo == 2 ))   //智能 分版模式下长按第四套 开关
			{
				timer_1s = SetTimer(hWnd,ID_TIMER_1S,1000,NULL);
				Switch[1][1] = 1;
			}
			else 
			{
				Switch[1][1] = 1;
			}
			printf("modNo = %d\n",modNo);
						
			break;
		case VK_F8:
			nosdstartwarning = 0;
			
			if(preset_value>0 &&(warningstopflag==0) ) 
			{				
				if(monenyInfo.curpages==0)
				{
					preset_value_set(-1);
					addSW = 0;

				}					
			}
			fota_clearzero();
			moneybunch_free(); //查询界面冠字码清零
			//memset(&monenyInfo,0,sizeof(MONEYDISP_S));
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F9:  //启动复位键
			fota_startstop();
			break;
		default:
			break;
	}
	return 0;
}

static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (timer_1s > 0)
	{
		KillTimer(hWnd, ID_TIMER_1S);
		timer_1s = 0;
	}

	switch(wParam)
	{
		case VK_F1:
			break;
		case VK_F2:
			break;
		case VK_F3:
			break;
		case VK_F4:
			if(Switch[1][2])
			{
				Switch[1][2] = 0;
				levelsetwin_create(hWnd);
			}
			break;
		case VK_F5:
			if(Switch[1][0])
			{
				Switch[1][0] = 0;

				if(modNo == 3)
					function_type_set(0);
				else
					function_type_set(modNo + 1);
				printf("modNo = %d\n",modNo);
			}
			break;
		case VK_F6:
			break;
		case VK_F7:
			if(Switch[1][1])
			{
				Switch[1][1] = 0;
				if(modNo == 1)
				{
					mixquery_window_create(hWnd);
				}
					
				else
				guanzihaowin_create(hWnd);
			}
			break;
		case VK_F8:
			break;
		default:
			break;
			
	}
	return 0;
}

/***********************************************************************************************
* Function Name	: Timer_1S_Handle
* Description		: 1S中断处理函数
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Timer_1S_Handle(HWND hWnd)
{
		
	if(Switch[1][0])		// 网发
	{
		Switch[1][0] = 0;

		if(Switch[0][0]!=2)
		{
			Switch[0][0] = 2;
			upload_param_save("网发开关",2); //网发功能close	
			printf("1Switch[0][0] = %d\n",Switch[0][0]);
		}
			
		else 
		{
			Switch[0][0] = 1;
			upload_param_save("网发开关",1); //网发功能open
			printf("1Switch[0][0] = %d\n",Switch[0][0]);
		}

	}
	else if(Switch[1][1])   // 第四套
	{
		Switch[1][1] = 0;
		if(Switch[0][1])			//兼容第四套关
		{
			Switch[0][1] = 0;	
			fota_money4_support_set(0);			
			fota_paramsave(1);

		}					
		else  						//兼容第四套开
		{
			Switch[0][1] = 1;
			fota_money4_support_set(1);
			
			fota_paramsave(1);
			
		}
	}
	else if(Switch[1][2])	// 挑残
	{
		Switch[1][2] = 0;
		if(Switch[0][2])       //挑残关
		{
			Switch[0][2] = 0;	
			fota_damaged_support_set(0);			
			fota_paramsave(1);
		}
		else 
		{
			Switch[0][2] = 1;		//挑残开
			fota_damaged_support_set(1);			
			fota_paramsave(1);
		}
	}

	InvalidateRect(hWnd, NULL, FALSE);
				
	return OK_T;
}

/***********************************************************************************************
* Function Name	: Timer_10Ms_Handle
* Description		: 10ms中断处理函数
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Timer_10Ms_Handle(HWND hWnd)
{

	
	static u32_t value;   //面值
	static u8_t pre_motorstate; //上次马达状态
	
	char buf[64];	
	status_t ret;
	HDC		hDC;
	u8_t error_id;

	hDC = GetDC(hWnd);
	// 报警界面
	error_id = Error_Warning(hWnd,&value);
	//if(error_id != OK_T)
	//	printf("Disp_Warning error!\n");
	//批量满报警
	motorstate = motor_status_get();
	if(presetfull)
	{
		if(warningstopflag==0&&motorstate==0)
		{
			memcpy(buf,"批量满",strlen("批量满"));
			Disp_Warning(hWnd, buf,1);

		}
		
		if(motorstate ==3&&pre_motorstate==0)
		{
	
			presetfull = 0; 			 
			
			Disp_Warning(hWnd, buf,0);
			
		}
		//printf("motorstate = %d,pre_motorstate= %d\n",motorstate,pre_motorstate);
	}
	
	if(motorstate!=pre_motorstate )
	{
		if(error_id == 0)
		
		pre_motorstate = motorstate;
	
	}

	// SD插拔检测
	ret = Disp_SD_Check(hWnd,hDC);
	if(ret != OK_T)
		printf("Disp_SD_Check error!\n");
	
	ReleaseDC(hWnd, hDC);	
	// 有跑钞
	if(moneydisp_info_status())
	{
		
		hDC = GetDC(hWnd);
		if(presetfull) //针对马达状态不对时消除批量满的报警
		{			
			{
		
				presetfull = 0; 			 
				
				Disp_Warning(hWnd, buf,0);
				
			}
			printf("2motorstate = %d,pre_motorstate= %d\n",motorstate,pre_motorstate);
		}
		ret = Disp_Motor_Run(hDC);
		if(ret != OK_T)
			printf("Disp_Motor_Run error!\n");
		
		ReleaseDC(hWnd, hDC);
		printf("you\n");
	}
	
	
	
	// 无跑钞，但有模式转换
	if(modNo != function_type_get())
	{
		ret = Disp_Mode_Change(hWnd);
		if(ret != OK_T)
			printf("Disp_Mode_Change error!\n");
	}	

	// 无跑钞，但有累加转换批量转换
	preset_value =	preset_value_get();  //批量一栏的显示
	if(preset_value==0)
		addSW = 1;
	else 
		addSW = 0;
	hDC = GetDC(hWnd);
	{
		//printf("final preset_value = %d,addSW = %d,modNo = %d,warningstopflag = %d,motorstate = %d\n",preset_value,addSW,modNo,warningstopflag,motorstate);


			if(preset_value==0&&(addSW==1)) //累加 ，只显示面额值
			{

				ret = Disp_Add_Status(hDC,&value,error_id,valueorpre);
				if(ret != OK_T)
					printf("Disp_Add_Status error!\n");
				valueorpre = 1;
			}
			else if(preset_value>0&&(addSW==0)&&(modNo != 1)) //非累加，有批量只显示批量值fei hunban
			{
				ret = Disp_Batch_Status(hDC,valueorpre);
				if(ret != OK_T)
					printf("Disp_Batch_Status error!\n");
				valueorpre = 2;
			}
			else if(((preset_value==-1)&&(addSW==0))||(modNo != 1) )  // fei lei jia  wu piliang huo hun ban
			{
				if((motorstate == 0)&&(warningstopflag==0))  //非累加，无批量，跑钞结束显示上次
				{
					ret = Disp_Last_Status(hDC,valueorpre);
					if(ret != OK_T)
						printf("Disp_Last_Status error!\n");
					valueorpre = 3;

				}
				else if((motorstate == 3)) //非累加，无批量，跑钞过程中显示面额/mianzhi
				{
					ret = Disp_Moneny_Status(hDC,&value,error_id,valueorpre);
					if(ret != OK_T)
						printf("Disp_Moneny_Status error!\n");
					valueorpre = 4;

				}


			}		
				
		}
		ReleaseDC(hWnd, hDC);
	
	return OK_T;
}

/***********************************************************************************************
* Function Name	: Disp_SD_Check
* Description		: 	SD卡插拔检测
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_SD_Check(HWND hwnd,HDC hDC)
{
	u32_t  sdtotal = 0,sdfree = 0,state  = 0;
	char buf[64];
	
	if(hDC == NULL)
		return ERROR_T;
	state = sdcard_meminfo_get(&sdtotal,&sdfree);
	if(state==0)   //0bukeyong
	{				
		sdflag2 = 1;
		if(fullflag == 1)
		{
			Disp_Warning(hwnd, buf,0);
			fullflag=0;
		}
		if(nosdstartwarning)
		{

				memset(buf,0,64);	
				memcpy(buf,"请插入SD卡按清零键跳过",strlen("请插入SD卡按清零键跳过"));
				Disp_Warning(hwnd,buf,1);

		}
		else if(sdflag1)
		{
			BitBlt(hDC,15 + 36*3,143 , 34, 35, hBgMemDC,15 + 36*3,143, SRCCOPY);
			sdflag1 = 0;
			warningstopflag = 0;
			
 		}			
		
	}				
	else if(state==1&&sdtotal>0&&sdfree>0)   //you SD 
	{
		sdflag1= 1;
		
		if(nosdstartwarning == 1)
		{
			Disp_Warning(hwnd, buf,0);
			nosdstartwarning = 0;
		}
		if(sdflag2&&(nosdstartwarning == 0))
		{

			BitBlt(hDC,15 + 36*3,143 , 34, 35, hSDMemDC,0, 0, SRCCOPY);
			printf("aasdtotal= %d,sdfree = %d,state  =%d\n",sdtotal,sdfree,state);
			if(sdfree<80 && (fullflag==0) )
			{
				memset(buf,0,64);
				memcpy(buf,"SD卡容量满",strlen("SD卡容量满"));
				Disp_Warning(hwnd,buf,1);
				printf("sdtotal= %d,sdfree = %d\n",sdtotal,sdfree);
				fullflag = 1;

			}		
			
			sdflag2 = 0;
		}
		
	}
	
	return OK_T;
}



/***********************************************************************************************
* Function Name	: Disp_Warning
* Description		: 告警相关
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static u8_t Error_Warning(HWND hWnd,u32_t *value)
{
	static u8_t olderror_id = 0;	
	u8_t error_id;
	s8_t	*errptr;
	s8_t 	buf[64];
	

	// 获取设备告警信息
	error_id = deverror_id_get();
	
	if(error_id)
	{
		errptr = deverror_str_get(error_id);
		olderror_id = error_id;
	}
	else
	{
		// 没有设备告警的情况下，获取纸币告警信息
		error_id = moneyerror_id_get();								
		if(error_id)//||presetfull)  //报警信息
		{
			errptr = moneyerror_str_get(error_id);
			olderror_id = error_id;				
		}	
		else if(error_id==0 && olderror_id>0)  //后改成马达转动
		{
			// 告警解除之后
			olderror_id = error_id;

			monenyInfo.enomination = *value;
			printf("monenyInfo.enomination  = %d\n",monenyInfo.enomination);
			warningstopflag = 0;
			Disp_Warning(hWnd, buf,0);
		}
		
			
	}
	// 显示告警信息
	if(error_id&&(nosdstartwarning == 0)) 
	{

		
		//printf("error_id = %d\n",error_id);
		if (errptr == NULL) 
		{
			sprintf(buf, "错误代码 0x%02x", olderror_id);

		}
		else
			memcpy(buf,errptr,strlen(errptr));
		
		Disp_Warning(hWnd, buf,1);
	}
	
	return error_id;
}
static status_t Disp_Warning(HWND hWnd,s8_t *buf,s8_t flag)
{
	HDC				hDC,hMemDC;
	HBITMAP			hBitmap, hOldBitmap;
	HFONT			hOldFont;
	u8_t i,len;
	// 显示告警信息
	hDC = GetDC(hWnd);
	if(flag)
	{
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 400, 45);
				hOldBitmap = SelectObject(hMemDC, hBitmap);
				
					BitBlt(hMemDC, 0, 0, 400, 45, hwarningMemDC, 0, 0, SRCCOPY);
					hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
						SetBkColor(hMemDC, RGB(0, 255, 0));
						SetBkMode(hMemDC, TRANSPARENT);;							
						SetTextColor(hMemDC,RGB(255,0 , 0));
						TextOut(hMemDC, 300-strlen(buf)*12, 10, buf, -1);
							
						BitBlt(hDC, 40, 180, 400,45,hMemDC, 0, 0, SRCCOPY);
					
					SelectObject(hMemDC, hOldFont);
				SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
		DeleteDC(hMemDC);
		
		warningstopflag = 1;
	}
	else 
	{
		BitBlt(hDC, 40, 180, 400,45,hBgMemDC,40, 180, SRCCOPY);
		memset(buf,0,sizeof(buf));
		
		sprintf(buf,"%d",monenyInfo.total_sum);
		len = strlen(buf); 
		if(len>6)
			len = 6;
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,440 - (len - i)*41 ,200 , 41, 57, hLtNumMemDC, (buf[i] - '0')*41, 0, SRCCOPY);				
		}
		
		warningstopflag = 0;
		printf("reflash is ok!\n");
	}
		
	ReleaseDC(hWnd, hDC);

	return OK_T;
}

/***********************************************************************************************
* Function Name	: Disp_Motor_Run
* Description		: 跑钞显示
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_Motor_Run(HDC hDC)
{
	static u32_t tot_count,amount,real,doubt;
	s8_t 	buf[64];
	s32_t 	len;
	u8_t 	i;
	
	moneydisp_info_get(&monenyInfo);
	if(monenyInfo.errnum<0)  //yi
	{
		monenyInfo.errnum = 0;
		printf("monenyInfo.errnum error\n");
	}
	if(monenyInfo.curpages< monenyInfo.errnum) //zhen
	{
		monenyInfo.curpages= monenyInfo.errnum ;
		printf("realpage error\n");
	}
	if(monenyInfo.total_sum<0)  //zong jin e 
	{

		monenyInfo.total_sum = 0;
		printf("monenyInfo.total_sum error\n");

	}
	if(monenyInfo.curpages < 0)
	{
		monenyInfo.curpages = 0;
		printf("monenyInfo.curpages error\n");

	}	
	
	// 总张数
	if(tot_count != monenyInfo.curpages )
	{	
		memset(buf,0,sizeof(buf));
//#ifdef debug			
printf("aamonenyInfo.curpages = %d\n",monenyInfo.curpages);	
//#endif					
		
		sprintf(buf,"%d",monenyInfo.curpages);
	
		len = strlen(buf);
		if(len>5)
			len = 5;
		if(tot_count>monenyInfo.curpages)
		{
			
			for(;i<5;i++)
			{
				BitBlt(hDC,440 - (i+1)*48,75 , 48, 100, hBgMemDC, 440 - (5 - i)*48,75, SRCCOPY);
			}

		}
		
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,440 - (len - i)*48,75 , 48, 120, hBgNumMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
		}
		tot_count = monenyInfo.curpages;
		if(preset_value>0&&(preset_value == tot_count))
		{
			presetfull = 1;
			
		}	
	}
	// 总金额
	if(amount != monenyInfo.total_sum)
	{
//#ifdef debug			
printf("ccmonenyInfo.total_sum = %d\n" ,monenyInfo.total_sum);	
//#endif	

		
		memset(buf,0,sizeof(buf));
		
		sprintf(buf,"%d",monenyInfo.total_sum);
		len = strlen(buf);
		if(len>7)
			len = 7;
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,440 - (len - i)*41 ,200 , 41, 57, hLtNumMemDC, (buf[i] - '0')*41, 0, SRCCOPY);				
		}
		if(amount>monenyInfo.total_sum)
		{
			for(;i<6;i++)
			{
				BitBlt(hDC,440 - (i+1)*41,200 , 41, 57, hBgMemDC, 440 - (5 - i)*41,200, SRCCOPY);
			}

		}
							
		amount = monenyInfo.total_sum;	
		
	}
#ifdef debug			
				printf("ddmonenyInfo.total_sum = %d\n" ,monenyInfo.total_sum);
#endif	

	
	// 疑币张数
	if(doubt != monenyInfo.errnum )
	{
		memset(buf,0,sizeof(buf));
//#ifdef debug			
printf("eemonenyInfo.errnum = %d\n" ,monenyInfo.errnum);
//#endif						
		
		sprintf(buf,"%d",monenyInfo.errnum);
		len = strlen(buf);
		if(len>5)
			len = 5;
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,110 + i*32,10 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);				
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,110 + i*32,10 , 32, 44, hBgMemDC,110 + i*32,10, SRCCOPY);
		}
		
		doubt = monenyInfo.errnum;	
	}
#ifdef debug			
				printf("ffmonenyInfo.errnum = %d\n" ,monenyInfo.errnum);
#endif	

	
	// 真币张数
	if(real != (monenyInfo.curpages - monenyInfo.errnum))
	{
		memset(buf,0,sizeof(buf));
//#ifdef debug			
	printf("gg(monenyInfo.curpages - monenyInfo.errnum) = %d\n" ,(monenyInfo.curpages - monenyInfo.errnum));
//#endif						
	
		sprintf(buf,"%d",(monenyInfo.curpages - monenyInfo.errnum));
		len = strlen(buf);
		if(len>5)
			len = 5;
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,320 + i*32,10 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);					
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,320 + i*32,10 , 32, 44, hBgMemDC,320 + i*32,10, SRCCOPY);
		}
		real = monenyInfo.curpages - monenyInfo.errnum;	
		
			
	}
#ifdef debug			
	printf("hh(monenyInfo.curpages - monenyInfo.errnum) = %d\n" ,(monenyInfo.curpages - monenyInfo.errnum));
#endif		
	return OK_T;
}

/***********************************************************************************************
* Function Name	: Disp_Mode_Change
* Description		:模式切换
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_Mode_Change(HWND hWnd)
{
	HDC 	hDC;
	hDC = GetDC(hWnd);

	modNo = function_type_get();
	

		switch(modNo)
			{
				case 2:
					countmodewin_create(hWnd);
					break;
				case 0://智能
					BitBlt(hDC,28 ,275 , 75, 43, hModMemDC, 36, 255, SRCCOPY);
					break;
				case 1://混版
					if(preset_value>0) 
					{
		
						preset_value_set(-1);
						//addchangeflag = 1;
						addSW = 0;
					}
					if(Switch[0][1])			//兼容第四套关
					{
						Switch[0][1] = 0;	
						fota_money4_support_set(0); 		
						fota_paramsave(1);
						BitBlt(hDC,15 + 36,143 , 34, 35, hBgMemDC,15 + 36,143, SRCCOPY);
					}
					BitBlt(hDC,28 ,275 , 75, 43, hModMemDC, 36, 255 - 52, SRCCOPY);		
					break;
				case 3://分版
					BitBlt(hDC,28 ,275 , 75, 43, hModMemDC, 36, 255 - 52*2, SRCCOPY);
					break;
				default:
					break;
			}

	printf("modNo=%d\n",modNo);
	ReleaseDC(hWnd, hDC);

	return OK_T;
}

/***********************************************************************************************
* Function Name	: Disp_Mode_Change
* Description		:显示累加状态
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_Add_Status(HDC hDC,u32_t *value,u8_t error_id,u8_t valueorpre)
{
	s8_t 	buf[64];
	s32_t 	len;
	u8_t 	i;
	
	// 从上次、批量切换到累加时，第一次需要刷成面值
	if(valueorpre !=1)
	{

		BitBlt(hDC,190 ,275 , 75, 43, hModMemDC, 281, 250, SRCCOPY);	//面值
		BitBlt(hDC,110 ,275 , 75, 43, hModMemDC, 158, 253, SRCCOPY);	// 累加
		memset(buf,0,sizeof(buf));
				
		sprintf(buf,"%d",monenyInfo.enomination);
		len = strlen(buf);
		
		for(i=0;i<len;i++)		 //显示面额
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);				
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}
		
		*value = monenyInfo.enomination;
		printf("222monenyInfo.enomination = %d\n",monenyInfo.enomination);

	}
	

	// 前后两张金额有变化时，需要刷新面值
    if(*value != monenyInfo.enomination &&(error_id==0))
	{
		memset(buf,0,sizeof(buf));
		
		sprintf(buf,"%d",monenyInfo.enomination);
		len = strlen(buf);
		
		for(i=0;i<len;i++)       //显示面额
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);				
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}
		
		*value = monenyInfo.enomination;
	}
	return OK_T;
	   
}

/***********************************************************************************************
* Function Name	: Disp_Batch_Status
* Description		: 显示批量状态
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_Batch_Status(HDC hDC,u8_t valueorpre)
{
	static s16_t oldpreset_value;
	s8_t 	buf[64];
	s32_t 	len;
	u8_t 	i;

	// 从上次、累加状态切换到批量时，第一次需要刷成批量
	if(valueorpre !=2)
	{
		BitBlt(hDC,110 ,275 , 75, 43, hBgMemDC, 110 ,275 , SRCCOPY);
		BitBlt(hDC,190 ,275 , 75, 43, hModMemDC, 281, 199, SRCCOPY);  // 批量
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",preset_value);
		len = strlen(buf);
		for(i=0;i<len;i++)
		{
		   BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY); 	   
		}	
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}
		oldpreset_value = preset_value;

	}
		
	if(oldpreset_value != preset_value)
	{

		oldpreset_value = preset_value;

		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",preset_value);
		len = strlen(buf);
		for(i=0;i<len;i++)
		{
		   BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);		   
		}	
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}
	}

	return OK_T;
}

/***********************************************************************************************
* Function Name	: Disp_Last_Status
* Description		: 显示上次状态
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_Last_Status(HDC hDC,u8_t valueorpre)
{
	static u32_t pre_pages;
	
	s8_t 	buf[64];
	s32_t 	len;
	u8_t 	i;

	if(valueorpre!=3)
	{
		BitBlt(hDC,110 ,275 , 75, 43, hBgMemDC, 110 ,275 , SRCCOPY);
	    BitBlt(hDC,190 ,275 , 75, 43, hModMemDC, 281, 150, SRCCOPY);//上次
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",monenyInfo.prepages);
		len = strlen(buf);
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);		 
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}	
		pre_pages = monenyInfo.prepages;
		printf("111monenyInfo.prepages = %d",monenyInfo.prepages);

	}
	 
	 
	if(pre_pages!=monenyInfo.prepages)
	{

		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",monenyInfo.prepages);
		len = strlen(buf);
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);		 
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}								 
		pre_pages = monenyInfo.prepages;
		printf("222monenyInfo.prepages = %d",monenyInfo.prepages);

	}
	return OK_T;
}

/***********************************************************************************************
* Function Name	: Disp_Moneny_Status
* Description		: 显示面值状态
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_Moneny_Status(HDC hDC,u32_t *value,u8_t error_id,u8_t valueorpre)
{
	s8_t 	buf[64];
	s32_t 	len;
	u8_t 	i;
	
	if(valueorpre!=4)
	{
		
		
		BitBlt(hDC,110 ,275 , 75, 43, hBgMemDC, 110 ,275 , SRCCOPY);
		BitBlt(hDC,190 ,275 , 75, 43, hModMemDC, 281, 250, SRCCOPY);
	    memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",monenyInfo.enomination);
		len = strlen(buf);
		
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);				
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}
		
		*value = monenyInfo.enomination;
	
	}
				
		  
   	if(*value != monenyInfo.enomination &&(error_id==0))
	{
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",monenyInfo.enomination);
		len = strlen(buf);
		
		for(i=0;i<len;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);				
		}
		for(;i<5;i++)
		{
			BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
		}
		
		*value = monenyInfo.enomination;
	}
	return OK_T;
}
HWND GetMainWinHwnd(void)
{
	return hMainWnd;
}


