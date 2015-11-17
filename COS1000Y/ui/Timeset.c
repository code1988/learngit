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
//static HDC		hfocusMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
//static HBITMAP	hfocusBitmap, hOldfocusBitmap;
//static char	blackname[11];

static int	old_x = 0;
static int	pos_x = 0;


static int save_timer_id = 0;
static int message[32];
static int pos[6][4] = {   {72,80,86,43},  {211,80,65,43} , {320,80,65,43},
						   {82,190,65,43}, {193,190,65,43}, {302,190,65,43}};
					
					
static int timepos[6][2] = {
							{108,90},  {243,90} , {352,90},
							{114,200}, {225,200}, {335,200}			
						   };
static char Day[12] = {31,28,31, 30,31,31, 31,30,30, 31,30,31};
static int year, months, day, hour, minute, second;

#define FLASH_TIMER_ID			600
static int		flash_timer_id = 0;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void refresh_keytable(HWND hWnd, int oldx, int oldy, int posx, int posy,int weight0,int height0);


int timeset_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"TIMESETWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"TIMESETWIN",
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
	int			datetime;
	
	hDC = GetDC(hWnd);
		hBrush = CreateSolidBrush(RGB(255, 255, 255));
	hBgMemDC = CreateCompatibleDC(hDC);													//创建兼容HDC
		hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);						    //创建兼容位图
		hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
			rect.left = 0;
			rect.right = 480;
			rect.top = 0;
			rect.bottom = 320;
			FillRect(hBgMemDC, &rect, hBrush);                                           //绘图之前进行位图清除
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/timesetwin.bmp", 0);     //显示	时间设置界面白色字样图
		
	//hfocusMemDC = CreateCompatibleDC(hDC);
	//	hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 480, 320);
	//	hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
	//		FillRect(hfocusMemDC, &rect, hBrush);
	//			GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 480, 320, "/bmp/fota/timeset1win.bmp", 0);     //	时间设置界面红色字样图
	
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	datetime = time(NULL);
	datetime_separate_f(datetime, &year, &months, &day, &hour, &minute, &second);
	message[0] = '\0';
	
//	flash_timer_id = SetTimer(hWnd, FLASH_TIMER_ID, 10, NULL);
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//printf("%s\n", __func__);
	if (flash_timer_id > 0)
		KillTimer(hWnd, FLASH_TIMER_ID);
	flash_timer_id = 0;
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	//SelectObject(hfocusMemDC, hOldfocusBitmap);
	//DeleteObject(hfocusBitmap);
	//DeleteDC(hfocusMemDC);
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	HBRUSH			hBrush;
	RECT			rect;
	HFONT			hOldFont;
    char			yearbuf[10];
	char			monthsbuf[10];
	char			daybuf[10];
	char			hourbuf[10];
	char			minutebuf[10];
	char			secondbuf[10];
	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
					hBrush = CreateSolidBrush(RGB(255,0,0));
						rect.left = pos[pos_x][0]+1;
						rect.right = pos[pos_x][0]+pos[pos_x][2]-1;
						rect.top = pos[pos_x][1]+1;
						rect.bottom = pos[pos_x][1]+pos[pos_x][3]-1;
						FillRect(hMemDC, &rect, hBrush);
					DeleteObject(hBrush);
					sprintf(yearbuf, "%d", year);
					sprintf(monthsbuf, "%d", months);
					sprintf(daybuf, "%d", day);
					sprintf(hourbuf, "%d", hour);
					sprintf(minutebuf, "%d", minute);
					sprintf(secondbuf, "%d", second);
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);
					SetTextColor(hMemDC, RGB(255, 0, 0));			
					TextOut(hMemDC, 165, 255, message, -1);
					
					SetTextColor(hMemDC, RGB(255, 255, 255));
											
					//BitBlt(hMemDC, pos[pos_x][0], pos[pos_x][1], pos[pos_x][2], pos[pos_x][3],hfocusMemDC, pos[pos_x][0], pos[pos_x][1], SRCCOPY);
					TextOut(hMemDC, timepos[0][0]-strlen(yearbuf)*6, timepos[0][1], yearbuf, -1);  
					TextOut(hMemDC, timepos[1][0]-strlen(monthsbuf)*6, timepos[1][1], monthsbuf, -1);
					TextOut(hMemDC, timepos[2][0]-strlen(daybuf)*6, timepos[2][1], daybuf, -1);
					TextOut(hMemDC, timepos[3][0]-strlen(hourbuf)*6, timepos[3][1], hourbuf, -1);
					TextOut(hMemDC, timepos[4][0]-strlen(minutebuf)*6, timepos[4][1], minutebuf, -1);
					TextOut(hMemDC, timepos[5][0]-strlen(secondbuf)*6, timepos[5][1], secondbuf, -1);
					BitBlt(hDC, 0, 0, 480, 320,			 hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont);
		
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
	
	pos_x = old_x;
	switch(wParam) {
		case VK_F1:     //right
			if(old_x == 2) 
			{
				pos_x = 0;
			}
			else if(old_x==5)
			{
				pos_x = 3;			
			}
			else
			{
				pos_x++;
			}		
			refresh_keytable(hWnd, pos[old_x][0] , pos[old_x][1], pos[pos_x][0], pos[pos_x][1],old_x,pos_x);
			//InvalidateRect(hWnd, NULL, FALSE);
			old_x = pos_x;
			break;
		case VK_F2:    //up
			if(old_x > 2) 
			{
				pos_x = old_x-3;
			}
			refresh_keytable(hWnd, pos[old_x][0] , pos[old_x][1], pos[pos_x][0], pos[pos_x][1],old_x,pos_x);
			//InvalidateRect(hWnd, NULL, FALSE);
			old_x = pos_x;
			break;
		case VK_F3:       //down
			if(old_x < 3) 
			{
				pos_x = old_x+3;
			}
			refresh_keytable(hWnd, pos[old_x][0] , pos[old_x][1], pos[pos_x][0], pos[pos_x][1],old_x,pos_x);
			//InvalidateRect(hWnd, NULL, FALSE);
			old_x = pos_x;
			break;
		case VK_F5:     //模式
			switch(pos_x)
			{
				case 0:
					if(year<2200)
						year++;
					break;
				case 1:
					if(months<12)
						months++;
					break;
				case 2:
					if((year%4==0&&year%100!=0)||(year%400==0)) //闰年
					{
						
						Day[1] = 29;
					}
					else 
					{	
						Day[1] = 28;
					}
					if(day<Day[months-1])
						day++;
					break;
				case 3:
					if(hour<24)
						hour++;
					break;
				case 4:
					if(minute<60)
						minute++;
					break;
				case 5:
					if(second<60)
						second++;
					break;
				default:
					break;
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:    //累加
			//printf("pos_x = %d\n",pos_x);   //debug
			switch(pos_x)
			{
				case 0:
					if(year>0)
						year--;
					break;
				case 1:
					if(months>1)
						months--;
					break;
				case 2:
					if((year%4==0&&year%100!=0)||(year%400==0)) //闰年
					{
						
						Day[1] = 29;
					}
					else 
					{	
						Day[1] = 28;
					}
					if(day>1)
						day--;
					break;
				case 3:
					if(hour>0)
					hour--;
					break;
				case 4:
					if(minute>0)
					minute--;
					break;
				case 5:
					if(second>0)
					second--;
					break;
				default:
					break;
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:     //查询键 保存
			//printf("pos_x = %d\n",pos_x);   //debug
			if (datetime_set_f(year, months, day,  hour,  minute, second)==0)
				strcpy(message, "保存成功");
			else 
				strcpy(message, "保存失败");
			
			if (save_timer_id)
				KillTimer(hWnd, save_timer_id);
						
			InvalidateRect(hWnd, NULL, FALSE);
			
			save_timer_id = SetTimer(hWnd, SAVE_TIMER_ID, 5000, NULL);
			break;
		case VK_F8:       //save
			pos_x = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			//printf("blacknum = %d\n", black_count_get(1));   //debug			
			break;
		case VK_F4:       //return
			pos_x = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		default:
			pos_x = 0;
			break;
	}
	return 0;
}


static void refresh_keytable(HWND hWnd, int oldx, int oldy, int posx, int posy,int Oldx,int Posx)
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	HFONT			hFont, hOldFont;
	RECT		rect;
	HBRUSH			hBrush;

	char oldbuf[10];
	char posbuf[10];
	int temptimepos[6][2] = {
							{37,10},  {32,10} , {32,10},
							{32,10}, {32,10}, {32,10}	};
	if ((posx == oldx) && (posy == oldy))
		return;
	switch (Oldx)
	{
		case 0:
			sprintf(oldbuf, "%d", year);
			break ;
		case 1:
			sprintf(oldbuf, "%d", months);
			break;
		case 2:
			sprintf(oldbuf, "%d", day);
			break;
		case 3:
			sprintf(oldbuf, "%d", hour);
			break;
		case 4:
			sprintf(oldbuf, "%d", minute);
			break;
		case 5:
			sprintf(oldbuf, "%d", second);
			break;	
		default:
			break;
		
	}
	switch (Posx)
	{
		case 0:
			sprintf(posbuf, "%d", year);
			break ;
		case 1:
			sprintf(posbuf, "%d", months);
			break;
		case 2:
			sprintf(posbuf, "%d", day);
			break;
		case 3:
			sprintf(posbuf, "%d", hour);
			break;
		case 4:
			sprintf(posbuf, "%d", minute);
			break;
		case 5:
			sprintf(posbuf, "%d", second);
			break;	
		default:
			break;
		
	}
	hDC = GetDC(hWnd);

		//目的位置，拷贝从红色图上，源位置，拷贝从底图上
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, pos[Posx][2], pos[Posx][3]);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, pos[Posx][2], pos[Posx][3], hBgMemDC, pos[Posx][0], pos[Posx][1], SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());	
					hBrush = CreateSolidBrush(RGB(255,0,0));
						rect.left = 2;
						rect.right = pos[Posx][2]-2;
						rect.top = 2;
						rect.bottom = pos[Posx][3]-2;
						FillRect(hMemDC, &rect, hBrush);
					DeleteObject(hBrush);				
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);						
					SetTextColor(hMemDC, RGB(255, 255, 255));
				//	BitBlt(hMemDC, pos[Posx][0], pos[Posx][1], pos[Posx][2], pos[Posx][3],hfocusMemDC,pos[Posx][0], pos[Posx][1], SRCCOPY);
					TextOut(hMemDC,temptimepos[Posx][0]-strlen(posbuf)*6, temptimepos[Posx][1],posbuf, -1);
					
					//printf("%s\n",posbuf);   //debug
					
					BitBlt(hDC, pos[Posx][0], pos[Posx][1], pos[Posx][2], pos[Posx][3],hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont);	
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
				
			
			hBitmap = CreateCompatibleBitmap(hMemDC,pos[Oldx][2], pos[Oldx][3]);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, pos[Oldx][2], pos[Oldx][3], hBgMemDC,  pos[Oldx][0], pos[Oldx][1], SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());				
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);						
					SetTextColor(hMemDC, RGB(255, 255, 255));				
					BitBlt(hMemDC,  pos[Oldx][0], pos[Oldx][1], pos[Oldx][2], pos[Oldx][3],hBgMemDC,  pos[Oldx][0], pos[Oldx][1], SRCCOPY);						
					TextOut(hMemDC, temptimepos[Oldx][0]-strlen(oldbuf)*6, temptimepos[Oldx][1],oldbuf, -1);  
					
				BitBlt(hDC,  pos[Oldx][0], pos[Oldx][1], pos[Oldx][2], pos[Oldx][3],hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont);	
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
			
		DeleteDC(hMemDC);
	
	ReleaseDC(hWnd, hDC);
}


