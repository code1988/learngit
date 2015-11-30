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

#define 		ID_TIMER_CMD0 	400	
#define 		ID_TIMER_SEND 	401	

static u32_t 	timer_cmd_s 	= 0;	// ֽ����Ϣ��ʱ��
static u32_t 	timer_send_s 	= 0;	// ֽ����Ϣ��ʱ��

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;


static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static status_t Disp_Net(HWND hWnd);
static void warning_window (HWND hWnd,s8_t *buf);



int adjustmenu_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"ADJUSTMENUWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"ADJUSTMENUWIN",
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
		case WM_COMMAND:
			
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
	hBgMemDC = CreateCompatibleDC(hDC);													//��������HDC
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);						    //��������λͼ
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);

	rect.left = 0;rect.right = WINDOW_WIDTH;rect.top = 0;rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);                                           //��ͼ֮ǰ����λͼ���
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/adjustmenuwin.bmp", 0);     //��ʾ	��������ɫ����ͼ


	
	//������־
	hwarningMemDC = CreateCompatibleDC(hDC);
	hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 400, 45);
	hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
	rect.left = 0; rect.right = 400; rect.top = 0; rect.bottom = 45;
	FillRect(hwarningMemDC, &rect, hBrush);
	GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 400, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);

	
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
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
	if(timer_cmd_s)
		KillTimer(hWnd, ID_TIMER_CMD0);
	timer_cmd_s = 0;
	if(timer_send_s)
		KillTimer(hWnd, ID_TIMER_SEND);
	timer_send_s = 0;
	
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC;
	PAINTSTRUCT		ps;


	hDC = BeginPaint(hWnd, &ps);
				
	BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
		
	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	status_t ret;

	switch(wParam) 
	{
		case ID_TIMER_CMD0:
			if(timer_cmd_s)
			{
				s32_t mode;
				if(timer_cmd_s)
				KillTimer(hWnd, ID_TIMER_CMD0);
				timer_cmd_s = 0;
				mode = mcu_mode_get();
				printf("MCU debug mode is: %d\n",mode);
				if(mode == MCU_DYNAMIC_DEBUG_MODE)
				{
					dynamicwin_create(hWnd);
				}
				else if(mode == MCU_STATIC_DEBUG_MODE)
				{
					staticwin_create(hWnd);
				}
			}
			break;
		case ID_TIMER_SEND:
			if(timer_send_s)
			{
				ret = Disp_Net(hWnd);
				if(ret != OK_T)
					printf("sending error!\n");
				
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
			if(!timer_cmd_s)
			{
				timer_cmd_s = SetTimer(hWnd,ID_TIMER_CMD0,1000,NULL);
				mcu_mode_set(MCU_DYNAMIC_DEBUG_MODE);
			}
			break;
		case VK_F2:  
			if(!timer_cmd_s)
			{
				timer_cmd_s = SetTimer(hWnd,ID_TIMER_CMD0,1000,NULL);
				mcu_mode_set(MCU_STATIC_DEBUG_MODE);
			}
			break;
		case VK_F3: 
			//��ʼ����ͼ
			image_transport_open();
			if(!timer_send_s)
			{
				timer_send_s = SetTimer(hWnd,ID_TIMER_SEND,1000,NULL);
			}
			break;
		case VK_F4:     
			//����ͼ���
			image_transport_clear();
			if(timer_send_s)
			{
				KillTimer(hWnd, ID_TIMER_SEND);
				timer_send_s = 0;
			}
			InvalidateRect(hWnd,NULL,FALSE);
			break;
		case VK_F5:    
			break;
		case VK_F6:    
			break;
		case VK_F7:  
			CISadjust_create(hWnd);
			break;
		case VK_F8:       //return
			if(timer_cmd_s)
			{
				KillTimer(hWnd, ID_TIMER_CMD0);
				timer_cmd_s = 0;
			}
			if(timer_send_s)
			{
				KillTimer(hWnd, ID_TIMER_SEND);
				timer_send_s = 0;
			}
			DestroyWindow(hWnd);
			break;
		case VK_F9:       //return	
			if(timer_cmd_s)
			{
				KillTimer(hWnd, ID_TIMER_CMD0);
				timer_cmd_s = 0;
			}
			if(timer_send_s)
			{
				KillTimer(hWnd, ID_TIMER_SEND);
				timer_send_s = 0;
			}
			
			DestroyWindow(GetMenuWinHwnd());
			break;
		default:
			break;
	}
	return 0;
}

/***********************************************************************************************
* Function Name	: Disp_Net
* Description		: ������ͼ
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/
static status_t Disp_Net(HWND hwnd)
{
	u8_t 	sendingnum = 0;//���ڷ��͵�����
	u8_t 	totalnum = 0;  //������
	s32_t   ret;
	char 	buf[64];

	ret =  image_transport_status(&totalnum,&sendingnum);
	printf("ret  =%d\n",ret);
	if(ret == 1)  //���ڷ��ʹ�ͼ��
	{
		sprintf(buf, "���ڷ��� %d / %d �Ŵ�ͼ", sendingnum,totalnum);
		
		warning_window(hwnd,buf);
		printf("sendingnum=%d,totalnum=%d\n",sendingnum,totalnum);
		
	}
	else if(ret == 10)  //��������
	{
		if(timer_send_s)
		{
			KillTimer(hwnd, ID_TIMER_SEND);
			timer_send_s = 0;
		}
	    InvalidateRect(hwnd,NULL,FALSE);
		
	}
	else if(ret == -1)  //����ʧ��
	{
		memcpy(buf,"����ʧ�ܣ�������ط�",strlen("����ʧ�ܣ�������ط�"));
		warning_window(hwnd,buf);
	}

	return OK_T;
}
/***********************************************************************************************
* Function Name	:  
* Description		:  ��ʾ����
* Input			:
* Return			:
* Note(s)			:
* Contributor		: 151129	CODE
***********************************************************************************************/


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


