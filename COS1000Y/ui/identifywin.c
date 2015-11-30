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

static HWND		hMainWnd = NULL;

#define KEY_TIMER_ID			610
static int		key_timer_id = 0;
static int		press_flag = 0;
static int		press_key = 0;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static unsigned char	typevalue[7] = {100, 1, 5, 10, 20, 50, 100};
static unsigned char	vervalue[4] = {99, 99, 5, 15};

static unsigned char	level;
static unsigned char	ver;
static unsigned char	type;
static unsigned char	dir;

static unsigned int		pages = 0;
static unsigned int		focus = 0;

static unsigned char	*pImage = NULL;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int vk_f5_keydown(HWND hWnd);
static int vk_f6_keydown(HWND hWnd);
static int vk_f5_keyup(HWND hWnd);
static int vk_f6_keyup(HWND hWnd);
static unsigned int focus_offset_calc(void);
static int focus_value_add(HWND hWnd, unsigned char value);
static int focus_value_sub(HWND hWnd, unsigned char value);

int identifywin_create(HWND hWnd)
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
		wndclass.lpszClassName  = (LPCSTR)"identifywin";
		RegisterClass(&wndclass);
	}
	/* create menu window */
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"identifywin",
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
		case WM_KEYUP:
			OnKeyUp(hWnd, wParam, lParam);
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
	pImage = image_identify_load();
	if (pImage == NULL)
		return -1;
	level = 1;
	ver = 0;
	type = 1;
	dir = 0;
	pages = 0;
	focus = 0;
	press_flag = 0;
	press_key = 0;
	key_timer_id = 0;
	hDC = GetDC(hWnd);
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);
	DeleteObject(hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/picdistinguishwin.bmp", 0);
	ReleaseDC(hWnd, hDC);
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (key_timer_id > 0)
		KillTimer(hWnd, KEY_TIMER_ID);
	key_timer_id = 0;
	press_flag = 0;
	press_key = 0;
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	if (pImage == NULL)
		image_identify_unload(pImage);
	pImage = NULL;
	level = 1;
	ver = 0;
	type = 1;
	dir = 0;
	pages = 0;
	focus = 0;
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP			hBitmap, hOldBitmap;
	HBRUSH			hBrush;
	HFONT			hOldFont;
	RECT			rect;
	PAINTSTRUCT		ps;
	int				i, j, len;
	unsigned char	*ptr;
	char			buf[4];

	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	hOldFont = SelectObject(hMemDC, GetFont24Handle());
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	i = focus/10;
	j = focus%10;
	if (i&0x01) {
		rect.left = 110 + (i/2)* 123 + 20;
		rect.right = rect.left + 42;
	}
	else {
		rect.left = 110 + (i/2)* 123;
		rect.right = rect.left + 16;
	}
	rect.top = 2 + j*32;
	rect.bottom = rect.top + 29;
	FillRect(hMemDC, &rect, hBrush);
	DeleteObject(hBrush);
	SetBkMode(hMemDC, TRANSPARENT);
	SetBkColor(hMemDC, RGB(0, 0, 0));
	SetTextColor(hMemDC, RGB(255, 255, 255));

	if (dir/2)
		sprintf(buf, "%dB", typevalue[type]);
	else
		sprintf(buf, "%dA", typevalue[type]);
	TextOut(hMemDC, 6, 10, buf, -1);
	sprintf(buf, "%dV%02d", level, vervalue[ver]);
	TextOut(hMemDC, 432, 10, buf, -1);

	ptr = image_identify_ptr_get(pImage, level, ver, type, dir);
	
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 10; j++) {
			int		index;
			index = i*10 + j + pages*30;
			if (index >= 84)
				continue;
			sprintf(buf, "E%02d", index);
			TextOut(hMemDC, 67 + i*123, 4 + j*32, buf, -1);
			if (ptr[index*2+1])
				TextOut(hMemDC, 112 + i*123, 4 + j*32, "1", -1);
			else
				TextOut(hMemDC, 112 + i*123, 4 + j*32, "0", -1);
			sprintf(buf, "%d", ptr[index*2]);
			len = strlen(buf);
			TextOut(hMemDC, 133 + i*123 + (3-len)*6, 4 + j*32, buf, -1);
		}
	}

	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOldFont);
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	EndPaint(hWnd, &ps);
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (key_timer_id > 0)
		KillTimer(hWnd, KEY_TIMER_ID);
	key_timer_id = 0;
	press_key = 0;
	press_flag = 0;
	switch(wParam) {
		case VK_F1:
			focus += 10;
			focus = focus%60;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F2:
			if (focus%10)
				focus--;
			else
				focus += 9;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F3:
			if (pages < 2)
				pages++;
			else
				pages = 0;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:
			if (dir/2) {
				dir = 0;
				if (type >= 6)
					type = 1;
				else
					type++;
			}
			else
				dir = 2;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F5:
			vk_f5_keydown(hWnd);
			break;
		case VK_F6:
			vk_f6_keydown(hWnd);
			break;
		case VK_F7:
			image_identify_save(pImage);
			break;
		case VK_F8:
			if (ver >= 3) {
				ver = 1;
				if (level >= 3)
					level = 1;
				else
					level++;
			}
			else
				ver++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F9:
			DestroyWindow(GetMenuWinHwnd());
			break;
		default:
			break;
	}
	return 0;
}

static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
		case VK_F5:
			vk_f5_keyup(hWnd);
			break;
		case VK_F6:
			vk_f6_keyup(hWnd);
			break;
	}
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == KEY_TIMER_ID) {
		if (press_key == 5) {
			focus_value_add(hWnd, 10);
			press_flag = 1;
		}
		else if (press_key == 6) {
			focus_value_sub(hWnd, 10);
			press_flag = 1;
		}
	}
	return 0;
}

static int vk_f5_keydown(HWND hWnd)
{
	unsigned int	offset;
	unsigned char	*ptr;
	
	offset = focus_offset_calc();
	if (offset >= 84*2)
		return -1;
	if (offset & 0x01) {
		ptr = image_identify_ptr_get(pImage, level, ver, type, dir);
		if (*(ptr+offset) == 0) {
			*(ptr+offset) = 1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		return 0;
	}
	key_timer_id = SetTimer(hWnd, KEY_TIMER_ID, 500, NULL);
	press_flag = 0;
	press_key = 5;
	return 0;
}

static int vk_f6_keydown(HWND hWnd)
{
	unsigned int	offset;
	unsigned char	*ptr;

	offset = focus_offset_calc();
	if (offset >= 84*2)
		return -1;
	if (offset & 0x01) {
		ptr = image_identify_ptr_get(pImage, level, ver, type, dir);
		if (*(ptr+offset)) {
			*(ptr+offset) = 0;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		return 0;
	}
	key_timer_id = SetTimer(hWnd, KEY_TIMER_ID, 500, NULL);
	press_flag = 0;
	press_key = 6;
	return 0;
}

static int vk_f5_keyup(HWND hWnd)
{
	if (press_key != 5)
		return 0;
	if (press_flag == 0)
		focus_value_add(hWnd, 1);
	if (key_timer_id > 0)
		KillTimer(hWnd, KEY_TIMER_ID);
	key_timer_id = 0;
	press_flag = 0;
	press_key = 0;
	return 0;
}

static int vk_f6_keyup(HWND hWnd)
{
	if (press_key != 6)
		return 0;
	if (press_flag == 0)
		focus_value_sub(hWnd, 1);
	if (key_timer_id > 0)
		KillTimer(hWnd, KEY_TIMER_ID);
	key_timer_id = 0;
	press_flag = 0;
	press_key = 0;
	return 0;
}

static unsigned int focus_offset_calc(void)
{
	unsigned int	offset = 0;
	
	offset = (focus/20)*10;
	offset += focus%10;
	offset = offset*2 + pages*60;
	if (((focus/10) & 0x01) == 0)
		offset += 1;
	
	return offset;
}

static int focus_value_add(HWND hWnd, unsigned char value)
{
	unsigned char	*ptr;
	unsigned char	offset;
	unsigned int	sum;

	offset = focus_offset_calc();
	if (offset & 0x01)
		return 0;
	ptr = image_identify_ptr_get(pImage, level, ver, type, dir);
	sum = *(ptr+offset);
	if (sum == 255)
		return 0;
	sum = sum + value;
	if (sum > 255)
		*(ptr+offset) = 255;
	else
		*(ptr+offset) += value;
	InvalidateRect(hWnd, NULL, FALSE);
	return 0;
}


static int focus_value_sub(HWND hWnd, unsigned char value)
{
	unsigned char	*ptr;
	unsigned char	offset;

	offset = focus_offset_calc();
	if (offset & 0x01)
		return 0;
	ptr = image_identify_ptr_get(pImage, level, ver, type, dir);
	
	if (*(ptr+offset) == 0)
		return 0;
	if (*(ptr+offset) >= value)
		*(ptr+offset) -= value;
	else
		*(ptr+offset) = 0;
	InvalidateRect(hWnd, NULL, FALSE);
	return 0;
}

