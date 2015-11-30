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


#define VERSION     "1.0.0.50"       //  UI工程版本号

#define Versionheight    31

#define ID_TIME_UPGRADE_DSP		600
static int		timer_upgrade_dsp_s 	= 0;

#define ID_TIME_UPGRADE_MCU		601
static int		timer_upgrade_mcu_s 	= 0;

#define ID_TIME_UPGRADE_ALL		602
static int  	timer_upgrade_all_s		= 0;

static HWND		hMainWnd = NULL;

static HDC		hBgMemDC = NULL;
static HBITMAP	hBgBitmap, hOldBgBitmap;


static HDC		hwarningMemDC = NULL;
static HBITMAP	hwarningBitmap,hOldwarningBitmap;

static HDC		hsavingMemDC = NULL;
static HBITMAP	hsavingBitmap,hOldsavingBitmap;

static HDC		hsaving1MemDC = NULL;
static HBITMAP	hsaving1Bitmap,hOldsaving1Bitmap;

static u8_t		focus = 0;
static u8_t		flag[8]={0};

static char version[8][32];

static int versionpos[2] = {245,50};
static	char iterm[7][6] = {"ARM","Data","DSP","FPGA","Model","MCU","LCD"};
static	char progress[3][4] = {".","..","..."};		
static u32_t  sdtotal = 0,sdfree = 0;

static LRESULT CALLBACK MainProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
//static void refresh_keytable(HWND hWnd,int Oldx,int Posx);
static void warning_window (HWND hWnd,s8_t *buf);
static void saving_window (HWND hWnd,s8_t *buf,int flag);





int version_window_create(HWND hWnd)
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
	wndclass.lpszClassName  = (LPCSTR)"VERSIONWIN";
	RegisterClass(&wndclass);
	hMainWnd = CreateWindowEx(0L,
							  (LPCSTR)"VERSIONWIN",
							  (LPCSTR)"",
							  WS_VISIBLE | WS_CHILD,
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
	int         i=0;
	hDC = GetDC(hWnd);
	
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	hBgMemDC = CreateCompatibleDC(hDC);													
	hBgBitmap = CreateCompatibleBitmap(hBgMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);						    
	hOldBgBitmap = SelectObject(hBgMemDC, hBgBitmap);
	rect.left = 0;rect.right = WINDOW_WIDTH;rect.top = 0;rect.bottom = WINDOW_HEIGHT;
	FillRect(hBgMemDC, &rect, hBrush);                                           	
	GdDrawImageFromFile(hBgMemDC->psd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "/bmp/fota/versionwin.bmp", 0);     
	//报警标志
	hwarningMemDC = CreateCompatibleDC(hDC);
	hwarningBitmap = CreateCompatibleBitmap(hwarningMemDC, 400, 45);
	hOldwarningBitmap = SelectObject(hwarningMemDC, hwarningBitmap); 
	rect.left = 0; rect.right = 400; rect.top = 0; rect.bottom = 45;
	FillRect(hwarningMemDC, &rect, hBrush);
	GdDrawImageFromFile(hwarningMemDC->psd, 0, 0, 400, 45, "/bmp/fota/kaijibaojingwin.bmp", 0);

	hsavingMemDC = CreateCompatibleDC(hDC);
	hsavingBitmap = CreateCompatibleBitmap(hsavingMemDC, 425, 180);
	hOldsavingBitmap = SelectObject(hsavingMemDC, hsavingBitmap); 
	rect.left = 0; rect.right = 425; rect.top = 0; rect.bottom = 180;
	FillRect(hsavingMemDC, &rect, hBrush);
	GdDrawImageFromFile(hsavingMemDC->psd, 0, 0, 425, 180, "/bmp/fota/savingwin.bmp", 0);

	hsaving1MemDC = CreateCompatibleDC(hDC);
	hsaving1Bitmap = CreateCompatibleBitmap(hsaving1MemDC, 425, 180);
	hOldsaving1Bitmap = SelectObject(hsaving1MemDC, hsaving1Bitmap); 
	rect.left = 0; rect.right = 425; rect.top = 0; rect.bottom = 180;
	FillRect(hsaving1MemDC, &rect, hBrush);
	GdDrawImageFromFile(hsaving1MemDC->psd, 0, 0, 425, 180, "/bmp/fota/saving1win.bmp", 0);

	
	DeleteObject(hBrush);
	ReleaseDC(hWnd, hDC);
	
	for(i = 0;i<10;i++)
		version[i][0] = '\0';
	arm_software_version_get(version[0]);   //ARM
	dsp_param_version_get(version[1]);		//para
	dsp_software_version_get(version[4]);	//DSP
	memcpy(version[2],version[4]+15,10);
	dsp_fpga_version_get(version[3]);		//FPGA
	dsp_module_version_get(version[4]);		//MODEL
	mcu_software_version_get(version[5]);   	//mcu
	memcpy(version[6],VERSION,strlen(VERSION));
	//memcpy(version[7],"一键升级",9);
	if (timer_upgrade_mcu_s > 0)
	{
		KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
		timer_upgrade_mcu_s = 0;

	}
	
	if (timer_upgrade_dsp_s > 0)
	{
		KillTimer(hWnd, ID_TIME_UPGRADE_DSP);
		timer_upgrade_dsp_s = 0;
	}

	if(timer_upgrade_all_s > 0 )
	{

		KillTimer(hWnd, ID_TIME_UPGRADE_ALL);
		timer_upgrade_all_s = 0;
	}
	return 0;
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

	if (timer_upgrade_mcu_s > 0)
	{
		KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
		timer_upgrade_mcu_s = 0;

	}
	
	if (timer_upgrade_dsp_s > 0)
	{
		KillTimer(hWnd, ID_TIME_UPGRADE_DSP);
		timer_upgrade_dsp_s = 0;
	}

	if(timer_upgrade_all_s > 0 )
	{

		KillTimer(hWnd, ID_TIME_UPGRADE_ALL);
		timer_upgrade_all_s = 0;
	}
	
	SelectObject(hBgMemDC, hOldBgBitmap);
	DeleteObject(hBgBitmap);
	DeleteDC(hBgMemDC);

	SelectObject(hwarningMemDC, hOldwarningBitmap);
	DeleteObject(hwarningBitmap);
	DeleteDC(hwarningMemDC);

	
	SelectObject(hsavingMemDC, hOldsavingBitmap);
	DeleteObject(hsavingBitmap);
	DeleteDC(hsavingMemDC);

	SelectObject(hsaving1MemDC, hOldsaving1Bitmap);
	DeleteObject(hsaving1Bitmap);
	DeleteDC(hsaving1MemDC);

	hMainWnd = NULL;
	return 0;
}

static int OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
	HDC				hDC, hMemDC;
	HBITMAP	hBitmap, hOldBitmap;
	PAINTSTRUCT		ps;
	HFONT			 hOldFont;
	u8_t i = 0;
	//char backup[32];
	hDC = BeginPaint(hWnd, &ps);
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hBgMemDC, 0, 0, SRCCOPY);
	
	hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
	SetBkColor(hMemDC, RGB(0, 255, 1));		// 背景颜色
	SetBkMode(hMemDC, TRANSPARENT);

	arm_software_version_get(version[0]);   //ARM
	dsp_param_version_get(version[1]);		//para
	dsp_software_version_get(version[4]);	//DSP
	memcpy(version[2],version[4]+15,10);
	dsp_fpga_version_get(version[3]);		//FPGA
	dsp_module_version_get(version[4]);		//MODEL
	mcu_software_version_get(version[5]);   	//mcu
	

	//获取机具编号
	device_name_get(version[7]);	
	//memcpy(version[7],jijuhao+11,14);
	for(i = 0;i<8;i++)
	{
		if(flag[i]==1)
			SetTextColor(hMemDC, RGB(0, 255, 0));
		else if(flag[i]==8)
			SetTextColor(hMemDC, RGB(255, 255, 0));
		else if (focus == i)
			SetTextColor(hMemDC, RGB(255, 0, 0));
		else
			SetTextColor(hMemDC, RGB(255, 255, 255));	
		TextOut(hMemDC, versionpos[0]-strlen(version[i])*6, versionpos[1]+i*Versionheight, version[i], -1);  
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
	status_t	ret;
	int i=0;
	s8_t buf[64];
	switch(wParam) {
		case VK_F1:    
            ret = system("cp /media/sdcard/upgrade/arm/image/*.bmp /bmp/fota/");			
            if(ret == 0)
            {
				memset(buf,0,64);
				memcpy(buf,"更新完毕，按查询键",strlen("更新完毕，按查询键"));
				warning_window(hWnd,buf);   
            }
            else
            {
				memset(buf,0,64);
				memcpy(buf,"更新失败，请重启",strlen("更新失败，请重启"));
				warning_window(hWnd,buf);   
            }
			break;
		case VK_F2:   
			break;
		case VK_F3:       //down
			focus = (focus + 6)%7;
			InvalidateRect(hWnd,NULL,FALSE);
			break;
		case VK_F4:       //return
			focus = (focus + 1)%7;
			InvalidateRect(hWnd,NULL,FALSE);
			break;	
		case VK_F5:     //升级
			if(sdcard_meminfo_get(&sdtotal,&sdfree)==0)  //
			{
				memset(buf,0,64);
				memcpy(buf,"请插入SD卡，并按查询键",strlen("请插入SD卡，并按查询键"));
				warning_window(hWnd,buf);  

			}
			else if(sdtotal>0&&sdfree>0)
			{

				if(focus == 5)			//		mcu
				{
					ret = mcu_upgrade_create("/media/sdcard/upgrade/mcu/mcu");
					printf("mcu_upgrade_create ret = %d\n", ret);
					if(ret == 0)
						timer_upgrade_mcu_s =  SetTimer(hWnd, ID_TIME_UPGRADE_MCU, 100, NULL);
					else 
					{
						flag[focus] = 8;			
						InvalidateRect(hWnd,NULL,FALSE);

					}
					
				}
				else if(focus == 2) 		//dsp
				{
					ret = fota_upgrade_create(1, "/media/sdcard/upgrade/dsp/dsp", NULL);
					printf("fota_upgrade_create dsp ret = %d\n",ret);
					if(ret == 0)
						timer_upgrade_dsp_s = SetTimer(hWnd, ID_TIME_UPGRADE_DSP, 100, NULL);
					else 
					{
						flag[focus] = 8;			
						InvalidateRect(hWnd,NULL,FALSE);

					}
				}
				else if(focus == 3) 		//fpga
				{
					ret = fota_upgrade_create(2, "/media/sdcard/upgrade/dsp/fpga", NULL);
					printf("fota_upgrade_create dsp ret = %d\n",ret);
					if(ret == 0)
						timer_upgrade_dsp_s = SetTimer(hWnd, ID_TIME_UPGRADE_DSP, 100, NULL);
					else 
					{
						flag[focus] = 8;			
						InvalidateRect(hWnd,NULL,FALSE);
					
					}

				}
				else if(focus == 4) 		//module
				{
					ret = fota_upgrade_create(3, "/media/sdcard/upgrade/dsp/module", NULL);
					printf("fota_upgrade_create dsp ret = %d\n",ret);
					if(ret == 0)
						timer_upgrade_dsp_s = SetTimer(hWnd, ID_TIME_UPGRADE_DSP, 100, NULL);
					else 
					{
						flag[focus] = 8;			
						InvalidateRect(hWnd,NULL,FALSE);

					}

				}
				else if(focus == 0) 		  //ARM
				{
					sprintf(buf, "正在升级 %s %s", iterm[focus],progress[2]);
					saving_window (hWnd,buf,210);

					ret = system("cp /media/sdcard/upgrade/arm/fsnpacket /usr/local/bak/");
					
					if(ret == 0)
						flag[focus] = 1;
					else 
					{
						flag[focus] = 8;			

					}
					printf("fota_upgrade_create arm ret = %d\n",ret);
					InvalidateRect(hWnd,NULL,FALSE);
				}
				else if(focus ==1)			//data
				{
					ret = system("cp /media/sdcard/upgrade/dsp/param.dat /db/");
					sprintf(buf, "正在升级 %s %s", iterm[focus],progress[2]);
					saving_window (hWnd,buf,210);
					printf("fota_upgrade_create data ret = %d\n",ret);
					if(ret == 0)
					{			 
						flag[focus] = 1;
						if (image_identify_export() == OK_T)
							image_identify_sync();
					}
					else 
					{
						flag[focus] = 8;			

					}

					
					printf("fota_upgrade_create data ret = %d\n",ret);
					InvalidateRect(hWnd,NULL,FALSE);
	
				}
				else if(focus == 6) 		// LCD
				{
					ret = system("cp /media/sdcard/upgrade/arm/appwin /usr/local/bak/");
					sprintf(buf, "正在升级 %s %s", iterm[focus],progress[2]);
					saving_window (hWnd,buf,210);
					if(ret == 0)
						flag[focus] = 1;
					else
						flag[focus] = 8;
					printf("fota_upgrade_create ui ret = %d\n",ret);
					InvalidateRect(hWnd,NULL,FALSE);
				} 

			}		
			break;
		case VK_F6:    //一键升级
			if(sdcard_meminfo_get(&sdtotal,&sdfree)==0)  //
			{
				memset(buf,0,64);
				memcpy(buf,"请插入SD卡，并按查询键",strlen("请插入SD卡，并按查询键"));
				warning_window(hWnd,buf);  

			}
			else if(sdtotal>0&&sdfree>0)
			{
				// 一键升级 
				printf("This is upgrade all key!\n");
				ret = upgrade_create("/media/sdcard/upgrade");
				if(ret == 0)
					timer_upgrade_all_s = SetTimer(hWnd, ID_TIME_UPGRADE_ALL, 1000, NULL);
				else
				{
					memset(buf,0,64);
					memcpy(buf,"升级失败，请重启",strlen("升级失败，请重启"));
					warning_window(hWnd,buf);	
					printf("upgrade_create fail\n");

				}				

			}
		 		
		
			break;
		case VK_F7:     //鏌ヨ閿?淇濆瓨
			InvalidateRect(hWnd,NULL,FALSE);
			break;
		case VK_F8:       //save		
			spi_keyalert();
			i = flag[0]+flag[1]+flag[2]+flag[3]+flag[4]+flag[5]+flag[6];
			printf("i= %d\n",i);
			if(i>7)
			{
				printf("restart\n");
				memset(buf,0,64);

				memcpy(buf,"升级失败，请重启",strlen("升级失败，请重启"));
				warning_window(hWnd,buf);  
				
									
			}
			else if(i<=7&&i>0)
			{
				printf("restart\n");
				memset(buf,0,64);
				memcpy(buf,"升级成功，请重启",strlen("升级成功，请重启"));
				warning_window(hWnd,buf);  
				
									
			}
			else 
			{
				DestroyWindow(hWnd);

			}
			break;	
		case VK_F9:       //save		
			spi_keyalert();
			i = flag[0]+flag[1]+flag[2]+flag[3]+flag[4]+flag[5]+flag[6];
			printf("flag[0] = %d,flag[1]= %d,flag[2]=%d,flag[3]=%d,flag[4]=%d,flag[5]=%d,flag[6]=%d\n",flag[0],flag[1],flag[2],flag[3],flag[4],flag[5],flag[6]);
			if(i>7)
			{
				printf("restart\n");
				memset(buf,0,64);
				memcpy(buf,"升级失败，请重启",strlen("升级失败，请重启"));
				warning_window(hWnd,buf);  
				
									
			}
			else if(i<=7&&i>0)
			{
				printf("restart\n");
				memset(buf,0,64);
				memcpy(buf,"升级成功，请重启",strlen("升级成功，请重启"));
				warning_window(hWnd,buf);  
				
									
			}
			else 
			{
				DestroyWindow(GetMenuWinHwnd());

			}
			break;
		default:
			break;
	}
	return 0;
}

static int OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	s32_t 	ret;
	s32_t 	errorcode;
	u8_t 	percent;
	

	UPGRADE_STATUS_S upgrade_s;
	s32_t st;
	u8_t i;
	s8_t buf[64];

	static u8_t j=0;
		switch(wParam) 
		{
			case ID_TIME_UPGRADE_ALL:	
				ret = upgrade_status_get(&upgrade_s);
				printf("upgrade_status_get ret = %d\n",ret);
				if(!ret)
				{	
					printf("upgrade_s.arm = %d,upgrade_s.param = %d,upgrade_s.dsp = %d,upgrade_s.fpga = %d,upgrade_s.module = %d,upgrade_s.mcu = %d,upgrade_s.total = %d\n",
						upgrade_s.arm,upgrade_s.param,upgrade_s.dsp,upgrade_s.fpga,upgrade_s.module,upgrade_s.mcu,upgrade_s.total);
					for(i=0;i<6;i++)
					{
						st = *((s32_t *)(&upgrade_s) + i);
						if(st == 0x10)
						{
							focus = i;
							sprintf(buf, "正在升级 %s %s", iterm[focus],progress[j]);
							j++;
							if(j>=3)
								j=0;
							saving_window (hWnd,buf,210);
							 
						}
						else if(st == 0x20&&flag[i] ==0)
						{
							flag[i] = 1;							
							
						}
						else if(st == -1&&flag[i] ==0)
						{
							flag[i] = 8;	 
						}
						else
							continue;
		
						
					}
		
					st = *((s32_t *)(&upgrade_s) + 6);
					if(st == 0x20&&flag[0]&&flag[1]&&flag[2]&&flag[3]&&flag[4]&&flag[5])
					{
						flag[6] = 1;
						if (timer_upgrade_mcu_s > 0)
						KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
						timer_upgrade_mcu_s = 0;
		
						InvalidateRect(hWnd,NULL,FALSE);
					printf("CCC");
					}
				}
				break;
			case ID_TIME_UPGRADE_MCU:
				
				ret = mcu_upgrade_status_get();
				printf("mcu_upgrade_status: %d\n",ret);	
				//进度条
				sprintf(buf, "正在升级 %s %s", iterm[focus],progress[j]);
				j++;
				if(j>=3)
					j=0;
				if(ret>=0&&ret<=6)
					saving_window (hWnd,buf,70*ret);
				else
					saving_window (hWnd,buf,420);
				
				if(ret == 10)
				{
					if (timer_upgrade_mcu_s > 0)
						KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
					timer_upgrade_mcu_s = 0;
					flag[focus] = 1;
					
					InvalidateRect(hWnd,NULL,FALSE);
					printf("DDD");
				}
				else if((ret == -10))
				{
					if (timer_upgrade_mcu_s > 0)
						KillTimer(hWnd, ID_TIME_UPGRADE_MCU);
					timer_upgrade_mcu_s = 0;
					printf("mcu upgrade fail!\n");
					flag[focus] = 8;
					InvalidateRect(hWnd,NULL,FALSE);
					printf("EEE");
				}
				break;
			case ID_TIME_UPGRADE_DSP:
				ret = fota_upgrade_status(&errorcode, &percent);
					//进度条
				printf("ret = %d,errorcode = %d,percent = %d\n",ret,errorcode,percent);
				sprintf(buf, "正在升级 %s %s", iterm[focus],progress[j]);
				j++;
				if(j>=3)
					j=0;
				if(ret>=0&&ret<=6)
					saving_window (hWnd,buf,70*ret);
				else
					saving_window (hWnd,buf,420);
				if(ret == UPGRADE_FINISH_STATUS)
				{
					if (timer_upgrade_dsp_s > 0)
					{
						printf("update is over\n");
						KillTimer(hWnd, ID_TIME_UPGRADE_DSP);
						timer_upgrade_dsp_s = 0;
		
					}
					fota_upgrade_destroy(); 
					
					if(errorcode==0)
					{
						flag[focus] = 1;
						InvalidateRect(hWnd,NULL,FALSE);
						printf("FFF");
						printf("fota_upgrade ok!\n");
						printf("focus = %d\n",focus);
		
					}
					else 
					{
						flag[focus] = 8;
						InvalidateRect(hWnd,NULL,FALSE);
						printf("fota_upgrade faild!\n");
						printf("GGG");
		
					}
				}		
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
static void saving_window (HWND hWnd,s8_t *buf,int flag)
{
	HDC 			hDC,hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	HFONT			 hOldFont;
	hDC = GetDC(hWnd);

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hMemDC, 425, 180);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
		BitBlt(hMemDC, 0, 0, 425, 180, hsavingMemDC, 0, 0, SRCCOPY);
		hOldFont = SelectObject(hMemDC, (HFONT)GetFont32Handle());
			SetBkColor(hMemDC, RGB(0, 255, 0));
			SetBkMode(hMemDC, TRANSPARENT);;							
			SetTextColor(hMemDC,RGB(255,0 , 0));
			
			
			BitBlt(hMemDC, 0, 0, flag,180,hsaving1MemDC, 0, 0, SRCCOPY);

			TextOut(hMemDC, 72, 100, buf, -1);
			BitBlt(hDC, 20, 150, 425,180,hMemDC, 0, 0, SRCCOPY);
		SelectObject(hMemDC, hOldFont);

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);


}



