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

static u8_t olderror_id = 0;

static s8_t 	modNo = 0;		// 模式切换0-智能 1-混点 2-计数3-分版
static u8_t		addSW = 0;		// 累加开关
static u8_t		Switch[2][4] = {1,1,1,1,	// 须长按的开关:网发、第四套、残币、SD、
								0,0,0,0};	// 开关选中状态0-未选中，1-选中	

static MONEYDISP_S monenyInfo;	// 纸币信息

static u32_t 		timer_paper_info_s 	= 0;	// 纸币信息定时器
static u32_t 		timer_long_key_s	= 0;	// 按键长按定时器
#define ID_TIMER_PAPER_INFO 	500	
#define ID_TIMER_LONG_KEY		501

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

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
	fotatcp_server_init(7566);
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
							  480,
							  320,
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
			if(wParam = 0xF0F0)
			{
				function_type_set(modNo + 1);
			}
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
	
	hDC = GetDC(hWnd);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));

	// 主界面图
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	rect.left = 0; rect.right = 480; rect.top = 0; rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/mainmenuwin.bmp", 0);

	// 模式、累加、面额图
	hModMemDC = CreateCompatibleDC(hDC);
	hModBitmap = CreateCompatibleBitmap(hModMemDC, 480, 320);
	hOldModBitmap = SelectObject(hModMemDC, hModBitmap); 
	rect.left = 0; rect.right = 480; rect.top = 0; rect.bottom = 320;
	FillRect(hModMemDC, &rect, hBrush);
	GdDrawImageFromFile(hModMemDC->psd, 0, 0, 480, 320, "/bmp/fota/mainmenu1win.bmp", 0);
	

	// 大数字黑底图
	hBgNumMemDC = CreateCompatibleDC(hDC);
	hBgNumBitmap = CreateCompatibleBitmap(hBgNumMemDC, 480, 100);
	hOldBgNumBitmap = SelectObject(hBgNumMemDC, hBgNumBitmap); 
	rect.left = 0; rect.right = 480; rect.top = 0; rect.bottom = 100;
	FillRect(hBgNumMemDC, &rect, hBrush);
	GdDrawImageFromFile(hBgNumMemDC->psd, 0, 0, 480, 100, "/bmp/fota/zongzhangshu.bmp", 0);

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
	hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 220, 45);
	hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
	rect.left = 0; rect.right = 220; rect.top = 0; rect.bottom = 45;
	FillRect(hwarningMemDC, &rect, hBrush);
	GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 220, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);


	timer_paper_info_s = SetTimer(hWnd,ID_TIMER_PAPER_INFO,10,NULL);
	
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
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
			

	if (timer_paper_info_s > 0)
	{
		KillTimer(hWnd, ID_TIMER_PAPER_INFO);
		timer_paper_info_s = 0;
	}
	
	hMainWnd = NULL;
	
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC,hMemDC;
	HBITMAP			hMemBitmap, hOldMemBitmap;
	PAINTSTRUCT		ps;
	u8_t 			buf[10];
	int 			len;
	u8_t 			i;

	hDC = BeginPaint(hWnd, &ps);
	
	hMemDC = CreateCompatibleDC(hDC);
	hMemBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
	hOldMemBitmap = SelectObject(hMemDC, hMemBitmap); 
	
	BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
	
	// 模式、累加、面额
	switch(modNo)
	{
		case 0:
			BitBlt(hMemDC,30 ,275 , 70, 40, hModMemDC, 40, 255, SRCCOPY);
			break;
		case 1:
			BitBlt(hMemDC,28 ,275 , 72, 40, hModMemDC, 36, 255 - 52, SRCCOPY);
			break;
		case 3:
			BitBlt(hMemDC,28 ,275 , 72, 40, hModMemDC, 36, 255 - 52*2, SRCCOPY);
			break;
		default:
			break;
	}
	if(addSW)
		BitBlt(hMemDC,110 ,275 , 70, 40, hModMemDC, 162, 253, SRCCOPY);
	BitBlt(hMemDC,190 ,275 , 70, 40, hModMemDC, 285, 253, SRCCOPY);
	
	// 疑张
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.errnum);
	len = strlen(buf);
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,110 + i*32,10 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);			
	}

	// 真张
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.curpages- monenyInfo.errnum);
	len = strlen(buf);
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,345 + i*32 ,10 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);			
	}

	// 总金额
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.money_sum);
	len = strlen(buf);
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,440 - (len - i)*41 ,200 , 41, 57, hLtNumMemDC, (buf[i] - '0')*41, 0, SRCCOPY);
	}

	// 总张数
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.curpages);
	len = strlen(buf);
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,440 - (len - i)*48,75 , 48, 100, hBgNumMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
	}

	// 面额值
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.enomination);
	len = strlen(buf);
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);			
	}
	
	// 网发
	if(Switch[0][0])
		BitBlt(hMemDC,15,143 , 34, 35, hNetMemDC,0, 0, SRCCOPY);	

	// 兼容第四套
	if(Switch[0][1])
		BitBlt(hMemDC,15 + 36,143 , 34, 35, h4MemDC,0, 0, SRCCOPY);	

	//挑残
	if(Switch[0][2])
		BitBlt(hMemDC,15 + 36*2,143 , 34, 35, hIncmptMemDC,0, 0, SRCCOPY);

	// SD
	if(Switch[0][3])
		BitBlt(hMemDC,15 + 36*3,143 , 34, 35, hSDMemDC,0, 0, SRCCOPY);
	
	BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOldMemBitmap);
	DeleteObject(hMemBitmap);
	DeleteDC(hMemDC);
	
	EndPaint(hWnd, &ps);
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	static u32_t tot_count,value,amount,doubt,real;
	u8_t buf[64];
	s32_t len;
	u8_t i,error_id;
	
	HDC				hDC,hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	HFONT			 hOldFont;
	s8_t	*errptr;

	switch(wParam) 
	{
		case ID_TIMER_PAPER_INFO:
			if(moneydisp_info_status())
			{
				moneydisp_info_get(&monenyInfo);

				hDC = GetDC(hWnd);

				
				
				// 总张数
				if(tot_count != monenyInfo.curpages)
				{	
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",monenyInfo.curpages);
				
					len = strlen(buf);
					if(tot_count>monenyInfo.curpages)
					{
						
						for(;i<5;i++)
						{
							BitBlt(hDC,440 - (i+1)*48,75 , 48, 100, hBgMemDC, 440 - (5 - i)*48,75, SRCCOPY);
						}

					}
					
					error_id = moneyerror_id_get();				
				
					if(error_id)  //报警信息
					{
						errptr = moneyerror_str_get(error_id);

						olderror_id = error_id;

						printf("%s\n",errptr);					
										
						hMemDC = CreateCompatibleDC(hDC);
						hBitmap = CreateCompatibleBitmap(hMemDC, 220, 45);
						hOldBitmap = SelectObject(hMemDC, hBitmap);
							BitBlt(hMemDC, 0, 0, 220, 45, hwarningMemDC, 0, 0, SRCCOPY);
							hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
								SetBkColor(hMemDC, RGB(0, 255, 0));
								SetBkMode(hMemDC, TRANSPARENT);;							
								SetTextColor(hMemDC,RGB(255,0 , 0));
								
								if (errptr == NULL) 
								{
									sprintf(buf, "错误代码 0x%02x", olderror_id);
									TextOut(hMemDC, 47, 10, buf, -1);
								}
								else
									TextOut(hMemDC, 47, 10, errptr, -1);

								
								BitBlt(hDC, 170, 110, 220,45,hMemDC, 0, 0, SRCCOPY);
							SelectObject(hMemDC, hOldFont);
					
						SelectObject(hMemDC, hOldBitmap);
						DeleteObject(hBitmap);
						DeleteDC(hMemDC);

					}	
					else if(error_id == 0 && error_id!=olderror_id)
					{
						olderror_id = error_id;
						BitBlt(hDC, 170, 110, 220,45,hBgMemDC, 170, 110, SRCCOPY);
					}
					
					for(i=0;i<len;i++)
					{
						BitBlt(hDC,440 - (len - i)*48,75 , 48, 120, hBgNumMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
					}
					tot_count = monenyInfo.curpages;
				}

				// 总金额
				if(amount != monenyInfo.total_sum)
				{
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",monenyInfo.total_sum);
					printf("totalsum= %d\n",monenyInfo.total_sum);
					len = strlen(buf);

					for(i=0;i<len;i++)
					{
						BitBlt(hDC,440 - (len - i)*41 ,200 , 41, 57, hLtNumMemDC, (buf[i] - '0')*41, 0, SRCCOPY);				
					}
					printf("i=%d\n",i);
					if(amount>monenyInfo.total_sum)
					{
						for(;i<6;i++)
						{
							BitBlt(hDC,440 - (i+1)*41,200 , 41, 57, hBgMemDC, 440 - (5 - i)*41,200, SRCCOPY);
						}


					}
					
					
					amount = monenyInfo.total_sum;	
				}
				
				// 当前纸币面值
				if(value != monenyInfo.enomination)
				{
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",monenyInfo.enomination);
					len = strlen(buf);
					
					for(i=0;i<len;i++)
					{
						BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);				
					}
					for(;i<3;i++)
					{
						BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
					}
					
					value = monenyInfo.enomination;
				}
				
				// 疑币张数
				if(doubt != monenyInfo.errnum)
				{
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",monenyInfo.errnum);
					len = strlen(buf);
					
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

				// 真币张数
				if(real != (monenyInfo.curpages - monenyInfo.errnum))
				{
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",(monenyInfo.curpages - monenyInfo.errnum));
					len = strlen(buf);
					
					for(i=0;i<len;i++)
					{
						BitBlt(hDC,345 + i*32,10 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY);					
					}
					for(;i<5;i++)
					{
						BitBlt(hDC,345 + i*32,10 , 32, 44, hBgMemDC,345 + i*32,10, SRCCOPY);
					}
					
					real = monenyInfo.curpages - monenyInfo.errnum;	
				}
				
				
				ReleaseDC(hWnd, hDC);
			}
		
			if(modNo != function_type_get())
			{
				hDC = GetDC(hWnd);

				modNo = function_type_get();
			
				switch(modNo)
				{
					case 2:
						countmodewin_create(hWnd);
						break;
					case 0:
						BitBlt(hDC,30 ,275 , 70, 40, hModMemDC, 40, 255, SRCCOPY);
						break;
					case 1:
						BitBlt(hDC,28 ,275 , 72, 40, hModMemDC, 36, 255 - 52, SRCCOPY);
						break;
					case 3:
						BitBlt(hDC,28 ,275 , 72, 40, hModMemDC, 36, 255 - 52*2, SRCCOPY);
						break;
					default:
						break;
				}
				printf("modNo = %d\n",modNo);

				ReleaseDC(hWnd, hDC);
			}

			break;
		case ID_TIMER_LONG_KEY:
			if (timer_long_key_s > 0)
			{
				KillTimer(hWnd, ID_TIMER_LONG_KEY);
				timer_long_key_s = 0;
			}
			u8_t i;
			for(i=0;i<4;i++)
			{
				if(Switch[1][i])
				{
					if(Switch[0][i])
						Switch[0][i] = 0;
					else 			
						Switch[0][i] = 1;

					Switch[1][i] = 0;
					break;
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);

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
		case VK_F2:
			break;
		case VK_F3:
			if(!timer_long_key_s)
			{
				timer_long_key_s = SetTimer(hWnd,ID_TIMER_LONG_KEY,1000,NULL);
				Switch[1][3] = 1;
			}
			break;
		case VK_F4:
			if(!timer_long_key_s)
			{
				timer_long_key_s = SetTimer(hWnd,ID_TIMER_LONG_KEY,1000,NULL);
				Switch[1][2] = 1;
			}
			break;
		case VK_F5:
			if(!timer_long_key_s)
			{
				timer_long_key_s = SetTimer(hWnd,ID_TIMER_LONG_KEY,1000,NULL);
				Switch[1][0] = 1;
				
			}	
			break;
		case VK_F6:
			if(addSW)
				addSW = 0;
			else
				addSW = 1;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:
			if(!timer_long_key_s)
			{
				timer_long_key_s = SetTimer(hWnd,ID_TIMER_LONG_KEY,1000,NULL);
				Switch[1][1] = 1;
			}
			printf("modNo = %d\n",modNo);
			if(modNo == 1)
				mixquery_window_create(hWnd);
			else
				guanzihaowin_create(hWnd);
			
			break;
		case VK_F8:
			memset(&monenyInfo,0,sizeof(MONEYDISP_S));
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		default:
			break;
	}
	return 0;
}

static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (timer_long_key_s > 0)
	{
		KillTimer(hWnd, ID_TIMER_LONG_KEY);
		timer_long_key_s = 0;
	}

	switch(wParam)
	{
		case VK_F1:
			break;
		case VK_F2:
			break;
		case VK_F3:
			if(Switch[1][3])
			{
				Switch[1][3] = 0;
				batchset_window_create(hWnd);
			}
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
			}
			break;
		case VK_F6:
			break;
		case VK_F7:
			break;
		case VK_F8:
			break;
		default:
			break;
			
	}
	return 0;
}



