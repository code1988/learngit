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



#define LONG_KEY_TIMER_ID  506
#define Uploadimgweight 55
#define Uploadimgheight 42

static char long_key_timer_id = 0;

static int	press_flag = 0;
static int	press_key = 0;

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;

static char Upload[21][4]={ "H11","M11","H12","M12","H21","M21","H22",\
							"M22","H31","M31","H32","M32","H41","M41",\
							"H42","M42","SW1","FMG","DKA","SSM","SDK"\
							};

static int	old_x = 0;
static int	pos_x = 0;


static char message[32];
static int uploadimg[21];
//¥˝ÃÓ ˝◊÷◊¯±Í
static int pos[2] = { 70,100};
					
//—°÷–µ•‘™∏Ò◊¯±Í					
static int rectpos[2] = {43,97 };							
						   

static  char	uploadimgbuf[21][10];

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);

static void refresh_keytable(HWND hWnd,int Oldx,int Posx ,int flag);

static int vk_f5_keydown(HWND hWnd);
static int vk_f5_keyup(HWND hWnd);
static int vk_f6_keydown(HWND hWnd);
static int vk_f6_keyup(HWND hWnd);
static int focus_value_sub(HWND hWnd, unsigned int value);
static int focus_value_add(HWND hWnd, unsigned int value);


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
		case WM_KEYUP:
			OnKeyUp(hWnd, wParam, lParam);
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
	int i= 0;
	hDC = GetDC(hWnd);
		hBrush = CreateSolidBrush(RGB(255, 255, 255));
	hBgMemDC = CreateCompatibleDC(hDC);													//ÂàõÂª∫ÂÖºÂÆπHDC
		hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);						    //ÂàõÂª∫ÂÖºÂÆπ‰ΩçÂõæ
		hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
			rect.left = 0;
			rect.right = WINDOW_WIDTH;
			rect.top = 0;
			rect.bottom = WINDOW_HEIGHT;
			FillRect(hBgMemDC, &rect, hBrush);                                           //ÁªòÂõæ‰πãÂâçËøõË°å‰ΩçÂõæÊ∏ÖÈô§
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/uploadimgwin.bmp", 0);     //ÁÅµÊïèÂ∫¶Ë∞ÉËäÇ1Âõæ
			
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	message[0] = '\0';
	for(i = 0;i<12;i++)
	{

		uploadimg[i] = upload_param_load(Upload[i], 23+i%2*36);

	}
	uploadimg[12] = upload_param_load(Upload[12], 60);
	uploadimg[13] = upload_param_load(Upload[13], 7);
	uploadimg[14] = upload_param_load(Upload[14], 23);
	uploadimg[15] = upload_param_load(Upload[15], 59);
	uploadimg[16] = upload_param_load(Upload[16], 0);
	uploadimg[17] = upload_param_load(Upload[17], 30);
	uploadimg[18] = upload_param_load(Upload[18], 255);
	uploadimg[19] = upload_param_load(Upload[19], 255);
	uploadimg[20] = upload_param_load(Upload[20], 100);
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);
	hMainWnd = NULL;
	if (long_key_timer_id > 0)
		KillTimer(hWnd, LONG_KEY_TIMER_ID); 
	long_key_timer_id = 0;
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
	u8_t i = 0;
	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
				//  ÂΩìÂâçÈÄâ‰∏≠ÂçïÂÖÉÊ†ºÊòæÁ§∫Á∫¢Ëâ≤
					hBrush = CreateSolidBrush(RGB(255,0,0));
						rect.left = rectpos[0];
						rect.right = rectpos[0]+Uploadimgweight ;
						rect.top = rectpos[1];
						rect.bottom = rectpos[1]+Uploadimgheight;
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
						TextOut(hMemDC, pos[0]-strlen(uploadimgbuf[i])*6+i%7*(Uploadimgweight), pos[1]+i/7*2*(Uploadimgheight), uploadimgbuf[i], -1);  
					}
					BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,hMemDC, 0, 0, SRCCOPY);
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
	pos_x = old_x;
	switch(wParam) {
		case VK_F1:     //”““∆
			if(pos_x<20)
			{
				pos_x++;	
				refresh_keytable(hWnd,old_x,pos_x,1);

			}
				
			old_x = pos_x;
			break;
		case VK_F2:    //◊Û“∆
			if(pos_x>0)
			{

				pos_x--;	
				refresh_keytable(hWnd,old_x,pos_x,1);

			}
				
			old_x = pos_x;
			break;
		case VK_F3:       //down
			
			break;
		case VK_F4:       //return
			
			break;	
		case VK_F5:     //º”1
			vk_f5_keydown(hWnd);
			break;
		case VK_F6:    //ºı1
			vk_f6_keydown(hWnd);
			break;
		case VK_F7:  //±£¥Ê≤¢∑µªÿ
			for(i = 0;i<21;i++)
			{
				upload_param_save(Upload[i],uploadimg[i]);
			}
			old_x = 0;
			pos_x = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F8:    
			
			break;
		case VK_F9:       //save
			old_x = 0;
			pos_x = 0;
			spi_keyalert();
			DestroyWindow(GetMenuWinHwnd());
			break;		
		default:
			break;
	}
	return 0;
}

static int OnKeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
		case VK_F5:
			vk_f5_keyup(hWnd);
			break;
		case VK_F6:
			vk_f6_keyup(hWnd);
			break;
	}
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == LONG_KEY_TIMER_ID) {
		if (press_key == 5) {
			focus_value_add(hWnd, 10);
			press_flag = 1;
		}
		else if (press_key == 6) {
			focus_value_sub(hWnd, 10);
			press_flag = 1;
		}
	}
	return 0;
}

static int vk_f5_keydown(HWND hWnd)
{
	printf("a\n");
	printf("number = %d \n",uploadimg[pos_x]);
	long_key_timer_id = SetTimer(hWnd, LONG_KEY_TIMER_ID, 500, NULL);
	press_flag = 0;
	press_key = 5;
	return 0;
}
static int vk_f5_keyup(HWND hWnd)
{
	if (press_key != 5)
		return 0;
	if (press_flag == 0)
		focus_value_add(hWnd, 1);
	if (long_key_timer_id > 0)
	{
		KillTimer(hWnd, LONG_KEY_TIMER_ID); 
		long_key_timer_id = 0;

	}
	printf("b\n");
	printf("number = %d \n",uploadimg[pos_x]);
	press_flag = 0;
	press_key = 0;
	return 0;
}
static int vk_f6_keydown(HWND hWnd)
{
	
	long_key_timer_id = SetTimer(hWnd, LONG_KEY_TIMER_ID, 500, NULL);
	press_flag = 0;
	press_key = 6;
	return 0;
}

static int vk_f6_keyup(HWND hWnd)
{
	if (press_key != 6)
		return 0;
	if (press_flag == 0)
		focus_value_sub(hWnd, 1);
	if (long_key_timer_id > 0)
	{
		KillTimer(hWnd, LONG_KEY_TIMER_ID); 
		long_key_timer_id = 0;

	}
		
	press_flag = 0;
	press_key = 0;
	return 0;
}

static int focus_value_add(HWND hWnd, unsigned int value)
{	

	printf("c\n");
		printf("number = %d \n",uploadimg[pos_x]);

	int  max[21]={24,59,24,59,24,59,24,59,24,59,24,59,100,10,24,59,1,255,255,255,255};
	if (uploadimg[pos_x] <= (max[pos_x]- value))
		uploadimg[pos_x] = uploadimg[pos_x] + value;
	refresh_keytable(hWnd,old_x,pos_x,0);
	printf("d\n");
	printf("number = %d \n",uploadimg[pos_x]);
	old_x = pos_x;
	return 0;
}


static int focus_value_sub(HWND hWnd, unsigned int value)
{
	int min=0;
	if (uploadimg[pos_x]>= min+value)
		uploadimg[pos_x] = uploadimg[pos_x] - value;
	refresh_keytable(hWnd,old_x,pos_x,0);
	old_x = pos_x;
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

		//ÁõÆÁöÑ‰ΩçÁΩÆÔºåÊã∑Ë¥ù‰ªéÁ∫¢Ëâ≤Âõæ‰∏äÔºåÊ∫ê‰ΩçÁΩÆÔºåÊã∑Ë¥ù‰ªéÂ∫ïÂõæ‰∏ä
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, Uploadimgweight, Uploadimgheight);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, Uploadimgweight, Uploadimgheight, hBgMemDC, rectpos[0]+Posx%7*(Uploadimgweight+1), rectpos[1]+Posx/7*2*(Uploadimgheight), SRCCOPY);
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
					TextOut(hMemDC,Uploadimgweight/2-strlen(uploadimgbuf[Posx])*6, 0,uploadimgbuf[Posx], -1);
					
					BitBlt(hDC,rectpos[0]+Posx%7*(Uploadimgweight+1), rectpos[1]+Posx/7*2*(Uploadimgheight), Uploadimgweight, Uploadimgheight,hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont);	
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
	if(flag == 1)
	{
			hBitmap = CreateCompatibleBitmap(hMemDC, Uploadimgweight, Uploadimgheight);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, Uploadimgweight, Uploadimgheight, hBgMemDC, rectpos[0]+Oldx%7*(Uploadimgweight+1), rectpos[1]+Oldx/7*2*(Uploadimgheight), SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());				
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT); 					
					SetTextColor(hMemDC, RGB(255, 255, 255));									
					TextOut(hMemDC,Uploadimgweight/2-strlen(uploadimgbuf[Oldx])*6, 0,uploadimgbuf[Oldx], -1);					
					BitBlt(hDC,rectpos[0]+Oldx%7*(Uploadimgweight+1), rectpos[1]+Oldx/7*2*(Uploadimgheight), Uploadimgweight, Uploadimgheight,hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont); 
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);

	}
		
			
			
		DeleteDC(hMemDC);
	
	ReleaseDC(hWnd, hDC);
}


