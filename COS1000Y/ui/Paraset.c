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
#include "fotawin.h"



static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

int parasetwin_create(HWND hWnd)
{
	WNDCLASS	wndclass;


	//MWInputWinRegister();

	wndclass.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc    = (WNDPROC)MainProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = 0;
	wndclass.hIcon          = 0;
	wndclass.hCursor        = 0;
	wndclass.hbrBackground  = NULL;
	wndclass.lpszMenuName   = NULL;
		wndclass.lpszClassName  = (LPCSTR)"parasetwin";
		RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"parasetwin",
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
		case WM_CHAR:
			{
				char buf[2];
				buf[0] = wParam;
				buf[1] = 0;
				printf("%s\n", buf);

			}
			break;
		case WM_COMMAND:
			if (wParam == 0xF0F0) 
			{
				identifywin_create(hWnd);
			}
			break;
		case WM_KEYDOWN:
			OnKeyDown(hWnd, wParam, lParam);
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
	HBRUSH		hBrush;
	
	hDC = GetDC(hWnd);
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0; rect.right = 480; rect.top = 0; rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);
	DeleteObject(hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/parasetwin.bmp", 0);
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

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) 
	{
		case VK_F1:     
			password_window_create(hWnd);
			break;
		case VK_F2:    
			blackname_window_create(hWnd);
			break;
		case VK_F3:       
			netnumber_window_create(hWnd);
			break;
		case VK_F4:     
			timeset_window_create( hWnd);
			break;
		case VK_F5:    
			myIP_window_create( hWnd);
			break;
		case VK_F6:     
			break;
		case VK_F7:       
			agetest_window_create( hWnd);
			break;
		case VK_F8:
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		default:
			break;
	}
	return 0;
}

HWND parasetwin_handle_get(void)
{
	return hMainWnd;
}
