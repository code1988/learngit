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

static char number[10];
static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HDC		hfocusMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;
static HBITMAP	hfocusBitmap, hOldfocusBitmap;
static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);



int password_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"PASSWORDWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"PASSWORDWIN",
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
	HBRUSH		hBrush;
	
	hDC = GetDC(hWnd);
	
	hBgMemDC = CreateCompatibleDC(hDC);													
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, 480, 320);						    
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	rect.left = 0;
	rect.right = 480;
	rect.top = 0;
	rect.bottom = 320;
	FillRect(hBgMemDC, &rect, hBrush);                                           
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, 480, 320, "/bmp/fota/passwordwin.bmp", 0);     

	hfocusMemDC = CreateCompatibleDC(hDC);
	hfocusBitmap = CreateCompatibleBitmap(hfocusMemDC, 450, 79);
	hOldfocusBitmap = SelectObject(hfocusMemDC, hfocusBitmap);	
	rect.left = 0;
	rect.right = 450;
	rect.top = 0;
	rect.bottom = 79;
	FillRect(hfocusMemDC, &rect, hBrush);
	GdDrawImageFromFile(hfocusMemDC->psd, 0, 0, 450, 79, "/bmp/fota/batnum.bmp", 0);    

	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	number[0] = '\0';
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
	
	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	int				i, len = 0;
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 480, 320);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, 480, 320, hBgMemDC, 0, 0, SRCCOPY);													
	len = strlen(number);
	for (i = 0; i < len; i++) 
	{
		BitBlt(hMemDC, 333 - (len-i)*45, 139, 45, 79, hfocusMemDC, (number[i] - '0') * 45, 3, SRCCOPY);
	}
	BitBlt(hDC, 0, 0, 480, 320, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);	
	EndPaint(hWnd, &ps);
	
	return 0;
}

static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

	char len = 0; 
	len = strlen(number);
	switch(wParam) {
		case VK_F1:     //left
			if(len<4)
			{				
				number[len] = '1';	
				number[len+1] = '\0';	
			}	
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F2:    //right
			if(len<4)
			{
				number[len] = '2';	
				number[len+1] = '\0';				
			}
			InvalidateRect(hWnd, NULL, FALSE);	
			break;
		case VK_F3:       //up
			if(len<4)
			{				
				number[len] = '3';	
				number[len+1] = '\0';
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F4:     //down
			if(len<4)
			{				
				number[len] = '4';	
				number[len+1] = '\0';	
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F5:    //clear  //å¼€ç›¸åº”ä½ç½®çš„è€åŒ–
			if(len<4)
			{
				number[len] = '5';	
				number[len+1] = '\0';				
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F6:     //yes   å…³ç›¸åº”ä½ç½®çš„è€åŒ–
			if(len<4)
			{
				
				number[len] = '6';	
				number[len+1] = '\0';
			}	
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F7:       //save
			if(len<4)
			{
				number[len] = '7';	
				number[len+1] = '\0';					
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_F8:       //return
			if(strcmp(number,"1256")==0)
			{
				printf("ok\n");
				SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)0xF0F0, (LPARAM)0);
				DestroyWindow(hWnd);
			}
			else 
			{
				number[0] = '\0';
				InvalidateRect(hWnd, NULL, FALSE);
			}			
			break;
		case VK_F9:       //return
			number[0] = '\0';
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd)); 
			break;
		default:
			number[0] = '\0';
			spi_keyalert();
			DestroyWindow(hWnd);
			SetFocus(GetParent(hWnd)); 
			break;
	}
	return 0;
}



