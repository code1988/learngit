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
static u32_t    totalpaper= 0;
static  MONEY_UNITINFO_S  paper[7];
static unsigned char pages =1;
static char banben[3][10]={"1999","2005","unclear"};
static char par[6]={1,5,10,20,50,100};
static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);

int guanzihaowin_create(HWND hWnd)
{
	WNDCLASS	wndclass;

	printf("this is begin!\n");
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
	wndclass.lpszClassName  = (LPCSTR)"GUANZIHAO";
	RegisterClass(&wndclass);
	printf("totalpaper2\n");

	/* create menu window */
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"GUANZIHAO",
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
	totalpaper = moneybunch_count_get();
	
	
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
	
	SetBkMode(hMemDC, TRANSPARENT);
	SetBkColor(hMemDC, RGB(0, 0, 0));
	SetTextColor(hMemDC, RGB(255, 255, 255));
	totalpaper = moneybunch_count_get();
	printf("totalpaper = %d\n",totalpaper);
	printf("totalpaper3\n");

	
	for (j = 1; j < 8; j++) {
		int		index;
		index = j + (pages-1)*7;
		printf("index = %d\n",index);
		if (index > totalpaper)
			continue;
		moneybunch_get(index-1,&paper[j-1]);
		memset(buf,0,17);
		sprintf(buf, "%04d", index);
		TextOut(hMemDC, 39-strlen(buf)*6, 56 + (j-1)*34, buf, -1);   //xuhao
		
		memset(buf,0,17);
		//sprintf(buf,"%04d",paper[index-1].ver);
		TextOut(hMemDC, 110-strlen(banben[paper[j-1].ver-1])*6, 56 + (j-1)*34, banben[paper[j-1].ver-1], strlen(banben[paper[j-1].ver-1])); //banben		
		
		memset(buf,0,17);
		sprintf(buf,"%d",par[paper[j-1].Valuta-1]);
		TextOut(hMemDC, 184-strlen(buf)*6, 56 + (j-1)*34, buf, strlen(buf)); //miane
		
		memset(buf,0,17);
		TextOut(hMemDC, 302-strlen(paper[j-1].crown)*6, 56 + (j-1)*34 ,paper[j-1].crown, strlen(paper[j-1].crown));  //guanzihao

		if(paper[j-1].flag==0)
			TextOut(hMemDC, 402, 56 + (j-1)*34 ,"Y", -1);  //guanzihao
		else
			TextOut(hMemDC, 448, 56 + (j-1)*34 ,"N", -1);  //guanzihao
		
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
	sprintf(buf+3, "%02d", (totalpaper+6)/7);
	TextOut(hMemDC, 407, 294, buf, -1);

	
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
	int i = 0;
	int j = 0;
	switch(wParam) {
		case VK_F1:
			if(pages<(totalpaper+6)/7)
				i = 7;
			else
				i = totalpaper%7;
			j = black_insert( paper[i-1].crown, 1);
			printf("1=%d\n",(totalpaper+6)/7);
			printf("2=%d\n",pages);
			printf("aaa%s\n",paper[i-1].crown);
			printf("3=%d\n",j);
			printf("4=%d\n",i);
			//InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F2:
			break;
		case VK_F3:
			break;
		case VK_F4:
			if(pages<(totalpaper+6)/7)
			{
				for(i = 0;i<7;i++)
				{
					black_insert( paper[i].crown, 1);
	
				}

			}
			else 
			{
				for(i = 0;i<(totalpaper%7);i++)
				{
					black_insert( paper[i].crown, 1);
	
				}

			}
			//InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F5:
			Blacknamelist_create(hWnd);
			break;
		case VK_F6:
			break;
		case VK_F7:
			if (pages < (totalpaper+6)/7)
				pages++;
			else
				pages = 1;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
			break;
		case VK_F8:
			totalpaper = 0;
			pages = 1;
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		default:
			break;
	}
	return 0;
}

