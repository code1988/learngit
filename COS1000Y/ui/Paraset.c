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
#include "fotawin.h"
//#include "device.h"



static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;

static char message[32];

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
//static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void warning_window (HWND hWnd);

int parasetwin_create(HWND hWnd)
{
	WNDCLASS	wndclass;


	//MWInputWinRegister();

	wndclass.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc    = (WNDPROC)MainProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = 0;
	wndclass.hIcon          = 0;
	wndclass.hCursor        = 0;
	wndclass.hbrBackground  = NULL;
	wndclass.lpszMenuName   = NULL;
		wndclass.lpszClassName  = (LPCSTR)"parasetwin";
		RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"parasetwin",
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
		case WM_CHAR:
			{
				char buf[2];
				buf[0] = wParam;
				buf[1] = 0;
				printf("%s\n", buf);

			}
			break;
		case WM_COMMAND:
			if (wParam == 0xF0F0) 
			{
				identifywin_create(hWnd);
			}
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
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/parasetwin.bmp", 0);

	hwarningMemDC = CreateCompatibleDC(hDC);
	hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 220, 45);
	hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
	rect.left = 0; rect.right = 220; rect.top = 0; rect.bottom = 45;
	FillRect(hwarningMemDC, &rect, hBrush);
	GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 220, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);
	
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
	SelectObject(hwarningMemDC, hOldwarningBitmap);
	DeleteObject(hwarningBitmap);
	DeleteDC(hwarningMemDC);
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
		
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	HFONT		    hOldFont;

	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);
					SetTextColor(hMemDC, RGB(255, 0, 0));			
					TextOut(hMemDC, 165, 255, message, -1);
					
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
	char Uploadb[21][4]={ "H11","M11","H12","M12","H21","M21","H22",\
								"M22","H31","M31","H32","M32","H41","M41",\
								"H42","M42","SW1","FMG","DKA","SSM","SDK"\
	
								};

	u8_t i;
	

	switch(wParam) 
	{
		case VK_F1:     
			password_window_create(hWnd);
			break;
		case VK_F2:    
			blackname_window_create(hWnd);
			break;
		case VK_F3:       
			netnumber_window_create(hWnd);
			break;
		case VK_F4:     
			timeset_window_create( hWnd);
			break;
		case VK_F5:    
			myIP_window_create( hWnd);
			break;
		case VK_F6:   //恢复出厂设置
			
			fota_reset_factory();
			upload_param_save("网发时间", 120);
			upload_param_save("网发张数", 200);
			upload_param_save("批量模式", 0);
			upload_param_save("备用1111", 0);
			upload_param_save("备用2222", 1);
			upload_param_save("备用3333", 1);
			for(i = 0;i<12;i++)
			{
		
				upload_param_save(Uploadb[i], 23+i%2*36);
		
			}
			 upload_param_save(Uploadb[12], 60);
			 upload_param_save(Uploadb[13], 7);
			 upload_param_save(Uploadb[14], 23);
			 upload_param_save(Uploadb[15], 59);
			 upload_param_save(Uploadb[16], 0);
			 upload_param_save(Uploadb[17], 30);
			 upload_param_save(Uploadb[18], 255);
			 upload_param_save(Uploadb[19], 255);
			 upload_param_save(Uploadb[20], 100);

			warning_window(hWnd); 
			break;
		case VK_F7:       
			agetest_window_create( hWnd);
			break;
		case VK_F8:
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F9:
			spi_keyalert();
			DestroyWindow(GetMenuWinHwnd());
			break;
		default:
			break;
	}
	return 0;
}

HWND parasetwin_handle_get(void)
{
	return hMainWnd;
}
static void warning_window (HWND hWnd)
{
	HDC 			hDC,hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	HFONT			 hOldFont;
	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 220, 45);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
		BitBlt(hMemDC, 0, 0, 220, 45, hwarningMemDC, 0, 0, SRCCOPY);
		hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
			SetBkColor(hMemDC, RGB(0, 255, 0));
			SetBkMode(hMemDC, TRANSPARENT);;							
			SetTextColor(hMemDC,RGB(255,0 , 0));
			
			TextOut(hMemDC, 48, 10, "请关机重启", -1);

			
			BitBlt(hDC, 180, 110, 220,45,hMemDC, 0, 0, SRCCOPY);
		SelectObject(hMemDC, hOldFont);

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);


}

