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
#include "utility.h"
#include "et850_data.h"

#define SAVE_TIMER_ID				601

#define Versionheight    28

#define ID_TIME_UPGRADE_DSP		600
static int		timer_upgrade_dsp_s 	= 0;

#define ID_TIME_UPGRADE_MCU		601
static int		timer_upgrade_mcu_s 	= 0;


static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static u8_t		focus = 0;
static u8_t		flag[6]={0};

static char version[6][32];

static int versionpos[2] = {291,50};
								
static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void refresh_keytable(HWND hWnd,int Oldx,int Posx);


int version_window_create(HWND hWnd)
{
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
	wndclass.lpszClassName  = (LPCSTR)"VERSIONWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"VERSIONWIN",
							  (LPCSTR)"",
							  WS_VISIBLE | WS_CHILD,
							  0,
							  0,
							  480,
							  320,
							  hWnd,
							  NULL,
							  NULL,
							  NULL);


	SetFocus(hMainWnd);
	return 0;
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
		case WM_TIMER:
			OnTimer(hWnd, wParam, lParam);
			break;
		case WM_DESTROY:
			OnDestroy(hWnd, wParam, lParam);
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
	HBRUSH	hBrush;	
	int         i=0;
	int ret;
	hDC = GetDC(hWnd);
	
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	hBgMemDC = CreateCompatibleDC(hDC);													
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);						    
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
	rect.left = 0;
	rect.right = 480;
	rect.top = 0;
	rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);                                           
	
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/versionwin.bmp", 0);     
	
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	for(i = 0;i<10;i++)
		version[i][0] = '\0';
	arm_software_version_get(version[0]);   //ARM
	dsp_param_version_get(version[1]);		//para
	dsp_software_version_get(version[4]);	//DSP
	memcpy(version[2],version[4]+11,14);
	dsp_fpga_version_get(version[3]);		//FPGA
	dsp_module_version_get(version[4]);		//MODEL
	mcu_software_version_get(version[5]);   	//mcu

	
	ret = system("cp /media/sdcard/upgrade/mcu/mcu /upgrade/mcu/mcu");
	if (ret) 
	{
		printf("no mcu file\n");
	}	

	
	
	
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

	if (timer_upgrade_mcu_s > 0)
		KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
	timer_upgrade_mcu_s = 0;

	if (timer_upgrade_dsp_s > 0)
		KillTimer(hWnd, ID_TIME_UPGRADE_DSP);
	timer_upgrade_dsp_s = 0;
	
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	HFONT			 hOldFont;
	char i = 0;
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
	
	hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
	SetBkColor(hMemDC, RGB(0, 255, 1));		// ±³¾°ÑÕÉ«
	SetBkMode(hMemDC, TRANSPARENT);
									
	for(i = 0;i<6;i++)
	{
		if(flag[i])
			SetTextColor(hMemDC, RGB(0, 255, 0));
		else if (focus == i)
			SetTextColor(hMemDC, RGB(255, 0, 0));
		else
			SetTextColor(hMemDC, RGB(255, 255, 255));	
		TextOut(hMemDC, versionpos[0]-strlen(version[i])*6, versionpos[1]+i*(Versionheight+i), version[i], -1);  
	}
	//TextOut(hMemDC, versionpos[0]-strlen(version[i])*6, versionpos[1]+5*(Versionheight+4), version[i], -1);

	BitBlt(hDC, 0, 0, 480, 320,hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOldFont);

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	status_t	ret;
	
	switch(wParam) {
		case VK_F1:    
			break;
		case VK_F2:   
			break;
		case VK_F3:       //down
			focus = (focus + 5)%6;
			InvalidateRect(hWnd,NULL,FALSE);
			break;
		case VK_F4:       //return
			focus = (focus + 1)%6;
			InvalidateRect(hWnd,NULL,FALSE);
			break;	
		case VK_F5:     //Éý¼¶
			if(focus == 5)
			{
				ret = mcu_upgrade_create();
				printf("mcu_upgrade_create ret = %d\n", ret);
				timer_upgrade_mcu_s =  SetTimer(hWnd, ID_TIME_UPGRADE_MCU, 100, NULL);
			}
			else if(focus == 2)
			{
				ret = fota_upgrade_create(1, "/media/sdcard/upgrade/dsp/dsp", NULL);
				printf("fota_upgrade_create dsp ret = %d\n",ret);
				timer_upgrade_dsp_s = SetTimer(hWnd, ID_TIME_UPGRADE_DSP, 100, NULL);
			}
			else if(focus == 3)
			{
				ret = fota_upgrade_create(2, "/media/sdcard/upgrade/dsp/fpga", NULL);
				printf("fota_upgrade_create dsp ret = %d\n",ret);
				timer_upgrade_dsp_s = SetTimer(hWnd, ID_TIME_UPGRADE_DSP, 100, NULL);
			}
			else if(focus == 4)
			{
				ret = fota_upgrade_create(3, "/media/sdcard/upgrade/dsp/module", NULL);
				printf("fota_upgrade_create dsp ret = %d\n",ret);
				timer_upgrade_dsp_s = SetTimer(hWnd, ID_TIME_UPGRADE_DSP, 100, NULL);
			}
			
			break;
		case VK_F6:    //ç´¯åŠ 
			break;
		case VK_F7:     //æŸ¥è¯¢é”?ä¿å­˜
			break;
		case VK_F8:       //save
			spi_keyalert();
			SetFocus(GetParent(hWnd));
			DestroyWindow(hWnd);
			break;		
		default:
			break;
	}
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	s32_t 	ret;
	s32_t 	errorcode;
	u8_t 	percent;
	
	switch(wParam) 
	{
		case ID_TIME_UPGRADE_MCU:
			ret = mcu_upgrade_status_get();
			printf("mcu_upgrade_status: %d\n",ret);
			if(ret == 10)
			{
				if (timer_upgrade_mcu_s > 0)
					KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
				timer_upgrade_mcu_s = 0;
				flag[5] = 1;
				InvalidateRect(hWnd,NULL,FALSE);
			}
			else if(ret == -10)
			{
				if (timer_upgrade_mcu_s > 0)
					KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
				timer_upgrade_mcu_s = 0;
				printf("mcu upgrade fail!\n");
			}
			break;
		case ID_TIME_UPGRADE_DSP:
			ret = fota_upgrade_status(&errorcode, &percent);
			if(ret == UPGRADE_FINISH_STATUS)
			{
				if (timer_upgrade_dsp_s > 0)
					KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
				timer_upgrade_dsp_s = 0;
				flag[2] = 1;
				InvalidateRect(hWnd,NULL,FALSE);
				printf("fota_upgrade ok!\n");
			}
			break;
		default:
			break;
	}
}



