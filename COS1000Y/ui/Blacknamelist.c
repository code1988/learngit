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

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static u32_t    totalname= 0;
static char  blacknames[7][16];

static char disp_total = -1;
static char pages = 1;
static char focus = 0; 

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);

int Blacknamelist_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"BLACKNAMELIST";
	RegisterClass(&wndclass);
	

	/* create menu window */
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"BLACKNAMELIST",
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
	
	hDC = GetDC(hWnd);
	hBgMemDC = CreateCompatibleDC(hDC);
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0; rect.right = 480; rect.top = 0; rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);
	DeleteObject(hBrush);
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/guanzihwin.bmp", 0);
	ReleaseDC(hWnd, hDC);
	totalname = black_count_get(1);
	printf("%d\n",totalname);
	
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
	HBITMAP			hBitmap, hOldBitmap;
	HFONT			hOldFont;
	HBRUSH			hBrush;
	RECT			rect;
	PAINTSTRUCT		ps;
	int			    j;
	//unsigned char	*ptr;
	char			buf[17];
	int datetime,year,months,day,hour,minute,second;
	datetime = time(NULL);
	datetime_separate_f(datetime, &year, &months, &day, &hour, &minute, &second);
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	hOldFont = SelectObject(hMemDC, GetFont24Handle());
	BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
	
	hBrush = CreateSolidBrush(RGB(255,0,0));
		rect.left = 9;
		rect.right = 70;
		rect.top = 57+focus*34;
		rect.bottom = 86+focus*34;
		FillRect(hMemDC, &rect, hBrush);
	DeleteObject(hBrush);



	SetBkMode(hMemDC, TRANSPARENT);
	SetBkColor(hMemDC, RGB(0, 0, 0));
	SetTextColor(hMemDC, RGB(255, 255, 255));
	disp_total = black_select(blacknames[0],(pages-1)*7,7,1 );
		
	for (j = 0; j < 7; j++) {
		int		index;
		index = j + 1 + (pages-1)*7;
		printf("j = %d\n",j);
		if (index > totalname)
			continue;
		printf("index = %d\n",index);
		//memset(blacknames[j],0,sizeof(blacknames[j]));
		//disp_total = black_select(blacknames[j],j+(pages-1)*7,1,1 );
		//printf("%s\n",blacknames[j]);
		//printf("disp_total =%d\n",disp_total );
		
		memset(buf,0,17);
		sprintf(buf, "%04d", index);				
		TextOut(hMemDC, 39-strlen(buf)*6, 56 + (j)*34, buf, -1);   //xuhao
		
		memset(buf,0,17);
		TextOut(hMemDC, 110-2*6, 56 + (j)*34,"NC", -1); //banben		
		
		
		//memset(buf,0,17);
		//TextOut(hMemDC, 302-strlen(blacknames[j])*6, 56 + (j)*34 ,blacknames[j], strlen(blacknames[j]));  //heimingdan
		TextOut(hMemDC, 302-10*6, 56 + (j)*34 ,blacknames[j], 10);  //heimingdan
		printf("%s\n",blacknames[j]);
		
		TextOut(hMemDC, 448, 56 + (j)*34 ,"N", -1);  //N
		
	}
	sprintf(buf, "%04d", year);
	buf[4] = '_';
	sprintf(buf+5, "%02d", months);
	buf[7] = '_';
	sprintf(buf+8, "%02d", day);
	buf[10] = ' ';
	sprintf(buf+11, "%02d", hour);
	buf[13] = ':';
	sprintf(buf+14, "%02d", minute);
	TextOut(hMemDC, 7, 294, buf, -1);

	memset(buf,0,17);
	sprintf(buf, "%02d", pages);
	buf[2] = '/';
	sprintf(buf+3, "%02d", (totalname+6)/7);
	TextOut(hMemDC, 407, 294, buf, -1);

	printf("totalpage = %d\n",(totalname+6)/7);
	
	BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);
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
			
			break;
		case VK_F2:
			if(focus>0)
				focus--;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F3:
			if(focus<6&&focus<(totalname%7-1))
				focus++;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:
			
			break;
		case VK_F5:
			printf("focus1 = %d\n",focus );
			black_delete(blacknames[focus]);
			if((focus+1)==totalname%7&&focus>0)
				focus--;
			printf("totalname%7 = %d\n",totalname%7 );
			printf("focus2 = %d\n",focus );
			totalname = black_count_get(1);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:
			 blacklist_clear(1);
			 focus = 0;
			 totalname = black_count_get(1);
			 InvalidateRect(hWnd, NULL, FALSE);
			 break;
		case VK_F7:
			if (pages < (totalname+6)/7)
				pages++;
			else
				pages = 1;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F8:
			totalname = 0;
			pages = 1;
			focus = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		default:
			break;
	}
	return 0;
}

