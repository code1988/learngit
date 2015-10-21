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

#define Uploadimgweight 52
#define Uploadimgheight 38

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;



static int	old_x = 0;
static int	pos_x = 0;


static int save_timer_id = 0;
static int message[32];
static char uploadimg[21];
//待填数字坐标
static int pos[2] = { 71,98};
					
//单元格坐标					
static int rectpos[2] = {44,98 };							
						   

static  char	uploadimgbuf[21][10];

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void refresh_keytable(HWND hWnd,int Oldx,int Posx ,int flag);


int uploadimg_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"UPLOADIMGWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"UPLOADIMGWIN",
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
		hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);						    //创建兼容位图
		hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
			rect.left = 0;
			rect.right = 480;
			rect.top = 0;
			rect.bottom = 320;
			FillRect(hBgMemDC, &rect, hBrush);                                           //绘图之前进行位图清除
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/uploadimgwin.bmp", 0);     //灵敏度调节1图
			
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	message[0] = '\0';
	
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
	HBRUSH			hBrush;
	RECT			rect;
	HFONT			 hOldFont;
	char i = 0;
	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
				//  当前选中单元格显示红色
					hBrush = CreateSolidBrush(RGB(255,0,0));
						rect.left = rectpos[0];
						rect.right = rectpos[0]+Uploadimgweight ;
						rect.top = rectpos[1];
						rect.bottom = rectpos[1]+Uploadimgheight*(pos_x+1);
						FillRect(hMemDC, &rect, hBrush);
					DeleteObject(hBrush);
					
					for(i = 0;i<21;i++)
					{
						sprintf(uploadimgbuf[i], "%d", uploadimg[i]);
					}
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);
					SetTextColor(hMemDC, RGB(255, 0, 0));			
					TextOut(hMemDC, 165, 255, message, -1);					
					SetTextColor(hMemDC,RGB(255, 255, 255));
					for(i = 0;i<21;i++)
					{						
						TextOut(hMemDC, pos[0]-strlen(uploadimgbuf[i])*6+i%7*(Uploadimgweight+i%7-1), pos[1]+i/7*2*(Uploadimgheight+i/7), uploadimgbuf[i], -1);  
					}
					BitBlt(hDC, 0, 0, 480, 320,hMemDC, 0, 0, SRCCOPY);
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
	int min=0;
	int  max[21]={24,59,24,59,24,59,24,59,24,59,24,59,100,10,24,59,1,255,255,255,255};
	pos_x = old_x;
	switch(wParam) {
		case VK_F1:     //����
			if(pos_x<20)
				pos_x++;	
			refresh_keytable(hWnd,old_x,pos_x,1);
			old_x = pos_x;
			break;
		case VK_F2:    //����
			if(pos_x>0)
				pos_x--;	
			refresh_keytable(hWnd,old_x,pos_x,1);
			old_x = pos_x;
			break;
		case VK_F3:       //down
			
			break;
		case VK_F4:       //return
			
			break;	
		case VK_F5:     //��1
			if(uploadimg[pos_x]<max[pos_x])
				uploadimg[pos_x]++;
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F6:    //累加
			if(uploadimg[pos_x]>min)
				uploadimg[pos_x]--;
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F7: 
			//if (datetime_set_f(year, months, day,  hour,  minute, second)==0)
			//	strcpy(message, "����ɹ�");
			//else 
			//	strcpy(message, "����ʧ��");
			
			if (save_timer_id)
				KillTimer(hWnd, save_timer_id);
						
			refresh_keytable(hWnd,old_x,pos_x,0);
			
			save_timer_id = SetTimer(hWnd, SAVE_TIMER_ID, 5000, NULL);
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			break;
		case VK_F8:     //���沢����
			
			break;
		case VK_F9:       //save
			pos_x = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd));
			//printf("blacknum = %d\n", black_count_get(1));   //debug			
			break;
		
		default:
			pos_x = 0;
			break;
	}
	return 0;
}


static void refresh_keytable(HWND hWnd,int Oldx,int Posx ,int flag)
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	HFONT			hOldFont;
	RECT			rect;
	HBRUSH			hBrush;

	
	sprintf(uploadimgbuf[Oldx], "%d", uploadimg[Oldx]);
	sprintf(uploadimgbuf[Posx], "%d", uploadimg[Posx]);
	
	hDC = GetDC(hWnd);

		//目的位置，拷贝从红色图上，源位置，拷贝从底图上
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, Uploadimgweight, Uploadimgheight);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, Uploadimgweight, Uploadimgheight, hBgMemDC, rectpos[0]+Posx%7*(Uploadimgweight+Posx%7-1), rectpos[1]+Posx/7*2*(Uploadimgheight+Posx/7), SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());	
					hBrush = CreateSolidBrush(RGB(255,0,0));
						rect.left = 0;
						rect.right = Uploadimgweight;
						rect.top = 0;
						rect.bottom = Uploadimgheight;
						FillRect(hMemDC, &rect, hBrush);
					DeleteObject(hBrush);				
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);						
					SetTextColor(hMemDC, RGB(255, 255, 255));
					TextOut(hMemDC,Uploadimgweight/2+Posx-strlen(uploadimgbuf[Posx])*6, 0,uploadimgbuf[Posx], -1);
					
					BitBlt(hDC,rectpos[0]+Posx%7*(Uploadimgweight+Posx%7-1), rectpos[1]+Posx/7*2*(Uploadimgheight+Posx/7), Uploadimgweight, Uploadimgheight,hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont);	
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
	if(flag == 1)
	{
			hBitmap = CreateCompatibleBitmap(hMemDC, Uploadimgweight, Uploadimgheight);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, Uploadimgweight, Uploadimgheight, hBgMemDC, rectpos[0]+Oldx%7*(Uploadimgweight+Oldx%7-1), rectpos[1]+Oldx/7*2*(Uploadimgheight+Oldx/7), SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());				
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT); 					
					SetTextColor(hMemDC, RGB(255, 255, 255));									
					TextOut(hMemDC,Uploadimgweight/2-strlen(uploadimgbuf[Oldx])*6+Oldx, 0,uploadimgbuf[Oldx], -1);					
					BitBlt(hDC,rectpos[0]+Oldx%7*(Uploadimgweight+Oldx%7-1), rectpos[1]+Oldx/7*2*(Uploadimgheight+Oldx/7), Uploadimgweight, Uploadimgheight,hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont); 
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);

	}
		
			
			
		DeleteDC(hMemDC);
	
	ReleaseDC(hWnd, hDC);
}


