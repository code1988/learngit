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
//#include "device.h"
#include "fotawin.h"

#define SAVE_TIMER_ID				601


static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HDC		hfocusMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;
static char	blackname[11];
static char x = 0;
static char y = 0;

static int save_timer_id = 0;
static char message[32];
static char getblack[34];
static char		character[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
														  'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
														  'W', 'X', 'Y' ,'Z', '.', '*', '_', '\\','/', ' ', ' ',
														'0', '1', '2','3', '4', '5', '6', '7', '8', '9', ' ' };

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void refresh_keytable(HWND hWnd, char old_x, char old_y, char pos_x, char pos_y);

int blackname_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"BLACKNAME";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"BLACKNAME",
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
	HBRUSH	hBrush;
	
	hDC = GetDC(hWnd);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
		hBgMemDC = CreateCompatibleDC(hDC);													//创建兼容HDC
		hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);						    //创建兼容位图
		hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
		
		rect.left = 0;rect.right = WINDOW_WIDTH;rect.top = 0;rect.bottom = WINDOW_HEIGHT;
		FillRect(hBgMemDC, &rect, hBrush);                                           //绘图之前进行位图清除
		
		GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/keyNETwin.bmp", 0);     //显示	黑名单白色字样图
		
		hfocusMemDC = CreateCompatibleDC(hDC);
		hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
		hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
		FillRect(hfocusMemDC, &rect, hBrush);
		GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/keyboard1win.bmp", 0);     //	黑名单黄色字样图
	
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	blackname[0] = '\0';
	message[0] = '\0';
	getblack[34] = '\0';
	
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
	if (save_timer_id)
		KillTimer(hWnd, SAVE_TIMER_ID);	
	save_timer_id = 0;
	hMainWnd = NULL;

	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	HFONT		    hOldFont;

	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);
					SetTextColor(hMemDC, RGB(255, 0, 0));			
					TextOut(hMemDC, 165, 255, message, -1);
					
					SetTextColor(hMemDC, RGB(255, 255, 255));
					TextOut(hMemDC, 140, 225, blackname, -1);
					TextOut(hMemDC, 125, 7, "黑名单设置界面", -1);
				
				
					BitBlt(hMemDC, 65+30*x, 47+42*y, 30, 42,hfocusMemDC, 65+30*x, 47+42*y, SRCCOPY);
					BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, SRCCOPY);
				SelectObject(hDC, hOldFont);
		
		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
		case SAVE_TIMER_ID:
				if (save_timer_id)
					KillTimer(hWnd, SAVE_TIMER_ID);
				save_timer_id = 0;
				message[0] = '\0';
				InvalidateRect(hWnd, NULL, FALSE);
				break;
		default:
				break;
	}
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	static u8_t wildcard = 0;	// 通配符使能标志位
	u8_t len = 0; 
	len = strlen(blackname);
	switch(wParam) {
		
		case VK_F1:    //right
		  if(x == 10) {
		  	x = 0;
				refresh_keytable(hWnd, 10, y, x, y);
		  }
		  else {
		  	x++;
				refresh_keytable(hWnd, x-1, y, x, y);
			}
			break;
		case VK_F2:     //left
		if(x == 0) {
			x = 10;
			refresh_keytable(hWnd, 0, y, x, y);
		}
		else {
			x--;
			refresh_keytable(hWnd, x+1, y, x, y);
		}
		break;
		case VK_F3:       //up
	    if(y == 0) {
	   		y = 3;
				refresh_keytable(hWnd, x, 0, x, y);
	   	}
	   	else {
	    	y--;
				refresh_keytable(hWnd, x, y+1, x, y);
			}
			break;
		case VK_F4:     //down
			if(y == 3) {
				y = 0;
				refresh_keytable(hWnd, x, 3, x, y);
			}
			else {
				y++;
				refresh_keytable(hWnd, x, y-1, x, y);
			}
			break;
		case VK_F5:    //clear
			if (len <= 0)
					break;
			if (len > 0) 
			{
				if(blackname[len - 1] == '*' && blackname[len - 2] != '*')
					wildcard = 0;
				
				blackname[len - 1 ] =  '\0';
				len--;
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:     //yes
			if (len >= 10)
				break;

			// 字符合法性判断
			if(character[y*11+x] == '*' && !wildcard)
			{
			}
			else if((character[y*11+x] >= '0' && character[y*11+x] <= '9') 
				|| (character[y*11+x] >= 'A' && character[y*11+x] <= 'Z'))
			{
				if(blackname[len - 1] == '*')
					wildcard = 1;
			}
			else
			{
				// 其余字符非法
				break;
			}

			blackname[len] = character[y*11+x];
			blackname[len+1] = '\0';
			
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:       //save
			if(len == 10)
			{
				if(strcmp(blackname,"**********"))
				{
					if(black_insert( blackname, 1)==0)
					{
						FotaBlackIndication();
						strcpy(message, "保存成功");
					}
					else 
						strcpy(message, "保存失败");
				}
				else
					strcpy(message, "保存失败");
			}
			else
				strcpy(message, "保存失败");	
			
			if (save_timer_id)
				KillTimer(hWnd, SAVE_TIMER_ID);			
			blackname[0] = '\0';
			InvalidateRect(hWnd, NULL, FALSE);
			save_timer_id = SetTimer(hWnd, SAVE_TIMER_ID, 3000, NULL);
			break;
		case VK_F8:       //return
			
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F9:       //return
			
			spi_keyalert();
			DestroyWindow(GetMenuWinHwnd());
			break;
		default:
			break;
	}
	return 0;
}


static void refresh_keytable(HWND hWnd, char old_x, char old_y, char pos_x, char pos_y)
{
	HDC				hDC;
	if ((pos_x == old_x) && (pos_y == old_y))
		return;
	hDC = GetDC(hWnd);

		BitBlt(hDC, 65+30*pos_x, 47+42*pos_y, 30, 42,hfocusMemDC, 65+30*pos_x, 47+42*pos_y, SRCCOPY);
		BitBlt(hDC, 65+30*old_x, 47+42*old_y, 30, 42,hBgMemDC, 65+30*old_x, 47+42*old_y, SRCCOPY);
	
	ReleaseDC(hWnd, hDC);
}


