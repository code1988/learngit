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

static int number = 0;
static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HDC		hfocusMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;
static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);


int batchset_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"BATCHWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"BATCHWIN",
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
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/bat.bmp", 0);     //显示	老化界面底图
		
	hfocusMemDC = CreateCompatibleDC(hDC);
		hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 450, 79);
		hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
		rect.left = 0;
		rect.right = 450;
		rect.top = 0;
		rect.bottom = 79;
			FillRect(hfocusMemDC, &rect, hBrush);
				GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 450, 79, "/bmp/fota/batnum.bmp", 0);     //	老化界面焦点图
	
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
//	number[0] = '\0';
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	SelectObject(hfocusMemDC, hOldfocusBitmap);
	DeleteObject(hfocusBitmap);
	DeleteDC(hfocusMemDC);
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	HFONT			hFont, hOldFont;
	int						i, len = 0;
	char					buf[10];
	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
				sprintf(buf, "%d", number);														
					len = strlen(buf);
					for (i = 0; i < len; i++) 
					{
						BitBlt(hMemDC, 343 - (len-i)*45, 150, 45, 79, hfocusMemDC, (buf[i] - '0') * 45, 0, SRCCOPY);
					}
					BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);
		
		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

	switch(wParam) {
		case VK_F1:     //left
			number += 100;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F2:    //right
			number += 10;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F3:       //up
			number++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:     //���沢����
		
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		case VK_F5:    //clear
			number -= 100;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:     //yes
			number -= 10;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:       //save
			number--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F8:       //return
			number = 0;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F9:       //return
			number = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		default:
			break;
	}
	return 0;
}



