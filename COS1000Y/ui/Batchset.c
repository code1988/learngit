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
//#include "et850_data.h"
#include "fotawin.h"

static s16_t 	number = 0;		// 批量值
static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static HDC		hfocusMemDC = NULL;
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
	hBgMemDC = CreateCompatibleDC(hDC);													
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);						   
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);

	rect.left = 0;rect.right = WINDOW_WIDTH;rect.top = 0;rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);                                           
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/bat.bmp", 0);     

	hfocusMemDC = CreateCompatibleDC(hDC);
	hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 450, 79);
	hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
	rect.left = 0;rect.right = 450;rect.top = 0;rect.bottom = 79;
	FillRect(hfocusMemDC, &rect, hBrush);
	GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 450, 79, "/bmp/fota/batnum.bmp", 0);     

	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);

	// 进入批量界面后首先获取一下当前的批量值
	// preset_value_get函数返回值: >0 - 批量值；0 - 累加状态；-1 - 正常状态
	number =  preset_value_get();
	if(number == -1)
		number = 0;
	printf("number = %d",number);
	
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
	HBITMAP			hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	int				i, len = 0;
	char			buf[5];
	
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);

	// 显示批量值，当前批量上限9999张
	sprintf(buf, "%d", number);														
	len = strlen(buf);
	for (i = 0; i < len; i++) 
	{
		BitBlt(hMemDC, 343 - (len-i)*45, 150, 45, 79, hfocusMemDC, (buf[i] - '0') * 45, 0, SRCCOPY);
	}
	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, SRCCOPY);

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
			if(number<9899)
				number += 100;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F2:    //right
			if(number<9989)
				number += 10;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F3:       //up
			if(number<9998)
				number++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:     //保存并返回
			if(number>0)
			{
				// 批量值大于0时，保存批量值
				preset_value_set(number);
			}
			else if(number == 0)
			{
				//批量值为0时，切换到正常状态，通知主界面
				preset_value_set(-1);
				
			}
			//SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)0xF0F1, (LPARAM)0);

			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F5:    //clear
			if(number>=100)
			number -= 100;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:     //yes
			if(number>=10)
			number -= 10;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:       //save
			if(number>=1)
			number--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F8:       // 清零并返回
			// 批量值清零，返回正常状态，通知主界面
			number = 0;
			preset_value_set(-1);
			//SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)0xF0F1, (LPARAM)0);
		    //moneydisp_info_clear();
		   // fota_clearzero();
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F9:       //return
			// 通过判断preset_value_get的返回值，决定进入到哪种状态
			//number =  preset_value_get();
			//if(number == -1)
				//SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)0xF0F1, (LPARAM)0);
			
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		default:
			break;
	}
	return 0;
}



