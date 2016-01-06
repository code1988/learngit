/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: main.c
* Author			: 
* Date First Issued	: 130722    
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2013	        : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "BaseDisp.h"
#include "display.h"
#include "grlib.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//�����ú���
void PraperMianMenu(void);
//ȡ�ñ���ɫ
void FetchBackColor(tRectangle Coordinate);
//������
void DisplayCursor(void);
//����ԭ�й��
void ClearCursor(void);
//��ʾICON�ĺ���
void printIcon(INT8U IMAGE,tRectangle Icon,tRectangle Coordinate);
//��ʾ����ͼƬ
void printBackImage(INT8U IMAGE);
//��ʾ����ͼƬ��Ԥ����
void PraPerprintBackImage(INT8U IMAGE);
//��������ʾ�ַ����ĺ���
void prints(INT8U size,INT16U x,INT16U y,INT8U *s);
//Э����غ���
void TXBYTE( INT8U i);
void TXWORD( INT16U i);
void frame_end();
//���������С��ʾ�ַ����ĺ���
void printsstring8(INT16U x,INT16U y,INT8U *s);
void printsstring32(INT16U x,INT16U y,INT8U *s);
void printsstring12(INT16U x,INT16U y,INT8U *s);
void printsstring24(INT16U x,INT16U y,INT8U *s);
//��ʾ�����˵������ֵĺ���(1���ַ�)
void print1charStrings16(tRectangle Coordinate,INT8U *s);
//��ʾ�����˵������ֵĺ���(2���ַ�)
void print2charStrings16(tRectangle Coordinate,INT8U *s);
//��ʾ�ַ���
void printStrings16(tRectangle Coordinate,INT8U *s);
//��ʾ�����ڲ鿴���˳��ڲ鿴�����
void printOutLetValueStrings16(tRectangle Coordinate,INT8U *s);
//��ʾ���ֺ����ַ���
void printCrownWordStrings16(tRectangle Coordinate,INT8U *s);
//���˵���ʾʱ��
void printMainMenuTimeStrings32(tRectangle Coordinate,INT8U *s);
//��ʾ32�Ŵ�С����
void printStrings32(tRectangle Coordinate,INT8U *s);
//ֱ�����16*16���ַ���
void printCharStrings16(tRectangle Coordinate,INT8U *s);
//���SPEED
void JcSpeedprints(INT8U size,INT16U x,INT16U y,INT8U *s);
//���PWM
void JcPwmprints(INT8U size,INT16U x,INT16U y,INT8U *s);
//��ʾ�汾��Ϣ
void printVersionStrings32(tRectangle Coordinate,INT8U *s);
//�����ϸ�鿴���ַ�����ӡ����
void printMixNums16(tRectangle Coordinate,INT8U *s);
//����������ַ����ĺ���
void Clear(INT8U size,INT16U x,INT16U y);
//���ֺ���
void handshake(void);
//����ǰ��ɫ�ͱ���ɫ
void setcolor(INT16U color1,INT16U color2);
//���������ɫ
void fillw(INT8U code,INT16U x1,INT16U y1,INT16U x2,INT16U y2);

//��ͨ�˵�����ʾ����
void DisplayCommonMenu(struct st_menu* pMenu, INT8U cursor);
//���������в˵�����ʾ����
void DisplayStartMenu(struct st_menu* pMenu, INT8U cursor);
//����˵��Ǻ���ʾ
void DisplayPasswordMenu(INT8U cursor);
//���˵���ʾ����
void DisplayMianMenu(struct st_menu* pMenu, INT8U cursor);

//������
void DisplayInfomationList(void);
//����ԭ�й��
void CleanInfomationList(void);
//��ʾ��α��������Ŀ��
void PraperAuthenticationDataSettings();
//�����˵�������ʾ
void DisplayParaMenu(void);
//��ʾ���޸���ʾ����
void DisplayEditParaMenu(void);
//������
void PraperLevelUpING(void);
//��������
void PraperLevelUpED(void);
//����ʧ��
void PraperLevelUpFail(void);
//��ʾ�����޸ĵ�ҳ��
void PraperParmSet(void);
//����ֵԽ��
void PraperAmountOver(void);
//���������ʾ
void PraperPasswordError(void);
//��������
void DisplayKeyBoard(void);
//����ԭ�й��
void ClearKeyBoard(void);
//����ʱ������
void PraperNetPostTime(void);
//������������
void PraperNetPostPages(void);
//���м��
void PraperBlankName(void);
//������
void PraperAreaCode(void);
//֧�к�
void PraperBranchNumber(void);
//�����
void PraperNetworkNumber(void);
//IP
void PraperIPNumber(void);
//��ֹ����1
void PraperBanTechnician1(void);
//��ֹ����2
void PraperBanTechnician2(void);
//��ֹ����3
void PraperBanTechnician3(void);
//Mask
void PraperMask(void);
//Gateway
void PraperGateWay(void);
//���ض˿�
void PraperLocalPort(void);
//Զ�˶˿�
void PraperDistalPort(void);
//Զ��IP
void PraperDistalIP(void);
//������
void PraperBlackList(void);
//������ҳ����ʾ
void PraperBlackListPage(void);
//��ʾ�û���Ϣ��3ҳ
void PraperInformation3Page(void);
//������ֵȼ���ѡ�б�ʶ
void DisplayClearLevelmark(void);
//��ֵȼ��ͼ�α�ȼ�ѡ�й�����
void ClearClearLevelmark(void);
//��ʾ��������ѡ��״̬
void DisplayCheckmark(void);
//������������ѡ��״̬
void ClearCheckmark(void);
//�ٶȵ��Բ˵�
void PraperSpeedSetMenu(void);

//֡Э��
void Fota_UIA_UART_Frame_Start(INT8U ack,INT32U Len,INT8U *Pdata);
void Fota_UIA_UART_Frame_End(INT8U INMode,INT8U *Pdata);

//�û���Ϣ��ʾ
void InformationDisp(void);
//��������ʾ
void blackListDisp(void);

//���Բ˵��õĺ���

//���������˵�
void PraperMcDebugMainMenu();
void printsstring16(INT16U x,INT16U y,INT8U *s);
void PraperOldTest();
void PraperMTDispTest();
void PraperSpeedSetMenuINMode();
void PraperSpeedSetMenuOutMode();
void PraperLevelUp();
void PraperMTADMenu();
void PraperUVADMenu();
void PraperInfoIRADMenu();
void PraperInfoMTADMenu();
void PraperInfoMGADMenu();
void PraperCheckCSMenu();
//��ӡ�����ڳ�����AD�������Ա��
void PraperInLetADMenu(void);
//����AD����
void PraperIRADMenu(void);
//CSУ�鿪ʼ
void PraperCheckCSStart(void);
//CSУ��ʧ��
void PraperCheckCSFail(void);
//CSͼ��У��ɹ�
void PraperCheckCSImageSuccess(void);
//CSУ���һ�׶γɹ�
void PraperCheckCSStage1Success(void);
//CSУ��ڶ��׶ο�ʼ
void PraperCheckCSStage2Start(void);
/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/


