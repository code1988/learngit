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

#define ID_TIMER_COUNT_PAPER_INFO 	503	
static u32_t 		timer_count_paper_info_s 	= 0;	// 纸币信息定时器


static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static HDC		hfocusMemDC = NULL;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;

static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;

static HDC		hModMemDC = NULL;
static HBITMAP	hModBitmap,hOldModBitmap;

static HDC 		hKyNumMemDC = NULL;
static HBITMAP	hKyNumBitmap,hOldKyNumBitmap;


static MONEYDISP_S monenyInfo;	// 纸币信息
static u8_t	addSWC = 1;		// 累加开关
//static u8_t addchangeflag = 0;
static s16_t preset_value = 0;		// 批量值





static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static u8_t Error_Warning(HWND hWnd);
static status_t Disp_Warning(HWND hWnd,s8_t *buf,s8_t flag);


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
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);						    //创建兼容位图
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
	rect.left = 0;rect.right = WINDOW_WIDTH;rect.top = 0;rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);                                           //绘图之前进行位图清除
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/countmodewin.bmp", 0);    

	hfocusMemDC = CreateCompatibleDC(hDC);
	hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, WINDOW_WIDTH, 100);
	hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = 100;
	FillRect(hfocusMemDC, &rect, hBrush);
	GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, WINDOW_WIDTH, 100, "/bmp/fota/zongzhangshu.bmp", 0);     

	hwarningMemDC = CreateCompatibleDC(hDC);
	hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 400, 45);
	hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
	rect.left = 0; rect.right = 400; rect.top = 0; rect.bottom = 45;
	FillRect(hwarningMemDC, &rect, hBrush);
	GdDrawImageFromFile(hwarningMemDC->psd, 0, 0,400, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);


		// 模式、累加、面额图
	hModMemDC = CreateCompatibleDC(hDC);
	hModBitmap = CreateCompatibleBitmap(hModMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldModBitmap = SelectObject(hModMemDC, hModBitmap); 
	rect.left = 0; rect.right = WINDOW_WIDTH; rect.top = 0; rect.bottom = WINDOW_HEIGHT;
	FillRect(hModMemDC, &rect, hBrush);
	GdDrawImageFromFile(hModMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/mainmenu1win.bmp", 0);

	// 可疑币黑底图
	hKyNumMemDC = CreateCompatibleDC(hDC);
	hKyNumBitmap = CreateCompatibleBitmap(hKyNumMemDC, WINDOW_HEIGHT, 44);
	hOldKyNumBitmap = SelectObject(hKyNumMemDC, hKyNumBitmap); 
	rect.left = 0; rect.right = WINDOW_HEIGHT; rect.top = 0; rect.bottom = 44;
	FillRect(hKyNumMemDC, &rect, hBrush);
	GdDrawImageFromFile(hKyNumMemDC->psd, 0, 0, WINDOW_HEIGHT, 44, "/bmp/fota/keyibiwin.bmp", 0);


	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	preset_value =	preset_value_get();
	timer_count_paper_info_s = SetTimer(hWnd,ID_TIMER_COUNT_PAPER_INFO,10,NULL);
	preset_value =	preset_value_get();
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

	SelectObject(hModMemDC, hOldModBitmap);
	DeleteObject(hModBitmap);
	DeleteDC(hModMemDC);

	SelectObject(hKyNumMemDC, hOldKyNumBitmap);
	DeleteObject(hKyNumBitmap);
	DeleteDC(hKyNumMemDC);
	if (timer_count_paper_info_s > 0)
	{
		KillTimer(hWnd, ID_TIMER_COUNT_PAPER_INFO);
		timer_count_paper_info_s = 0;
	}
	moneydisp_info_get(&monenyInfo);

	hMainWnd = NULL;

	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	char 			buf[10];
	int 			len;
	u8_t 			i;
	hDC = BeginPaint(hWnd, &ps);
	
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);

	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.curpages);
	len = strlen(buf);
	if(len>5)
		len = 5;
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,440 - (len - i)*48,125 , 48, 100, hfocusMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
	}

	if((preset_value==0) && (addSWC==1)) 
	{

		BitBlt(hMemDC,110 ,275 , 75, 43, hModMemDC, 158, 253, SRCCOPY);
		
	}
	BitBlt(hMemDC,190 ,275 , 75, 43, hModMemDC, 281, 150, SRCCOPY);
	
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",monenyInfo.prepages);
	len = strlen(buf);
	if(len>5)
		len = 5;
	for(i=0;i<len;i++)
	{
		BitBlt(hMemDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY); 			
	}
	
	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	
	EndPaint(hWnd, &ps);
	
	return 0;
}
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	static u32_t tot_count,pre_pages;
	char buf[64];
	s32_t len;
	u8_t i,error_id;
	
	HDC				hDC;
	
	switch(wParam) 
	{
		case ID_TIMER_COUNT_PAPER_INFO:	
			if(moneydisp_info_status())
			{
				moneydisp_info_get(&monenyInfo);
		
				hDC = GetDC(hWnd);		
			
				// 总张数
				if((tot_count != monenyInfo.curpages)&&(error_id==0))
				{	
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",monenyInfo.curpages);
				
					len = strlen(buf);	
					if(len>5)
						len = 5;
					if(tot_count>monenyInfo.curpages)
					{
						for(;i<5;i++)
						{
							BitBlt(hDC,440 - (i+1)*48,125 , 48, 100, hBgMemDC, 440 - (5 - i)*48,125, SRCCOPY);
						}

					}
					for(i=0;i<len;i++)
					{
						BitBlt(hDC,440 - (len - i)*48,125, 48, 100, hfocusMemDC, (buf[i] - '0')*48, 0, SRCCOPY);			
					}					
					tot_count = monenyInfo.curpages;
				}	
				ReleaseDC(hWnd, hDC);
			}	
			//显示报警信息
			hDC = GetDC(hWnd);
			error_id = Error_Warning(hWnd);
			ReleaseDC(hWnd, hDC);

			//显示累加和上次信息
			preset_value =	preset_value_get();	
			if(preset_value==0)
				addSWC = 1;
			else 
				addSWC = 0;
			hDC = GetDC(hWnd);
			   if((preset_value==0)&&(addSWC==1)) //累加 ，只显示面额值
			   {									 
		   			BitBlt(hDC,110 ,275 , 75, 43, hModMemDC, 158, 253, SRCCOPY);
			   }
			   else  if((preset_value==-1)&&(addSWC==0))//fei累加
			   {									 
		   			BitBlt(hDC,110 ,275 , 75, 43, hBgMemDC, 110 ,275, SRCCOPY);
			   }
				
			   if(pre_pages!=monenyInfo.prepages)
			   {
					
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d",monenyInfo.prepages);
					len = strlen(buf);
					if(len>5)
						len = 5;
					for(i=0;i<len;i++)
					{
						BitBlt(hDC,280 + i*32,272 , 32, 44, hKyNumMemDC, (buf[i] - '0')*32, 0, SRCCOPY); 		
					}
					for(;i<5;i++)
					{
						BitBlt(hDC,280 + i*32,272 , 32, 44, hBgMemDC, 280 + i*32,267, SRCCOPY);
					}								
					pre_pages = monenyInfo.prepages;
				   
			   }

			ReleaseDC(hWnd, hDC);	
								
			break;
		default:
			break;
	}
	return 0;
}

/***********************************************************************************************
* Function Name	: Disp_Warning
* Description		: 告警相关
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static u8_t Error_Warning(HWND hWnd)
{
	static u8_t olderror_id = 0;
	
	u8_t error_id;
	s8_t	*errptr;
	s8_t 	buf[64];
	

	// 获取设备告警信息
	error_id = deverror_id_get();
	
	if(error_id)
	{
		errptr = deverror_str_get(error_id);
		olderror_id = error_id;
	}
	else
	{
		// 没有设备告警的情况下，获取纸币告警信息
		error_id = moneyerror_id_get();								
		if(error_id)//  //报警信息
		{
			errptr = moneyerror_str_get(error_id);
			olderror_id = error_id;				
		}	
		else if(error_id==0 && olderror_id>0)  //后改成马达转动
		{
			// 告警解除之后
			olderror_id = error_id;
			Disp_Warning(hWnd, buf,0);
		}
		
			
	}
	// 显示告警信息
	if(error_id) 
	{

		if (errptr == NULL) 
		{
			sprintf(buf, "错误代码 0x%02x", olderror_id);

		}
		else
			memcpy(buf,errptr,strlen(errptr));
		
		Disp_Warning(hWnd, buf,1);
	}
	
	return error_id;
}
static status_t Disp_Warning(HWND hWnd,s8_t *buf,s8_t flag)
{
	HDC				hDC,hMemDC;
	HBITMAP			hBitmap, hOldBitmap;
	HFONT			hOldFont;
	u8_t i,len;
	// 显示告警信息
	hDC = GetDC(hWnd);
	if(flag)
	{
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, 400, 45);
				hOldBitmap = SelectObject(hMemDC, hBitmap);
				
					BitBlt(hMemDC, 0, 0, 400, 45, hwarningMemDC, 0, 0, SRCCOPY);
					hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
						SetBkColor(hMemDC, RGB(0, 255, 0));
						SetBkMode(hMemDC, TRANSPARENT);;							
						SetTextColor(hMemDC,RGB(255,0 , 0));
						TextOut(hMemDC, 300-strlen(buf)*12, 10, buf, -1);
							
						BitBlt(hDC, 40, 180, 400,45,hMemDC, 0, 0, SRCCOPY);
					
					SelectObject(hMemDC, hOldFont);
				SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
		DeleteDC(hMemDC);
		
	}
	else 
	{
		BitBlt(hDC, 40, 180, 400,45,hBgMemDC,40, 180, SRCCOPY);
		memset(buf,0,sizeof(buf));
		
		sprintf(buf,"%d",monenyInfo.curpages);
		len = strlen(buf);
  		for(i=0;i<len;i++)
  		{
  			BitBlt(hDC,440 - (len - i)*48,125, 48, 100, hfocusMemDC, (buf[i] - '0')*48, 0, SRCCOPY);				
  		}					
	}
		
	ReleaseDC(hWnd, hDC);

	return OK_T;
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
			fota_clearzero();
			SendMessage(GetParent(hWnd),WM_COMMAND, (WPARAM)0xF0F0, (LPARAM)0);
			DestroyWindow(hWnd);
			break;
		case VK_F6: 
			if((preset_value==0))
			{
				preset_value_set(-1);
				 
				addSWC= 0;

			}
				
			else if((preset_value==-1))
			{
				preset_value_set(0);
				 
				addSWC=1;

			}
			break;
		case VK_F7:  
			guanzihaowin_create(hWnd);
			break;
		case VK_F8:
			 
			fota_clearzero();
			 
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F9:  //启动复位键
			fota_startstop();
			break;
		default:
			break;
	}
	return 0;

}


