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
#include "fotawin.h"


static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void warning_window (HWND hWnd,s8_t *buf);


int datamanage_window_create(HWND hWnd)
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
		wndclass.lpszClassName  = (LPCSTR)"DATAMANAGEWIN";
		RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"DATAMANAGEWIN",
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
		case WM_COMMAND:			
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
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/datamanagewin.bmp", 0);

	

	//报警标志
	hwarningMemDC = CreateCompatibleDC(hDC);
	hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 400, 45);
	hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
	rect.left = 0; rect.right = 400; rect.top = 0; rect.bottom = 45;
	FillRect(hwarningMemDC, &rect, hBrush);
	GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 400, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);
	
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SelectObject(hwarningMemDC, hOldwarningBitmap);
	DeleteObject(hwarningBitmap);
	DeleteDC(hwarningMemDC);

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
	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	EndPaint(hWnd, &ps);
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	status_t ret;
	char buf[64];

	switch(wParam) {
		case VK_F1:     //left
			break;
		case VK_F2:    //SD卡参数导入
			if(ret == OK_T)
			{
				memset(buf,0,64);
				memcpy(buf,"导入完毕，按币种键刷新",strlen("导入完毕，按批量键刷新"));
				warning_window(hWnd,buf);	

			}
			else if(ret != OK_T )
			{

				memset(buf,0,64);
				memcpy(buf,"导入失败，按币种键刷新",strlen("导入失败，按批量键刷新"));
				warning_window(hWnd,buf); 
			}		
			break;
		case VK_F3:       //up
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:     //down
			break;
		case VK_F5:    //clear
			break;
		case VK_F6:     //yes
			uploadimg_window_create(hWnd);
			break;
		case VK_F7:       //save
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

static void warning_window (HWND hWnd,s8_t *buf)
{
	HDC 			hDC,hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	HFONT			 hOldFont;
	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 400, 45);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
		BitBlt(hMemDC, 0, 0, 400, 45, hwarningMemDC, 0, 0, SRCCOPY);
		hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
			SetBkColor(hMemDC, RGB(0, 255, 0));
			SetBkMode(hMemDC, TRANSPARENT);;							
			SetTextColor(hMemDC,RGB(255,0 , 0));
			
			TextOut(hMemDC, 48, 10, buf, -1);

			
			BitBlt(hDC, 40, 110,400,45,hMemDC, 0, 0, SRCCOPY);
		SelectObject(hMemDC, hOldFont);

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);


}

