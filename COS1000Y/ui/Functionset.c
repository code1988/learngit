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


#define Funcweight 130
#define Funcheight 49

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;



static int	old_x = 0;
static int	pos_x = 0;


static char message[32];
static int function[6];

static char F[6][9] = {"����ʱ��","��������","����ģʽ","����1111","��������","����3333"};
//������������
static int pos[2] = { 277,20};
					
//��Ԫ������					
static int rectpos[2] = {212,14 };							
						   

static  char	functionbuf[6][10];

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
//static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void  refresh_keytable(HWND hWnd,int Oldx,int Posx,int flag);



int function_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"FUNCTIONWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"FUNCTIONWIN",
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
			rect.left = 0;
			rect.right = WINDOW_WIDTH;
			rect.top = 0;
			rect.bottom = WINDOW_HEIGHT;
			FillRect(hBgMemDC, &rect, hBrush);                                           //��ͼ֮ǰ����λͼ���
			
			GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/functionwin.bmp", 0);     //�����ȵ���1ͼ
			
		DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	message[0] = '\0';
	function[0] = upload_param_load(F[0], 120);
	function[1] = upload_param_load(F[1], 200);
	function[2] = upload_param_load(F[2], 0);
	function[3] = upload_param_load(F[3], 0);
	function[4] = upload_param_load(F[4], 1);
	function[5] = upload_param_load(F[5], 1);
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
	u8_t i = 0;
	hDC = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
				//  ��ǰѡ�е�Ԫ����ʾ��ɫ
					hBrush = CreateSolidBrush(RGB(255,0,0));
						rect.left = rectpos[0];
						rect.right = rectpos[0]+Funcweight ;
						rect.top = rectpos[1]+Funcheight*pos_x;
						rect.bottom = rectpos[1]+Funcheight*(pos_x+1);
						FillRect(hMemDC, &rect, hBrush);
					DeleteObject(hBrush);
					
					for(i = 0;i<6;i++)
					{
						sprintf(functionbuf[i], "%d", function[i]);
					}
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);
					SetTextColor(hMemDC, RGB(255, 0, 0));			
					TextOut(hMemDC, 165, 255, message, -1);					
					SetTextColor(hMemDC,RGB(255, 255, 255));
					for(i = 0;i<6;i++)
					{						
						TextOut(hMemDC, pos[0]-strlen(functionbuf[i])*6, pos[1]+Funcheight*i, functionbuf[i], -1);  
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
	int min[6]={60,1,0,0,1,1};
	int  max[6]={600,1000,2,1,3,2};
	int i = 0;
	pos_x = old_x;
	switch(wParam) {
		case VK_F1:     //right
			if(function[pos_x]<max[pos_x])
				function[pos_x]++;	
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F2:    //up
			if(function[pos_x]<(max[pos_x]-10)&&pos_x<2)
				function[pos_x]+=10;
			else if(pos_x<2)
				function[pos_x]= max[pos_x];
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F3:       //down
			if((function[pos_x]<(max[pos_x]-100))&&(pos_x<2))
				function[pos_x]+=100;
			else if(pos_x<2)
				function[pos_x]=max[pos_x];
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F4:       //return
			if(pos_x<5)
			{
				pos_x++;
				refresh_keytable(hWnd,old_x,pos_x,1);

			}
				
			else if(pos_x>=5)
			{
				pos_x = 0;
				refresh_keytable(hWnd,old_x,pos_x,1);

			}
				
			
			old_x = pos_x;
			break;	
		case VK_F5:     //ģʽ
			if(function[pos_x]>min[pos_x])
				function[pos_x]--;
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F6:    //�ۼ�
			if(function[pos_x]>(min[pos_x]+10)&&pos_x<2)
				function[pos_x]-=10;
			else if(pos_x<2)
				function[pos_x]=min[pos_x];
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F7: 
			if(function[pos_x]>(min[pos_x]+100)&&pos_x<2)
				function[pos_x]-=100;
			else if(pos_x<2)
				function[pos_x]=min[pos_x];
			refresh_keytable(hWnd,old_x,pos_x,0);
			old_x = pos_x;
			break;
		case VK_F8:     //��ѯ�� ����
			for(i = 0;i<6;i++)
			{
				upload_param_save(F[i],function[i]);
				printf("%d\n",function[i]);

			}
			if(function[2]==0)
			{
				preset_value_set(200);
				//SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)0xF0F1, (LPARAM)0);

			}
					
			else if(function[2]==1)
			{
				preset_value_set(100);
				//SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)0xF0F1, (LPARAM)0);

			}
			else if(function[2]==2)
				preset_value_set(-1);
			if(function[3]==0)
			{
				fota_money4_support_set(0); //close
				fota_paramsave(1);

			}
				
			else if(function[3]==1)
			{
				fota_money4_support_set(1); //open
				fota_paramsave(1);

			}
				
				
			pos_x = 0;
			old_x = 0;
			spi_keyalert();
			DestroyWindow(hWnd);
			break;
		case VK_F9:       //save
			pos_x = 0;
			old_x = 0;
			spi_keyalert();
			DestroyWindow(GetMenuWinHwnd());					
			break;
		
		default:
			break;
	}
	return 0;
}


static void refresh_keytable(HWND hWnd,int Oldx,int Posx,int flag)
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	HFONT			hOldFont;
	RECT			rect;
	HBRUSH			hBrush;

	sprintf(functionbuf[Oldx], "%d", function[Oldx]);
	sprintf(functionbuf[Posx], "%d", function[Posx]);
	
	hDC = GetDC(hWnd);

		//Ŀ��λ�ã������Ӻ�ɫͼ�ϣ�Դλ�ã������ӵ�ͼ��
		hMemDC = CreateCompatibleDC(hDC);
			hBitmap = CreateCompatibleBitmap(hMemDC, Funcweight, Funcheight);
			hOldBitmap = SelectObject(hMemDC, hBitmap);
				BitBlt(hMemDC, 0, 0, Funcweight, Funcheight, hBgMemDC, rectpos[0], rectpos[1]+Funcheight*Posx, SRCCOPY);
				hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());	
					hBrush = CreateSolidBrush(RGB(255,0,0));
						rect.left = 0;
						rect.right = Funcweight;
						rect.top = 0;
						rect.bottom = Funcheight;
						FillRect(hMemDC, &rect, hBrush);
					DeleteObject(hBrush);				
					SetBkColor(hMemDC, RGB(0, 255, 0));
					SetBkMode(hMemDC, TRANSPARENT);						
					SetTextColor(hMemDC, RGB(255, 255, 255));
					TextOut(hMemDC,Funcweight/2-strlen(functionbuf[Posx])*6, 0,functionbuf[Posx], -1);
					
					BitBlt(hDC,rectpos[0], rectpos[1]+Funcheight*Posx, Funcweight, Funcheight,hMemDC, 0, 0, SRCCOPY);
				SelectObject(hMemDC, hOldFont);	
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBitmap);
				
	if(flag == 1)
	{

				hBitmap = CreateCompatibleBitmap(hMemDC, Funcweight, Funcheight);
				hOldBitmap = SelectObject(hMemDC, hBitmap);
					BitBlt(hMemDC, 0, 0, Funcweight, Funcheight, hBgMemDC, rectpos[0], rectpos[1]+Funcheight*Oldx, SRCCOPY);
					hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());				
						SetBkColor(hMemDC, RGB(0, 255, 0));
						SetBkMode(hMemDC, TRANSPARENT); 					
						SetTextColor(hMemDC, RGB(255, 255, 255));									
						TextOut(hMemDC,Funcweight/2-strlen(functionbuf[Oldx])*6, 0,functionbuf[Oldx], -1);					
						BitBlt(hDC,rectpos[0], rectpos[1]+Funcheight*Oldx, Funcweight, Funcheight,hMemDC, 0, 0, SRCCOPY);
					SelectObject(hMemDC, hOldFont); 
				SelectObject(hMemDC, hOldBitmap);
				DeleteObject(hBitmap);



	}
			
			
		DeleteDC(hMemDC);
	
	ReleaseDC(hWnd, hDC);
}

