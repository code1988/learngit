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



static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HDC		hfocusMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;
static char	keynum[15] = {0};
static char x = 0;
static char y = 0;

//static char		character[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
//														  'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
//														  'W', 'X', 'Y' ,'Z', '.', '*', '_', '\\','/', ' ', ' ',
//														'0', '1', '2','3', '4', '5', '6', '7', '8', '9', ' ' };
 

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
//static void refresh_keytable(HWND hWnd, char old_x, char old_y, char pos_x, char pos_y);

int myMACwin_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"MYMAC";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"MYMAC",
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
	s8_t mac[6];
	hDC = GetDC(hWnd);
	hBgMemDC = CreateCompatibleDC(hDC);													//创建兼容HDC
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);						    //创建兼容位图
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0;
	rect.right = WINDOW_WIDTH;
	rect.top = 0;
	rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);                                           //绘图之前进行位图清除
	
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/keyboardwin.bmp", 0);     //显示	网点号白色字样图

	hfocusMemDC = CreateCompatibleDC(hDC);
	hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
	FillRect(hfocusMemDC, &rect, hBrush);
	GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/keyboard1win.bmp", 0);     //	网点号黄色字样图

	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	keynum[0] = '\0';
	//net1_mac_get(keynum);
	
	ifmac_get_f("eth0", mac);
	
	printf("x0 =%x\n",mac[0]);
	printf("x1 =%x\n",mac[1]);
	printf("x2 =%x\n",mac[2]);
	printf("x3 =%x\n",mac[3]);
	printf("x4 =%x\n",mac[4]);
	printf("x5 =%x\n",mac[5]);

	
		sprintf(keynum,"%02x",mac[0]);
		sprintf(keynum+2,"%02x",mac[1]);
	
	
		sprintf(keynum+4,"%02x",mac[2]);
	
		sprintf(keynum+6,"%02x",mac[3]);
	
		sprintf(keynum+8,"%02x",mac[4]);
	
		sprintf(keynum+10,"%02x",mac[5]);
	printf("%s\n",keynum);
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
	s8_t            i = 0,j=0;
	
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
	SetBkColor(hMemDC, RGB(0, 255, 0));
	SetBkMode(hMemDC, TRANSPARENT);
	SetTextColor(hMemDC, RGB(255, 0, 0));			
	
	SetTextColor(hMemDC, RGB(255, 255, 255));
	TextOut(hMemDC, 77, 240, "My MAC Addr", -1);
	j = strlen(keynum);
	
	for(i=0;i<j/2;i++)
	{
		TextOut(hMemDC, 77+50*i, 283, keynum+2*i, 2);	
		if(i<5)
			TextOut(hMemDC, 110+50*i, 283, "-", 1);
		
	}
	TextOut(hMemDC, 77+50*(j/2), 283, keynum+2*i, j%2);	
	
	
	
	
	TextOut(hMemDC, 155, 7, "机器MAC输入", -1);

	
	BitBlt(hMemDC, 65+30*x, 47+42*y, 30, 42,hfocusMemDC, 65+30*x, 47+42*y, SRCCOPY);
	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hDC, hOldFont);
	
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
		
		case VK_F1:    //right
			break;
		case VK_F2:     //left		
			break;
		case VK_F3:       //up	   
			break;
		case VK_F4:     //down			
			break;
		case VK_F5:    //clear			
			break;
		case VK_F6:     //yes			
			break;
		case VK_F7:       //save
		case VK_F8:   	
			spi_keyalert();
			DestroyWindow(hWnd);
			myIP_window_create(parasetwin_handle_get());
			break;
		default:
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
	}
	return 0;
}

/*
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
*/

