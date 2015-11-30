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

static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;

static u32_t    totalpaper= 0;		// 总张数
static  MONEY_UNITINFO_S  paper[7];	// 每页的7张纸币
static unsigned char pages =1;
static char banben[4][10]={"1999","2005","2015","unclear"};
static char par[6]={1,5,10,20,50,100};
static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void warning_window (HWND hWnd,s8_t *buf);


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
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
		hBgMemDC = CreateCompatibleDC(hDC);
		hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
		hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap); 		
		rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
		FillRect(hBgMemDC, &rect, hBrush);		
		GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/guanzihwin.bmp", 0);

		//报警标志
		hwarningMemDC = CreateCompatibleDC(hDC);
		hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 400, 45);
		hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
		rect.left = 0; rect.right = 400; rect.top = 0; rect.bottom = 45;
		FillRect(hwarningMemDC, &rect, hBrush);
		GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 400, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	totalpaper = moneybunch_count_get();
	
	
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	
	SelectObject(hwarningMemDC, hOldwarningBitmap);
	DeleteObject(hwarningBitmap);
	DeleteDC(hwarningMemDC);
	hMainWnd = NULL;
	printf("go out B\n");
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
	hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	hOldFont = SelectObject(hMemDC, GetFont24Handle());
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	
	SetBkMode(hMemDC, TRANSPARENT);
	SetBkColor(hMemDC, RGB(0, 0, 0));
	SetTextColor(hMemDC, RGB(255, 255, 255));
	totalpaper = moneybunch_count_get();
	printf("totalpaper = %d\n",totalpaper);

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
		sprintf(buf,"%s",paper[j-1].crown);
		TextOut(hMemDC, 302-strlen(buf)*6, 56 + (j-1)*34 ,buf, strlen(buf));  //guanzihao

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
	if((totalpaper+6)/7==0)
	{
		sprintf(buf+3, "%02d", 1);
	}
	else 
		sprintf(buf+3, "%02d", (totalpaper+6)/7);
	TextOut(hMemDC, 407, 294, buf, -1);

	
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
	int i = 0;
	int j = 0;
	MONEY_UNITINFO_S per_info;
	char *p;
	u32_t totalname;
	status_t ret;
	char buf[64];
	static u8_t loadfinish = 0;
	switch(wParam) {
		case VK_F1:
			loadfinish = 1;
			printf("F1loadfinish  = %d\n",loadfinish);
			moneybunch_get(totalpaper - 1,&per_info);
			totalname = black_count_get(1);
			if(totalname<199)
			{
				printf("Insert blackname:%s\r\n",per_info.crown);			
				j = black_insert(per_info.crown, 1);			
				ret  = FotaBlackIndication();	
				if(ret == OK_T)
				{
					loadfinish = 0;
					memset(buf,0,64);
					memcpy(buf,"加载完毕，按币种键刷新",strlen("加载完毕，按币种键刷新"));
				    warning_window(hWnd,buf);   

				}
				else if(ret != OK_T )
				{

					loadfinish = 0;

					memset(buf,0,64);
					memcpy(buf,"加载失败，按币种键刷新",strlen("加载失败，按币种键刷新"));
					warning_window(hWnd,buf); 
				}
				

			}	
			else
			{
				loadfinish  = 0;
				memset(buf,0,64);
				memcpy(buf,"黑名单数超过最大限制",strlen("黑名单数超过最大限制"));
				warning_window(hWnd,buf);

			}
			printf("F1-1loadfinish  = %d\n",loadfinish);
			break;
		case VK_F2: 
			if(loadfinish==0)
				InvalidateRect(hWnd, NULL, FALSE);
			printf("F3loadfinish  = %d\n",loadfinish);
			break;
		case VK_F3:
			printf("F3loadfinish  = %d\n",loadfinish);
			break;
		case VK_F4:
				
			 	loadfinish = 1;
				printf("F4loadfinish  = %d\n",loadfinish);
				p = (char *)malloc(16*totalpaper);
				memset(p,0,16*totalpaper);
				totalname = black_count_get(1);
				if((totalname+totalpaper)>200)
					totalpaper = 200-totalname;
				for(i = 0;i<totalpaper;i++)
				{
					moneybunch_get(i,&per_info);
					memcpy(p+16*i,per_info.crown,10);
					printf("%dst : %s\r\n",i+1,p+16*i);
				}
				memset(buf,0,64);
				memcpy(buf,"正在加载",strlen("正在加载"));
				warning_window(hWnd,buf); 

				printf("Total count is %d\r\n",totalpaper);
				
				ret = blacklist_insert(p,totalpaper,1);
				printf("ret = %d\n",ret);
				ret = FotaBlackIndication();
				printf("ret =%d\n",ret);
				free(p);
				if(ret == OK_T)
				{
					loadfinish = 0;
					memset(buf,0,64);

					memcpy(buf,"加载完毕，按币种键刷新",strlen("加载完毕，按币种键刷新"));
				    warning_window(hWnd,buf);   

				}
				else if(ret != OK_T )
				{
					loadfinish = 0;

					memset(buf,0,64);

					memcpy(buf,"加载失败，按币种键刷新",strlen("加载失败，按币种键刷新"));
					warning_window(hWnd,buf); 

				}
				printf("ret = %d\n",ret);
				printf("F4-1loadfinish  = %d\n",loadfinish);
			break;
		case VK_F5:
			if(loadfinish==0)
				Blacknamelist_create(hWnd);
			printf("F5loadfinish  = %d\n",loadfinish);
			break;
		case VK_F6://打印
		printf("F6loadfinish  = %d\n",loadfinish);
			break;
		case VK_F7:
			if(loadfinish==0)
			{
				if (pages < (totalpaper+6)/7)
					pages++;
				else
					pages = 1;
				InvalidateRect(hWnd, NULL, FALSE);
				

			}
			printf("F7loadfinish  = %d\n",loadfinish);
			break;
		case VK_F8:
			if(loadfinish==0)
			{
				spi_keyalert();
				pages = 1;
				DestroyWindow(hWnd);
				

			}
			printf("F8loadfinish  = %d\n",loadfinish);
			break;
		case VK_F9:
			if(loadfinish==0)
			{
				spi_keyalert();
				pages = 1;
				DestroyWindow(hWnd);

			}
			printf("F9loadfinish  = %d\n",loadfinish);
			break;
		default:
			break;
	}
	return 0;
}

static void warning_window (HWND hWnd,s8_t *buf)
{
	HDC 			hDC,hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	HFONT			 hOldFont;
	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 400, 45);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
		BitBlt(hMemDC, 0, 0, 400, 45, hwarningMemDC, 0, 0, SRCCOPY);
		hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
			SetBkColor(hMemDC, RGB(0, 255, 0));
			SetBkMode(hMemDC, TRANSPARENT);;							
			SetTextColor(hMemDC,RGB(255,0 , 0));
			
			TextOut(hMemDC, 48, 10, buf, -1);

			
			BitBlt(hDC, 40, 110,400,45,hMemDC, 0, 0, SRCCOPY);
		SelectObject(hMemDC, hOldFont);

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);


}


