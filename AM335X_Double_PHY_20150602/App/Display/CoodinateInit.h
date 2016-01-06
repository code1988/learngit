/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.c
* Author			: 
* Date First Issued	: 130722    
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
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
/* Private define-----------------------------------------------------------------------------*/
#define UNVAULE 0x00
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//主菜单总张数的艺术字
#define TOTALNUM01 BaseMainMenuNumCoordinate[1][5]
#define TOTALNUM06 BaseMainMenuNumCoordinate[1][0]
#define TOTALNUM07 BaseMainMenuNumCoordinate[1][1]
#define TOTALNUM08 BaseMainMenuNumCoordinate[1][2]
#define TOTALNUM09 BaseMainMenuNumCoordinate[1][3]
#define TOTALNUM00 BaseMainMenuNumCoordinate[1][4]
#define TOTALNUM02 BaseMainMenuNumCoordinate[1][6]
#define TOTALNUM03 BaseMainMenuNumCoordinate[1][7]
#define TOTALNUM04 BaseMainMenuNumCoordinate[1][8]
#define TOTALNUM05 BaseMainMenuNumCoordinate[1][9]

//主菜单总张数显示的坐标
#define TOTALNUMCOORDINATE01 BaseTotalPage[0]
#define TOTALNUMCOORDINATE02 BaseTotalPage[1]
#define TOTALNUMCOORDINATE03 BaseTotalPage[2]
#define TOTALNUMCOORDINATE04 BaseTotalPage[3]

//主菜单总金额的艺术字
#define TOTALSUM00 BaseMainMenuNumCoordinate[2][0]
#define TOTALSUM01 BaseMainMenuNumCoordinate[2][1]
#define TOTALSUM02 BaseMainMenuNumCoordinate[2][2]
#define TOTALSUM03 BaseMainMenuNumCoordinate[2][3]
#define TOTALSUM04 BaseMainMenuNumCoordinate[2][4]
#define TOTALSUM05 BaseMainMenuNumCoordinate[2][5]
#define TOTALSUM06 BaseMainMenuNumCoordinate[2][6]
#define TOTALSUM07 BaseMainMenuNumCoordinate[2][7]
#define TOTALSUM08 BaseMainMenuNumCoordinate[2][8]
#define TOTALSUM09 BaseMainMenuNumCoordinate[2][9]

//主菜单总金额显示的坐标
#define TOTALSUMCOORDINATE01 BaseTotalSum[0]
#define TOTALSUMCOORDINATE02 BaseTotalSum[1]
#define TOTALSUMCOORDINATE03 BaseTotalSum[2]
#define TOTALSUMCOORDINATE04 BaseTotalSum[3]
#define TOTALSUMCOORDINATE05 BaseTotalSum[4]
#define TOTALSUMCOORDINATE06 BaseTotalSum[5]
#define TOTALSUMCOORDINATE07 BaseTotalSum[6]


//主菜单退钞口的艺术字
#define TOTALOUTLET00 BaseMainMenuNumCoordinate[0][0]
#define TOTALOUTLET01 BaseMainMenuNumCoordinate[0][1]
#define TOTALOUTLET02 BaseMainMenuNumCoordinate[0][2]
#define TOTALOUTLET03 BaseMainMenuNumCoordinate[0][3]
#define TOTALOUTLET04 BaseMainMenuNumCoordinate[0][4]
#define TOTALOUTLET05 BaseMainMenuNumCoordinate[0][5]
#define TOTALOUTLET06 BaseMainMenuNumCoordinate[0][6]
#define TOTALOUTLET07 BaseMainMenuNumCoordinate[0][7]
#define TOTALOUTLET08 BaseMainMenuNumCoordinate[0][8]
#define TOTALOUTLET09 BaseMainMenuNumCoordinate[0][9]

//主菜单退钞口显示的坐标
#define TOTALOUTLETCOORDINATE01 BaseOutLetCoordinate[0][0]
#define TOTALOUTLETCOORDINATE02 BaseOutLetCoordinate[0][1]
#define TOTALOUTLETCOORDINATE03 BaseOutLetCoordinate[0][2]
#define TOTALINLETCOORDINATE01 BaseOutLetCoordinate[1][0]
#define TOTALINLETCOORDINATE02 BaseOutLetCoordinate[1][1]
#define TOTALINLETCOORDINATE03 BaseOutLetCoordinate[1][2]
#define TOTALBATCHCOORDINATE01 BaseOutLetCoordinate[2][0]
#define TOTALBATCHCOORDINATE02 BaseOutLetCoordinate[2][1]
#define TOTALBATCHCOORDINATE03 BaseOutLetCoordinate[2][2]
#define TOTALDENOMCOORDINATE01 BaseOutLetCoordinate[3][0]
#define TOTALDENOMCOORDINATE02 BaseOutLetCoordinate[3][1]
#define TOTALDENOMCOORDINATE03 BaseOutLetCoordinate[3][2]

//主菜单存取款模式和网络图标
#define MAINMODEICON01 BaseModeIcon[0]
#define MAINMODEICON02 BaseModeIcon[1]
#define MAINMODEICON03 BaseModeIcon[2]
#define MAINMODEICON04 BaseModeIcon[3]
#define MAINMODEICON05 BaseModeIcon[4]
#define MAINMODEICON06 BaseModeIcon[5]
#define MAINMODEICON07 BaseModeIcon[6]
#define MAINMODEICON08 BaseModeIcon[7]
#define MAINMODEICON09 BaseModeIcon[8]

#define NETABLE BaseNetIcon[0]
#define NETUNABLE BaseNetIcon[1]

//主菜单模式和网络图标坐标
#define MAINMODECOORDINATE01 BaseMainMenuModeCoordinate[0]
#define MAINMODECOORDINATE02 BaseMainMenuModeCoordinate[1]
#define NETCOORDINATE BaseNetCoordinate

//主菜单时间显示
#define TIMECOORDINATE BaseTimeCoordinate

//接钞金额
#define AMOUNTMODE01 BaseInletTitle[0]
#define AMOUNTMODE02 BaseInletTitle[1]
#define AMOUNTCOORDINATE BaseTotalSumCoordinate

//参数菜单
#define   OUTLETPARAM001  BaseOutLetParam[0][0]
#define   OUTLETPARAM002  BaseOutLetParam[0][1] 
#define   OUTLETPARAM003  BaseOutLetParam[0][2] 
#define   OUTLETPARAM004  BaseOutLetParam[0][3] 
#define   OUTLETPARAM005  BaseOutLetParam[0][4]
#define   OUTLETPARAM006  BaseOutLetParam[0][5]
#define   OUTLETPARAM007  BaseOutLetParam[0][6]
#define   OUTLETPARAM008  BaseOutLetParam[0][7]
#define   OUTLETPARAM009  BaseOutLetParam[0][8]
#define   OUTLETPARAM00A  BaseOutLetParam[0][9]
#define   OUTLETPARAM011  BaseOutLetParam[1][0]
#define   OUTLETPARAM012  BaseOutLetParam[1][1]
#define   OUTLETPARAM013  BaseOutLetParam[1][2]
#define   OUTLETPARAM014  BaseOutLetParam[1][3]
#define   OUTLETPARAM015  BaseOutLetParam[1][4]
#define   OUTLETPARAM016  BaseOutLetParam[1][5]
#define   OUTLETPARAM017  BaseOutLetParam[1][6]
#define   OUTLETPARAM018  BaseOutLetParam[1][7]
#define   OUTLETPARAM019  BaseOutLetParam[1][8]
#define   OUTLETPARAM01A  BaseOutLetParam[1][9]
#define   OUTLETPARAM021  BaseOutLetParam[2][0]
#define   OUTLETPARAM022  BaseOutLetParam[2][1]
#define   OUTLETPARAM023  BaseOutLetParam[2][2]
#define   OUTLETPARAM024  BaseOutLetParam[2][3] 
#define   OUTLETPARAM025  BaseOutLetParam[2][4] 
#define   OUTLETPARAM026  BaseOutLetParam[2][5]
#define   OUTLETPARAM027  BaseOutLetParam[2][6] 
#define   OUTLETPARAM028  BaseOutLetParam[2][7] 
#define   OUTLETPARAM029  BaseOutLetParam[2][8] 
#define   OUTLETPARAM02A  BaseOutLetParam[2][9] 
#define   OUTLETPARAM031  BaseOutLetParam[3][0]
#define   OUTLETPARAM032  BaseOutLetParam[3][1] 
#define   OUTLETPARAM033  BaseOutLetParam[3][2] 
#define   OUTLETPARAM034  BaseOutLetParam[3][3] 
#define   OUTLETPARAM035  BaseOutLetParam[3][4] 
#define   OUTLETPARAM036  BaseOutLetParam[3][5]
#define   OUTLETPARAM037  BaseOutLetParam[3][6] 
#define   OUTLETPARAM038  BaseOutLetParam[3][7] 
#define   OUTLETPARAM039  BaseOutLetParam[3][8] 
#define   OUTLETPARAM03A  BaseOutLetParam[3][9] 
#define   OUTLETPARAM041  BaseOutLetParam[4][0]
#define   OUTLETPARAM042  BaseOutLetParam[4][1] 
#define   OUTLETPARAM043  BaseOutLetParam[4][2] 
#define   OUTLETPARAM044  BaseOutLetParam[4][3] 
#define   OUTLETPARAM045  BaseOutLetParam[4][4] 
#define   OUTLETPARAM046  BaseOutLetParam[4][5]
#define   OUTLETPARAM047  BaseOutLetParam[4][6] 
#define   OUTLETPARAM048  BaseOutLetParam[4][7] 
#define   OUTLETPARAM049  BaseOutLetParam[4][8] 
#define   OUTLETPARAM04A  BaseOutLetParam[4][9] 
#define   OUTLETPARAM051  BaseOutLetParam[5][0]
#define   OUTLETPARAM052  BaseOutLetParam[5][1] 
#define   OUTLETPARAM053  BaseOutLetParam[5][2] 
#define   OUTLETPARAM054  BaseOutLetParam[5][3] 
#define   OUTLETPARAM055  BaseOutLetParam[5][4] 
#define   OUTLETPARAM056  BaseOutLetParam[5][5]
#define   OUTLETPARAM057  BaseOutLetParam[5][6] 
#define   OUTLETPARAM058  BaseOutLetParam[5][7] 
#define   OUTLETPARAM059  BaseOutLetParam[5][8] 
#define   OUTLETPARAM05A  BaseOutLetParam[5][9] 
#define   OUTLETPARAM061  BaseOutLetParam[6][0]
#define   OUTLETPARAM062  BaseOutLetParam[6][1] 
#define   OUTLETPARAM063  BaseOutLetParam[6][2] 
#define   OUTLETPARAM064  BaseOutLetParam[6][3] 
#define   OUTLETPARAM065  BaseOutLetParam[6][4] 
#define   OUTLETPARAM066  BaseOutLetParam[6][5]
#define   OUTLETPARAM067  BaseOutLetParam[6][6] 
#define   OUTLETPARAM068  BaseOutLetParam[6][7] 
#define   OUTLETPARAM069  BaseOutLetParam[6][8] 
#define   OUTLETPARAM06A  BaseOutLetParam[6][9] 

//鉴伪参数查看
#define   CHECKPARAM000   BaseCheckParam[0][0]
#define   CHECKPARAM001   BaseCheckParam[0][1]
#define   CHECKPARAM002   BaseCheckParam[0][2]
#define   CHECKPARAM003   BaseCheckParam[0][3]
#define   CHECKPARAM004   BaseCheckParam[0][4]
#define   CHECKPARAM005   BaseCheckParam[0][5]
#define   CHECKPARAM006   BaseCheckParam[0][6]
#define   CHECKPARAM007   BaseCheckParam[0][7]
#define   CHECKPARAM008   BaseCheckParam[0][8]
#define   CHECKPARAM009   BaseCheckParam[0][9]
#define   CHECKPARAM010   BaseCheckParam[1][0]
#define   CHECKPARAM011   BaseCheckParam[1][1]
#define   CHECKPARAM012   BaseCheckParam[1][2]
#define   CHECKPARAM013   BaseCheckParam[1][3]
#define   CHECKPARAM014   BaseCheckParam[1][4]
#define   CHECKPARAM015   BaseCheckParam[1][5]
#define   CHECKPARAM016   BaseCheckParam[1][6]
#define   CHECKPARAM017   BaseCheckParam[1][7]
#define   CHECKPARAM018   BaseCheckParam[1][8]
#define   CHECKPARAM019   BaseCheckParam[1][9]
#define   CHECKPARAM020   BaseCheckParam[2][0]
#define   CHECKPARAM021   BaseCheckParam[2][1]
#define   CHECKPARAM022   BaseCheckParam[2][2]
#define   CHECKPARAM023   BaseCheckParam[2][3]
#define   CHECKPARAM024   BaseCheckParam[2][4]
#define   CHECKPARAM025   BaseCheckParam[2][5]
#define   CHECKPARAM026   BaseCheckParam[2][6]
#define   CHECKPARAM027   BaseCheckParam[2][7]
#define   CHECKPARAM028   BaseCheckParam[2][8]
#define   CHECKPARAM029   BaseCheckParam[2][9]
#define   CHECKPARAM030   BaseCheckParam[3][0]
#define   CHECKPARAM031   BaseCheckParam[3][1]
#define   CHECKPARAM032   BaseCheckParam[3][2]
#define   CHECKPARAM033   BaseCheckParam[3][3]
#define   CHECKPARAM034   BaseCheckParam[3][4]
#define   CHECKPARAM035   BaseCheckParam[3][5]
#define   CHECKPARAM036   BaseCheckParam[3][6]
#define   CHECKPARAM037   BaseCheckParam[3][7]
#define   CHECKPARAM038   BaseCheckParam[3][8]
#define   CHECKPARAM039   BaseCheckParam[3][9]
#define   CHECKPARAM040   BaseCheckParam[4][0]
#define   CHECKPARAM041   BaseCheckParam[4][1]
#define   CHECKPARAM042   BaseCheckParam[4][2]
#define   CHECKPARAM043   BaseCheckParam[4][3]
#define   CHECKPARAM044   BaseCheckParam[4][4]
#define   CHECKPARAM045   BaseCheckParam[4][5]
#define   CHECKPARAM046   BaseCheckParam[4][6]
#define   CHECKPARAM047   BaseCheckParam[4][7]
#define   CHECKPARAM048   BaseCheckParam[4][8]
#define   CHECKPARAM049   BaseCheckParam[4][9]
#define   CHECKPARAM050   BaseCheckParam[5][0]
#define   CHECKPARAM051   BaseCheckParam[5][1]
#define   CHECKPARAM052   BaseCheckParam[5][2]
#define   CHECKPARAM053   BaseCheckParam[5][3]
#define   CHECKPARAM054   BaseCheckParam[5][4]
#define   CHECKPARAM055   BaseCheckParam[5][5]
#define   CHECKPARAM056   BaseCheckParam[5][6]
#define   CHECKPARAM057   BaseCheckParam[5][7]
#define   CHECKPARAM058   BaseCheckParam[5][8]
#define   CHECKPARAM059   BaseCheckParam[5][9]
#define   CHECKPARAM060   BaseCheckParam[6][0]
#define   CHECKPARAM061   BaseCheckParam[6][1]
#define   CHECKPARAM062   BaseCheckParam[6][2]
#define   CHECKPARAM063   BaseCheckParam[6][3]
#define   CHECKPARAM064   BaseCheckParam[6][4]
#define   CHECKPARAM065   BaseCheckParam[6][5]
#define   CHECKPARAM066   BaseCheckParam[6][6]
#define   CHECKPARAM067   BaseCheckParam[6][7]
#define   CHECKPARAM068   BaseCheckParam[6][8]
#define   CHECKPARAM069   BaseCheckParam[6][9]
#define   CHECKPARAM070   BaseCheckParam[7][0]
#define   CHECKPARAM071   BaseCheckParam[7][1]
#define   CHECKPARAM072   BaseCheckParam[7][2]
#define   CHECKPARAM073   BaseCheckParam[7][3]
#define   CHECKPARAM074   BaseCheckParam[7][4]
#define   CHECKPARAM075   BaseCheckParam[7][5]
#define   CHECKPARAM076   BaseCheckParam[7][6]
#define   CHECKPARAM077   BaseCheckParam[7][7]
#define   CHECKPARAM078   BaseCheckParam[7][8]
#define   CHECKPARAM079   BaseCheckParam[7][9]
#define   CHECKPARAM080   BaseCheckParam[8][0]
#define   CHECKPARAM081   BaseCheckParam[8][1]
#define   CHECKPARAM082   BaseCheckParam[8][2]
#define   CHECKPARAM083   BaseCheckParam[8][3]
#define   CHECKPARAM084   BaseCheckParam[8][4]
#define   CHECKPARAM085   BaseCheckParam[8][5]
#define   CHECKPARAM086   BaseCheckParam[8][6]
#define   CHECKPARAM087   BaseCheckParam[8][7]
#define   CHECKPARAM088   BaseCheckParam[8][8]
#define   CHECKPARAM089   BaseCheckParam[8][9]
#define   CHECKPARAM090   BaseCheckParam[9][0]
#define   CHECKPARAM091   BaseCheckParam[9][1]
#define   CHECKPARAM092   BaseCheckParam[9][2]
#define   CHECKPARAM093   BaseCheckParam[9][3]
#define   CHECKPARAM094   BaseCheckParam[9][4]
#define   CHECKPARAM095   BaseCheckParam[9][5]
#define   CHECKPARAM096   BaseCheckParam[9][6]
#define   CHECKPARAM097   BaseCheckParam[9][7]
#define   CHECKPARAM098   BaseCheckParam[9][8]
#define   CHECKPARAM099   BaseCheckParam[9][9]

//鉴伪参数修改
#define   EDITPARAM000   BaseEditParam[0][0]
#define   EDITPARAM001   BaseEditParam[0][1]
#define   EDITPARAM002   BaseEditParam[0][2]
#define   EDITPARAM003   BaseEditParam[0][3]
#define   EDITPARAM004   BaseEditParam[0][4]
#define   EDITPARAM005   BaseEditParam[0][5]
#define   EDITPARAM006   BaseEditParam[0][6]
#define   EDITPARAM007   BaseEditParam[0][7]
#define   EDITPARAM008   BaseEditParam[0][8]
#define   EDITPARAM009   BaseEditParam[0][9]
#define   EDITPARAM00A   BaseEditParam[0][10]
#define   EDITPARAM00B   BaseEditParam[0][11]
#define   EDITPARAM010   BaseEditParam[1][0]
#define   EDITPARAM011   BaseEditParam[1][1]
#define   EDITPARAM012   BaseEditParam[1][2]
#define   EDITPARAM013   BaseEditParam[1][3]
#define   EDITPARAM014   BaseEditParam[1][4]
#define   EDITPARAM015   BaseEditParam[1][5]
#define   EDITPARAM016   BaseEditParam[1][6]
#define   EDITPARAM017   BaseEditParam[1][7]
#define   EDITPARAM018   BaseEditParam[1][8]
#define   EDITPARAM019   BaseEditParam[1][9]
#define   EDITPARAM01A   BaseEditParam[1][10]
#define   EDITPARAM01B   BaseEditParam[1][11]
#define   EDITPARAM020   BaseEditParam[2][0]
#define   EDITPARAM021   BaseEditParam[2][1]
#define   EDITPARAM022   BaseEditParam[2][2]
#define   EDITPARAM023   BaseEditParam[2][3]
#define   EDITPARAM024   BaseEditParam[2][4]
#define   EDITPARAM025   BaseEditParam[2][5]
#define   EDITPARAM026   BaseEditParam[2][6]
#define   EDITPARAM027   BaseEditParam[2][7]
#define   EDITPARAM028   BaseEditParam[2][8]
#define   EDITPARAM029   BaseEditParam[2][9]
#define   EDITPARAM02A   BaseEditParam[2][10]
#define   EDITPARAM02B   BaseEditParam[2][11]
#define   EDITPARAM030   BaseEditParam[3][0]
#define   EDITPARAM031   BaseEditParam[3][1]
#define   EDITPARAM032   BaseEditParam[3][2]
#define   EDITPARAM033   BaseEditParam[3][3]
#define   EDITPARAM034   BaseEditParam[3][4]
#define   EDITPARAM035   BaseEditParam[3][5]
#define   EDITPARAM036   BaseEditParam[3][6]
#define   EDITPARAM037   BaseEditParam[3][7]
#define   EDITPARAM038   BaseEditParam[3][8]
#define   EDITPARAM039   BaseEditParam[3][9]
#define   EDITPARAM03A   BaseEditParam[3][10]
#define   EDITPARAM03B   BaseEditParam[3][11]
#define   EDITPARAM040   BaseEditParam[4][0]
#define   EDITPARAM041   BaseEditParam[4][1]
#define   EDITPARAM042   BaseEditParam[4][2]
#define   EDITPARAM043   BaseEditParam[4][3]
#define   EDITPARAM044   BaseEditParam[4][4]
#define   EDITPARAM045   BaseEditParam[4][5]
#define   EDITPARAM046   BaseEditParam[4][6]
#define   EDITPARAM047   BaseEditParam[4][7]
#define   EDITPARAM048   BaseEditParam[4][8]
#define   EDITPARAM049   BaseEditParam[4][9]
#define   EDITPARAM04A   BaseEditParam[4][10]
#define   EDITPARAM04B   BaseEditParam[4][11]
#define   EDITPARAM050   BaseEditParam[5][0]
#define   EDITPARAM051   BaseEditParam[5][1]
#define   EDITPARAM052   BaseEditParam[5][2]
#define   EDITPARAM053   BaseEditParam[5][3]
#define   EDITPARAM054   BaseEditParam[5][4]
#define   EDITPARAM055   BaseEditParam[5][5]
#define   EDITPARAM056   BaseEditParam[5][6]
#define   EDITPARAM057   BaseEditParam[5][7]
#define   EDITPARAM058   BaseEditParam[5][8]
#define   EDITPARAM059   BaseEditParam[5][9]
#define   EDITPARAM05A   BaseEditParam[5][10]
#define   EDITPARAM05B   BaseEditParam[5][11]
#define   EDITPARAM060   BaseEditParam[6][0]
#define   EDITPARAM061   BaseEditParam[6][1]
#define   EDITPARAM062   BaseEditParam[6][2]
#define   EDITPARAM063   BaseEditParam[6][3]
#define   EDITPARAM064   BaseEditParam[6][4]
#define   EDITPARAM065   BaseEditParam[6][5]
#define   EDITPARAM066   BaseEditParam[6][6]
#define   EDITPARAM067   BaseEditParam[6][7]
#define   EDITPARAM068   BaseEditParam[6][8]
#define   EDITPARAM069   BaseEditParam[6][9]
#define   EDITPARAM06A   BaseEditParam[6][10]
#define   EDITPARAM06B   BaseEditParam[6][11]
#define   EDITPARAM070   BaseEditParam[7][0]
#define   EDITPARAM071   BaseEditParam[7][1]
#define   EDITPARAM072   BaseEditParam[7][2]
#define   EDITPARAM073   BaseEditParam[7][3]
#define   EDITPARAM074   BaseEditParam[7][4]
#define   EDITPARAM075   BaseEditParam[7][5]
#define   EDITPARAM076   BaseEditParam[7][6]
#define   EDITPARAM077   BaseEditParam[7][7]
#define   EDITPARAM078   BaseEditParam[7][8]
#define   EDITPARAM079   BaseEditParam[7][9]
#define   EDITPARAM07A   BaseEditParam[7][10]
#define   EDITPARAM07B   BaseEditParam[7][11]
#define   EDITPARAM080   BaseEditParam[8][0]
#define   EDITPARAM081   BaseEditParam[8][1]
#define   EDITPARAM082   BaseEditParam[8][2]
#define   EDITPARAM083   BaseEditParam[8][3]
#define   EDITPARAM084   BaseEditParam[8][4]
#define   EDITPARAM085   BaseEditParam[8][5]
#define   EDITPARAM086   BaseEditParam[8][6]
#define   EDITPARAM087   BaseEditParam[8][7]
#define   EDITPARAM088   BaseEditParam[8][8]
#define   EDITPARAM089   BaseEditParam[8][9]
#define   EDITPARAM08A   BaseEditParam[8][10]
#define   EDITPARAM08B   BaseEditParam[8][11]
#define   EDITPARAM090   BaseEditParam[9][0]
#define   EDITPARAM091   BaseEditParam[9][1]
#define   EDITPARAM092   BaseEditParam[9][2]
#define   EDITPARAM093   BaseEditParam[9][3]
#define   EDITPARAM094   BaseEditParam[9][4]
#define   EDITPARAM095   BaseEditParam[9][5]
#define   EDITPARAM096   BaseEditParam[9][6]
#define   EDITPARAM097   BaseEditParam[9][7]
#define   EDITPARAM098   BaseEditParam[9][8]
#define   EDITPARAM099   BaseEditParam[9][9]
#define   EDITPARAM09A   BaseEditParam[9][10]
#define   EDITPARAM09B   BaseEditParam[9][11]

#define   EDITPARAMCOLOR01 BaseEditParamColor[0]
#define   EDITPARAMCOLOR02 BaseEditParamColor[1]
#define   EDITPARAMCOLOR03 BaseEditParamColor[2]
#define   EDITPARAMCOLOR04 BaseEditParamColor[3]

/* Private function prototypes----------------------------------------------------------------*/
//主菜单坐标的初始化
void InitMianMenu();
//数字转化为字符串
void bin_to_char(INT8U Dat,INT8U *Ptr);
//混点明细张数和金额转化为字符串
void Mix_bin_to_char(INT8U* Dat,INT8U *Ptr);
//马达速度调试
void JcSpeed_bin_to_char(INT8U* Dat,INT8U *Ptr);
//马达PWM调试
void JcPWM_bin_to_char(INT8U* Dat,INT8U *Ptr);
//主菜单根据模式显示不同信息的区域的显示
void PraperMianMenu(void);

//主菜单的总金额或累计金额的显示
void MianMenuAmountNumDisp(void);
//主菜单接钞口显示
void MianMenuInletNumDisp(void);
//主菜单退钞口显示
void MianMenuOutletNumDisp(void);
//主菜单批量显示
void MianMenuBatchNumDisp(void);
//主菜单面额显示
void MianMenuDenomNumDisp(void);
//主菜单总张数显示
void MianMenuTotalNumDisp(void);
//主菜单操作员签到菜单
void MianMenuSignInDisp(void);
//主菜单交易号显示
void MianMenuAnsactionDisp(void);
//批量菜单批量张数显示
void BatchMenuNumDisp(void);
//混点明细查询菜单的张数和金额显示
void MixMenuNumDisp(void);
//退钞口明细查看
void OutLetDetailDisp(void);
//接钞口明细查看
void InLetDetailDisp(void);
//清分等级显示
void CleanLevelDisp(void);
//鉴伪参数显示
void AuthenticationLevelDisp(void);
//显示上一次签到人员的名字
void KeyBoardDispNum(INT8U *DispNum);
//显示上一次签到人员的名字
void KeyBoardDispSignInNum(INT8U *DispNum);
//显示版本信息
void VersionDisp(void);
//马达调试
void JcSpeedDispMenu(void);
//进钞出钞AD数据
void InLetADDispMenu(void);
//红外AD数据
void IRADDispMenu(void);
//大小磁头信息
void MTADDispMenu(void);
//UV信息
void UVADDispMenu(void);
//MT数据查看
void InfoMTWaveformDispMenu(void);
//走钞IR信息
void InfoIRADDispMenu(void);
//走钞MT
void InfoMTADDispMenu(void);
//走钞MG
void InfoMGADDispMenu(void);
//参数设置1位显示
void bin_to_1char(INT8U Dat,INT8U *Ptr);
//参数设置2位显示
void bin_to_2char(INT8U Dat,INT8U *Ptr);
/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


