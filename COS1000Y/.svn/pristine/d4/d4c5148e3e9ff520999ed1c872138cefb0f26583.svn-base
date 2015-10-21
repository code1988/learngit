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




static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static u32_t 	timer_cmd_s 	= 0;	// 纸币信息定时器
#define 		ID_TIMER_CMD0 	400	

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);



int adjustmenu_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"ADJUSTMENUWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"ADJUSTMENUWIN",
							  (LPCSTR)"",
							  WS_CHILD | WS_VISIBLE,
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
		case WM_COMMAND:
			
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
	hDC = GetDC(hWnd);
	hBgMemDC = CreateCompatibleDC(hDC);													//创建兼容HDC
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);						    //创建兼容位图
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0;
	rect.right = 480;
	rect.top = 0;
	rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);                                           //绘图之前进行位图清除
	
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/adjustmenuwin.bmp", 0);     //显示	黑名单白色字样图

	
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC;
	PAINTSTRUCT		ps;


	hDC = BeginPaint(hWnd, &ps);
	

				
	BitBlt(hDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
		
	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) 
	{
		case ID_TIMER_CMD0:
			if(timer_cmd_s)
			{
				s32_t mode;
				
				KillTimer(hWnd, ID_TIMER_CMD0);
				timer_cmd_s = 0;
				mode = mcu_mode_get();
				printf("MCU debug mode is: %d\n",mode);
				if(mode == MCU_DYNAMIC_DEBUG_MODE)
				{
					dynamicwin_create(hWnd);
				}
				else if(mode == MCU_STATIC_DEBUG_MODE)
				{
					staticwin_create(hWnd);
				}
			}
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
			if(!timer_cmd_s)
			{
				timer_cmd_s = SetTimer(hWnd,ID_TIMER_CMD0,1000,NULL);
				mcu_mode_set(MCU_DYNAMIC_DEBUG_MODE);
			}
			break;
		case VK_F2:  
			if(!timer_cmd_s)
			{
				timer_cmd_s = SetTimer(hWnd,ID_TIMER_CMD0,1000,NULL);
				mcu_mode_set(MCU_STATIC_DEBUG_MODE);
			}
			break;
		case VK_F3:       
			break;
		case VK_F4:     
			break;
		case VK_F5:    
			break;
		case VK_F6:    
			break;
		case VK_F7:       
			break;
		case VK_F8:       //return
			if(timer_cmd_s)
			{
				KillTimer(hWnd, ID_TIMER_CMD0);
				timer_cmd_s = 0;
			}
			SetFocus(GetParent(hWnd));
			DestroyWindow(hWnd);
			break;
		default:
			break;
	}
	return 0;
}



