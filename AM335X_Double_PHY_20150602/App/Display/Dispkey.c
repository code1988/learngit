/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: key.c
* Author			: ��ҫ
* Date First Issued	: 02/20/2014 
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "Dispkey.h"
#include "display.h"
#include "DwinPotocol.h"
#include "CoodinateInit.h"
#include "DisplayMain.h"
#include "DispkeyFunction.h"
#include "RecevCanPotocol.h"
/* Private define-----------------------------------------------------------------------------*/
#define BLACKLISTMAXPAGES 0x02
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//��������ʱ�����ֵ�����û�ȷ���Ժ󱣴浽ȫ�ֱ���BatchNum��
INT16U TemBatchNum = 0;
//�����ڲ鿴���ĵ�ǰ��ҳ��
INT8U OutLetKeyPage = 0;
//�����ڲ鿴���ĵ�ǰ��ҳ��
INT8U InLetKeyPage = 0;
//ͨ��CAN�����巢����Ϣ������
extern INT8U CanTxBuff[8];
//����ֵ
extern INT16U BatchNum;
//�鿴��α����������
extern tRectangle BaseCheckParam[10][10];
//�޸ļ�α����������
extern tRectangle BaseEditParam[10][12];
//��α�����ı���ɫȡ�õ�����
extern tRectangle BaseEditParamColor[4];
//��α�����ĺ�����
extern INT8U Paramvertical;
//��α������������
extern INT8U Paramhorizontal;
//��α�����޸ĵĵ�ǰҳ��
extern INT8U EditParamPage;
//��α�������޸��õ�������
extern INT8U AuthenticationDataSettings[3][90];
//��α�����ݴ����
extern INT8U TempAuthenticationDataSettings[3][90];
//�����ںͽ����ڵ���ϸ�鿴����ҳ��
extern INT8U OutLetTotalPage;
extern INT8U InLetTotalPage;
//��ֵȼ�
extern INT8U ClearLevel[6];
//���̹����ʾ�ĺ�����
extern INT8U KeyBoardvertical;
//���̹����ʾ��������
extern INT8U KeyBoardhorizontal;
//��Ϣ�鿴��ҳ��
extern INT8U InfomationList;
//��Ϣ�鿴��ǰѡ�еڼ���
extern INT8U InfomationPage;

//��ֵȼ�����ʱ�����ֵ
INT8U TemClearLevel[6];
//��ֵȼ���������ֵ��������λ��꣩
INT8U ClearLevelhorizontal = 0;
//��α�ȼ�
extern INT8U AuthenticationLevel[7];
//��α�ȼ�����ʱ�����ֵ
INT8U TemAuthenticationLevel[7] = {0};
//��α�ȼ���������ֵ��������λ��꣩
INT8U AuthenticationLevelhorizontal = 0;
//�洢���������
INT8U Password[5] = {0};
//��������ѡ�еĵڼ�λ
INT8U PasswordCursor = 0;
//�ۼ�ģʽ
extern INT8U SUMMATIONMODE;
//��ȡ��ģʽ
extern INT8U DEPWITHDRAWMODE;
//���ģʽ
extern INT8U CLEARINGMODE;
//���͸�CAN����Ϣ������
INT8U UIA_Send[8] = {0};
//���͸�CAN����Ϣ
extern _BSP_MESSAGE Dsend_message; 
extern _BSP_MESSAGE Flashsend_message; 
//������
extern INT8U BlackList[200][12];
//�߳���Ϣ�鿴
extern INT8U InfoADpage;
//��α��Ϣҳ�����
extern INT8U AuthenticationDataPages;
//����Աǩ��
INT8U SignIn[11] = {0};
INT8U TempSignIn[11] = {0};
INT8U SignInNUM = 0;
//���׺�����
INT8U AnsactionNum[11] = {0};
INT8U TempAnsactionNum[11] = {0};
INT8U AnsactionNumNUM = 0;
//����ʱ��
INT8U NetPostTime[6] = {0};
INT8U TempNetPostTime[6] = {0};
INT8U NetPostTimeNUM = 0;
//��������
INT8U NetPostPage[2] = {0};
INT8U TempNetPostPage[2] = {0};
INT8U NetPostPageNUM = 0;
//���м��
INT8U BlankName[16] = {0};
INT8U TempBlankName[16] = {0};
INT8U BlankNameNUM = 0;
//������
INT8U AreaNum[16] = {0};
INT8U TempAreaNum[16] = {0};
INT8U AreaNumNUM = 0;
//֧�к�
INT8U BranchNum[16] = {0};
INT8U TempBranchNum[16] = {0};
INT8U BranchNumNUM = 0;
//�����
INT8U LatticePoint[11] = {0};
INT8U TempLatticePoint[11] = {0};
INT8U LatticePointNUM = 0;
//���������к�
INT8U MachineNo[16] = {0};
INT8U TempMachineNo[16] = {0};
INT8U MachineNoNUM = 0;
//IP����
INT8U IP[13] = {0};
INT8U TempIP[13] = {0};
INT8U IPNUM = 0;
//MASK����
INT8U Mask[13] = {0};
INT8U TempMask[13] = {0};
INT8U MaskNUM = 0;
//GW����
INT8U GateWay[13] = {0};
INT8U TempGateWay[13] = {0};
INT8U GateWayNUM = 0;
//���ض˿�����
INT8U LocalPort[5] = {0};
INT8U TempLocalPort[5] = {0};
INT8U LocalPortNUM = 0;
//Զ�˿�����
INT8U DiskPort[5] = {0};
INT8U TempDiskPort[5] = {0};
INT8U DiskPortNUM = 0;
//Զ��IP
INT8U DiskIP[13] = {0};
INT8U TempDiskIP[13] = {0};
INT8U DiskIPNUM = 0;
//��ֹ����ʱ��
INT8U BanTechnician[3][16] = {0};
INT8U TempBanTechnician[3][16] = {0};
INT8U BanTechnicianNUM = 0;
INT8U BanTechnicianPage = 0;
//MAC
INT8U Mac[13] = {0};
//ʱ��
INT8U Time[6] = {0};
INT8U TempTime[6] = {0};
INT8U TimeNUM = 0;
//����������
INT8U TempBlackLists[13] = {0};
INT8U BlackLists[2][7][13] = {0};
INT8U BlackListPage = 0;
INT8U BlackListhorizontal = 0;
INT8U BlackListKeyNum = 0;

INT8U JcSpeedMode = 0x01;
//�Ƿ��޸Ĺ�����
INT8U EditParmMode = 0x00;

extern INT8U KeyBoardMAP[4][12];
extern tRectangle BaseSignInCoordinate[16];
//��￪ͣ����־
extern INT8U MTStatus;
//�˵���־
extern INT8U MainMenuStatus;
//У���ʾ
extern INT8U CSCheckFlag;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: DealWithKey
* Description	: ��������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void DealWithKey(INT8U *keyval)
{
    gps_CurMenu->function2(keyval); 
}

/***********************************************************************************************
* Function		: KeyEvent_0
* Description	: ���˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_0(INT8U *keyval)
{    
    static INT8U MTContrl[3] = {0};
    static INT8U MCmode[3] = {0};
    switch(*keyval)
    {
        case 1:
              //��ȥ�˵�
              //���ͽ���˵���ָ������ARM
              MainMenuStatus = OUTMAINMENU;
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              //��ʾ�˵��Ľ���
              gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[7];
              printBackImage(gps_CurMenu->FrameID);
              break;
        case 2:
              //����
              //���ͽ���˵���ָ������ARM
              MainMenuStatus = OUTMAINMENU;
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[0];
              printBackImage(gps_CurMenu->FrameID);
              break;
        case 3:
              //����
              //���ͽ���˵���ָ������ARM
              MainMenuStatus = OUTMAINMENU;
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[1];
              printBackImage(gps_CurMenu->FrameID);
              BatchMenuNumDisp();
              break;
        case 4:
              //�ȼ�
              //���ͽ���˵���ָ������ARM
              MainMenuStatus = OUTMAINMENU;
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[5];
              CleanLevelDisp();
              DisplayClearLevelmark();
              break;
        case 5:
              //ģʽ
              if(CLEARINGMODE < 5 )
              {
                  CLEARINGMODE++;
              }
              else
              {
                  CLEARINGMODE = 0;
              }
              MCmode[0] = CLEARINGMODE;
              MCmode[1] = DEPWITHDRAWMODE;
              MCmode[2] = SUMMATIONMODE;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMODE;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MCmode;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              PraperMianMenu();
              break;
        case 6:
              //�ۼ�
              SUMMATIONMODE = !SUMMATIONMODE;
              MCmode[0] = CLEARINGMODE;
              MCmode[1] = DEPWITHDRAWMODE;
              MCmode[2] = SUMMATIONMODE;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMODE;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MCmode;
              OSQPost (canEvent,&Dsend_message);
              OSTimeDlyHMSM(0,0,0,10);
              PraperMianMenu();
              break;
        case 7:
              //��ѯ
              //���ͽ���˵���ָ������ARM
              MainMenuStatus = OUTMAINMENU;
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              if(CLEARINGMODE == MIXEDMODE)
              {
                  //��ʾ�����ϸ�˵�
                  Dsend_message.MsgID = APP_COMFROM_UI;
                  Dsend_message.DivNum = UIACANMIXEDSEARCH;
                  OSQPost (canEvent,&Dsend_message);
                  OSTimeDlyHMSM(0,0,0,10);
                  gps_CurMenu = &gsMenu0_3;
                  printBackImage(gps_CurMenu->FrameID);
              }
              else 
              {
                  //��ʾ�˳��ڹ��Ӻ���鿴�˵�
//                  Dsend_message.MsgID = APP_COMFROM_UI;
//                  Dsend_message.DivNum = UIACANOUTLETSEARCH;
//                  OSQPost (canEvent,&Dsend_message);
//                  OSTimeDlyHMSM(0,0,0,10);
//                  gps_CurMenu = &gsMenu0_4;
//                  printBackImage(gps_CurMenu->FrameID);
                    gps_CurMenu = &gsMenu0_4;
                    printBackImage(gps_CurMenu->FrameID);
                    //ˢ���˳��ڲ鿴�˵�
                    OutLetDetailDisp();

              }
              break;
          case 8:
              //DoNothing
              break;
          case 11:
              //���
              DEPWITHDRAWMODE = WITHDRAWMODE;
              MCmode[0] = CLEARINGMODE;
              MCmode[1] = DEPWITHDRAWMODE;
              MCmode[2] = SUMMATIONMODE;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMODE;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MCmode;
              OSQPost (canEvent,&Dsend_message);
              OSTimeDlyHMSM(0,0,0,10);
              PraperMianMenu();
              break;
          case 10:
              //ȡ��
              DEPWITHDRAWMODE = DEPOSITMODE;
              MCmode[0] = CLEARINGMODE;
              MCmode[1] = DEPWITHDRAWMODE;
              MCmode[2] = SUMMATIONMODE;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMODE;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MCmode;
              OSQPost (canEvent,&Dsend_message);
              OSTimeDlyHMSM(0,0,0,10);
              PraperMianMenu();
              
              break;
          case 12:
              //���ͣ��
              if(MTStatus == MTENABLE)
              {
                  MTStatus = MTDISABLE;
              }
              else
              {
                  MTStatus = MTENABLE;
              }
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              //������������ͣ��
              break;
        default:
              break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_1
* Description	: ���ֲ˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_1(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //Do Nothing
            break;
        case 8:
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_2
* Description	: �����˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_2(INT8U *keyval)
{
    INT8U batch[4] = {0}; 
    *keyval&=0x0f;
    if(*keyval>9)
          return;
    switch(*keyval)
    {
        case 1:
            if(TemBatchNum<900)
            {
                TemBatchNum = TemBatchNum+100;
            }
            else
            {
                PraperAmountOver();
                OSTimeDlyHMSM(0,0,1,0);
            }
            BatchMenuNumDisp();
            break;
        case 2:
            if(TemBatchNum<990)
            {
                TemBatchNum = TemBatchNum+10;
            }
            else
            {
                PraperAmountOver();
                OSTimeDlyHMSM(0,0,1,0);
            }
            BatchMenuNumDisp();
            break;
        case 3:
            if(TemBatchNum<999)
            {
                TemBatchNum = TemBatchNum+1;
            }
            else
            {
                PraperAmountOver();
                OSTimeDlyHMSM(0,0,1,0);
            }
            BatchMenuNumDisp();
            break;
        case 4:
            //������ֵ���͸�MC
            BatchNum = TemBatchNum;
            batch[0] = BatchNum>>8;    
            batch[1] = BatchNum&0xFF;    
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANBATCH;
            Dsend_message.DataLen = 2;
            Dsend_message.pData = batch;
            OSQPost (canEvent,&Dsend_message); //���͸�����
            OSTimeDlyHMSM(0,0,0,10);  
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 5:
            if(TemBatchNum>=100)
            {
                TemBatchNum = TemBatchNum-100;
            }
            else
            {
                PraperAmountOver();
                OSTimeDlyHMSM(0,0,1,0);
            }
            BatchMenuNumDisp();
            break;
        case 6:
            if(TemBatchNum>=10)
            {
                TemBatchNum = TemBatchNum-10;
            }
            else
            {
                PraperAmountOver();
                OSTimeDlyHMSM(0,0,1,0);
            }
            BatchMenuNumDisp();
            break;
        case 7:
            if(TemBatchNum>=1)
            {
                TemBatchNum = TemBatchNum-1;
            }
            else
            {
                PraperAmountOver();
                OSTimeDlyHMSM(0,0,1,0);
            }
            BatchMenuNumDisp();   
            break;
        case 8:
            TemBatchNum = 0;
            BatchMenuNumDisp(); 
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_3
* Description	: �����ϸ�˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_3(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            //Do Nothing
            break;
        case 7:
            //�����˳��ڹ��Ӻ���鿴�˵�
            gps_CurMenu = &gsMenu0_4;
            printBackImage(gps_CurMenu->FrameID);
            OutLetDetailDisp();
            break;
        case 8:
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_4
* Description	: �˱ҿڹ��ֺ���鿴�˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_4(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            //Do Nothing
            break;
        case 6:
            if(OutLetKeyPage < (OutLetTotalPage/9))
            {
                OutLetKeyPage++;
                printBackImage(gps_CurMenu->FrameID);
                OutLetDetailDisp();
            }
            else 
            {
                OutLetKeyPage = 0;
                printBackImage(gps_CurMenu->FrameID);
                OutLetDetailDisp();
            }
            break;
        case 7:
            //���ؽӳ��ڲ鿴��֡���͸�MC
            OutLetKeyPage = 0;
            gps_CurMenu = &gsMenu0_5;
            printBackImage(gps_CurMenu->FrameID);
            InLetDetailDisp();
            break;
        case 8:
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_5
* Description	: �ӳ��ڹ��ֺ���鿴�˵��İ�������
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_5(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            //Do Nothing
            break;
        case 6:
            if(InLetKeyPage < (InLetTotalPage/9))
            {
                InLetKeyPage++;
                printBackImage(gps_CurMenu->FrameID);
                InLetDetailDisp();
            }
            else 
            {
                InLetKeyPage = 0;
                printBackImage(gps_CurMenu->FrameID);
                InLetDetailDisp();
            }
            break;
        case 7:
            if(CLEARINGMODE == MIXEDMODE)
            {
                //������������ϸ���ݵ�֡��MC
                InLetKeyPage = 0;
                Dsend_message.MsgID = APP_COMFROM_UI;
                Dsend_message.DivNum = UIACANMIXEDSEARCH;
                OSQPost (canEvent,&Dsend_message);
                //���ػ����ϸ�˵�
                gps_CurMenu = &gsMenu0_3;
                printBackImage(gps_CurMenu->FrameID);
            }
            else
            {
                InLetKeyPage = 0;
                //�����˳��ڲ鿴��֡���͸�MC
                gps_CurMenu = &gsMenu0_4;
                printBackImage(gps_CurMenu->FrameID);
                OutLetDetailDisp();
            }
            break;
        case 8:
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_6
* Description	: ��ֵȼ��˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_6(INT8U *keyval)
{
    INT8U CleanleveltoFlash[8] = {0};
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
            if(TemClearLevel[ClearLevelhorizontal] < 5)
            {
                TemClearLevel[ClearLevelhorizontal]++;
            }
            else
            {
                TemClearLevel[ClearLevelhorizontal] = 1;
            }
            break;
        case 2:
            if(TemClearLevel[ClearLevelhorizontal] > 1)
            {
                TemClearLevel[ClearLevelhorizontal]--;
            }
            else
            {
                TemClearLevel[ClearLevelhorizontal] = 5;
            }
            break;
        case 3:
            if(ClearLevelhorizontal<5)
            {
                ClearLevelhorizontal++;
            }
            else
            {
                ClearLevelhorizontal = 0; 
            }
            break;
        case 4:
            //��ҳ
            DisplayCommonMenu(&gsMenu0_7,0);
            memcpy(TemAuthenticationLevel,AuthenticationLevel,7);
            AuthenticationLevelDisp();
            DisplayClearLevelmark();
            return;
        case 5:
            //Do Nothing
            return;
        case 6:
            //����
            CleanleveltoFlash[0] = 0x10;    //�洢��ֵȼ���ID�ţ���λ��
            CleanleveltoFlash[1] = 0x02;    //�洢��ֵȼ���ID�ţ���λ��
            memcpy(ClearLevel,TemClearLevel,6);
            memcpy(CleanleveltoFlash+2,ClearLevel,6);
            Flashsend_message.MsgID = APP_COMFROM_UI;
            Flashsend_message.DataLen = 8;
            Flashsend_message.pData = CleanleveltoFlash;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢            
            //������ֵȼ������ARM
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum= UIACANCLEARLEVEL;
            Dsend_message.DataLen = 6;
            Dsend_message.pData = ClearLevel;
            OSQPost (canEvent,&Dsend_message);
            ClearClearLevelmark();
            break;
        case 7:
            //Do Nothing
            break;
        case 8:
            ClearLevelhorizontal = 0;
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayClearLevelmark();
            return;
        default:
          break;
    }
    CleanLevelDisp();
    DisplayClearLevelmark();
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_7
* Description	: ��α�ȼ��˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_7(INT8U *keyval)
{
    INT8U AuthenticationLeveltoFlash[9] = {0};
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
            if(TemAuthenticationLevel[AuthenticationLevelhorizontal] < 5)
            {
                TemAuthenticationLevel[AuthenticationLevelhorizontal]++;
            }
            else
            {
                TemAuthenticationLevel[AuthenticationLevelhorizontal] = 1;
            }
            break;
        case 2:
            if(TemAuthenticationLevel[AuthenticationLevelhorizontal] > 1)
            {
                TemAuthenticationLevel[AuthenticationLevelhorizontal]--;
            }
            else
            {
                TemAuthenticationLevel[AuthenticationLevelhorizontal] = 5;
            }
            break;
        case 3:
            if(AuthenticationLevelhorizontal<6)
            {
                AuthenticationLevelhorizontal++;
            }
            else
            {
                AuthenticationLevelhorizontal = 0;
            }
            break;
        case 4:
            DisplayCommonMenu(&gsMenu0_6,0);
            memcpy(TemClearLevel,ClearLevel,6);
            CleanLevelDisp();
            DisplayClearLevelmark();
            return;
        case 5:
            //Do Nothing
            break;
        case 6:
            //����
            AuthenticationLeveltoFlash[0] = 0x10;    //�洢��α�ȼ���ID�ţ���λ��
            AuthenticationLeveltoFlash[1] = 0x03;    //�洢��α�ȼ���ID�ţ���λ��
            memcpy(AuthenticationLevel,TemAuthenticationLevel,7);
            memcpy(AuthenticationLeveltoFlash+2,AuthenticationLevel,7);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 9;
            Flashsend_message.pData = AuthenticationLeveltoFlash;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            //���ͼ�α�ȼ������ARM
            Dsend_message.MsgID = APP_COMFROM_UI;//���͵�һ֡
            Dsend_message.DivNum = UIACANAUTHENTICLEVEL1;
            Dsend_message.DataLen = 7;
            Dsend_message.pData = AuthenticationLevel;
            OSQPost (canEvent,&Dsend_message); 
            OSTimeDlyHMSM(0,0,0,10);//��ʱ��һ��Ҫ
            DisplayClearLevelmark();
            break;
        case 7:
            break;
        case 8:
            //Do Nothing
            AuthenticationLevelhorizontal = 0;
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            return;
        default:
          break;
    }
    AuthenticationLevelDisp();
    DisplayClearLevelmark();
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_8
* Description	: �˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_8(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
            //����Աǩ��
            //����Աǩ��
            memcpy(TempSignIn,SignIn,10);
            DisplayCommonMenu(&gsMenu_14,0);
            KeyBoardDispSignInNum(SignIn);
            DisplayKeyBoard();
            break;
        case 2:
            //����Աǩ��
            //���
            memset(TempSignIn,0,11);
            DisplayCommonMenu(&gsMenu_15,0);
            break;
        case 3:
            //���׺�����
            memset(AnsactionNum,0,11);
            DisplayCommonMenu(&gsMenu_16,0);
            DisplayKeyBoard();
            break;
        case 4:
            //�û���Ϣ
            DisplayCommonMenu(&gsMenu_17,0);
            DisplayInfomationList();
            break;
        case 5:
            //����������
            DisplayCommonMenu(&gsMenu_23,0);
            DisplayCheckmark();
            blackListDisp();
            PraperBlackListPage();
            break;
        case 6:
            //���ݲ鿴
            gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[5];
            printBackImage(gps_CurMenu->FrameID);
            DisplayParaMenu();
            break;
        case 7:
            //����ά��
            gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[6];
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_11
*Description	: �鿴�����˵��İ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_11(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
              break;
        case 7:
              //�鿴�˵���ҳ����
              break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_12
*Description	: �����޸ĵİ�������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2/2/2015	�γ�
***********************************************************************************************/
void KeyEvent_12(INT8U *keyval)
{
    INT8U AuthenticationParam[273] = {0};
      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            DealCursor(*keyval);
            //��괦��
            break;
        case 5:
            //������1
            DealWithParm(1); 
            EditParmMode = 0x01;
            break;
        case 6:
            //������1
			DealWithParm(0);
            EditParmMode = 0x01;
            break;
        case 7:
            if(EditParmMode==0)
            {
                //�鿴�˵���ҳ����
                if(AuthenticationDataPages == 2)
                {
                    AuthenticationDataPages = 0;
                }
                else
                {
                    AuthenticationDataPages++;
                }
                DisplayCommonMenu(&gsMenu0_12,0);
                PraperAuthenticationDataSettings();
                DisplayEditParaMenu();
            }
            else
            {
                //�����޸ĵĲ���
                gps_CurMenu = gps_CurMenu->Menu_pFather;
                printBackImage(gps_CurMenu->FrameID);
                EditParmMode = 0x00;
            }
            break;
        case 8:
            //����
            ClearCursor();
            //�����������ʹ�ܷ�ҳ��
            EditParmMode = 0x00;
            Dsend_message.MsgID = APP_COMFROM_UI;
            //��α�������͸�CAN����
            Dsend_message.DivNum = UIACANAUTHENTICATION;  
            Dsend_message.DataLen = 270;
            Dsend_message.pData = (INT8U *)TempAuthenticationDataSettings;
            OSQPost (canEvent,&Dsend_message);   
            OSTimeDly(SYS_DELAY_50ms);
            //��α�������͸�SPIFLASH����
            AuthenticationParam[0] = 0x10;    //�����޸ĵ�ID�ţ���λ��
            AuthenticationParam[1] = 0x14;    //�����޸ĵ�ID�ţ���λ��
            memcpy(AuthenticationParam+2,TempAuthenticationDataSettings,270);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 272;
            Flashsend_message.pData = AuthenticationParam;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            OSTimeDly(SYS_DELAY_250ms);
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            memcpy(AuthenticationDataSettings,TempAuthenticationDataSettings,270);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_14
*Description	: ����Աǩ��
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_14(INT8U *keyval)
{
    INT8U SignIntoNand[13] = {0x03};
    INT8U i = 0;
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempSignIn[SignInNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            SignInNUM++;
            if(SignInNUM>9)SignInNUM=9;
            setcolor(0xFFFF,0x0000);
            while(TempSignIn[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempSignIn[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempSignIn,0,11);
            SignInNUM = 0;
            while(i<10)
            {
                printStrings32(BaseSignInCoordinate[i],&TempSignIn[i]);
                i++;
            }
            break;
        case 7:
            //����
            SignIntoNand[0] = 0x10;    //����Աǩ������ʱ�洢��FLASH��
            SignIntoNand[1] = 0x55;    //����Աǩ������ʱ����
            memcpy(SignIn,TempSignIn,10);
            memcpy(SignIntoNand+2,SignIn,10);
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_15
*Description	: ����Աǩ��
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_15(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            //DoNothing
            break;
        case 7:
            //ǩ��
            memset(SignIn,0,11);
            SignInNUM = 0;
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //����
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_16
*Description	: ���뽻�׺�
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_16(INT8U *keyval)
{
    INT8U i = 0;
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempAnsactionNum[AnsactionNumNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            AnsactionNumNUM++;
            if(AnsactionNumNUM>9)AnsactionNumNUM=9;
            setcolor(0xFFFF,0x0000);
            while(TempAnsactionNum[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempAnsactionNum[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempAnsactionNum,0,11);
            AnsactionNumNUM = 0;
            while(i<10)
            {
                printStrings32(BaseSignInCoordinate[i],&TempAnsactionNum[i]);
                i++;
            }
            break;
        case 7:
            //����
            memcpy(AnsactionNum,TempAnsactionNum,10);
            memset(TempAnsactionNum,0,11);
            AnsactionNumNUM = 0;
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //����
            memset(TempAnsactionNum,0,11);
            AnsactionNumNUM = 0;
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_17
*Description	: ��Ϣ�鿴1
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_17(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
            //DoNothings
            break;   
        case 3:
            //������һ�����
            CleanInfomationList();
            //����һ��,�����ʾ
            if(InfomationList==6)
            {
                InfomationList = 0;
            }
            else
            {
                InfomationList++;
            }
            DisplayInfomationList();
            break;
        case 4:
            //��ҳ
            //������һ�����
            CleanInfomationList();
            InfomationPage = 1;
            InfomationList = 0;
            DisplayCommonMenu(&gsMenu_18,0);
            DisplayInfomationList();
            break;
        case 5:
            //����
            //������һ�����
            CleanInfomationList();
            switch(InfomationList)
            {
                case 0x00:  
                    //����ʱ��
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperNetPostTime();
                    break;
                case 0x01:  
                    //��������
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperNetPostPages();
                    break;
                case 0x02:  
                    //���м��
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBlankName();
                    break;
                case 0x03:  
                    //������
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperAreaCode();
                    break;
                case 0x04:  
                    //֧�к�
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBranchNumber();
                    break;
                case 0x05:  
                    //�����
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperNetworkNumber();
                    break;
                case 0x06:  
                    //IP
                    DisplayCommonMenu(&gsMenu_50,0);
                    KeyBoardDispNum(IP);
                    DisplayKeyBoard();
					PraperIPNumber();
                    break;
                default:
                    //IP
                    DisplayCommonMenu(&gsMenu_50,0);
                    KeyBoardDispNum(IP);
                    DisplayKeyBoard();
					PraperIPNumber();
                    break;        
            }
            break;
        case 6:
        case 7:
            //DoNothing
            break;
        case 8:
            //����
            CleanInfomationList();
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_18
*Description	: ��Ϣ�鿴2
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_18(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
            //DoNothings
            break;   
        case 3:
            //������һ�����
            CleanInfomationList();
            //����һ��,�����ʾ
            if(InfomationList==5)
            {
                InfomationList = 0;
            }
            else
            {
                InfomationList++;
            }
            DisplayInfomationList();
            break;
        case 4:
            //��ҳ
            //������һ�����
            CleanInfomationList();
            InfomationPage = 2;
            InfomationList = 0;
            DisplayCommonMenu(&gsMenu_24,0);
            PraperInformation3Page();
            DisplayInfomationList();
            break;
        case 5:
            //����
            //������һ�����
            CleanInfomationList();
            switch(InfomationList)
            {
                case 0x00:  
                    //MAC
                    DisplayCommonMenu(&gsMenu_26,0);
                    KeyBoardDispNum(Mac);
                    DisplayKeyBoard();
                    break;
                case 0x01:  
                    //���������к�
                    DisplayCommonMenu(&gsMenu_25,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
                    break;
                case 0x02:  
                    //��ֹ����1
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBanTechnician1();
                    break;
                case 0x03:  
                    //��ֹ����2
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBanTechnician2();
                    break;
                case 0x04:  
                    //��ֹ����3
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBanTechnician3();
                    break;
                case 0x05:  
                    //ʱ��
                    DisplayCommonMenu(&gsMenu_20,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
                    break;
                default:
                    //ʱ��
                    DisplayCommonMenu(&gsMenu_20,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
                    break;        
            }
            break;
        case 6:
        case 7:
            //DoNothing
            break;
        case 8:
            //����
            CleanInfomationList();
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_19
*Description	: ���������
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_19(INT8U *keyval)
{
    INT8U i = 0;      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempLatticePoint[LatticePointNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            LatticePointNUM++;
            if(LatticePointNUM>9)LatticePointNUM=9;
            setcolor(0xFFFF,0x0000);
            while(TempLatticePoint[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempLatticePoint[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempLatticePoint,0,11);
            LatticePointNUM = 0;
            while(i<10)
            {
                printStrings32(BaseSignInCoordinate[i],&TempLatticePoint[i]);
                i++;
            }
            break;
        case 7:
            //����
            memcpy(LatticePoint,TempLatticePoint,10);
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_20
*Description	: ʱ������
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2014	�γ�
***********************************************************************************************/
void KeyEvent_20(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
            //�������
            break;
        case 2:
            //�������
            break;
        case 3:
            //�������
            break;
        case 4:
            //��ҳ
            break;
        case 5:
            //��һ
            break;
        case 6:
            //��һ
            break;
        case 7:
            //����
            break;
        case 8:
            //����
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_21
*Description	: �汾��Ϣ1
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_21(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
             break;
        case 5:
            //��ҳ���汾��Ϣ2
            DisplayCommonMenu(&gsMenu_22,0);
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            //����
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_22
*Description	: �汾��Ϣ2
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_22(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
             break;
        case 5:
            //��ҳ���汾��Ϣ1
            DisplayCommonMenu(&gsMenu_21,0);
            VersionDisp();
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            //����
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_23
*Description	: ����������
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_23(INT8U *keyval)
{
    static INT8U FlashBlcakList[202] = {0};
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
             Dealblacklist(*keyval);
             PraperBlackListPage();
             break;
        case 5:
            //���
            /* �������ü��Ժ�Ǩ�Ƶ������������˵� */
            DisplayCommonMenu(&gsMenu_28,0);
            DisplayKeyBoard();
            PraperBlackList();
            KeyBoardDispNum(BlackLists[BlackListPage][BlackListhorizontal]);
            break;
        case 6:
            //ɾ��
            DeleteBlack();
            break;
        case 7:
            //�޸�
            /* �������ü��Ժ�Ǩ�Ƶ������������˵� */
            DisplayCommonMenu(&gsMenu_28,0);
            DisplayKeyBoard();
            PraperBlackList();
            break;
        case 8:
            //����,�浽��FLASH��
            FlashBlcakList[0] = 0x10;    //�洢Զ�˶˿ڵ�ID�ţ���λ��
            FlashBlcakList[1] = 0x15;    //�洢Զ�˶˿ڵ�ID�ţ���λ��
            memcpy(FlashBlcakList+2,BlackLists,200);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 202;
            Flashsend_message.pData = FlashBlcakList;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_24
*Description	: ��Ϣ�鿴3
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_24(INT8U *keyval)
{
	*keyval&=0x0f;
	if(*keyval>9)
			return;
	
	switch(*keyval)
	{
		case 1:
		case 2:
			//DoNothings
            break;    
		case 3:
			//������һ�����
			CleanInfomationList();
			//����һ��,�����ʾ
			if(InfomationList==4)
			{
				InfomationList = 0;
			}
			else
			{
				InfomationList++;
			}
			DisplayInfomationList();
			break;
		case 4:
			//��ҳ
			//������һ�����
			CleanInfomationList();
			InfomationPage = 0;
			InfomationList = 0;
			DisplayCommonMenu(&gsMenu_17,0);
			DisplayInfomationList();
			break;
		case 5:
			//����
			//������һ�����
			CleanInfomationList();
			switch(InfomationList)
			{
				case 0x00:	
					//MASK
					DisplayCommonMenu(&gsMenu_51,0);
					KeyBoardDispNum(Mask);
					DisplayKeyBoard();
					PraperMask();
					break;
				case 0x01:	
					//GW
					DisplayCommonMenu(&gsMenu_52,0);
					KeyBoardDispNum(GateWay);
					DisplayKeyBoard();
					PraperGateWay();
					break;
				case 0x02:	
					//���ض˿�
					DisplayCommonMenu(&gsMenu_53,0);
					KeyBoardDispNum(LocalPort);
					DisplayKeyBoard();
					PraperLocalPort();
					break;
				case 0x03:	
					//Զ�˶˿�
					DisplayCommonMenu(&gsMenu_54,0);
					KeyBoardDispNum(DiskPort);
					DisplayKeyBoard();
					PraperDistalPort();
					break;
				case 0x04:	
					//Զ��IP
					DisplayCommonMenu(&gsMenu_55,0);
					KeyBoardDispNum(DiskIP);
					DisplayKeyBoard();
					PraperDistalIP();
					break;
				default:
					//MASK
					DisplayCommonMenu(&gsMenu_51,0);
					KeyBoardDispNum(LatticePoint);
					DisplayKeyBoard();
                    PraperMask();
					break;		  
			}
			break;
		case 6:
		case 7:
			//DoNothing
			break;
		case 8:
            //������һ�����
			CleanInfomationList();
			//����
			/* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
			gps_CurMenu = gps_CurMenu->Menu_pFather;
			printBackImage(gps_CurMenu->FrameID);
			break;
		default:
		  break;
	}
	return;
}

/***********************************************************************************************
* Function	: KeyEvent_25
*Description	: ������
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_25(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
        switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
             Dealblacklist(*keyval);
             break;
        case 5:
            //���
            /* �������ü��Ժ�Ǩ�Ƶ������������˵� */
            DisplayCommonMenu(&gsMenu_14,0);
            DisplayKeyBoard();
            break;
        case 6:
            //ɾ��
            DeleteBlack();
            break;
        case 7:
            //�޸�
            /* �������ü��Ժ�Ǩ�Ƶ������������˵� */
            DisplayCommonMenu(&gsMenu_14,0);
            DisplayKeyBoard();
            break;
        case 8:
            //����
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_26
*Description	: MAC
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_26(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //DoNothing
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_27
*Description	: ��������
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_27(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            //Do Nothing
            break;
        case 6:
            //��Page����1ҳ��ʱ����Է�ҳ
            break;
        case 7:
            //��ҳ��������Ϣ����10����ʱ��ҳ

            break;
        case 8:
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_28
*Description	: ����������
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 12/2/2015	�γ�
***********************************************************************************************/
void KeyEvent_28(INT8U *keyval)
{
    INT8U i = 0;
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempBlackLists[BlackListKeyNum] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            BlackListKeyNum++;
            if(BlackListKeyNum>11)BlackListKeyNum=11;
            setcolor(0xFFFF,0x0000);
            while(TempBlackLists[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempBlackLists[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempBlackLists,0,12);
            BlackListKeyNum = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskPort[i]);
                i++;
            }
            break;
        case 7:
            //����          
            memcpy(BlackLists[BlackListPage][BlackListhorizontal],TempBlackLists,12);
            memset(TempBlackLists,0,12);
            BlackListKeyNum = 0;
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayCheckmark();
            blackListDisp();
            PraperBlackListPage();
            break;
        case 8:
            //����
            memset(TempBlackLists,0,12);
            BlackListKeyNum = 0;
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayCheckmark();
            blackListDisp();
            PraperBlackListPage();
            break;
        default:
            break;
    }
}

/***********************************************************************************************
* Function		: KeyEvent_40
*Description	: �������Բ˵�
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_40(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
    static _BSP_MESSAGE pMsg;
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
              //�������� ��ʱ��������ͼ
              pMsg.MsgID = SEND_REQ | SUB_ID_BIG_IMAGE;				
              pMsg.DivNum = DSP2;
              SYSPost(W5500BEvent,&pMsg);
              break;
        case 2:
              //�������� 
              DisplayCommonMenu(&gsMenu0_12,0);
              PraperAuthenticationDataSettings();
              DisplayEditParaMenu();
              memcpy(TempAuthenticationDataSettings,AuthenticationDataSettings,270);
              break;
        case 3:    
              //���������˵�
              DisplayCommonMenu(&gsMenu_79,0);
              PraperMcDebugMainMenu();
              break;
        case 4:
              //���ͽ����ϻ�
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGOLDTEST;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10); 
              DisplayCommonMenu(&gsMenu_90,0);              
              PraperOldTest();
              break;
        case 5:
              //�汾��Ϣ
              DisplayCommonMenu(&gsMenu_21,0);
              VersionDisp();
              break;
        case 6:
              //����
              DisplayCommonMenu(&gsMenu_84,0);
              PraperLevelUp();
              break;
        case 7:
              //������ʾ
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGWAVEFORM;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10); 
              DisplayCommonMenu(&gsMenu_91,0);              
              PraperMTDispTest();
              break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}


/***********************************************************************************************
* Function		: KeyEvent_41
*Description	: ����鿴�˵�
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 06/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_41(INT8U *keyval)
{
    INT8U adminPassword[4] ={0x03,0x01,0x03,0x01}; 
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //��������
            Password[PasswordCursor] = *keyval;
            if(PasswordCursor<4)PasswordCursor++;
            DisplayPasswordMenu(PasswordCursor);
            break;
        case 8:
            //�Ƚ�����
            PasswordCursor = 0;
            if(memcmp(Password,adminPassword,4))
            {
                //�������
                memset(Password,0,4);
                PraperPasswordError();
                OSTimeDlyHMSM(0,0,1,0);
                //������һ���˵�
                gps_CurMenu = gps_CurMenu->Menu_pFather;
                printBackImage(gps_CurMenu->FrameID);
            }
            else
            {
                //������ȷ
                memset(Password,0,4);
                DisplayCommonMenu(&gsMenu_40,0);
            }            
            OSTimeDlyHMSM(0,0,0,200);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_50
*Description	: IP
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_50(INT8U *keyval)
{
    INT8U FlashIP[6] = {0};
    INT8U i = 0;      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempIP[IPNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            IPNUM++;
            if(IPNUM>11)IPNUM=11;
            setcolor(0xFFFF,0x0000);
            while(TempIP[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempIP[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempIP,0,12);
            IPNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempIP[i]);
                i++;
            }
            break;
        case 7:
            //����
            FlashIP[0] = 0x10;    //�洢IP��ID�ţ���λ��
            FlashIP[1] = 0x04;    //�洢IP��ID�ţ���λ��
            memcpy(IP,TempIP,12);
            IPChartoNum(FlashIP,IP);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashIP;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_51
*Description	: MASK
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_51(INT8U *keyval)
{
    INT8U FlashMask[6] = {0}; 
    INT8U i = 0;      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempMask[MaskNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            MaskNUM++;
            if(MaskNUM>11)MaskNUM=11;
            setcolor(0xFFFF,0x0000);
            while(TempMask[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempMask[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempMask,0,12);
            MaskNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempMask[i]);
                i++;
            }
            break;
        case 7:
            //����
            FlashMask[0] = 0x10;    //�洢MASK��ID�ţ���λ��
            FlashMask[1] = 0x05;    //�洢MASK��ID�ţ���λ��
            memcpy(Mask,TempMask,12);
            IPChartoNum(FlashMask,Mask);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashMask;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_52
*Description	: GW
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_52(INT8U *keyval)
{
    INT8U FlashGw[6] = {0}; 
    INT8U i = 0;      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempGateWay[GateWayNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            GateWayNUM++;
            if(GateWayNUM>11)GateWayNUM=11;
            setcolor(0xFFFF,0x0000);
            while(TempGateWay[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempGateWay[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempGateWay,0,12);
            GateWayNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempGateWay[i]);
                i++;
            }
            break;
        case 7:
            //����
            FlashGw[0] = 0x10;    //�洢GW��ID�ţ���λ��
            FlashGw[1] = 0x06;    //�洢GW��ID�ţ���λ��
            memcpy(GateWay,TempGateWay,12);
            IPChartoNum(FlashGw,GateWay);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashGw;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_53
*Description	: ���ض˿�
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_53(INT8U *keyval)
{
    INT8U FlashLocalPort[6] = {0};
    INT8U i = 0;      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempLocalPort[LocalPortNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            LocalPortNUM++;
            if(LocalPortNUM>3)LocalPortNUM=3;
            setcolor(0xFFFF,0x0000);
            while(TempLocalPort[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempLocalPort[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempLocalPort,0,4);
            LocalPortNUM = 0;
            while(i<4)
            {
                printStrings32(BaseSignInCoordinate[i],&TempLocalPort[i]);
                i++;
            }
            break;
        case 7:
            //����
            FlashLocalPort[0] = 0x10;    //�洢���ض˿ڵ�ID�ţ���λ��
            FlashLocalPort[1] = 0x08;    //�洢���ض˿ڵ�ID�ţ���λ��
            memcpy(LocalPort,TempLocalPort,4);
            PortChartoNum(FlashLocalPort,LocalPort);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 4;
            Flashsend_message.pData = FlashLocalPort;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_54
*Description	: Զ�˶˿�
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_54(INT8U *keyval)
{
    INT8U FlashDiskPort[6] = {0};
    INT8U i = 0;      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempDiskPort[DiskPortNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            DiskPortNUM++;
            if(DiskPortNUM>3)DiskPortNUM=3;
            setcolor(0xFFFF,0x0000);
            while(TempDiskPort[i])
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskPort[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempDiskPort,0,4);
            DiskPortNUM = 0;
            while(i<4)
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskPort[i]);
                i++;
            }
            break;
        case 7:
            //����
            FlashDiskPort[0] = 0x10;    //�洢Զ�˶˿ڵ�ID�ţ���λ��
            FlashDiskPort[1] = 0x0A;    //�洢Զ�˶˿ڵ�ID�ţ���λ��
            memcpy(DiskPort,TempDiskPort,4);
            PortChartoNum(FlashDiskPort,DiskPort);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 4;
            Flashsend_message.pData = FlashDiskPort;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_55
*Description	: Զ��IP
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_55(INT8U *keyval)
{
    INT8U FlashDiskIP[6] = {0};
    INT8U i = 0;      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            //��������
            DealKeyBoard(*keyval);
            break;
        case 5:
            //ѡȡ
            TempDiskIP[DiskIPNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
            DiskIPNUM++;
            if(DiskIPNUM>11)DiskIPNUM=11;
            setcolor(0xFFFF,0x0000);
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskIP[i]);
                i++;
            }
            break;
        case 6:
            //���
            memset(TempDiskIP,0,12);
            DiskIPNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskIP[i]);
                i++;
            }
            break;
        case 7:
            //����
            FlashDiskIP[0] = 0x10;    //�洢Զ��IP��ID�ţ���λ��
            FlashDiskIP[1] = 0x09;    //�洢Զ��IP��ID�ţ���λ��
            memcpy(DiskIP,TempDiskIP,12);
            IPChartoNum(FlashDiskIP,DiskIP);
            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashDiskIP;
            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //����
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_56
*Description	: ��ֹ����ʱ��
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_56(INT8U *keyval)
{
//    INT8U i = 0;      
//    *keyval&=0x0f;
//    if(*keyval>9)
//            return;
//    
//    switch(*keyval)
//    {
//        case 1:
//        case 2:
//        case 3:
//        case 4:
//            //��������
//            DealKeyBoard(*keyval);
//            break;
//        case 5:
//            //ѡȡ
//            TempBanTechnician[BanTechnicianPage][BanTechnicianNUM] = KeyBoardMAP[KeyBoardhorizontal][KeyBoardvertical];
//            BanTechnicianNUM++;
//            if(BanTechnicianNUM>15)BanTechnicianNUM=15;
//            setcolor(0xFFFF,0x0000);
//            while(TempBanTechnician[BanTechnicianPage][i])
//            {
//                printStrings32(BaseSignInCoordinate[i],&TempBanTechnician[BanTechnicianPage][i]);
//                i++;
//            }
//            break;
//        case 6:
//            //���
//            memset(TempBanTechnician[BanTechnicianPage],0,16);
//            BanTechnicianNUM = 0;
//            while(i<15)
//            {
//                printStrings32(BaseSignInCoordinate[i],&TempBanTechnician[BanTechnicianPage][i]);
//                i++;
//            }
//            break;
//        case 7:
//            //����
//            memcpy(BanTechnician,TempBanTechnician,10);
////            Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
////            Flashsend_message.DataLen = 12;
////            Flashsend_message.pData = SignIntoNand;
////            OSQPost (FlashEvent,&Flashsend_message); //���͸�FLASH���д洢
//            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
//            gps_CurMenu = gps_CurMenu->Menu_pFather;
//            printBackImage(gps_CurMenu->FrameID);
//            break;
//        case 8:
//            //����
//            gps_CurMenu = gps_CurMenu->Menu_pFather;
//            printBackImage(gps_CurMenu->FrameID);
//            DisplayInfomationList();
//            break;
//        default:
//            break;
//    }
//    return;
}

/***********************************************************************************************
* Function	: KeyEvent_79
*Description	: ����
* Input		: 
* Output		: 
* Note(s)		: 
* Contributor  : 31/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_79(INT8U *keyval)
{
    static INT8U MTContrl[3] = {0};
    static INT8U JSmode[2] = {0};
    static _BSP_MESSAGE pMsg;
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
              //�������
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGJCSPEED;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              DisplayCommonMenu(&gsMenu_80,0);
              PraperSpeedSetMenu();
              PraperSpeedSetMenuINMode();
              JSmode[0] = JcSpeedMode;           
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); 
              break;
        case 2:
              //�����ӳ�AD              
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGINLETAD;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              DisplayCommonMenu(&gsMenu_83,0);
              PraperInLetADMenu();
              break;
        case 3:
			  //����AD��
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGIRAD;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              DisplayCommonMenu(&gsMenu_82,0);
              PraperIRADMenu();
              break;
        case 4:
              //��С��ͷAD              
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGMGAD;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              DisplayCommonMenu(&gsMenu_85,0);
              PraperMTADMenu();
              break;
        case 5:
              //ӫ��AD
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGUVAD;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              DisplayCommonMenu(&gsMenu_86,0);
              PraperUVADMenu();
              break;
        case 6:
              //�߳���Ϣ
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGINFOAD;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              DisplayCommonMenu(&gsMenu_87,0);
              PraperInfoIRADMenu();
              break;
        case 7:
              //CS����
              pMsg.MsgID =  SEND_REQ | SUB_ID_ADJ;				
              pMsg.DivNum = DSP2;
              pMsg.pData = g_cTmpBuf;
              pMsg.DataLen = g_sFileObject.fsize;
              SYSPost(W5500BEvent,&pMsg);
              OSTimeDlyHMSM(0,0,0,20); 
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              MTContrl[2] = MCDEBUGCHECKCS;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 3;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message);  
              DisplayCommonMenu(&gsMenu_81,0);
              CSCheckFlag = TRUE;
              PraperCheckCSMenu(); 
              break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_80
*Description	: �ٶȵ��Բ˵�
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor   	: 06/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_80(INT8U *keyval)
{
    static INT8U MTContrl[3] = {0};
    static INT8U JSmode[2] = {0};
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
              //ѡ�н���ģʽ
              PraperSpeedSetMenu();
              PraperSpeedSetMenuINMode();
              JcSpeedMode = 0x01;
              JSmode[0] = JcSpeedMode;           
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 1;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //���͸���� 
              break;
        case 2:
              //���ģʽ
              PraperSpeedSetMenu();
              PraperSpeedSetMenuOutMode();
              JcSpeedMode = 0x02;
              JSmode[0] = JcSpeedMode;           
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 1;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //���͸���� 
              break;
        case 3:
        case 4:
              //DoNothigs
              break;
        case 5:
              //+1
              if(JcSpeedMode == 0x01)
              {
                  JcSpeedMode = 0x01;
                  JSmode[0] = JcSpeedMode;
                  JSmode[1] = 0x03;
              }
              else if(JcSpeedMode == 0x02)
              {
                  JcSpeedMode = 0x02;
                  JSmode[0] = JcSpeedMode;
                  JSmode[1] = 0x01;
              }
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //���͸���� 
              break;
        case 6:
              //-1
              if(JcSpeedMode == 0x01)
              {
                  JcSpeedMode = 0x01;
                  JSmode[0] = JcSpeedMode;
                  JSmode[1] = 0x04;
              }
              else if(JcSpeedMode == 0x02)
              {
                  JcSpeedMode = 0x02;
                  JSmode[0] = JcSpeedMode;
                  JSmode[1] = 0x02;
              }
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //���͸����  
              break;
        case 7:
              //����
              JSmode[1] = 0x05;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //���͸����  
              break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            PraperMcDebugMainMenu();
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_81
*Description	: CSУ��
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_81(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            //DoNothings 
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_82
*Description    : ������Բ˵�
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_82(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //DoNothings           
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            PraperMcDebugMainMenu();
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_83
*Description	: �����ӳ�AD
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_83(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //DoNothings           
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            PraperMcDebugMainMenu();
            break;
        default:
          break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_84
*Description	        : �����˵�
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	�γ�
***********************************************************************************************/
void KeyEvent_84(INT8U *keyval)
{
    INT8U UILevelUp =0x01;
    INT8U DSPLevelUp =0x02;
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
              //UI ARM����
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
              Flashsend_message.DivNum = 2;
              Flashsend_message.pData = &DSPLevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message); //���͸�SD��
              break;
        case 2:
              //DSP����
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
              Flashsend_message.DivNum = 1;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message); //���͸�SD��
              break;
        case 3:
              //�����ģ������
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
              Flashsend_message.DivNum = 4;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message); //���͸�SD��
              break;
        case 4:
              //FPGA����
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
              Flashsend_message.DivNum = 3;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message);//���͸�SD��
              break;
        case 5:
              //���ARM����
              PraperLevelUpING();
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANLEVELUPSTART;
              OSQPost (canEvent,&Dsend_message);
              OSTimeDlyHMSM(0,0,0,20);
              Flashsend_message.MsgID = APP_COMFROM_UI;//���ɹ������ļ���С��Ϣ���ݹ�ȥ
              Flashsend_message.DivNum = 5;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message);//���͸�SD��
              break;
        case 6:
        case 7:
            //DoNothings           
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;

    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_87
*Description	�� �߳���Ϣ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 31/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_87(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
      
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //��ҳ
            DisplayCommonMenu(&gsMenu_88,0);
            PraperInfoMTADMenu();
            InfoMTADDispMenu();
            InfoADpage = 1;
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            PraperMcDebugMainMenu();
            break;

    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_88
*Description	�� �߳�MT��Ϣ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 31/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_88(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //��ҳ
            DisplayCommonMenu(&gsMenu_89,0);
            PraperInfoMGADMenu();
            InfoMGADDispMenu();
            InfoADpage = 2;
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            PraperMcDebugMainMenu();
            break;

    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_89
*Description	�� �߳�MG��Ϣ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 31/1/2015	�γ�
***********************************************************************************************/
void KeyEvent_89(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //��ҳ
            DisplayCommonMenu(&gsMenu_87,0);
            PraperInfoIRADMenu();
            InfoIRADDispMenu();
            InfoADpage = 0;
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            PraperMcDebugMainMenu();
            break;

    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_90
*Description	�� �ϻ�����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 6/2/2015	�γ�
***********************************************************************************************/
void KeyEvent_90(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //DONothings
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;

    }
    return;
}

/***********************************************************************************************
* Function	: KeyEvent_91
*Description	:MT������ʾ
* Input		: 
* Output		: 
* Note(s)		: 
* Contributor	: 4/3/2015	�γ�
***********************************************************************************************/
void KeyEvent_91(INT8U *keyval)
{
    INT8U MTContrl[3] = {0};
    
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //DONothings
            break;
        case 8:
            /* �Ѿ�����ײ�˵���ֱ�ӷ��ػ�ִ������������ȡ����function */
            //�˳�����ģʽ
            MTContrl[0] = MTStatus;
            MTContrl[1] = MainMenuStatus;
            MTContrl[2] = MCDEBUGOUTMODE;
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANMOTORSET;
            Dsend_message.DataLen = 3;
            Dsend_message.pData = MTContrl;
            OSQPost (canEvent,&Dsend_message);
            OSTimeDlyHMSM(0,0,0,20);
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
    }
    return;
}


/************************(C)COPYRIGHT 2010 �㽭��̩****END OF FILE****************************/
