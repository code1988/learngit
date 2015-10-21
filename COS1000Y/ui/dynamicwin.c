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

static u32_t 	timer_cmd_s 	= 0;	// 纸币信息定时器
#define 		ID_TIMER_CMD1 	401	

static DYNAMICDEBUG_S dynamic;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

int dynamicwin_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"DYNAMIC";
	RegisterClass(&wndclass);

	/* create menu window */
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"DYNAMIC",
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
	
	hDC = GetDC(hWnd);
	
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0; rect.right = 480; rect.top = 0; rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/dynamicwin.bmp", 0);

	timer_cmd_s = SetTimer(hWnd,ID_TIMER_CMD1,1000,NULL);

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
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	HFONT			hOldFont;
	u8_t			i;
	
	hDC = BeginPaint(hWnd, &ps);

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);

	hOldFont = SelectObject(hMemDC, (HFONT)GetFont24Handle());
	SetBkColor(hMemDC, RGB(0, 255, 0));
	SetBkMode(hMemDC, TRANSPARENT);
	SetTextColor(hMemDC, RGB(255, 255, 255));

	s8_t buf[8];

	
	for(i=0;i<6;i++)
	{
		memset(buf,0,8);
		sprintf(buf,"%03d",dynamic.ir[i]);
		TextOut(hMemDC, 58, 72 + 28*i,buf, strlen(buf));
	}

	for(i=0;i<6;i++)
	{
		memset(buf,0,8);
		sprintf(buf,"%03d",dynamic.tape[i]);
		TextOut(hMemDC, 126, 72 + 28*i, buf, strlen(buf));
	}

	for(i=0;i<6;i++)
	{
		memset(buf,0,8);
		sprintf(buf,"%03d",dynamic.pulse[i]);
		TextOut(hMemDC, 270, 72 + 28*i, buf, strlen(buf));
	}

	
	for(i=0;i<4;i++)
	{
		memset(buf,0,8);
		sprintf(buf,"%03d",dynamic.magnetic[i]);
		TextOut(hMemDC, 400, 72 + 28*i, buf, strlen(buf));
	}

	BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hDC, hOldFont);
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
			mcu_mode_set(MCU_NORMAL_MODE);
			break;
		default:
			break;
	}
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) 
	{
		case ID_TIMER_CMD1:
			if(timer_cmd_s)
			{
				s32_t mode;

				mode = mcu_mode_get();
				if(!mode)
				{
					KillTimer(hWnd, ID_TIMER_CMD1);
					timer_cmd_s = 0;
					DestroyWindow(hWnd);
					SetFocus(GetParent(hWnd));
				}

				debugdynamic_read((u8_t *)(&dynamic),debugdynamic_count_get() - 1,1);
				printf("pulse:%d %d %d %d %d %d\n",dynamic.pulse[0],dynamic.pulse[1],dynamic.pulse[2],dynamic.pulse[3],dynamic.pulse[4],dynamic.pulse[5]);
				printf("ir:%d %d %d %d %d %d\n",dynamic.ir[0],dynamic.ir[1],dynamic.ir[2],dynamic.ir[3],dynamic.ir[4],dynamic.ir[5]);
				printf("tape:%d %d %d %d %d %d\n",dynamic.tape[0],dynamic.tape[1],dynamic.tape[2],dynamic.tape[3],dynamic.tape[4],dynamic.tape[5]);
				
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		default:
				break;
	}
	return 0;
}

