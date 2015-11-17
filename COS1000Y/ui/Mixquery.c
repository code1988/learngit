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

#define Mixqueryheight    40


static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static MONEYDISP_S monenyInfo;	// 纸币信息


static char papernum[7];
static int papersum[7];

static int papernumpos[2] = {240,46};
static int papersumpos[2] = {395,46};
								
static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void refresh_keytable(HWND hWnd,int Oldx,int Posx);


int mixquery_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"MIXQUERYWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"MIXQUERYWIN",
							  (LPCSTR)"",
							  WS_VISIBLE | WS_CHILD,
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
	int         i=0;
	hDC = GetDC(hWnd);
		hBrush = CreateSolidBrush(RGB(255, 255, 255));
	hBgMemDC = CreateCompatibleDC(hDC);													
		hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);						    
		hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
			rect.left = 0;
			rect.right = 480;
			rect.top = 0;
			rect.bottom = 320;
			FillRect(hBgMemDC, &rect, hBrush);                                           
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/mixquerywi.bmp", 0);     
			
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);

	printf("this is mingxi!\r\n");
	memset(papernum,0,sizeof(papernum));
	memset(papersum,0,sizeof(papersum));
	
	moneydisp_info_get(&monenyInfo);
	//memcpy(papernum,monenyInfo.Valuta,6);
	for(i = 0;i<6;i++)
	papernum[i]	= monenyInfo.Valuta[i];
	papernum[6]= monenyInfo.curpages - monenyInfo.errnum;
	
	papersum[0]=papernum[0]*1;
	papersum[1]=papernum[1]*5;
	papersum[2]=papernum[2]*10;
	papersum[3]=papernum[3]*20;
	papersum[4]=papernum[4]*50;
	papersum[5]=papernum[5]*100;
	papersum[6]=monenyInfo.total_sum;

	for(i = 0;i<7;i++)
	{

		printf("%d = %d   %d\n",i,papernum[i],papersum[i]);
	}

	printf("this is mingxi2!\r\n");
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
	char i = 0;
	char papernumbuf[7][10];
    char papersumbuf[7][10];


	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
			
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);
					SetTextColor(hMemDC, RGB(255, 255, 255));	
					memset(papernumbuf,0,sizeof(papernumbuf));
					memset(papernumbuf,0,sizeof(papersumbuf));
					for(i = 0;i<7;i++)
					{
						sprintf(papernumbuf[i],"%d",papernum[i]);	
						sprintf(papersumbuf[i],"%d",papersum[i]);

					}
					for(i = 0;i<7;i++)
					{

						printf("%d = %d   %d\n",i,papernum[i],papersum[i]);
					}					
											
					for(i = 0;i<7;i++)
					{						
						TextOut(hMemDC, papernumpos[0]-strlen(papernumbuf[i])*6, papernumpos[1]+i*(Mixqueryheight), papernumbuf[i], -1); 
						
						TextOut(hMemDC, papersumpos[0]-strlen(papersumbuf[i])*6, papersumpos[1]+i*(Mixqueryheight), papersumbuf[i], -1);
					}
					//TextOut(hMemDC, papersumpos[0]-strlen(papersumbuf[6])*6, papersumpos[1]+6*(Mixqueryheight), papersumbuf[6], -1);
					BitBlt(hDC, 0, 0, 480, 320,hMemDC, 0, 0, SRCCOPY);
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
		case VK_F1:     //right
			break;
		case VK_F2:    //up
			break;
		case VK_F3:       //down
			break;
		case VK_F4:       //return
			break;	
		case VK_F5:     //模式
			break;
		case VK_F6:    //累加
			break;
		case VK_F7:     //查询锿保存
			guanzihaowin_create(hWnd);
			break;
		case VK_F8:       //save
			spi_keyalert();
			SetFocus(GetParent(hWnd));
			DestroyWindow(hWnd);
			break;		
		default:
			break;
	}
	return 0;
}




