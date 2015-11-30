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
//#include "et850_data.h"
//#include "device.h"
#include "fotawin.h"

#define ID_TIMER_PARA		505
static int id_timer_para = 0;

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static unsigned char	level;
static unsigned char	ver;
static unsigned char	type;
static unsigned char	dir;


static unsigned int		pages = 0;

static unsigned char	*pImage = NULL;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

int picdistinguishwin_create(HWND hWnd)
{
	WNDCLASS	wndclass;
	static int	bregister = 0;

	if (!bregister) {
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
		wndclass.lpszClassName  = (LPCSTR)"picdistinguishwin";
		RegisterClass(&wndclass);
	}
	/* create menu window */
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"picdistinguishwin",
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
	HBRUSH		hBrush;

	pages = 0;
	imageinfo_refresh();
	hDC = GetDC(hWnd);
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);
	DeleteObject(hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/picdistinguish1win.bmp", 0);	
	ReleaseDC(hWnd, hDC);
	id_timer_para = SetTimer(hWnd,ID_TIMER_PARA,10,NULL);
	pImage = image_identify_load();
	if (pImage == NULL)
		return -1;
	level = 1;	
	ver = 0;
	type=1;
	dir=0;
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	if (id_timer_para > 0)
	{
		KillTimer(hWnd, ID_TIMER_PARA);
		id_timer_para = 0;
	}
	if (pImage == NULL)
		image_identify_unload(pImage);
	pImage = NULL;
	level = 1;
	ver = 0;
	type = 1;
	dir = 0;
	hMainWnd = NULL;
	return 0;
}
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_TIMER_PARA) {
		InvalidateRect(hWnd, NULL, FALSE);		
	}
	return 0;
}
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP			hMemBitmap, hOldMemBitmap;
	PAINTSTRUCT		ps;
	unsigned int	ret;
	HFONT			hOldFont;
	s32_t			i, j, index;
	u8_t			ptr[84];
	char	buf[10];
	s32_t	len;
	unsigned char	*stdptr;			
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hMemBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldMemBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	hOldFont = SelectObject(hMemDC, (HFONT)GetFont24Handle());
	SetBkMode(hMemDC, TRANSPARENT);
	SetBkColor(hMemDC, RGB(0,0,0));
	
	ret = 0;
	imageinfo_read(ptr);
	ptr[5] = fota_sensitive_image_get();
	level =  ptr[5];	
	ver = ptr[2];
	type=ptr[3];
	dir=ptr[4];

	stdptr = image_identify_ptr_get(pImage, level, ver, type, dir);
	
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 10; j++) {
			
			index = pages*30 + i*10 + j;
			if (index >= 84)
				continue;
			if(stdptr[index]==0)//close
				SetTextColor(hMemDC, RGB(68, 68, 68));
			else
				SetTextColor(hMemDC, RGB(255, 255, 255));
			sprintf(buf, "%02d", index);
			TextOut(hMemDC, 10 + i*146, 10 + j*31, buf, -1);

			sprintf(buf, "%d", stdptr[index*2]);
			len = strlen(buf);
			TextOut(hMemDC, 60 + i*146-len*6, 10 + j*31, buf, -1);

			
			sprintf(buf, "%d", ptr[index]);
			len = strlen(buf);
			TextOut(hMemDC, 110 + i*146-len*6, 10 + j*31, buf, -1);
		}
	}
	SelectObject(hMemDC, hOldFont);
	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, MWROP_SRCCOPY);
	SelectObject(hMemDC, hOldMemBitmap);
	DeleteObject(hMemBitmap);
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
			if (pages == 2)
				pages = 0;
			else
				pages++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:
			if (pages == 0)
				pages = 2;
			else
				pages--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:
			imageinfo_refresh();
			break;
		case VK_F8:
			DestroyWindow(hWnd);
			break;
		case VK_F9:
			DestroyWindow(GetMenuWinHwnd());
			break;
		default:
			break;
	}
	return 0;
}




