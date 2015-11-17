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

#define SAVE_TIMER_ID				601


static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HDC		hfocusMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;
static char	keynum[40];
static char branch[40];

static char x = 0;
static char y = 0;

static int save_timer_id = 0;
static int message[32];
static char		character[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
														  'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
														  'W', 'X', 'Y' ,'Z', '.', '*', '_', '\\','/', ' ', ' ',
														'0', '1', '2','3', '4', '5', '6', '7', '8', '9', ' ' };
 



#define FLASH_TIMER_ID			600
static int		flash_timer_id = 0;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void refresh_keytable(HWND hWnd, char old_x, char old_y, char pos_x, char pos_y);

int netnumber_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"NETNUMBER";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"NETNUMBER",
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
	int state;
	
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
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/keyboardwin.bmp", 0);     //显示	网点号白色字样图
		
	hfocusMemDC = CreateCompatibleDC(hDC);
		hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 480, 320);
		hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
			FillRect(hfocusMemDC, &rect, hBrush);
				GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 480, 320, "/bmp/fota/keyboard1win.bmp", 0);     //	网点号黄色字样图
	
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	keynum[0] = '\0';
	message[0] = '\0';
	branch[0] = '\0';
	printf("message1 = %d\n",strlen(message));
					printf("message1\n");
					
	state = device_organize_get(branch,keynum);
	printf("branch = %d\n",strlen(branch));
	printf("message = %d\n",strlen(message));				
	printf("keynum = %d\n",strlen(keynum));
	if(state!=0)
		printf("there is not netnumber");
		
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
	if (flash_timer_id > 0)
		KillTimer(hWnd, FLASH_TIMER_ID);
	flash_timer_id = 0;
	hMainWnd = NULL;

	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;

	
	HFONT			 hOldFont;

	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);
					SetTextColor(hMemDC, RGB(255, 0, 0));	
					if(strlen(message)!=0)
						TextOut(hMemDC, 165, 255, message,strlen(message));
					printf("message = %d\n",strlen(message));
					printf("message\n");
					SetTextColor(hMemDC, RGB(255, 255, 255));
					TextOut(hMemDC, 125, 255, keynum, -1);
					TextOut(hMemDC, 154, 7, "网点号码输入", -1);
				
				
					BitBlt(hMemDC, 65+30*x, 47+42*y, 30, 42,hfocusMemDC, 65+30*x, 47+42*y, SRCCOPY);
					BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);
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
	char len = 0; 
	len = strlen(keynum);
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
			if (len > 0) {
				keynum[len - 1 ] =  '\0';
				len--;
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:     //yes
			if (len >= 14)
				break;
			keynum[len] = character[y*11+x];
			keynum[len+1] = '\0';
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:       //save
			if(device_organize_set( branch,keynum)==0)
				strcpy(message, "保存成功");
			else 
				strcpy(message, "保存失败");
			//printf("blacknum = %d\n", black_count_get(1));   //debug
			if (save_timer_id)
				KillTimer(hWnd, save_timer_id);			
			keynum[0] = '\0';
			InvalidateRect(hWnd, NULL, FALSE);
			save_timer_id = SetTimer(hWnd, SAVE_TIMER_ID, 1000, NULL);
			break;
		case VK_F8:       //return
			keynum[0] = '\0';
			message[0] = '\0';
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
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

