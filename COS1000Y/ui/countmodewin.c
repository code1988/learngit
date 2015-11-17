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

#define ID_TIMER_COUNT_PAPER_INFO 	503	
static u32_t 		timer_count_paper_info_s 	= 0;	// 纸币信息定时器


static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static HDC		hfocusMemDC = NULL;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;

static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;


static MONEYDISP_S monenyInfo;	// 纸币信息
static u8_t olderror_id = 0;





static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

int countmodewin_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"COUNTMODE";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"COUNTMODE",
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
	
	hDC = GetDC(hWnd);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	
	hBgMemDC = CreateCompatibleDC(hDC);													//创建兼容HDC
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);						    //创建兼容位图
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
	rect.left = 0;rect.right = 480;rect.top = 0;rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);                                           //绘图之前进行位图清除
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/countmodewin.bmp", 0);    

	hfocusMemDC = CreateCompatibleDC(hDC);
	hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 480, 100);
	hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
	FillRect(hfocusMemDC, &rect, hBrush);
	GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 480, 100, "/bmp/fota/zongzhangshu.bmp", 0);     

	hwarningMemDC = CreateCompatibleDC(hDC);
		hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 220, 45);
		hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
		FillRect(hwarningMemDC, &rect, hBrush);
		GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 220, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);

	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	timer_count_paper_info_s = SetTimer(hWnd,ID_TIMER_COUNT_PAPER_INFO,10,NULL);
	
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
	SelectObject(hwarningMemDC, hOldwarningBitmap);
	DeleteObject(hwarningBitmap);
	DeleteDC(hwarningMemDC);
			
	if (timer_count_paper_info_s > 0)
	{
		KillTimer(hWnd, ID_TIMER_COUNT_PAPER_INFO);
		timer_count_paper_info_s = 0;
	}

	hMainWnd = NULL;

	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	u8_t 			buf[10];
	int 			len;
	u8_t 			i;
	hDC = BeginPaint(hWnd, &ps);
	
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	
	BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);

	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.curpages);
	len = strlen(buf);
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,440 - (len - i)*48,125 , 48, 100, hfocusMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
	}
	
	BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	
	EndPaint(hWnd, &ps);
	
	return 0;
}
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	static u32_t tot_count;
	u8_t buf[64];
	s32_t len;
	u8_t i,error_id;
	
	HDC				hDC,hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	HFONT			 hOldFont;
	s8_t	*errptr;

	switch(wParam) 
	{
		case ID_TIMER_COUNT_PAPER_INFO:
			if(moneydisp_info_status())
			{
				moneydisp_info_get(&monenyInfo);
		
				hDC = GetDC(hWnd);
		
			
				// 总张数
				if(tot_count != monenyInfo.curpages)
				{	
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",monenyInfo.curpages);
				
					len = strlen(buf);	
					if(tot_count>monenyInfo.curpages)
					{
						for(;i<5;i++)
						{
							BitBlt(hDC,440 - (i+1)*48,125 , 48, 100, hBgMemDC, 440 - (5 - i)*48,125, SRCCOPY);
						}

					}
					
					error_id = moneyerror_id_get(); 			
							
					if(error_id)  //报警信息
					{
						errptr = moneyerror_str_get(error_id);
			
						olderror_id = error_id;
			
						printf("%s\n",errptr);					
										
						hMemDC = CreateCompatibleDC(hDC);
						hBitmap = CreateCompatibleBitmap(hMemDC, 220, 45);
						hOldBitmap = SelectObject(hMemDC, hBitmap);
							BitBlt(hMemDC, 0, 0, 220, 45, hwarningMemDC, 0, 0, SRCCOPY);
							hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
								SetBkColor(hMemDC, RGB(0, 255, 0));
								SetBkMode(hMemDC, TRANSPARENT);;							
								SetTextColor(hMemDC,RGB(255,0 , 0));
								
								if (errptr == NULL) 
								{
									sprintf(buf, "错误代码 0x%02x", olderror_id);
									TextOut(hMemDC, 47, 10, buf, -1);
								}
								else
									TextOut(hMemDC, 47, 10, errptr, -1);
			
								
								BitBlt(hDC, 170, 110, 220,45,hMemDC, 0, 0, SRCCOPY);
							SelectObject(hMemDC, hOldFont);
					
						SelectObject(hMemDC, hOldBitmap);
						DeleteObject(hBitmap);
						DeleteDC(hMemDC);
			
					}	
					else if(error_id == 0 && error_id!=olderror_id)
					{
						olderror_id = error_id;
						BitBlt(hDC, 170, 110, 220,45,hBgMemDC, 170, 110, SRCCOPY);
					}
					for(i=0;i<len;i++)
					{
						BitBlt(hDC,440 - (len - i)*48,125, 48, 100, hfocusMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
					}					
					tot_count = monenyInfo.curpages;
				}	
				printf("papers2\n");
				ReleaseDC(hWnd, hDC);
			}			
			break;
		default:
			break;
	}
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
		
		case VK_F1:    
			break;
		case VK_F2:    
			break;
		case VK_F3:       
			break;
		case VK_F4:     
			break;
		case VK_F5:  
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)0xF0F0, (LPARAM)0);
			DestroyWindow(hWnd);
			break;
		case VK_F6:     
			break;
		case VK_F7:  
			guanzihaowin_create(hWnd);
			break;
		case VK_F8:
			memset(&monenyInfo,0,sizeof(MONEYDISP_S));
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		default:
			break;
	}
	return 0;

}


