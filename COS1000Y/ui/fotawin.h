#ifndef _FOTAWIN_H_
#define _FOTAWIN_H_
#include "et850_data.h"
#include "monitor.h"
#include "fota_analyze.h"
#include "device.h"
#include "utypes.h"
#define WINDOW_WIDTH		480
#define WINDOW_HEIGHT		320




//���������ľ��
extern HWND GetMainWinHwnd(void);
//������˵����
extern HWND GetMenuWinHwnd(void);
//��ò������ý�����
extern HWND parasetwin_handle_get(void);
//���˵�����
extern int menuwin_create(HWND hWnd);


//�������ý���
extern int batchset_window_create(HWND hWnd);
//�ȼ����ý���
extern int levelsetwin_create(HWND hWnd);
//�����ϸ����
extern int mixquery_window_create(HWND hWnd);
//�������ѯ����
extern int guanzihaowin_create(HWND hWnd);
//������ʾ����
extern int picdistinguishwin_create(HWND hWnd);
//�������ý���
extern int password_window_create(HWND hWnd);
//�������ý���
extern int parasetwin_create(HWND hWnd);
//�汾����
extern int version_window_create(HWND hWnd);
//�������ý���
extern int function_window_create(HWND hWnd);
//���ݹ������
extern int datamanage_window_create(HWND hWnd);
//���������ý���
extern int blackname_window_create(HWND hWnd);
//������������
extern int netnumber_window_create(HWND hWnd);
//ʱ�����ý���
extern int timeset_window_create(HWND hWnd);
//����IP���ý���
extern int myIP_window_create(HWND hWnd);
//�ϻ�����
extern int agetest_window_create(HWND hWnd);

//������IP���ý���
extern int serverIPwin_create(HWND hWnd);
//���������������
extern int myMaskwin_create(HWND hWnd);
//�������ؽ���
extern int myGatewin_create(HWND hWnd);
//�����˿ڽ���
extern int myPortwin_create(HWND hWnd);
//�������˿ڽ���
extern int serverPortwin_create(HWND hWnd);
//����MAC��ַ�������
extern int myMACwin_create(HWND hWnd);
//��̬���Խ���
extern int dynamicwin_create(HWND hWnd);
//��̬���Խ���
extern int staticwin_create(HWND hWnd);
//CISУ�����
extern int CISadjust_create(HWND hWnd);
//����ģʽ����
extern int countmodewin_create(HWND hWnd);
//�ϴ�ͼ��ʱ�����ý���
extern int uploadimg_window_create(HWND hWnd);
//CIS�������ý���
extern int identifywin_create(HWND hWnd);
//���Խ���
extern int adjustmenu_window_create(HWND hWnd);
//�������б����
extern int Blacknamelist_create(HWND hWnd);

#endif