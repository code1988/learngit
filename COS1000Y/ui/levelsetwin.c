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

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static int levelpos[2] = {198,59};
static char focus[3]={0,0,0};
static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);

int levelsetwin_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"LEVELSETWIN";
	RegisterClass(&wndclass);

	/* create menu window */
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"LEVELSETWIN",
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
	int i;

	
	hDC = GetDC(hWnd);
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);
	DeleteObject(hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/levelsetwin.bmp", 0);
	ReleaseDC(hWnd, hDC);

	focus[0] = fota_sensitive_image_get();
	focus[1] = fota_sensitive_normal_get();
	focus[2] = fota_sensitive_tape_get();
	for(i = 0;i<3;i++)
	{

printf("focus = %d\n",focus[i]);
	}
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
	HFONT			 hOldFont;
	char buf[2];
	u8_t i = 0,j = 0;
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	
	hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
	SetBkColor(hMemDC, RGB(0, 255, 1));		// ±³¾°ÑÕÉ«
	SetBkMode(hMemDC, TRANSPARENT);
									
	for(j = 0;j<3;j++)
	{

		for(i = 0;i<3;i++)
		{		
				if (focus[j] == (i+1-j/2))
					SetTextColor(hMemDC, RGB(255, 0, 0));
				else
					SetTextColor(hMemDC, RGB(255, 255, 255));	
				sprintf(buf, "%d", (i+1-j/2));
				TextOut(hMemDC, levelpos[0]-6+i*86, levelpos[1]+j*56,buf , -1);  
		}


	}
	
	

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
			if(focus[0]>1)
				focus[0]--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F2:
			if(focus[1]>1)
				focus[1]--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F3:
			if(focus[2]>0)
				focus[2]--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:
			fota_sensitive_image_set(focus[0]);
			fota_sensitive_normal_set(focus[1]);
			fota_sensitive_tape_set(focus[2]);
			printf("focus0 = %d, focus1 = %d ,focus2 = %d\n",focus[0],focus[1],focus[2]);	
			fota_paramsave(1);	
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F5:
			if(focus[0]<3)
				focus[0]++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:
			if(focus[1]<3)
				focus[1]++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:
			if(focus[2]<2)
				focus[2]++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F8:
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F9:
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		default:
			break;
	}
	return 0;
}

