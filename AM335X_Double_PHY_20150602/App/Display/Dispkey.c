/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: key.c
* Author			: 王耀
* Date First Issued	: 02/20/2014 
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
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
//批量的临时保存的值，等用户确定以后保存到全局变量BatchNum中
INT16U TemBatchNum = 0;
//出钞口查看到的当前的页码
INT8U OutLetKeyPage = 0;
//进钞口查看到的当前的页码
INT8U InLetKeyPage = 0;
//通过CAN给马达板发送信息的数组
extern INT8U CanTxBuff[8];
//批量值
extern INT16U BatchNum;
//查看鉴伪参数的坐标
extern tRectangle BaseCheckParam[10][10];
//修改鉴伪参数的坐标
extern tRectangle BaseEditParam[10][12];
//鉴伪参数的背景色取得的坐标
extern tRectangle BaseEditParamColor[4];
//鉴伪参数的横坐标
extern INT8U Paramvertical;
//鉴伪参数的纵坐标
extern INT8U Paramhorizontal;
//鉴伪参数修改的当前页码
extern INT8U EditParamPage;
//鉴伪参数的修改用到的数组
extern INT8U AuthenticationDataSettings[3][90];
//鉴伪参数暂存变量
extern INT8U TempAuthenticationDataSettings[3][90];
//出钞口和进钞口的明细查看的总页数
extern INT8U OutLetTotalPage;
extern INT8U InLetTotalPage;
//清分等级
extern INT8U ClearLevel[6];
//键盘光标显示的横坐标
extern INT8U KeyBoardvertical;
//键盘光标显示的纵坐标
extern INT8U KeyBoardhorizontal;
//信息查看的页码
extern INT8U InfomationList;
//信息查看当前选中第几条
extern INT8U InfomationPage;

//清分等级的临时保存的值
INT8U TemClearLevel[6];
//清分等级的纵坐标值（用来定位光标）
INT8U ClearLevelhorizontal = 0;
//鉴伪等级
extern INT8U AuthenticationLevel[7];
//鉴伪等级的临时保存的值
INT8U TemAuthenticationLevel[7] = {0};
//鉴伪等级的纵坐标值（用来定位光标）
INT8U AuthenticationLevelhorizontal = 0;
//存储密码的数组
INT8U Password[5] = {0};
//密码现在选中的第几位
INT8U PasswordCursor = 0;
//累加模式
extern INT8U SUMMATIONMODE;
//存取款模式
extern INT8U DEPWITHDRAWMODE;
//清分模式
extern INT8U CLEARINGMODE;
//发送给CAN的消息的内容
INT8U UIA_Send[8] = {0};
//发送给CAN的消息
extern _BSP_MESSAGE Dsend_message; 
extern _BSP_MESSAGE Flashsend_message; 
//黑名单
extern INT8U BlackList[200][12];
//走钞信息查看
extern INT8U InfoADpage;
//鉴伪信息页码计数
extern INT8U AuthenticationDataPages;
//操作员签到
INT8U SignIn[11] = {0};
INT8U TempSignIn[11] = {0};
INT8U SignInNUM = 0;
//交易号输入
INT8U AnsactionNum[11] = {0};
INT8U TempAnsactionNum[11] = {0};
INT8U AnsactionNumNUM = 0;
//网发时间
INT8U NetPostTime[6] = {0};
INT8U TempNetPostTime[6] = {0};
INT8U NetPostTimeNUM = 0;
//网发张数
INT8U NetPostPage[2] = {0};
INT8U TempNetPostPage[2] = {0};
INT8U NetPostPageNUM = 0;
//银行简称
INT8U BlankName[16] = {0};
INT8U TempBlankName[16] = {0};
INT8U BlankNameNUM = 0;
//地区号
INT8U AreaNum[16] = {0};
INT8U TempAreaNum[16] = {0};
INT8U AreaNumNUM = 0;
//支行号
INT8U BranchNum[16] = {0};
INT8U TempBranchNum[16] = {0};
INT8U BranchNumNUM = 0;
//网点号
INT8U LatticePoint[11] = {0};
INT8U TempLatticePoint[11] = {0};
INT8U LatticePointNUM = 0;
//机器号序列号
INT8U MachineNo[16] = {0};
INT8U TempMachineNo[16] = {0};
INT8U MachineNoNUM = 0;
//IP输入
INT8U IP[13] = {0};
INT8U TempIP[13] = {0};
INT8U IPNUM = 0;
//MASK输入
INT8U Mask[13] = {0};
INT8U TempMask[13] = {0};
INT8U MaskNUM = 0;
//GW输入
INT8U GateWay[13] = {0};
INT8U TempGateWay[13] = {0};
INT8U GateWayNUM = 0;
//本地端口输入
INT8U LocalPort[5] = {0};
INT8U TempLocalPort[5] = {0};
INT8U LocalPortNUM = 0;
//远端口输入
INT8U DiskPort[5] = {0};
INT8U TempDiskPort[5] = {0};
INT8U DiskPortNUM = 0;
//远端IP
INT8U DiskIP[13] = {0};
INT8U TempDiskIP[13] = {0};
INT8U DiskIPNUM = 0;
//禁止网发时间
INT8U BanTechnician[3][16] = {0};
INT8U TempBanTechnician[3][16] = {0};
INT8U BanTechnicianNUM = 0;
INT8U BanTechnicianPage = 0;
//MAC
INT8U Mac[13] = {0};
//时间
INT8U Time[6] = {0};
INT8U TempTime[6] = {0};
INT8U TimeNUM = 0;
//黑名单输入
INT8U TempBlackLists[13] = {0};
INT8U BlackLists[2][7][13] = {0};
INT8U BlackListPage = 0;
INT8U BlackListhorizontal = 0;
INT8U BlackListKeyNum = 0;

INT8U JcSpeedMode = 0x01;
//是否修改过参数
INT8U EditParmMode = 0x00;

extern INT8U KeyBoardMAP[4][12];
extern tRectangle BaseSignInCoordinate[16];
//马达开停机标志
extern INT8U MTStatus;
//菜单标志
extern INT8U MainMenuStatus;
//校验标示
extern INT8U CSCheckFlag;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: DealWithKey
* Description	: 按键动作
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
* Description	: 主菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
***********************************************************************************************/
void KeyEvent_0(INT8U *keyval)
{    
    static INT8U MTContrl[3] = {0};
    static INT8U MCmode[3] = {0};
    switch(*keyval)
    {
        case 1:
              //进去菜单
              //发送进入菜单的指令给马达ARM
              MainMenuStatus = OUTMAINMENU;
              MTContrl[0] = MTStatus;
              MTContrl[1] = MainMenuStatus;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANMOTORSET;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = MTContrl;
              OSQPost (canEvent,&Dsend_message); 
              OSTimeDlyHMSM(0,0,0,10);
              //显示菜单的界面
              gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[7];
              printBackImage(gps_CurMenu->FrameID);
              break;
        case 2:
              //币种
              //发送进入菜单的指令给马达ARM
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
              //批量
              //发送进入菜单的指令给马达ARM
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
              //等级
              //发送进入菜单的指令给马达ARM
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
              //模式
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
              //累加
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
              //查询
              //发送进入菜单的指令给马达ARM
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
                  //显示混点明细菜单
                  Dsend_message.MsgID = APP_COMFROM_UI;
                  Dsend_message.DivNum = UIACANMIXEDSEARCH;
                  OSQPost (canEvent,&Dsend_message);
                  OSTimeDlyHMSM(0,0,0,10);
                  gps_CurMenu = &gsMenu0_3;
                  printBackImage(gps_CurMenu->FrameID);
              }
              else 
              {
                  //显示退钞口冠子号码查看菜单
//                  Dsend_message.MsgID = APP_COMFROM_UI;
//                  Dsend_message.DivNum = UIACANOUTLETSEARCH;
//                  OSQPost (canEvent,&Dsend_message);
//                  OSTimeDlyHMSM(0,0,0,10);
//                  gps_CurMenu = &gsMenu0_4;
//                  printBackImage(gps_CurMenu->FrameID);
                    gps_CurMenu = &gsMenu0_4;
                    printBackImage(gps_CurMenu->FrameID);
                    //刷新退钞口查看菜单
                    OutLetDetailDisp();

              }
              break;
          case 8:
              //DoNothing
              break;
          case 11:
              //存款
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
              //取款
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
              //马达停机
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
              //暂用于马达紧急停机
              break;
        default:
              break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_1
* Description	: 币种菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
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
* Description	: 批量菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
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
            //将批量值发送给MC
            BatchNum = TemBatchNum;
            batch[0] = BatchNum>>8;    
            batch[1] = BatchNum&0xFF;    
            Dsend_message.MsgID = APP_COMFROM_UI;
            Dsend_message.DivNum = UIACANBATCH;
            Dsend_message.DataLen = 2;
            Dsend_message.pData = batch;
            OSQPost (canEvent,&Dsend_message); //发送给马达板
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
* Description	: 混点明细菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
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
            //返回退钞口冠子号码查看菜单
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
* Description	: 退币口冠字号码查看菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
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
            //返回接钞口查看的帧发送给MC
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
* Description	: 接钞口冠字号码查看菜单的按键动作
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
                //发送请求混点明细数据的帧给MC
                InLetKeyPage = 0;
                Dsend_message.MsgID = APP_COMFROM_UI;
                Dsend_message.DivNum = UIACANMIXEDSEARCH;
                OSQPost (canEvent,&Dsend_message);
                //返回混点明细菜单
                gps_CurMenu = &gsMenu0_3;
                printBackImage(gps_CurMenu->FrameID);
            }
            else
            {
                InLetKeyPage = 0;
                //返回退钞口查看的帧发送给MC
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
* Description	: 清分等级菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
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
            //翻页
            DisplayCommonMenu(&gsMenu0_7,0);
            memcpy(TemAuthenticationLevel,AuthenticationLevel,7);
            AuthenticationLevelDisp();
            DisplayClearLevelmark();
            return;
        case 5:
            //Do Nothing
            return;
        case 6:
            //保存
            CleanleveltoFlash[0] = 0x10;    //存储清分等级的ID号（高位）
            CleanleveltoFlash[1] = 0x02;    //存储清分等级的ID号（低位）
            memcpy(ClearLevel,TemClearLevel,6);
            memcpy(CleanleveltoFlash+2,ClearLevel,6);
            Flashsend_message.MsgID = APP_COMFROM_UI;
            Flashsend_message.DataLen = 8;
            Flashsend_message.pData = CleanleveltoFlash;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储            
            //发送清分等级到马达ARM
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
* Description	: 鉴伪等级菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
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
            //保存
            AuthenticationLeveltoFlash[0] = 0x10;    //存储鉴伪等级的ID号（高位）
            AuthenticationLeveltoFlash[1] = 0x03;    //存储鉴伪等级的ID号（低位）
            memcpy(AuthenticationLevel,TemAuthenticationLevel,7);
            memcpy(AuthenticationLeveltoFlash+2,AuthenticationLevel,7);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 9;
            Flashsend_message.pData = AuthenticationLeveltoFlash;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            //发送鉴伪等级到马达ARM
            Dsend_message.MsgID = APP_COMFROM_UI;//发送第一帧
            Dsend_message.DivNum = UIACANAUTHENTICLEVEL1;
            Dsend_message.DataLen = 7;
            Dsend_message.pData = AuthenticationLevel;
            OSQPost (canEvent,&Dsend_message); 
            OSTimeDlyHMSM(0,0,0,10);//延时，一定要
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
* Description	: 菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
***********************************************************************************************/
void KeyEvent_8(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
            //操作员签到
            //操作员签到
            memcpy(TempSignIn,SignIn,10);
            DisplayCommonMenu(&gsMenu_14,0);
            KeyBoardDispSignInNum(SignIn);
            DisplayKeyBoard();
            break;
        case 2:
            //操作员签退
            //清除
            memset(TempSignIn,0,11);
            DisplayCommonMenu(&gsMenu_15,0);
            break;
        case 3:
            //交易号输入
            memset(AnsactionNum,0,11);
            DisplayCommonMenu(&gsMenu_16,0);
            DisplayKeyBoard();
            break;
        case 4:
            //用户信息
            DisplayCommonMenu(&gsMenu_17,0);
            DisplayInfomationList();
            break;
        case 5:
            //黑名单设置
            DisplayCommonMenu(&gsMenu_23,0);
            DisplayCheckmark();
            blackListDisp();
            PraperBlackListPage();
            break;
        case 6:
            //数据查看
            gps_CurMenu = gps_CurMenu->Menu_paKidsMenu[5];
            printBackImage(gps_CurMenu->FrameID);
            DisplayParaMenu();
            break;
        case 7:
            //调试维护
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
*Description	: 查看参数菜单的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20/12/2014	宋超
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
              //查看菜单翻页函数
              break;
        case 8:
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 参数修改的按键动作
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2/2/2015	宋超
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
            //光标处理
            break;
        case 5:
            //参数加1
            DealWithParm(1); 
            EditParmMode = 0x01;
            break;
        case 6:
            //参数减1
			DealWithParm(0);
            EditParmMode = 0x01;
            break;
        case 7:
            if(EditParmMode==0)
            {
                //查看菜单翻页函数
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
                //撤销修改的参数
                gps_CurMenu = gps_CurMenu->Menu_pFather;
                printBackImage(gps_CurMenu->FrameID);
                EditParmMode = 0x00;
            }
            break;
        case 8:
            //清光标
            ClearCursor();
            //清除撤销键，使能翻页键
            EditParmMode = 0x00;
            Dsend_message.MsgID = APP_COMFROM_UI;
            //鉴伪参数发送给CAN任务
            Dsend_message.DivNum = UIACANAUTHENTICATION;  
            Dsend_message.DataLen = 270;
            Dsend_message.pData = (INT8U *)TempAuthenticationDataSettings;
            OSQPost (canEvent,&Dsend_message);   
            OSTimeDly(SYS_DELAY_50ms);
            //鉴伪参数发送给SPIFLASH保存
            AuthenticationParam[0] = 0x10;    //参数修改的ID号（高位）
            AuthenticationParam[1] = 0x14;    //参数修改的ID号（低位）
            memcpy(AuthenticationParam+2,TempAuthenticationDataSettings,270);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 272;
            Flashsend_message.pData = AuthenticationParam;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            OSTimeDly(SYS_DELAY_250ms);
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 操作员签到
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempSignIn,0,11);
            SignInNUM = 0;
            while(i<10)
            {
                printStrings32(BaseSignInCoordinate[i],&TempSignIn[i]);
                i++;
            }
            break;
        case 7:
            //保存
            SignIntoNand[0] = 0x10;    //操作员签到，暂时存储到FLASH中
            SignIntoNand[1] = 0x55;    //操作员签到，暂时不用
            memcpy(SignIn,TempSignIn,10);
            memcpy(SignIntoNand+2,SignIn,10);
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //返回
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
*Description	: 操作员签退
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
            //签退
            memset(SignIn,0,11);
            SignInNUM = 0;
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //返回
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 输入交易号
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempAnsactionNum,0,11);
            AnsactionNumNUM = 0;
            while(i<10)
            {
                printStrings32(BaseSignInCoordinate[i],&TempAnsactionNum[i]);
                i++;
            }
            break;
        case 7:
            //保存
            memcpy(AnsactionNum,TempAnsactionNum,10);
            memset(TempAnsactionNum,0,11);
            AnsactionNumNUM = 0;
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //返回
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
*Description	: 信息查看1
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/12/2014	宋超
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
            //消除上一个光标
            CleanInfomationList();
            //向下一条,光标显示
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
            //翻页
            //消除上一个光标
            CleanInfomationList();
            InfomationPage = 1;
            InfomationList = 0;
            DisplayCommonMenu(&gsMenu_18,0);
            DisplayInfomationList();
            break;
        case 5:
            //设置
            //消除上一个光标
            CleanInfomationList();
            switch(InfomationList)
            {
                case 0x00:  
                    //网发时间
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperNetPostTime();
                    break;
                case 0x01:  
                    //网发张数
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperNetPostPages();
                    break;
                case 0x02:  
                    //银行简称
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBlankName();
                    break;
                case 0x03:  
                    //地区号
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperAreaCode();
                    break;
                case 0x04:  
                    //支行号
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBranchNumber();
                    break;
                case 0x05:  
                    //网点号
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
            //返回
            CleanInfomationList();
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 信息查看2
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	宋超
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
            //消除上一个光标
            CleanInfomationList();
            //向下一条,光标显示
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
            //翻页
            //消除上一个光标
            CleanInfomationList();
            InfomationPage = 2;
            InfomationList = 0;
            DisplayCommonMenu(&gsMenu_24,0);
            PraperInformation3Page();
            DisplayInfomationList();
            break;
        case 5:
            //设置
            //消除上一个光标
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
                    //机器号序列号
                    DisplayCommonMenu(&gsMenu_25,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
                    break;
                case 0x02:  
                    //禁止网发1
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBanTechnician1();
                    break;
                case 0x03:  
                    //禁止网发2
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBanTechnician2();
                    break;
                case 0x04:  
                    //禁止网发3
                    DisplayCommonMenu(&gsMenu_19,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
					PraperBanTechnician3();
                    break;
                case 0x05:  
                    //时间
                    DisplayCommonMenu(&gsMenu_20,0);
                    KeyBoardDispNum(LatticePoint);
                    DisplayKeyBoard();
                    break;
                default:
                    //时间
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
            //返回
            CleanInfomationList();
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 网点号输入
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempLatticePoint,0,11);
            LatticePointNUM = 0;
            while(i<10)
            {
                printStrings32(BaseSignInCoordinate[i],&TempLatticePoint[i]);
                i++;
            }
            break;
        case 7:
            //保存
            memcpy(LatticePoint,TempLatticePoint,10);
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;
        case 8:
            //返回
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
*Description	: 时间设置
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2014	宋超
***********************************************************************************************/
void KeyEvent_20(INT8U *keyval)
{
    *keyval&=0x0f;
    if(*keyval>9)
            return;
    
    switch(*keyval)
    {
        case 1:
            //光标右移
            break;
        case 2:
            //光标左移
            break;
        case 3:
            //光标下移
            break;
        case 4:
            //翻页
            break;
        case 5:
            //加一
            break;
        case 6:
            //减一
            break;
        case 7:
            //保存
            break;
        case 8:
            //返回
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 版本信息1
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
            //翻页到版本信息2
            DisplayCommonMenu(&gsMenu_22,0);
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            //返回
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 版本信息2
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
            //翻页到版本信息1
            DisplayCommonMenu(&gsMenu_21,0);
            VersionDisp();
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            //返回
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 黑名单设置
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
            //添加
            /* 按了设置键以后迁移到黑名单入力菜单 */
            DisplayCommonMenu(&gsMenu_28,0);
            DisplayKeyBoard();
            PraperBlackList();
            KeyBoardDispNum(BlackLists[BlackListPage][BlackListhorizontal]);
            break;
        case 6:
            //删除
            DeleteBlack();
            break;
        case 7:
            //修改
            /* 按了设置键以后迁移到黑名单入力菜单 */
            DisplayCommonMenu(&gsMenu_28,0);
            DisplayKeyBoard();
            PraperBlackList();
            break;
        case 8:
            //返回,存到了FLASH中
            FlashBlcakList[0] = 0x10;    //存储远端端口的ID号（高位）
            FlashBlcakList[1] = 0x15;    //存储远端端口的ID号（低位）
            memcpy(FlashBlcakList+2,BlackLists,200);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 202;
            Flashsend_message.pData = FlashBlcakList;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
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
*Description	: 信息查看3
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
			//消除上一个光标
			CleanInfomationList();
			//向下一条,光标显示
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
			//翻页
			//消除上一个光标
			CleanInfomationList();
			InfomationPage = 0;
			InfomationList = 0;
			DisplayCommonMenu(&gsMenu_17,0);
			DisplayInfomationList();
			break;
		case 5:
			//设置
			//消除上一个光标
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
					//本地端口
					DisplayCommonMenu(&gsMenu_53,0);
					KeyBoardDispNum(LocalPort);
					DisplayKeyBoard();
					PraperLocalPort();
					break;
				case 0x03:	
					//远端端口
					DisplayCommonMenu(&gsMenu_54,0);
					KeyBoardDispNum(DiskPort);
					DisplayKeyBoard();
					PraperDistalPort();
					break;
				case 0x04:	
					//远端IP
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
            //消除上一个光标
			CleanInfomationList();
			//返回
			/* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 机器号
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 20/12/2014	宋超
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
            //添加
            /* 按了设置键以后迁移到黑名单入力菜单 */
            DisplayCommonMenu(&gsMenu_14,0);
            DisplayKeyBoard();
            break;
        case 6:
            //删除
            DeleteBlack();
            break;
        case 7:
            //修改
            /* 按了设置键以后迁移到黑名单入力菜单 */
            DisplayCommonMenu(&gsMenu_14,0);
            DisplayKeyBoard();
            break;
        case 8:
            //返回
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
* Contributor	: 20/12/2014	宋超
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
            //返回
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
*Description	: 开机出错
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	宋超
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
            //当Page大于1页的时候可以翻页
            break;
        case 7:
            //翻页当错误信息大于10条的时候翻页

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
*Description	: 黑名单输入
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 12/2/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempBlackLists,0,12);
            BlackListKeyNum = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskPort[i]);
                i++;
            }
            break;
        case 7:
            //保存          
            memcpy(BlackLists[BlackListPage][BlackListhorizontal],TempBlackLists,12);
            memset(TempBlackLists,0,12);
            BlackListKeyNum = 0;
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayCheckmark();
            blackListDisp();
            PraperBlackListPage();
            break;
        case 8:
            //返回
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
*Description	: 参数调试菜单
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 15/1/2015	宋超
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
              //功能设置 暂时用来传大图
              pMsg.MsgID = SEND_REQ | SUB_ID_BIG_IMAGE;				
              pMsg.DivNum = DSP2;
              SYSPost(W5500BEvent,&pMsg);
              break;
        case 2:
              //参数设置 
              DisplayCommonMenu(&gsMenu0_12,0);
              PraperAuthenticationDataSettings();
              DisplayEditParaMenu();
              memcpy(TempAuthenticationDataSettings,AuthenticationDataSettings,270);
              break;
        case 3:    
              //马达调试主菜单
              DisplayCommonMenu(&gsMenu_79,0);
              PraperMcDebugMainMenu();
              break;
        case 4:
              //发送进入老化
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
              //版本信息
              DisplayCommonMenu(&gsMenu_21,0);
              VersionDisp();
              break;
        case 6:
              //升级
              DisplayCommonMenu(&gsMenu_84,0);
              PraperLevelUp();
              break;
        case 7:
              //波形显示
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
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 密码查看菜单
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 06/12/2014	宋超
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
            //输入密码
            Password[PasswordCursor] = *keyval;
            if(PasswordCursor<4)PasswordCursor++;
            DisplayPasswordMenu(PasswordCursor);
            break;
        case 8:
            //比较密码
            PasswordCursor = 0;
            if(memcmp(Password,adminPassword,4))
            {
                //密码出错
                memset(Password,0,4);
                PraperPasswordError();
                OSTimeDlyHMSM(0,0,1,0);
                //返回上一级菜单
                gps_CurMenu = gps_CurMenu->Menu_pFather;
                printBackImage(gps_CurMenu->FrameID);
            }
            else
            {
                //密码正确
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
* Contributor	: 15/1/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempIP,0,12);
            IPNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempIP[i]);
                i++;
            }
            break;
        case 7:
            //保存
            FlashIP[0] = 0x10;    //存储IP的ID号（高位）
            FlashIP[1] = 0x04;    //存储IP的ID号（低位）
            memcpy(IP,TempIP,12);
            IPChartoNum(FlashIP,IP);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashIP;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            break;
        case 8:
            //返回
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
* Contributor	: 15/1/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempMask,0,12);
            MaskNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempMask[i]);
                i++;
            }
            break;
        case 7:
            //保存
            FlashMask[0] = 0x10;    //存储MASK的ID号（高位）
            FlashMask[1] = 0x05;    //存储MASK的ID号（低位）
            memcpy(Mask,TempMask,12);
            IPChartoNum(FlashMask,Mask);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashMask;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //返回
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
* Contributor	: 15/1/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempGateWay,0,12);
            GateWayNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempGateWay[i]);
                i++;
            }
            break;
        case 7:
            //保存
            FlashGw[0] = 0x10;    //存储GW的ID号（高位）
            FlashGw[1] = 0x06;    //存储GW的ID号（低位）
            memcpy(GateWay,TempGateWay,12);
            IPChartoNum(FlashGw,GateWay);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashGw;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //返回
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
*Description	: 本地端口
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempLocalPort,0,4);
            LocalPortNUM = 0;
            while(i<4)
            {
                printStrings32(BaseSignInCoordinate[i],&TempLocalPort[i]);
                i++;
            }
            break;
        case 7:
            //保存
            FlashLocalPort[0] = 0x10;    //存储本地端口的ID号（高位）
            FlashLocalPort[1] = 0x08;    //存储本地端口的ID号（低位）
            memcpy(LocalPort,TempLocalPort,4);
            PortChartoNum(FlashLocalPort,LocalPort);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 4;
            Flashsend_message.pData = FlashLocalPort;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //返回
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
*Description	: 远端端口
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempDiskPort,0,4);
            DiskPortNUM = 0;
            while(i<4)
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskPort[i]);
                i++;
            }
            break;
        case 7:
            //保存
            FlashDiskPort[0] = 0x10;    //存储远端端口的ID号（高位）
            FlashDiskPort[1] = 0x0A;    //存储远端端口的ID号（低位）
            memcpy(DiskPort,TempDiskPort,4);
            PortChartoNum(FlashDiskPort,DiskPort);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 4;
            Flashsend_message.pData = FlashDiskPort;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //返回
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
*Description	: 远端IP
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	宋超
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
            //按键处理
            DealKeyBoard(*keyval);
            break;
        case 5:
            //选取
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
            //清除
            memset(TempDiskIP,0,12);
            DiskIPNUM = 0;
            while(i<12)
            {
                printStrings32(BaseSignInCoordinate[i],&TempDiskIP[i]);
                i++;
            }
            break;
        case 7:
            //保存
            FlashDiskIP[0] = 0x10;    //存储远端IP的ID号（高位）
            FlashDiskIP[1] = 0x09;    //存储远端IP的ID号（低位）
            memcpy(DiskIP,TempDiskIP,12);
            IPChartoNum(FlashDiskIP,DiskIP);
            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
            Flashsend_message.DataLen = 6;
            Flashsend_message.pData = FlashDiskIP;
            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            DisplayInfomationList();
            PraperInformation3Page();
            break;
        case 8:
            //返回
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
*Description	: 禁止网发时段
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 15/1/2015	宋超
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
//            //按键处理
//            DealKeyBoard(*keyval);
//            break;
//        case 5:
//            //选取
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
//            //清除
//            memset(TempBanTechnician[BanTechnicianPage],0,16);
//            BanTechnicianNUM = 0;
//            while(i<15)
//            {
//                printStrings32(BaseSignInCoordinate[i],&TempBanTechnician[BanTechnicianPage][i]);
//                i++;
//            }
//            break;
//        case 7:
//            //保存
//            memcpy(BanTechnician,TempBanTechnician,10);
////            Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
////            Flashsend_message.DataLen = 12;
////            Flashsend_message.pData = SignIntoNand;
////            OSQPost (FlashEvent,&Flashsend_message); //发送给FLASH进行存储
//            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
//            gps_CurMenu = gps_CurMenu->Menu_pFather;
//            printBackImage(gps_CurMenu->FrameID);
//            break;
//        case 8:
//            //返回
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
*Description	: 调试
* Input		: 
* Output		: 
* Note(s)		: 
* Contributor  : 31/1/2015	宋超
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
              //电机调试
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
              //进钞接钞AD              
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
			  //红外AD管
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
              //大小磁头AD              
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
              //荧光AD
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
              //走钞信息
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
              //CS检验
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
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
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
*Description	: 速度调试菜单
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor   	: 06/12/2014	宋超
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
              //选中进钞模式
              PraperSpeedSetMenu();
              PraperSpeedSetMenuINMode();
              JcSpeedMode = 0x01;
              JSmode[0] = JcSpeedMode;           
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 1;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //发送给马达 
              break;
        case 2:
              //清分模式
              PraperSpeedSetMenu();
              PraperSpeedSetMenuOutMode();
              JcSpeedMode = 0x02;
              JSmode[0] = JcSpeedMode;           
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 1;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //发送给马达 
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
              OSQPost (canEvent,&Dsend_message); //发送给马达 
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
              OSQPost (canEvent,&Dsend_message); //发送给马达  
              break;
        case 7:
              //保存
              JSmode[1] = 0x05;
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANJCSPEED;
              Dsend_message.DataLen = 2;
              Dsend_message.pData = JSmode;
              OSQPost (canEvent,&Dsend_message); //发送给马达  
              break;
        case 8:
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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
*Description	: CS校验
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	宋超
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
*Description    : 红外调试菜单
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	宋超
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
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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
*Description	: 进钞接钞AD
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	宋超
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
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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
*Description	        : 升级菜单
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	        : 06/12/2014	宋超
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
              //UI ARM升级
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
              Flashsend_message.DivNum = 2;
              Flashsend_message.pData = &DSPLevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message); //发送给SD卡
              break;
        case 2:
              //DSP升级
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
              Flashsend_message.DivNum = 1;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message); //发送给SD卡
              break;
        case 3:
              //人民币模板升级
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
              Flashsend_message.DivNum = 4;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message); //发送给SD卡
              break;
        case 4:
              //FPGA升级
              PraperLevelUpING();
              Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
              Flashsend_message.DivNum = 3;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message);//发送给SD卡
              break;
        case 5:
              //马达ARM升级
              PraperLevelUpING();
              Dsend_message.MsgID = APP_COMFROM_UI;
              Dsend_message.DivNum = UIACANLEVELUPSTART;
              OSQPost (canEvent,&Dsend_message);
              OSTimeDlyHMSM(0,0,0,20);
              Flashsend_message.MsgID = APP_COMFROM_UI;//读成功，把文件大小信息传递过去
              Flashsend_message.DivNum = 5;
              Flashsend_message.pData = &UILevelUp;
              OSQPost (MMCSDEvent,&Flashsend_message);//发送给SD卡
              break;
        case 6:
        case 7:
            //DoNothings           
            break;
        case 8:
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            gps_CurMenu = gps_CurMenu->Menu_pFather;
            printBackImage(gps_CurMenu->FrameID);
            break;

    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent_87
*Description	： 走钞信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 31/1/2015	宋超
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
            //翻页
            DisplayCommonMenu(&gsMenu_88,0);
            PraperInfoMTADMenu();
            InfoMTADDispMenu();
            InfoADpage = 1;
            break;
        case 8:
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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
*Description	： 走钞MT信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 31/1/2015	宋超
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
            //翻页
            DisplayCommonMenu(&gsMenu_89,0);
            PraperInfoMGADMenu();
            InfoMGADDispMenu();
            InfoADpage = 2;
            break;
        case 8:
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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
*Description	： 走钞MG信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 31/1/2015	宋超
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
            //翻页
            DisplayCommonMenu(&gsMenu_87,0);
            PraperInfoIRADMenu();
            InfoIRADDispMenu();
            InfoADpage = 0;
            break;
        case 8:
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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
*Description	： 老化测试
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 6/2/2015	宋超
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
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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
*Description	:MT数据显示
* Input		: 
* Output		: 
* Note(s)		: 
* Contributor	: 4/3/2015	宋超
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
            /* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
            //退出调试模式
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


/************************(C)COPYRIGHT 2010 浙江方泰****END OF FILE****************************/
