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



static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HDC		hfocusMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;
static char	keynum[15]={0};
static char x = 0;
static char y = 3;
static u16_t myPort;
static char		character[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
														  'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
														  'W', 'X', 'Y' ,'Z', '.', '*', '_', '\\','/', ' ', ' ',
														'0', '1', '2','3', '4', '5', '6', '7', '8', '9', ' ' };
 

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void refresh_keytable(HWND hWnd, char old_x, char old_y, char pos_x, char pos_y);

int myPortwin_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"MYPORT";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"MYPORT",
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
			//OnTimer(hWnd, wParam, lParam);
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
	
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/keyboardwin.bmp", 0);     //显示	网点号白色字样图

	hfocusMemDC = CreateCompatibleDC(hDC);
	hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 480, 320);
	hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
	FillRect(hfocusMemDC, &rect, hBrush);
	GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 480, 320, "/bmp/fota/keyboard1win.bmp", 0);     //	网点号黄色字样图

	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	keynum[0] = '\0';
	tcpmonitor_cntl_port_get(&myPort);
	sprintf(keynum,"%d",myPort);
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
	HFONT			hOldFont;
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
	hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
	SetBkColor(hMemDC, RGB(0, 255, 0));
	SetBkMode(hMemDC, TRANSPARENT);
	SetTextColor(hMemDC, RGB(255, 0, 0));			
	
	SetTextColor(hMemDC, RGB(255, 255, 255));

	// 显示"My Port No"
	TextOut(hMemDC, 77, 240, "My Port No", -1);

	// 显示输入端口数字
	TextOut(hMemDC, 77, 283, keynum,5);	
	
	// 显示"机器端口输入"
	TextOut(hMemDC, 155, 7, "机器端口输入", -1);

	// 选中字符的反黄显示
	BitBlt(hMemDC, 65+30*x, 47+42*y, 30, 42,hfocusMemDC, 65+30*x, 47+42*y, SRCCOPY);
	BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hDC, hOldFont);
	
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	
	s8_t len = 0; 
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
			break;
		case VK_F4:     //down
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
			if (len >= 5)
				break;
			keynum[len] = character[y*11+x];
			keynum[len+1] = '\0';
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:       //save
			printf("new local port:%s\n",keynum);
			tcpmonitor_cntl_port_get(&myPort);
			printf("old local port:%d\n",myPort);
			myPort = atoi(keynum);
			printf("%d\n",myPort);
			tcpmonitor_cntl_port_set(myPort);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F8: 
			spi_keyalert();
			DestroyWindow(hWnd);
			serverPortwin_create(parasetwin_handle_get());
			break;
		default:
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
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

