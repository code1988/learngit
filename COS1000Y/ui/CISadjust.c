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
//#include "device.h"

#define CISADJUST_TIMER_ID				602


static int CISadjust_timer_id = 0;
static char message[32];

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

int CISadjust_create(HWND hWnd)
{
	WNDCLASS	wndclass;

	/* register menu windows */
	wndclass.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc    = (WNDPROC)MainProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = 0;
	wndclass.hIcon          = 0;
	wndclass.hCursor        = 0;
	wndclass.hbrBackground  = NULL;
	wndclass.lpszMenuName   = NULL;
	wndclass.lpszClassName  = (LPCSTR)"CISADJUST";
	RegisterClass(&wndclass);

	/* create menu window */
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"CISADJUST",
							  (LPCSTR)"ET850",
							  WS_CHILD | WS_VISIBLE,
							  0,
							  0,
							  WINDOW_WIDTH,
							  WINDOW_HEIGHT,
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
		case WM_DESTROY:
			OnDestroy(hWnd, wParam, lParam);
			break;
		case WM_TIMER:
			OnTimer(hWnd, wParam, lParam);
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
	int state;
	hDC = GetDC(hWnd);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);
	
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/CISadjust1win.bmp", 0);
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	state  =  image_verification_start();
	if(state != 0)
	 	strcpy(message, "与图像板通信失败");
	else 
	{
		strcpy(message, "进入校验请等待");
		CISadjust_timer_id = SetTimer(hWnd, CISADJUST_TIMER_ID, 10, NULL);		
		msleep_f(2000);
		dsp_reset();
		
	}	
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (CISadjust_timer_id > 0)
	{
		KillTimer(hWnd, CISADJUST_TIMER_ID);
		CISadjust_timer_id = 0;
	}
	

	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	hMainWnd = NULL;
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int status;
	HDC				hDC;
	HFONT			 hOldFont;
	int pos[2];
	char buf[72];
	buf[0]='\0';
	switch(wParam) {
		case CISADJUST_TIMER_ID:
				status =  image_verfication_get();
				switch(status)
				{
					case VERIFICATION_STEP1_STATUS:
						strcpy(buf, "校验步骤1成功");
						pos[0] = 113;
						pos[1] =  133;
						break;
					case VERIFICATION_STEP2_STATUS:
						strcpy(buf, "校验步骤2成功");
						pos[0] = 113;
						pos[1] =  173;
						break;
					case VERIFICATION_FINISH_STATUS:
						strcpy(buf, "校验成功，请关机重启");
						pos[0] = 113;
						pos[1] =  213;
						break;
					case VERIFICATION_FAILED_STATUS:
						strcpy(buf, "校验失败，请关机重启");
						pos[0] = 113;
						pos[1] = 213;
						break;
					default:
						break;				

				}

				hDC = GetDC(hWnd);								
					hOldFont = SelectObject(hDC, (HFONT)GetFont32Handle());
						SetBkColor(hDC, RGB(0, 255, 0));
						SetBkMode(hDC, TRANSPARENT);;							
						SetTextColor(hDC,RGB(255, 255, 255));
						TextOut(hDC, pos[0], pos[1], buf, -1);	
					SelectObject(hDC, hOldFont);
				ReleaseDC(hWnd, hDC);				
				break;
		default:
				break;
	}
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC 			hDC, hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	PAINTSTRUCT 	ps;

	HFONT			 hOldFont;
	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);;							
					SetTextColor(hMemDC,RGB(255, 255, 255));
					TextOut(hMemDC, 113, 91, message, -1); 
					BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont);
		
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
		DeleteDC(hMemDC);	
	EndPaint(hWnd, &ps);

	
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
		case VK_F1:
			break;
		case VK_F2:
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
		case VK_F8:
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F9:
			spi_keyalert();
			DestroyWindow(GetMenuWinHwnd());
			break;
		default:
			break;
	}
	return 0;
}

