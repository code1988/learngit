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

#define Weight  130
#define Height  55 

static char pos = 0;
static char test_on = 0;
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


int agetest_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"AGETESTWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"AGETESTWIN",
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
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/agetestwin.bmp", 0);     //显示	老化界面底图
		
	hfocusMemDC = CreateCompatibleDC(hDC);
		hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 480, 320);
		hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
			FillRect(hfocusMemDC, &rect, hBrush);
				GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 480, 320, "/bmp/fota/agetest1win.bmp", 0);     //	老化界面焦点图
	
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
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

	hDC = BeginPaint(hWnd, &ps);
	
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
						
								
				BitBlt(hMemDC, 160, 57+pos*Height, Weight, Height,hfocusMemDC, 160, 57+pos*Height, SRCCOPY);
				if(test_on)
					BitBlt(hMemDC,  294, 57+pos*Height, 40, 57,hfocusMemDC, 294, 57, SRCCOPY);	
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
			break;
		case VK_F2:    //right
			break;
		case VK_F3:       //up
			if(pos>0)
				pos--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:     //down
			if(pos<3)
				pos++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F5:    //clear  //开相应位置的老化
			test_on = 1;
			mcu_mode_set(MCU_BURN_IN_MODE);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:     //yes   关相应位置的老化
			test_on = 0;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:       //save
			
			break;
		case VK_F8:       //return
			pos = 0;
			test_on = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		default:
			break;
	}
	return 0;
}



