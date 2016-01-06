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
#include "CoodinateInit.h"
#include "DwinPotocol.h"
#include "BaseDisp.h"
#include "grlib.h"
#include "DisplayMain.h"
#include "version.h"



/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//图标保存的地址
//主菜单需要显示的艺术字的坐标
INT8U MainMenuNumCoordinate[3][10][8] = {//退钞口的0-4坐标
                                         {{0x00,0x40,0x00,0x04,0x00,0x50,0x00,0x20},{0x00,0x65,0x00,0x04,0x00,0x75,0x00,0x20},{0x00,0x8C,0x00,0x04,0x00,0x9B,0x00,0x20},{0x00,0xB1,0x00,0x04,0x00,0xC0,0x00,0x20},{0x00,0xD7,0x00,0x04,0x00,0xE6,0x00,0x20},
                                         //退钞口1-5坐标
                                          {0x00,0xFE,0x00,0x04,0x01,0x0E,0x00,0x20},{0x01,0x24,0x00,0x04,0x01,0x34,0x00,0x20},{0x01,0x4A,0x00,0x04,0x01,0x5A,0x00,0x20},{0x01,0x70,0x00,0x04,0x01,0x80,0x00,0x20},{0x01,0x96,0x00,0x04,0x01,0xA6,0x00,0x20}},
                                         {//总张数6-0坐标
                                          {0x00,0x48,0x00,0x28,0x00,0x7B,0x00,0x7D},{0x00,0x8E,0x00,0x28,0x00,0xC0,0x00,0x7D},{0x00,0xD5,0x00,0x28,0x01,0x08,0x00,0x7D},{0x01,0x1B,0x00,0x28,0x01,0x4E,0x00,0x7D},{0x01,0x61,0x00,0x28,0x01,0x94,0x00,0x7D},
                                          //总张数1-5坐标
                                          {0x00,0x48,0x00,0x84,0x00,0x7B,0x00,0xD9},{0x00,0x8E,0x00,0x84,0x00,0xC1,0x00,0xD9},{0x00,0xD5,0x00,0x84,0x01,0x08,0x00,0xD9},{0x01,0x1B,0x00,0x84,0x01,0x4E,0x00,0xD9},{0x01,0x61,0x00,0x84,0x01,0x94,0x00,0xD9}},
                                         {//总金额 0-5
                                          {0x00,0x04,0x00,0xE2,0x00,0x1E,0x01,0x0E},{0x00,0x36,0x00,0xE2,0x00,0x4F,0x01,0x0E},{0x00,0x67,0x00,0xE2,0x00,0x81,0x01,0x0E},{0x00,0x98,0x00,0xE2,0x00,0xB2,0x01,0x0E},{0x00,0xCA,0x00,0xE2,0x00,0xE4,0x01,0x0E},
                                          //总金额 6-9
                                          {0x00,0xFB,0x00,0xE2,0x01,0x15,0x01,0x0E},{0x01,0x2D,0x00,0xE2,0x01,0x47,0x01,0x0E},{0x01,0x5E,0x00,0xE2,0x01,0x78,0x01,0x0E},{0x01,0x90,0x00,0xE2,0x01,0xAA,0x01,0x0E},{0x01,0xC1,0x00,0xE2,0x01,0xDB,0x01,0x0E}},
                                          };

//接钞金额，累计金额title
INT8U InletTitle[2][8] = {{0x00,0x12,0x00,0x42,0x00,0x5D,0x00,0x57},
                          {0x00,0x65,0x00,0x42,0x00,0xB0,0x00,0x57},
                        };

//模式icon的坐标
INT8U ModeIcon[9][8] = {{0x00,0x13,0x00,0x8D,0x00,0x3A,0x00,0xA3},
                  {0x00,0x44,0x00,0x8D,0x00,0x6C,0x00,0xA3},
                  {0x00,0x77,0x00,0x8D,0x00,0x9E,0x00,0xA3},
                  {0x00,0xA8,0x00,0x8D,0x00,0xD0,0x00,0xA3},
                  {0x00,0xDB,0x00,0x8D,0x01,0x03,0x00,0xA3},
                  {0x01,0x0D,0x00,0x8D,0x01,0x34,0x00,0xA3},
                  {0x01,0x3F,0x00,0x8D,0x01,0x66,0x00,0xA3},
                  {0x01,0x71,0x00,0x8D,0x01,0x98,0x00,0xA3},
                  {0x01,0xA3,0x00,0x8D,0x01,0xCA,0x00,0xA3},
                 };

INT8U NetIcon[2][8] = {{0x00,0x12,0x00,0x63,0x00,0x46,0x00,0x83},{0x00,0x65,0x00,0x63,0x00,0x99,0x00,0x83}};

//图标需要显示的地址
//出钞口，接钞口，面额，批量的艺术字需要显示的坐标
INT8U OutLetCoordinate[4][3][4] = {//退钞口三位数显示的坐标
                                {{0x00,0x52,0x00,0x29},{0x00,0x60,0x00,0x29},{0x00,0x6F,0x00,0x29}},
                                //接钞口三位数显示的坐标
                                {{0x00,0x52,0x00,0x5E},{0x00,0x60,0x00,0x5E},{0x00,0x6F,0x00,0x5E}},
                                //面额三位数显示的坐标
                                {{0x00,0x52,0x00,0x91},{0x00,0x60,0x00,0x91},{0x00,0x6F,0x00,0x91}},
                                //批量三位数显示的坐标
                                {{0x00,0x52,0x00,0xBE},{0x00,0x60,0x00,0xBE},{0x00,0x6F,0x00,0xBE}}};

//总张数显示的艺术字的坐标
INT8U TotalPage[4][4] = {{0x00,0xF9,0x00,0x37},
                        {0x01,0x2C,0x00,0x37},
                        {0x01,0x63,0x00,0x37},
                        {0x01,0x9A,0x00,0x37}};

//总金额显示的艺术字的坐标
INT8U TotalSum[7][4] = {{0x01,0x01,0x00,0xAF},
                        {0x01,0x1D,0x00,0xAF},
                        {0x01,0x3A,0x00,0xAF},
                        {0x01,0x56,0x00,0xAF},
                        {0x01,0x73,0x00,0xAF},
                        {0x01,0x90,0x00,0xAF},
                        {0x01,0xAC,0x00,0xAF},};

//接钞金额，总金额需要显示的位置
INT8U TotalSumCoordinate[4] =  {0x00,0xA6,0x00,0xBA};

//网络图标的坐标
INT8U NetCoordinate[4] = {0x00,0xAC,0x00,0xE7};

//时间的坐标
INT8U TimeCoordinate[4] = {0x01,0x09,0x00,0xEC};

//显示模式的坐标
INT8U MainMenuModeCoordinate[2][4] = {{0x00,0x19,0x00,0xED},{0x00,0x61,0x00,0xED},};

//批量参数显示的坐标
INT8U BatchParamColor[4] = {0x00,0xA1,0x00,0x35};

//退币口查看需要显示的坐标
INT8U OutLetParam[7][10][4] = {{{0x00,0x05,0x00,0x1E},{0x00,0x05,0x00,0x38},{0x00,0x05,0x00,0x55},{0x00,0x05,0x00,0x71},{0x00,0x05,0x00,0x8B},{0x00,0x05,0x00,0xA6},{0x00,0x05,0x00,0xC2},{0x00,0x05,0x00,0xDE},{0x00,0x05,0x00,0xF8},{0x00,0x05,0x01,0x13},},
                             {{0x00,0x39,0x00,0x1E},{0x00,0x39,0x00,0x38},{0x00,0x39,0x00,0x55},{0x00,0x39,0x00,0x71},{0x00,0x39,0x00,0x8B},{0x00,0x39,0x00,0xA6},{0x00,0x39,0x00,0xC2},{0x00,0x05,0x00,0xDE},{0x00,0x05,0x00,0xF8},{0x00,0x05,0x01,0x13},},
                             {{0x00,0x78,0x00,0x1E},{0x00,0x78,0x00,0x38},{0x00,0x78,0x00,0x55},{0x00,0x78,0x00,0x71},{0x00,0x78,0x00,0x8B},{0x00,0x78,0x00,0xA6},{0x00,0x78,0x00,0xC2},{0x00,0x05,0x00,0xDE},{0x00,0x05,0x00,0xF8},{0x00,0x05,0x01,0x13},},
                             {{0x00,0xB4,0x00,0x1E},{0x00,0xB4,0x00,0x38},{0x00,0xB4,0x00,0x55},{0x00,0xB4,0x00,0x71},{0x00,0xB4,0x00,0x8B},{0x00,0xB4,0x00,0xA6},{0x00,0xB4,0x00,0xC2},{0x00,0x05,0x00,0xDE},{0x00,0x05,0x00,0xF8},{0x00,0x05,0x01,0x13},},
                             {{0x00,0xF5,0x00,0x1E},{0x00,0xF5,0x00,0x38},{0x00,0xF5,0x00,0x55},{0x00,0xF5,0x00,0x71},{0x00,0xF5,0x00,0x8B},{0x00,0xF5,0x00,0xA6},{0x00,0xF5,0x00,0xC2},{0x00,0x05,0x00,0xDE},{0x00,0x05,0x00,0xF8},{0x00,0x05,0x01,0x13},},
                             {{0x01,0x2B,0x00,0x1E},{0x01,0x2B,0x00,0x38},{0x01,0x2B,0x00,0x55},{0x01,0x2B,0x00,0x71},{0x01,0x2B,0x00,0x8B},{0x01,0x2B,0x00,0xA6},{0x01,0x2B,0x00,0xC2},{0x00,0x05,0x00,0xDE},{0x00,0x05,0x00,0xF8},{0x00,0x05,0x01,0x13},},
                             {{0x01,0xB1,0x00,0x1E},{0x01,0xB1,0x00,0x38},{0x01,0xB1,0x00,0x55},{0x01,0xB1,0x00,0x71},{0x01,0xB1,0x00,0x8B},{0x01,0xB1,0x00,0xA6},{0x01,0xB1,0x00,0xC2},{0x00,0x05,0x00,0xDE},{0x00,0x05,0x00,0xF8},{0x00,0x05,0x01,0x13},},
};

//查看鉴伪参数需要显示的坐标
INT8U CheckParam[10][10][4] = {{{0x00,0x39,0x00,0x26},{0x00,0x65,0x00,0x26},{0x00,0x85,0x00,0x26},{0x00,0xB0,0x00,0x26},{0x00,0xC5,0x00,0x26},{0x00,0xEF,0x00,0x26},{0x01,0x1A,0x00,0x26},{0x01,0x3B,0x00,0x26},{0x01,0x66,0x00,0x26},{0x01,0x7A,0x00,0x26},},
                               {{0x00,0x39,0x00,0x3C},{0x00,0x65,0x00,0x3C},{0x00,0x85,0x00,0x3C},{0x00,0xB0,0x00,0x3C},{0x00,0xC5,0x00,0x3C},{0x00,0xEF,0x00,0x3C},{0x01,0x1A,0x00,0x3C},{0x01,0x3B,0x00,0x3C},{0x01,0x66,0x00,0x3C},{0x01,0x7A,0x00,0x3C},},
                               {{0x00,0x39,0x00,0x52},{0x00,0x65,0x00,0x52},{0x00,0x85,0x00,0x52},{0x00,0xB0,0x00,0x52},{0x00,0xC5,0x00,0x52},{0x00,0xEF,0x00,0x52},{0x01,0x1A,0x00,0x52},{0x01,0x3B,0x00,0x52},{0x01,0x66,0x00,0x52},{0x01,0x7A,0x00,0x52},},
                               {{0x00,0x39,0x00,0x68},{0x00,0x65,0x00,0x68},{0x00,0x85,0x00,0x68},{0x00,0xB0,0x00,0x68},{0x00,0xC5,0x00,0x68},{0x00,0xEF,0x00,0x68},{0x01,0x1A,0x00,0x68},{0x01,0x3B,0x00,0x68},{0x01,0x66,0x00,0x68},{0x01,0x7A,0x00,0x68},},
                               {{0x00,0x39,0x00,0x7E},{0x00,0x65,0x00,0x7E},{0x00,0x85,0x00,0x7E},{0x00,0xB0,0x00,0x7E},{0x00,0xC5,0x00,0x7E},{0x00,0xEF,0x00,0x7E},{0x01,0x1A,0x00,0x7E},{0x01,0x3B,0x00,0x7E},{0x01,0x66,0x00,0x7E},{0x01,0x7A,0x00,0x7E},},
                               {{0x00,0x39,0x00,0x94},{0x00,0x65,0x00,0x94},{0x00,0x85,0x00,0x94},{0x00,0xB0,0x00,0x94},{0x00,0xC5,0x00,0x94},{0x00,0xEF,0x00,0x94},{0x01,0x1A,0x00,0x94},{0x01,0x3B,0x00,0x94},{0x01,0x66,0x00,0x94},{0x01,0x7A,0x00,0x94},},
                               {{0x00,0x39,0x00,0xAA},{0x00,0x65,0x00,0xAA},{0x00,0x85,0x00,0xAA},{0x00,0xB0,0x00,0xAA},{0x00,0xC5,0x00,0xAA},{0x00,0xEF,0x00,0xAA},{0x01,0x1A,0x00,0xAA},{0x01,0x3B,0x00,0xAA},{0x01,0x66,0x00,0xAA},{0x01,0x7A,0x00,0xAA},},
                               {{0x00,0x39,0x00,0xC0},{0x00,0x65,0x00,0xC0},{0x00,0x85,0x00,0xC0},{0x00,0xB0,0x00,0xC0},{0x00,0xC5,0x00,0xC0},{0x00,0xEF,0x00,0xC0},{0x01,0x1A,0x00,0xC0},{0x01,0x3B,0x00,0xC0},{0x01,0x66,0x00,0xC0},{0x01,0x7A,0x00,0xC0},},
                               {{0x00,0x39,0x00,0xD6},{0x00,0x65,0x00,0xD6},{0x00,0x85,0x00,0xD6},{0x00,0xB0,0x00,0xD6},{0x00,0xC5,0x00,0xD6},{0x00,0xEF,0x00,0xD6},{0x01,0x1A,0x00,0xD6},{0x01,0x3B,0x00,0xD6},{0x01,0x66,0x00,0xD6},{0x01,0x7A,0x00,0xD6},},
                               {{0x00,0x39,0x00,0xEC},{0x00,0x65,0x00,0xEC},{0x00,0x85,0x00,0xEC},{0x00,0xB0,0x00,0xEC},{0x00,0xC5,0x00,0xEC},{0x00,0xEF,0x00,0xEC},{0x01,0x1A,0x00,0xEC},{0x01,0x3B,0x00,0xEC},{0x01,0x66,0x00,0xEC},{0x01,0x7A,0x00,0xEC},},

};

//查看鉴伪参数需要显示的坐标
INT8U EditParam[10][12][4] = {{{0x00,0x32,0x00,0x26},{0x00,0x5A,0x00,0x26},{0x00,0x78,0x00,0x26},{0x00,0x9D,0x00,0x26},{0x00,0xB6,0x00,0x26},{0x00,0xDC,0x00,0x26},{0x00,0xF9,0x00,0x26},{0x01,0x1F,0x00,0x26},{0x01,0x37,0x00,0x26},{0x01,0x5D,0x00,0x26},{0x01,0x7A,0x00,0x26},{0x01,0x9F,0x00,0x26},},
                              {{0x00,0x32,0x00,0x3C},{0x00,0x5A,0x00,0x3C},{0x00,0x78,0x00,0x3C},{0x00,0x9D,0x00,0x3C},{0x00,0xB6,0x00,0x3C},{0x00,0xDC,0x00,0x3C},{0x00,0xF9,0x00,0x3C},{0x01,0x1F,0x00,0x3C},{0x01,0x37,0x00,0x3C},{0x01,0x5D,0x00,0x3C},{0x01,0x7A,0x00,0x3C},{0x01,0x9F,0x00,0x3C},},
                              {{0x00,0x32,0x00,0x52},{0x00,0x5A,0x00,0x52},{0x00,0x78,0x00,0x52},{0x00,0x9D,0x00,0x52},{0x00,0xB6,0x00,0x52},{0x00,0xDC,0x00,0x52},{0x00,0xF9,0x00,0x52},{0x01,0x1F,0x00,0x52},{0x01,0x37,0x00,0x52},{0x01,0x5D,0x00,0x52},{0x01,0x7A,0x00,0x52},{0x01,0x9F,0x00,0x52},},
                              {{0x00,0x32,0x00,0x68},{0x00,0x5A,0x00,0x68},{0x00,0x78,0x00,0x68},{0x00,0x9D,0x00,0x68},{0x00,0xB6,0x00,0x68},{0x00,0xDC,0x00,0x68},{0x00,0xF9,0x00,0x68},{0x01,0x1F,0x00,0x68},{0x01,0x37,0x00,0x68},{0x01,0x5D,0x00,0x68},{0x01,0x7A,0x00,0x68},{0x01,0x9F,0x00,0x68},},
                              {{0x00,0x32,0x00,0x7E},{0x00,0x5A,0x00,0x7E},{0x00,0x78,0x00,0x7E},{0x00,0x9D,0x00,0x7E},{0x00,0xB6,0x00,0x7E},{0x00,0xDC,0x00,0x7E},{0x00,0xF9,0x00,0x7E},{0x01,0x1F,0x00,0x7E},{0x01,0x37,0x00,0x7E},{0x01,0x5D,0x00,0x7E},{0x01,0x7A,0x00,0x7E},{0x01,0x9F,0x00,0x7E},},
                              {{0x00,0x32,0x00,0x94},{0x00,0x5A,0x00,0x94},{0x00,0x78,0x00,0x94},{0x00,0x9D,0x00,0x94},{0x00,0xB6,0x00,0x94},{0x00,0xDC,0x00,0x94},{0x00,0xF9,0x00,0x94},{0x01,0x1F,0x00,0x94},{0x01,0x37,0x00,0x94},{0x01,0x5D,0x00,0x94},{0x01,0x7A,0x00,0x94},{0x01,0x9F,0x00,0x94},},
                              {{0x00,0x32,0x00,0xAA},{0x00,0x5A,0x00,0xAA},{0x00,0x78,0x00,0xAA},{0x00,0x9D,0x00,0xAA},{0x00,0xB6,0x00,0xAA},{0x00,0xDC,0x00,0xAA},{0x00,0xF9,0x00,0xAA},{0x01,0x1F,0x00,0xAA},{0x01,0x37,0x00,0xAA},{0x01,0x5D,0x00,0xAA},{0x01,0x7A,0x00,0xAA},{0x01,0x9F,0x00,0xAA},},
                              {{0x00,0x32,0x00,0xC0},{0x00,0x5A,0x00,0xC0},{0x00,0x78,0x00,0xC0},{0x00,0x9D,0x00,0xC0},{0x00,0xB6,0x00,0xC0},{0x00,0xDC,0x00,0xC0},{0x00,0xF9,0x00,0xC0},{0x01,0x1F,0x00,0xC0},{0x01,0x37,0x00,0xC0},{0x01,0x5D,0x00,0xC0},{0x01,0x7A,0x00,0xC0},{0x01,0x9F,0x00,0xC0},},
                              {{0x00,0x32,0x00,0xD6},{0x00,0x5A,0x00,0xD6},{0x00,0x78,0x00,0xD6},{0x00,0x9D,0x00,0xD6},{0x00,0xB6,0x00,0xD6},{0x00,0xDC,0x00,0xD6},{0x00,0xF9,0x00,0xD6},{0x01,0x1F,0x00,0xD6},{0x01,0x37,0x00,0xD6},{0x01,0x5D,0x00,0xD6},{0x01,0x7A,0x00,0xD6},{0x01,0x9F,0x00,0xD6},},
                              {{0x00,0x32,0x00,0xEC},{0x00,0x5A,0x00,0xEC},{0x00,0x78,0x00,0xEC},{0x00,0x9D,0x00,0xEC},{0x00,0xB6,0x00,0xEC},{0x00,0xDC,0x00,0xEC},{0x00,0xF9,0x00,0xEC},{0x01,0x1F,0x00,0xEC},{0x01,0x37,0x00,0xEC},{0x01,0x5D,0x00,0xEC},{0x01,0x7A,0x00,0xEC},{0x01,0x9F,0x00,0xEC},},
};

//查看鉴伪参数显示的背景颜色
INT8U EditParamColor[4][4] = {{0x02,0x9B,0x00,0x52},{0x02,0x9B,0x00,0x52},{0x02,0x9B,0x00,0x52},{0x02,0x9B,0x00,0x52}};

//批量参数显示的坐标
INT8U BatchParam[3][4] = {{0x00,0xA5,0x00,0x4E},{0x00,0xD7,0x00,0x4E},{0x01,0x09,0x00,0x4E}};

//主菜单操作员签到显示的坐标
INT8U MainMenuSignInCoordinate[10][4] = {{0x00,0x3C,0x00,0x02},{0x00,0x48,0x00,0x02},{0x00,0x54,0x00,0x02},{0x00,0x60,0x00,0x02},{0x00,0x6C,0x00,0x02},{0x00,0x78,0x00,0x02},{0x00,0x84,0x00,0x02},{0x00,0x90,0x00,0x02},{0x00,0x9C,0x00,0x02},{0x00,0xA8,0x00,0x02}};

//主菜单操作员签到显示的坐标
INT8U MainMenuAnsactionCoordinate[10][4] = {{0x01,0x35,0x00,0x02},{0x01,0x41,0x00,0x02},{0x01,0x4D,0x00,0x02},{0x01,0x59,0x00,0x02},{0x01,0x65,0x00,0x02},{0x01,0x71,0x00,0x02},{0x01,0x7D,0x00,0x02},{0x01,0x89,0x00,0x02},{0x01,0x95,0x00,0x02},{0x01,0xA1,0x00,0x02}};

//混点明细查看需要显示的坐标
INT8U MixNumCoordinate[2][7][4] = {{{0x00,0x80,0x00,0x39},{0x00,0x80,0x00,0x59},{0x00,0x80,0x00,0x77},
                                    {0x00,0x80,0x00,0x98},{0x00,0x80,0x00,0xB4},{0x00,0x80,0x00,0xD2},{0x00,0x80,0x00,0xF2},},
                                   {{0x01,0x2C,0x00,0x39},{0x01,0x2C,0x00,0x59},{0x01,0x2C,0x00,0x77},
                                    {0x01,0x2C,0x00,0x98},{0x01,0x2C,0x00,0xB4},{0x01,0x2C,0x00,0xD2},{0x01,0x2C,0x00,0xF2},},};

//退钞口查看显示的坐标
INT8U OutLetDetailCoordinate[9][7][4] = {{{0x00,0x10,0x00,0x20},{0x00,0x43,0x00,0x20},{0x00,0x84,0x00,0x20},{0x00,0xBE,0x00,0x20},{0x00,0xFD,0x00,0x20},{0x01,0x38,0x00,0x20},{0x01,0xBC,0x00,0x20},},
                                         {{0x00,0x10,0x00,0x3C},{0x00,0x43,0x00,0x3C},{0x00,0x84,0x00,0x3C},{0x00,0xBE,0x00,0x3C},{0x00,0xFD,0x00,0x3C},{0x01,0x38,0x00,0x3C},{0x01,0xBC,0x00,0x3C},},
                                         {{0x00,0x10,0x00,0x58},{0x00,0x43,0x00,0x58},{0x00,0x84,0x00,0x58},{0x00,0xBE,0x00,0x58},{0x00,0xFD,0x00,0x58},{0x01,0x38,0x00,0x58},{0x01,0xBC,0x00,0x58},},
                                         {{0x00,0x10,0x00,0x74},{0x00,0x43,0x00,0x74},{0x00,0x84,0x00,0x74},{0x00,0xBE,0x00,0x74},{0x00,0xFD,0x00,0x74},{0x01,0x38,0x00,0x74},{0x01,0xBC,0x00,0x74},},
                                         {{0x00,0x10,0x00,0x90},{0x00,0x43,0x00,0x90},{0x00,0x84,0x00,0x90},{0x00,0xBE,0x00,0x90},{0x00,0xFD,0x00,0x90},{0x01,0x38,0x00,0x90},{0x01,0xBC,0x00,0x90},},
                                         {{0x00,0x10,0x00,0xAB},{0x00,0x43,0x00,0xAB},{0x00,0x84,0x00,0xAB},{0x00,0xBE,0x00,0xAB},{0x00,0xFD,0x00,0xAB},{0x01,0x38,0x00,0xAB},{0x01,0xBC,0x00,0xAB},},
                                         {{0x00,0x10,0x00,0xC6},{0x00,0x43,0x00,0xC6},{0x00,0x84,0x00,0xC6},{0x00,0xBE,0x00,0xC6},{0x00,0xFD,0x00,0xC6},{0x01,0x38,0x00,0xC6},{0x01,0xBC,0x00,0xC6},},
                                         {{0x00,0x10,0x00,0xE1},{0x00,0x43,0x00,0xE1},{0x00,0x84,0x00,0xE1},{0x00,0xBE,0x00,0xE1},{0x00,0xFD,0x00,0xE1},{0x01,0x38,0x00,0xE1},{0x01,0xBC,0x00,0xE1},},
                                         {{0x00,0x10,0x00,0xFD},{0x00,0x43,0x00,0xFD},{0x00,0x84,0x00,0xFD},{0x00,0xBE,0x00,0xFD},{0x00,0xFD,0x00,0xFD},{0x01,0x38,0x00,0xFD},{0x01,0xBC,0x00,0xFD},},
};

//清分等级设置的坐标
INT8U CleanLevelSettingsCoordinate[6][5][4] = {{{0x00,0xA2,0x00,0x35},{0x00,0xD0,0x00,0x35},{0x00,0xFD,0x00,0x35},{0x01,0x29,0x00,0x35},{0x01,0x55,0x00,0x35},},
                                               {{0x00,0xA2,0x00,0x51},{0x00,0xD0,0x00,0x51},{0x00,0xFD,0x00,0x51},{0x01,0x29,0x00,0x51},{0x01,0x55,0x00,0x51},},
                                               {{0x00,0xA2,0x00,0x6E},{0x00,0xD0,0x00,0x6E},{0x00,0xFD,0x00,0x6E},{0x01,0x29,0x00,0x6E},{0x01,0x55,0x00,0x6E},},
                                               {{0x00,0xA2,0x00,0x8A},{0x00,0xD0,0x00,0x8A},{0x00,0xFD,0x00,0x8A},{0x01,0x29,0x00,0x8A},{0x01,0x55,0x00,0x8A},},
                                               {{0x00,0xA2,0x00,0xA8},{0x00,0xD0,0x00,0xA8},{0x00,0xFD,0x00,0xA8},{0x01,0x29,0x00,0xA8},{0x01,0x55,0x00,0xA8},},
                                               {{0x00,0xA2,0x00,0xC3},{0x00,0xD0,0x00,0xC3},{0x00,0xFD,0x00,0xC3},{0x01,0x29,0x00,0xC3},{0x01,0x55,0x00,0xC3},},
};

//清分等级设置的选中的坐标
INT8U CleanLevelSettingsSelectCoordinate[6][4] = {{0x00,0x73,0x00,0x32},{0x00,0x73,0x00,0x4F},{0x00,0x73,0x00,0x6D},{0x00,0x73,0x00,0x89},{0x00,0x73,0x00,0xA4},{0x00,0x73,0x00,0xC0}};


//鉴伪等级设置的坐标
INT8U AuthenticationSettingsCoordinate[7][5][4] = {{{0x00,0xBB,0x00,0x24},{0x00,0xE7,0x00,0x24},{0x01,0x12,0x00,0x24},{0x01,0x3E,0x00,0x24},{0x01,0x6C,0x00,0x24},},
                                                   {{0x00,0xBB,0x00,0x46},{0x00,0xE7,0x00,0x46},{0x01,0x12,0x00,0x46},{0x01,0x3E,0x00,0x46},{0x01,0x6C,0x00,0x46},},
                                                   {{0x00,0xBB,0x00,0x66},{0x00,0xE7,0x00,0x66},{0x01,0x12,0x00,0x66},{0x01,0x3E,0x00,0x66},{0x01,0x6C,0x00,0x66},},
                                                   {{0x00,0xBB,0x00,0x88},{0x00,0xE7,0x00,0x88},{0x01,0x12,0x00,0x88},{0x01,0x3E,0x00,0x88},{0x01,0x6C,0x00,0x88},},
                                                   {{0x00,0xBB,0x00,0xA6},{0x00,0xE7,0x00,0xA6},{0x01,0x12,0x00,0xA6},{0x01,0x3E,0x00,0xA6},{0x01,0x6C,0x00,0xA6},},
                                                   {{0x00,0xBB,0x00,0xC7},{0x00,0xE7,0x00,0xC7},{0x01,0x12,0x00,0xC7},{0x01,0x3E,0x00,0xC7},{0x01,0x6C,0x00,0xC7},},
                                                   {{0x00,0xBB,0x00,0xE8},{0x00,0xE7,0x00,0xE8},{0x01,0x12,0x00,0xE8},{0x01,0x3E,0x00,0xE8},{0x01,0x6C,0x00,0xE8},},
};

//鉴伪等级设置选中的坐标
INT8U AuthenticationSettingsSelectCoordinate[7][4] = {{0x00,0x8E,0x00,0x24},{0x00,0x8E,0x00,0x45},{0x00,0x8E,0x00,0x63},{0x00,0x8E,0x00,0x85},{0x00,0x8E,0x00,0xA3},{0x00,0x8E,0x00,0xC4},{0x00,0x8E,0x00,0xE5}};

//键盘的坐标
INT8U KeyBoardSettingsCoordinate[8][12][4] = {{{0x00,0x3B,0x00,0x14},{0x00,0x5B,0x00,0x14},{0x00,0x7A,0x00,0x14},{0x00,0x9A,0x00,0x14},{0x00,0xBA,0x00,0x14},{0x00,0xD9,0x00,0x14},{0x00,0xF9,0x00,0x14},{0x01,0x18,0x00,0x14},{0x01,0x38,0x00,0x14},{0x01,0x57,0x00,0x14},{0x01,0x77,0x00,0x14},{0x00,0x3B,0x00,0x32}},
                                               {{0x00,0x3B,0x00,0x32},{0x00,0x5B,0x00,0x32},{0x00,0x7A,0x00,0x32},{0x00,0x9A,0x00,0x32},{0x00,0xBA,0x00,0x32},{0x00,0xD9,0x00,0x32},{0x00,0xF9,0x00,0x32},{0x01,0x18,0x00,0x32},{0x01,0x38,0x00,0x32},{0x01,0x57,0x00,0x32},{0x01,0x77,0x00,0x32},{0x00,0x3B,0x00,0x55}},
                                               {{0x00,0x3B,0x00,0x55},{0x00,0x5B,0x00,0x55},{0x00,0x7A,0x00,0x55},{0x00,0x9A,0x00,0x55},{0x00,0xBA,0x00,0x55},{0x00,0xD9,0x00,0x55},{0x00,0xF9,0x00,0x55},{0x01,0x18,0x00,0x55},{0x01,0x38,0x00,0x55},{0x01,0x57,0x00,0x55},{0x01,0x77,0x00,0x49},{0x01,0x77,0x00,0x5F}},
                                               {{0x00,0x3B,0x00,0x75},{0x00,0x5B,0x00,0x75},{0x00,0x7A,0x00,0x75},{0x00,0x9A,0x00,0x75},{0x00,0xBA,0x00,0x75},{0x00,0xD9,0x00,0x75},{0x00,0xF9,0x00,0x75},{0x01,0x18,0x00,0x75},{0x01,0x38,0x00,0x75},{0x01,0x57,0x00,0x75},{0x01,0x77,0x00,0x75},{0x00,0x3B,0x00,0x14}},
                                               {{0x00,0x57,0x00,0x30},{0x00,0x77,0x00,0x30},{0x00,0x96,0x00,0x30},{0x00,0xB6,0x00,0x30},{0x00,0xD6,0x00,0x30},{0x00,0xF5,0x00,0x30},{0x01,0x15,0x00,0x30},{0x01,0x35,0x00,0x30},{0x01,0x54,0x00,0x30},{0x01,0x72,0x00,0x30},{0x01,0x94,0x00,0x30},{0x00,0x57,0x00,0x51}},
                                               {{0x00,0x57,0x00,0x51},{0x00,0x77,0x00,0x51},{0x00,0x96,0x00,0x51},{0x00,0xB6,0x00,0x51},{0x00,0xD6,0x00,0x51},{0x00,0xF5,0x00,0x51},{0x01,0x15,0x00,0x51},{0x01,0x35,0x00,0x51},{0x01,0x54,0x00,0x51},{0x01,0x72,0x00,0x51},{0x01,0x94,0x00,0x48},{0x01,0x77,0x00,0x5C}},
                                               {{0x00,0x57,0x00,0x72},{0x00,0x77,0x00,0x72},{0x00,0x96,0x00,0x72},{0x00,0xB6,0x00,0x72},{0x00,0xD6,0x00,0x72},{0x00,0xF5,0x00,0x72},{0x01,0x15,0x00,0x72},{0x01,0x35,0x00,0x72},{0x01,0x54,0x00,0x72},{0x01,0x72,0x00,0x72},{0x01,0x94,0x00,0x5C},{0x01,0x94,0x00,0x72}},
                                               {{0x00,0x57,0x00,0x94},{0x00,0x77,0x00,0x94},{0x00,0x96,0x00,0x94},{0x00,0xB6,0x00,0x94},{0x00,0xD6,0x00,0x94},{0x00,0xF5,0x00,0x94},{0x01,0x15,0x00,0x94},{0x01,0x35,0x00,0x94},{0x01,0x54,0x00,0x94},{0x01,0x72,0x00,0x94},{0x01,0x94,0x00,0x94},{0x00,0x57,0x00,0x30}},
};

//操作员签到显示的坐标
INT8U SignInCoordinate[16][4] = {{0x00,0xA1,0x00,0xC8},{0x00,0xAB,0x00,0xC8},{0x00,0xB5,0x00,0xC8},{0x00,0xBF,0x00,0xC8},{0x00,0xC9,0x00,0xC8},{0x00,0xD3,0x00,0xC8},{0x00,0xDD,0x00,0xC8},{0x00,0xE7,0x00,0xC8},{0x00,0xF1,0x00,0xC8},{0x00,0xFB,0x00,0xC8},{0x01,0x05,0x00,0xC8},{0x01,0x0F,0x00,0xC8},{0x01,0x19,0x00,0xC8},{0x01,0x23,0x00,0xC8},{0x01,0x2D,0x00,0xC8},{0x01,0x37,0x00,0xC8}};

//清分等级设置图标的坐标
INT8U CleanLevelSettingsIcon[5][8] = {{0x00,0x15,0x00,0xB4,0x00,0x29,0x00,0xC8},{0x00,0x5B,0x00,0xB4,0x00,0x6F,0x00,0xC8},{0x00,0xA0,0x00,0xB4,0x00,0xB4,0x00,0xC8},{0x00,0xE6,0x00,0xB4,0x00,0xFA,0x00,0xC8},{0x01,0x2B,0x00,0xB4,0x01,0x3F,0x00,0xC8}};

//每个键盘坐标对应的值
INT8U KeyBoardMAP[4][12] = {{0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x2E,0x51},
                            {0x51,0x57,0x45,0x52,0x54,0x59,0x55,0x49,0x4F,0x50,0x2F,0x41},
                            {0x41,0x53,0x44,0x46,0x47,0x48,0x4A,0x4B,0x4C,0x2A,0x5C,0x5F},
                            {0x3F,0x5A,0x58,0x43,0x56,0x42,0x4E,0x4D,0x2D,0x23,0x3A,0x3F},};

//每条黑名单标志对应的坐标
INT8U BlackListCoordinate[7][8] = {{0x00,0x75,0x00,0x1B,0x00,0x9A,0x00,0x36},
                                    {0x00,0x75,0x00,0x39,0x00,0x9A,0x00,0x53},
                                    {0x00,0x75,0x00,0x56,0x00,0x9A,0x00,0x71},
                                    {0x00,0x75,0x00,0x74,0x00,0x9A,0x00,0x8F},
                                    {0x00,0x75,0x00,0x92,0x00,0x9A,0x00,0xAD},
                                    {0x00,0x75,0x00,0xB0,0x00,0x9A,0x00,0xCB},
                                    {0x00,0x75,0x00,0xCE,0x00,0x9A,0x01,0xE9},};

//每条黑名单显示内容对应的坐标
INT8U BlackListNumberCoordinate[7][8] = {{0x00,0xAA,0x00,0x20,0x00,0x00,0x00,0x00},
                                         {0x00,0xAA,0x00,0x3D,0x00,0x00,0x00,0x00},
                                         {0x00,0xAA,0x00,0x5A,0x00,0x00,0x00,0x00},
                                         {0x00,0xAA,0x00,0x78,0x00,0x00,0x00,0x00},
                                         {0x00,0xAA,0x00,0x96,0x00,0x00,0x00,0x00},
                                         {0x00,0xAA,0x00,0xB4,0x00,0x00,0x00,0x00},
                                         {0x00,0xAA,0x00,0xD3,0x00,0x00,0x00,0x00},};

//版本信息对应的坐标
INT8U VersionCoordinate[3][7][8] = {{{0x00,0x5A,0x00,0x35,0x00,0x00,0x00,0x00},{0x00,0x5A,0x00,0x52,0x00,0x00,0x00,0x00},{0x00,0x5A,0x00,0x6E,0x00,0x00,0x00,0x00},{0x00,0x5A,0x00,0x8A,0x00,0x00,0x00,0x00},{0x00,0x5A,0x00,0xA5,0x00,0x00,0x00,0x00},{0x00,0x5A,0x00,0xC1,0x00,0x00,0x00,0x00},{0x00,0x5A,0x00,0xDC,0x00,0x00,0x01,0x00}},
                                    {{0x00,0xB4,0x00,0x35,0x00,0x00,0x00,0x00},{0x00,0xB4,0x00,0x52,0x00,0x00,0x00,0x00},{0x00,0xB4,0x00,0x6E,0x00,0x00,0x00,0x00},{0x00,0xB4,0x00,0x8A,0x00,0x00,0x00,0x00},{0x00,0xB4,0x00,0xA5,0x00,0x00,0x00,0x00},{0x00,0xB4,0x00,0xC1,0x00,0x00,0x00,0x00},{0x00,0xB4,0x00,0xDC,0x00,0x00,0x01,0x00}},
                                    {{0x01,0x75,0x00,0x35,0x00,0x00,0x00,0x00},{0x01,0x75,0x00,0x52,0x00,0x00,0x00,0x00},{0x01,0x75,0x00,0x6E,0x00,0x00,0x00,0x00},{0x01,0x75,0x00,0x8A,0x00,0x00,0x00,0x00},{0x01,0x75,0x00,0xA5,0x00,0x00,0x00,0x00},{0x01,0x75,0x00,0xC1,0x00,0x00,0x00,0x00},{0x01,0x75,0x00,0xDC,0x00,0x00,0x01,0x00}}};

//信息查看对应的坐标
INT8U InfomationCoordinate[3][7][8] = {{{0x00,0xE0,0x00,0x24,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x43,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x62,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x81,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xA0,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xBF,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xDE,0x00,0x00,0x00,0x00}},
                                       {{0x00,0xE0,0x00,0x2A,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x4D,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x72,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x95,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xB8,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xDD,0x00,0x00,0x00,0x00},{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
                                       {{0x00,0xE0,0x00,0x24,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x43,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x62,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0x81,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xA0,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xBF,0x00,0x00,0x00,0x00},{0x00,0xE0,0x00,0xDE,0x00,0x00,0x00,0x00}},};


tRectangle BaseMainMenuNumCoordinate[3][10];
tRectangle BaseInletTitle[2];
tRectangle BaseModeIcon[9];
tRectangle BaseNetIcon[2];
tRectangle BaseOutLetCoordinate[4][3];
tRectangle BaseTotalPage[4];
tRectangle BaseTotalSum[7];
tRectangle BaseTotalSumCoordinate;
tRectangle BaseNetCoordinate;
tRectangle BaseTimeCoordinate;
tRectangle BaseMainMenuModeCoordinate[2];
tRectangle BaseMainMenuSignInCoordinate[10];
tRectangle BaseMainMenuAnsactionCoordinate[10];
tRectangle BaseOutLetParam[7][10];
tRectangle BaseCheckParam[10][10];
tRectangle BaseEditParam[10][12];
tRectangle BaseEditParamColor[4];
tRectangle BaseBatchParam[3];
tRectangle BaseBatchParamColor;
tRectangle BaseMixNumCoordinate[2][7];
tRectangle BaseOutLetDetailCoordinate[9][7];
tRectangle BaseCleanLevelSettingsCoordinate[6][5];
tRectangle BaseCleanLevelSettingsSelectCoordinate[6];
tRectangle BaseCleanLevelSettingsIcon[5];
tRectangle BaseAuthenticationSettingsCoordinate[7][5];
tRectangle BaseAuthenticationSettingsSelectCoordinate[7];
tRectangle BaseKeyBoardSettingsCoordinate[8][12];
tRectangle BaseSignInCoordinate[16];
tRectangle BaseBlackListCoordinate[7];
tRectangle BaseBlackListNumberCoordinate[7];
tRectangle BaseVersionCoordinate[3][7];
tRectangle BaseInfomationCoordinate[3][7];

//清分模式
extern INT8U CLEARINGMODE;
//存取款模式
extern INT8U DEPWITHDRAWMODE;
//累加模式
extern INT8U SUMMATIONMODE;
//网络是否开启
extern INT8U NETFLAG;
//批量值
extern INT16U TemBatchNum;
//主菜单的can协议帧
extern INT8U MainMenuDisp[13];
//混点明细查询界面
extern INT8U MixedDisp[18];
//退钞口冠字号查询
extern INT8U OUTLETDisp[200][24];
//接钞口冠字号查询
extern INT8U INLETDisp[300][24];
//退钞口冠字号查询当前的页码
extern INT8U OutLetKeyPage;
//接钞口冠字号查询当前的页码
extern INT8U InLetKeyPage;
//临时保存的清分等级
extern INT8U TemClearLevel[6];
//临时保存的清分等级
extern INT8U TemAuthenticationLevel[7];
//主菜单批量值
extern INT16U BatchNum;
//操作员签到保存的名字
extern INT8U SignIn[11];
//操作员签到的名字长度
extern INT8U SignInNUM;
//交易号
extern INT8U AnsactionNum[11];
//交易号输入的长度
extern INT8U AnsactionNumNUM;
//马达调试
extern INT8U JcSpeedDisp[6];
//进钞出钞AD
extern INT8U InLetADDisp[15];
//红外AD
extern INT8U IRADDisp[30];
//大小磁头
extern INT8U MTDisp[30];
//荧光采样
extern INT8U UVDisp[30];
//走钞红外
extern INT8U InfoIRDisp[50];
//走钞MT
extern INT8U InfoMTDisp[50];
//走钞MG
extern INT8U InfoMGDisp[6][50];
//走钞MG
extern INT8U MTwaveform[50];

//刷新主菜单的标志，当进钞口或者出钞口或者面额变化时刷新主菜单
INT8U INchangflag = 0;
INT8U OUTchangflag = 0;
INT32U DenomNum = 0;
INT8U Version[7][25] = {0x00};
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		:
* Description	: 清分设置
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 17/11/2014	songchao
***********************************************************************************************/
void InitMianMenu()
{
    INT8U circul1,circul2;

    for(circul1 = 0; circul1 < 10;circul1++)
    {
        BaseMainMenuSignInCoordinate[circul1].sXMin = MainMenuSignInCoordinate[circul1][0]*256+MainMenuSignInCoordinate[circul1][1];
        BaseMainMenuSignInCoordinate[circul1].sYMin = MainMenuSignInCoordinate[circul1][2]*256+MainMenuSignInCoordinate[circul1][3];
        BaseMainMenuSignInCoordinate[circul1].sXMax = UNVAULE;
        BaseMainMenuSignInCoordinate[circul1].sYMax = UNVAULE;
    }

    for(circul1 = 0; circul1 < 10;circul1++)
    {
        BaseMainMenuAnsactionCoordinate[circul1].sXMin = MainMenuAnsactionCoordinate[circul1][0]*256+MainMenuAnsactionCoordinate[circul1][1];
        BaseMainMenuAnsactionCoordinate[circul1].sYMin = MainMenuAnsactionCoordinate[circul1][2]*256+MainMenuAnsactionCoordinate[circul1][3];
        BaseMainMenuAnsactionCoordinate[circul1].sXMax = UNVAULE;
        BaseMainMenuAnsactionCoordinate[circul1].sYMax = UNVAULE;
    }
    
    for(circul1 = 0; circul1 < 3;circul1++)
    {
        for(circul2 = 0; circul2 < 10;circul2++)
        {
            BaseMainMenuNumCoordinate[circul1][circul2].sXMin = MainMenuNumCoordinate[circul1][circul2][0]*256+MainMenuNumCoordinate[circul1][circul2][1];
            BaseMainMenuNumCoordinate[circul1][circul2].sYMin = MainMenuNumCoordinate[circul1][circul2][2]*256+MainMenuNumCoordinate[circul1][circul2][3];
            BaseMainMenuNumCoordinate[circul1][circul2].sXMax = MainMenuNumCoordinate[circul1][circul2][4]*256+MainMenuNumCoordinate[circul1][circul2][5];
            BaseMainMenuNumCoordinate[circul1][circul2].sYMax = MainMenuNumCoordinate[circul1][circul2][6]*256+MainMenuNumCoordinate[circul1][circul2][7];
        }
    }

    for(circul1 = 0; circul1 < 2;circul1++)
    {
        BaseInletTitle[circul1].sXMin = InletTitle[circul1][0]*256+InletTitle[circul1][1];
        BaseInletTitle[circul1].sYMin = InletTitle[circul1][2]*256+InletTitle[circul1][3];
        BaseInletTitle[circul1].sXMax = InletTitle[circul1][4]*256+InletTitle[circul1][5];
        BaseInletTitle[circul1].sYMax = InletTitle[circul1][6]*256+InletTitle[circul1][7];
    }

    for(circul1 = 0; circul1 < 9;circul1++)
    {
        BaseModeIcon[circul1].sXMin = ModeIcon[circul1][0]*256+ModeIcon[circul1][1];
        BaseModeIcon[circul1].sYMin = ModeIcon[circul1][2]*256+ModeIcon[circul1][3];
        BaseModeIcon[circul1].sXMax = ModeIcon[circul1][4]*256+ModeIcon[circul1][5];
        BaseModeIcon[circul1].sYMax = ModeIcon[circul1][6]*256+ModeIcon[circul1][7];
    }

    for(circul1 = 0; circul1 < 2;circul1++)
    {
        BaseNetIcon[circul1].sXMin = NetIcon[circul1][0]*256+NetIcon[circul1][1];
        BaseNetIcon[circul1].sYMin = NetIcon[circul1][2]*256+NetIcon[circul1][3];
        BaseNetIcon[circul1].sXMax = NetIcon[circul1][4]*256+NetIcon[circul1][5];
        BaseNetIcon[circul1].sYMax = NetIcon[circul1][6]*256+NetIcon[circul1][7];
    }

    for(circul1 = 0; circul1 < 4;circul1++)
    {
        for(circul2 = 0; circul2 < 3;circul2++)
        {
            BaseOutLetCoordinate[circul1][circul2].sXMin = OutLetCoordinate[circul1][circul2][0]*256+OutLetCoordinate[circul1][circul2][1];
            BaseOutLetCoordinate[circul1][circul2].sYMin = OutLetCoordinate[circul1][circul2][2]*256+OutLetCoordinate[circul1][circul2][3];
            BaseOutLetCoordinate[circul1][circul2].sXMax = UNVAULE;
            BaseOutLetCoordinate[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 4;circul1++)
    {
        BaseTotalPage[circul1].sXMin = TotalPage[circul1][0]*256+TotalPage[circul1][1];
        BaseTotalPage[circul1].sYMin = TotalPage[circul1][2]*256+TotalPage[circul1][3];
        BaseTotalPage[circul1].sXMax = UNVAULE;
        BaseTotalPage[circul1].sYMax = UNVAULE;
    }

    for(circul1 = 0; circul1 < 7;circul1++)
    {
        BaseTotalSum[circul1].sXMin = TotalSum[circul1][0]*256+TotalSum[circul1][1];
        BaseTotalSum[circul1].sYMin = TotalSum[circul1][2]*256+TotalSum[circul1][3];
        BaseTotalSum[circul1].sXMax = UNVAULE;
        BaseTotalSum[circul1].sYMax = UNVAULE;
    }

    BaseTotalSumCoordinate.sXMin = TotalSumCoordinate[0]*256+TotalSumCoordinate[1];
    BaseTotalSumCoordinate.sYMin = TotalSumCoordinate[2]*256+TotalSumCoordinate[3];
    BaseTotalSumCoordinate.sXMax = UNVAULE;
    BaseTotalSumCoordinate.sYMax = UNVAULE;

    BaseNetCoordinate.sXMin = NetCoordinate[0]*256+NetCoordinate[1];
    BaseNetCoordinate.sYMin = NetCoordinate[2]*256+NetCoordinate[3];
    BaseNetCoordinate.sXMax = UNVAULE;
    BaseNetCoordinate.sYMax = UNVAULE;

    BaseTimeCoordinate.sXMin = TimeCoordinate[0]*256+TimeCoordinate[1];
    BaseTimeCoordinate.sYMin = TimeCoordinate[2]*256+TimeCoordinate[3];
    BaseTimeCoordinate.sXMax = UNVAULE;
    BaseTimeCoordinate.sYMax = UNVAULE;
    
    for(circul1 = 0; circul1 < 2;circul1++)
    {
        BaseMainMenuModeCoordinate[circul1].sXMin = MainMenuModeCoordinate[circul1][0]*256+MainMenuModeCoordinate[circul1][1];
        BaseMainMenuModeCoordinate[circul1].sYMin = MainMenuModeCoordinate[circul1][2]*256+MainMenuModeCoordinate[circul1][3];
        BaseMainMenuModeCoordinate[circul1].sXMax = UNVAULE;
        BaseMainMenuModeCoordinate[circul1].sYMax = UNVAULE;
    }

    for(circul1 = 0; circul1 < 7;circul1++)
    {
        for(circul2 = 0; circul2 < 10;circul2++)
        {
            BaseOutLetParam[circul1][circul2].sXMin = OutLetParam[circul1][circul2][0]*256+OutLetParam[circul1][circul2][1];
            BaseOutLetParam[circul1][circul2].sYMin = OutLetParam[circul1][circul2][2]*256+OutLetParam[circul1][circul2][3];
            BaseOutLetParam[circul1][circul2].sXMax = UNVAULE;
            BaseOutLetParam[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 10;circul1++)
    {
        for(circul2 = 0; circul2 < 10;circul2++)
        {
            BaseCheckParam[circul1][circul2].sXMin = CheckParam[circul1][circul2][0]*256+CheckParam[circul1][circul2][1];
            BaseCheckParam[circul1][circul2].sYMin = CheckParam[circul1][circul2][2]*256+CheckParam[circul1][circul2][3];
            BaseCheckParam[circul1][circul2].sXMax = UNVAULE;
            BaseCheckParam[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 10;circul1++)
    {
        for(circul2 = 0; circul2 < 12;circul2++)
        {
            BaseEditParam[circul1][circul2].sXMin = EditParam[circul1][circul2][0]*256+EditParam[circul1][circul2][1];
            BaseEditParam[circul1][circul2].sYMin = EditParam[circul1][circul2][2]*256+EditParam[circul1][circul2][3];
            BaseEditParam[circul1][circul2].sXMax = UNVAULE;
            BaseEditParam[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 4;circul1++)
    {
        BaseEditParamColor[circul1].sXMin = EditParamColor[circul1][0]*256+EditParamColor[circul1][1];
        BaseEditParamColor[circul1].sYMin = EditParamColor[circul1][2]*256+EditParamColor[circul1][3];
        BaseEditParamColor[circul1].sXMax = UNVAULE;
        BaseEditParamColor[circul1].sYMax = UNVAULE;
    }

    for(circul1 = 0; circul1 < 3;circul1++)
    {
        BaseBatchParam[circul1].sXMin = BatchParam[circul1][0]*256+BatchParam[circul1][1];
        BaseBatchParam[circul1].sYMin = BatchParam[circul1][2]*256+BatchParam[circul1][3];
        BaseBatchParam[circul1].sXMax = UNVAULE;
        BaseBatchParam[circul1].sYMax = UNVAULE;
    }

    BaseBatchParamColor.sXMin = BatchParamColor[0]*256+BatchParamColor[1];
    BaseBatchParamColor.sYMin = BatchParamColor[2]*256+BatchParamColor[3];
    BaseBatchParamColor.sXMax = UNVAULE;
    BaseBatchParamColor.sYMax = UNVAULE;

    for(circul1 = 0; circul1 < 2;circul1++)
    {
        for(circul2 = 0; circul2 < 7;circul2++)
        {
            BaseMixNumCoordinate[circul1][circul2].sXMin = MixNumCoordinate[circul1][circul2][0]*256+MixNumCoordinate[circul1][circul2][1];
            BaseMixNumCoordinate[circul1][circul2].sYMin = MixNumCoordinate[circul1][circul2][2]*256+MixNumCoordinate[circul1][circul2][3];
            BaseMixNumCoordinate[circul1][circul2].sXMax = UNVAULE;
            BaseMixNumCoordinate[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 9;circul1++)
    {
        for(circul2 = 0; circul2 < 7;circul2++)
        {
            BaseOutLetDetailCoordinate[circul1][circul2].sXMin = OutLetDetailCoordinate[circul1][circul2][0]*256+OutLetDetailCoordinate[circul1][circul2][1];
            BaseOutLetDetailCoordinate[circul1][circul2].sYMin = OutLetDetailCoordinate[circul1][circul2][2]*256+OutLetDetailCoordinate[circul1][circul2][3];
            BaseOutLetDetailCoordinate[circul1][circul2].sXMax = UNVAULE;
            BaseOutLetDetailCoordinate[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 6;circul1++)
    {
        for(circul2 = 0; circul2 < 5;circul2++)
        {
            BaseCleanLevelSettingsCoordinate[circul1][circul2].sXMin = CleanLevelSettingsCoordinate[circul1][circul2][0]*256+CleanLevelSettingsCoordinate[circul1][circul2][1];
            BaseCleanLevelSettingsCoordinate[circul1][circul2].sYMin = CleanLevelSettingsCoordinate[circul1][circul2][2]*256+CleanLevelSettingsCoordinate[circul1][circul2][3];
            BaseCleanLevelSettingsCoordinate[circul1][circul2].sXMax = UNVAULE;
            BaseCleanLevelSettingsCoordinate[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 6;circul1++)
    {
        BaseCleanLevelSettingsSelectCoordinate[circul1].sXMin = CleanLevelSettingsSelectCoordinate[circul1][0]*256 + CleanLevelSettingsSelectCoordinate[circul1][1];
        BaseCleanLevelSettingsSelectCoordinate[circul1].sYMin = CleanLevelSettingsSelectCoordinate[circul1][2]*256 + CleanLevelSettingsSelectCoordinate[circul1][3];
        BaseCleanLevelSettingsSelectCoordinate[circul1].sXMax = UNVAULE;
        BaseCleanLevelSettingsSelectCoordinate[circul1].sYMax = UNVAULE;
    }
    
    for(circul1 = 0; circul1 < 5;circul1++)
    {
        BaseCleanLevelSettingsIcon[circul1].sXMin = CleanLevelSettingsIcon[circul1][0]*256+CleanLevelSettingsIcon[circul1][1];
        BaseCleanLevelSettingsIcon[circul1].sYMin = CleanLevelSettingsIcon[circul1][2]*256+CleanLevelSettingsIcon[circul1][3];
        BaseCleanLevelSettingsIcon[circul1].sXMax = CleanLevelSettingsIcon[circul1][4]*256+CleanLevelSettingsIcon[circul1][5];
        BaseCleanLevelSettingsIcon[circul1].sYMax = CleanLevelSettingsIcon[circul1][6]*256+CleanLevelSettingsIcon[circul1][7];
    }

    for(circul1 = 0; circul1 < 7;circul1++)
    {
        for(circul2 = 0; circul2 < 5;circul2++)
        {
            BaseAuthenticationSettingsCoordinate[circul1][circul2].sXMin = AuthenticationSettingsCoordinate[circul1][circul2][0]*256+AuthenticationSettingsCoordinate[circul1][circul2][1];
            BaseAuthenticationSettingsCoordinate[circul1][circul2].sYMin = AuthenticationSettingsCoordinate[circul1][circul2][2]*256+AuthenticationSettingsCoordinate[circul1][circul2][3];
            BaseAuthenticationSettingsCoordinate[circul1][circul2].sXMax = UNVAULE;
            BaseAuthenticationSettingsCoordinate[circul1][circul2].sYMax = UNVAULE;
        }
    }
    
    for(circul1 = 0; circul1 < 7;circul1++)
    {
        BaseAuthenticationSettingsSelectCoordinate[circul1].sXMin = AuthenticationSettingsSelectCoordinate[circul1][0]*256 + AuthenticationSettingsSelectCoordinate[circul1][1];
        BaseAuthenticationSettingsSelectCoordinate[circul1].sYMin = AuthenticationSettingsSelectCoordinate[circul1][2]*256 + AuthenticationSettingsSelectCoordinate[circul1][3];
        BaseAuthenticationSettingsSelectCoordinate[circul1].sXMax = UNVAULE;
        BaseAuthenticationSettingsSelectCoordinate[circul1].sYMax = UNVAULE;

    }
    
    for(circul1 = 0; circul1 < 4;circul1++)
    {
        for(circul2 = 0; circul2 < 12;circul2++)
        {
            BaseKeyBoardSettingsCoordinate[circul1][circul2].sXMin = KeyBoardSettingsCoordinate[circul1][circul2][0]*256+KeyBoardSettingsCoordinate[circul1][circul2][1];
            BaseKeyBoardSettingsCoordinate[circul1][circul2].sYMin = KeyBoardSettingsCoordinate[circul1][circul2][2]*256+KeyBoardSettingsCoordinate[circul1][circul2][3];
            BaseKeyBoardSettingsCoordinate[circul1][circul2].sXMax = KeyBoardSettingsCoordinate[circul1+4][circul2][0]*256+KeyBoardSettingsCoordinate[circul1+4][circul2][1];
            BaseKeyBoardSettingsCoordinate[circul1][circul2].sYMax = KeyBoardSettingsCoordinate[circul1+4][circul2][2]*256+KeyBoardSettingsCoordinate[circul1+4][circul2][3];
        }
    }

    for(circul1 = 0; circul1 < 16;circul1++)
    {
        BaseSignInCoordinate[circul1].sXMin = SignInCoordinate[circul1][0]*256+SignInCoordinate[circul1][1];
        BaseSignInCoordinate[circul1].sYMin = SignInCoordinate[circul1][2]*256+SignInCoordinate[circul1][3];
        BaseSignInCoordinate[circul1].sXMax = UNVAULE;
        BaseSignInCoordinate[circul1].sYMax = UNVAULE;
    }

    for(circul1 = 0; circul1 < 7;circul1++)
    {
        BaseBlackListCoordinate[circul1].sXMin = BlackListCoordinate[circul1][0]*256+BlackListCoordinate[circul1][1];
        BaseBlackListCoordinate[circul1].sYMin = BlackListCoordinate[circul1][2]*256+BlackListCoordinate[circul1][3];
        BaseBlackListCoordinate[circul1].sXMax = BlackListCoordinate[circul1][4]*256+BlackListCoordinate[circul1][5];
        BaseBlackListCoordinate[circul1].sYMax = BlackListCoordinate[circul1][6]*256+BlackListCoordinate[circul1][7];
    }

    for(circul1 = 0; circul1 < 7;circul1++)
    {
        BaseBlackListNumberCoordinate[circul1].sXMin = BlackListNumberCoordinate[circul1][0]*256+BlackListNumberCoordinate[circul1][1];
        BaseBlackListNumberCoordinate[circul1].sYMin = BlackListNumberCoordinate[circul1][2]*256+BlackListNumberCoordinate[circul1][3];
        BaseBlackListNumberCoordinate[circul1].sXMax = UNVAULE;
        BaseBlackListNumberCoordinate[circul1].sYMax = UNVAULE;
    }
    
    for(circul1 = 0; circul1 < 3;circul1++)
    {
        for(circul2 = 0; circul2 < 7;circul2++)
        {
            BaseVersionCoordinate[circul1][circul2].sXMin = VersionCoordinate[circul1][circul2][0]*256+VersionCoordinate[circul1][circul2][1];
            BaseVersionCoordinate[circul1][circul2].sYMin = VersionCoordinate[circul1][circul2][2]*256+VersionCoordinate[circul1][circul2][3];
            BaseVersionCoordinate[circul1][circul2].sXMax = UNVAULE;
            BaseVersionCoordinate[circul1][circul2].sYMax = UNVAULE;
        }
    }

    for(circul1 = 0; circul1 < 3;circul1++)
    {
        for(circul2 = 0; circul2 < 7;circul2++)
        {
            BaseInfomationCoordinate[circul1][circul2].sXMin = InfomationCoordinate[circul1][circul2][0]*256+InfomationCoordinate[circul1][circul2][1];
            BaseInfomationCoordinate[circul1][circul2].sYMin = InfomationCoordinate[circul1][circul2][2]*256+InfomationCoordinate[circul1][circul2][3];
            BaseInfomationCoordinate[circul1][circul2].sXMax = UNVAULE;
            BaseInfomationCoordinate[circul1][circul2].sYMax = UNVAULE;
        }
    }
}

/***********************************************************************************************
* Function		: bin_to_char
* Description	: 数字转化为字符转
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 17/11/2014	songchao
***********************************************************************************************/

void bin_to_char(INT8U Dat,INT8U *Ptr)
{
    if(Dat/100)
    {
        Ptr[2] = Dat%10 + '0';
        Dat = Dat/10;
        Ptr[1] = Dat%10 + '0';
        Dat = Dat/10;
        Ptr[0] = Dat + '0';
        Ptr[3] = 0x00;
    }
    else if(Dat/10)
    {
        Ptr[1] = Dat/10 + '0';
        Dat = Dat%10;
        Ptr[2] = Dat + '0';
        Ptr[0] = ' ';
        Ptr[3] = 0x00;
    }
    else
    {
        Ptr[0] = ' ';
        Ptr[1] = ' ';
        Ptr[2] = Dat + '0';
        Ptr[3] = 0x00;
    }
}

/***********************************************************************************************
* Function		: bin_to_2char
* Description	: 数字转化为字符转（参数设置里面的2位长度的字符串）
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 17/11/2014	songchao
***********************************************************************************************/
void bin_to_2char(INT8U Dat,INT8U *Ptr)
{
    if(Dat/10)
    {
        Ptr[0] = Dat/10 + '0';
        Dat = Dat%10;
        Ptr[1] = Dat + '0';
        Ptr[2] = 0x00;
    }
    else
    {
        Ptr[0] = ' ';
        Ptr[1] = Dat + '0';
        Ptr[2] = 0x00;
    }
}

/***********************************************************************************************
* Function		: bin_to_1char
* Description	: 数字转化为字符转（参数设置里面的1位长度的字符串）
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 17/11/2014	songchao
***********************************************************************************************/
void bin_to_1char(INT8U Dat,INT8U *Ptr)
{
    Ptr[0] = Dat + '0';
    Ptr[1] = 0x00;
}

/***********************************************************************************************
* Function		: Mix_bin_to_char
* Description	: 混点明细数字转化为字符转
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 17/11/2014	songchao
***********************************************************************************************/
void Mix_bin_to_char(INT8U* Dat,INT8U *Ptr)
{
    INT64U totalnum = 0;
    totalnum = Dat[0]*256*256*256 + Dat[1]*256*256 + Dat[2]*256 + Dat[3];

    if(totalnum/100000000)
    {
        Ptr[8] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[7] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[6] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[9] = 0x00;
    }
    else if(totalnum/10000000)
    {
        Ptr[7] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[6] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[8] = 0x00;
    }
    else if(totalnum/1000000)
    {
        Ptr[6] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[7] = 0x00;
    }
    else if(totalnum/100000)
    {
        Ptr[5] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[6] = 0x00;
    }
    else if(totalnum/10000)
    {
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[5] = 0x00;
    }
    else if(totalnum/1000)
    {
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[4] = 0x00;
    }
    else if(totalnum/100)
    {
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[3] = 0x00;
    }
    else if(totalnum/10)
    {
        Ptr[0] = totalnum/10 + '0';
        totalnum = totalnum%10;
        Ptr[1] = totalnum + '0';
        Ptr[2] = 0x00;
    }
    else
    {
        Ptr[0] = totalnum + '0';
        Ptr[1] = 0x00;
    }
}

/***********************************************************************************************
* Function		: JcSpeed_bin_to_char
* Description	        : 马达速度调试
* Input			:
* Output		:
* Note(s)		:
* Contributor	        : 20/1/2015	songchao
***********************************************************************************************/
void JcSpeed_bin_to_char(INT8U* Dat,INT8U *Ptr)
{
    INT64U totalnum = 0;
    totalnum = Dat[0]*256 + Dat[1];

    if(totalnum/10000)
    {
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[5] = 0x00;
    }
    else if(totalnum/1000)
    {
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum + '0';
        Ptr[0] = ' ';
        Ptr[5] = 0x00;
    }
    else if(totalnum/100)
    {
        Ptr[4] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[3] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[2] = totalnum + '0';
        Ptr[1] = ' ';
        Ptr[0] = ' ';
        Ptr[5] = 0x00;
    }
    else if(totalnum/10)
    {
        Ptr[4] = totalnum/10 + '0';
        totalnum = totalnum%10;
        Ptr[3] = totalnum + '0';
        Ptr[2] = ' ';
        Ptr[1] = ' ';
        Ptr[0] = ' ';
        Ptr[5] = 0x00;
    }
    else
    {
        Ptr[0] = ' ';
        Ptr[1] = ' ';
        Ptr[2] = ' ';
        Ptr[3] = ' ';
        Ptr[4] = totalnum + '0';
        Ptr[5] = 0x00;
    }
}

/***********************************************************************************************
* Function		: JcPWM_bin_to_char
* Description	        : 马达PWM调试
* Input			:
* Output		:
* Note(s)		:
* Contributor	        : 20/1/2015	songchao
***********************************************************************************************/
void JcPWM_bin_to_char(INT8U* Dat,INT8U *Ptr)
{
    INT64U totalnum = 0;
    totalnum =Dat[0];

    if(totalnum/100)
    {
        Ptr[2] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[1] = totalnum%10 + '0';
        totalnum = totalnum/10;
        Ptr[0] = totalnum + '0';
        Ptr[3] = 0x00;
    }
    else if(totalnum/10)
    {
        Ptr[0] = ' ';
        Ptr[1] = totalnum/10 + '0';
        totalnum = totalnum%10;
        Ptr[2] = totalnum + '0';
        Ptr[3] = 0x00;
    }
    else
    {
        Ptr[0] = ' ';
        Ptr[1] = ' ';
        Ptr[2] = totalnum + '0';
        Ptr[3] = 0x00;
    }
}

/********************************************************************
* Function Name : PraperMianMenu()
* Description   : 主菜单显示函数
* Input         : -*pMenu,当前待显菜单结构.
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void PraperMianMenu(void)
{
    //累加模式的开得情况下显示累计金额，不开的情况下显示接钞金额
    if(SUMMATIONMODE == SUMMATIONOFF)
    {
        printIcon(IMAGEICON2,AMOUNTMODE01,AMOUNTCOORDINATE);
    }
    else
    {
        printIcon(IMAGEICON2,AMOUNTMODE02,AMOUNTCOORDINATE);
    }

    //存取款模式
    if(DEPWITHDRAWMODE == DEPOSITMODE)
    {
      printIcon(IMAGEICON2,MAINMODEICON02,MAINMODECOORDINATE02);
    }
    else
    {
      printIcon(IMAGEICON2,MAINMODEICON03,MAINMODECOORDINATE02);
    }

    //清分模式
    switch(CLEARINGMODE)
    {
        case INTELLIGENTMODE:
            printIcon(IMAGEICON2,MAINMODEICON01,MAINMODECOORDINATE01);
            break;
        case MIXEDMODE:
            printIcon(IMAGEICON2,MAINMODEICON09,MAINMODECOORDINATE01);
            break;
        case EDITIONMODE:
            printIcon(IMAGEICON2,MAINMODEICON08,MAINMODECOORDINATE01);
            break;
        case ATMMODE:
            printIcon(IMAGEICON2,MAINMODEICON07,MAINMODECOORDINATE01);
            break;
        case COMPLEXMODE:
            printIcon(IMAGEICON2,MAINMODEICON06,MAINMODECOORDINATE01);
            break;
        case COUNTMODE:
            printIcon(IMAGEICON2,MAINMODEICON05,MAINMODECOORDINATE01);
            break;
    }

    if(NETFLAG == NETABL)
    {
        printIcon(IMAGEICON2,NETABLE,NETCOORDINATE);
    }
    else
    {
        printIcon(IMAGEICON2,NETUNABLE,NETCOORDINATE);
    }

	//设置字体颜色
	setcolor(0xFFFF,0x0000);
	
    //表格
    printMainMenuTimeStrings32(TIMECOORDINATE,"2015.3.3 12:01");
    
    MianMenuSignInDisp();
    MianMenuAnsactionDisp();
}

/********************************************************************
* Function Name : MianMenuTotalNumDisp()
* Description   : 主菜单总张数显示
* Input         : -*pMenu,当前待显菜单结构.
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuTotalNumDisp(void)
{
    INT64U totalnum = 0;

    totalnum = MainMenuDisp[TOTALNUM1BYTE]*256*256*256 + MainMenuDisp[TOTALNUM2BYTE]*256*256 + MainMenuDisp[TOTALNUM3BYTE]*256 +MainMenuDisp[TOTALNUM4BYTE];

    if(totalnum>=9999)
    {
        printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE04);
        printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE03);
        printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE02);
        printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE01);
    }
    else
    {
        switch(totalnum%10)
        {
            case 0x00:
                printIcon(IMAGEICON1,TOTALNUM00,TOTALNUMCOORDINATE04);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALNUM01,TOTALNUMCOORDINATE04);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALNUM02,TOTALNUMCOORDINATE04);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALNUM03,TOTALNUMCOORDINATE04);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALNUM04,TOTALNUMCOORDINATE04);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALNUM05,TOTALNUMCOORDINATE04);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALNUM06,TOTALNUMCOORDINATE04);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALNUM07,TOTALNUMCOORDINATE04);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALNUM08,TOTALNUMCOORDINATE04);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE04);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALNUM00,TOTALNUMCOORDINATE03);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALNUM01,TOTALNUMCOORDINATE03);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALNUM02,TOTALNUMCOORDINATE03);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALNUM03,TOTALNUMCOORDINATE03);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALNUM04,TOTALNUMCOORDINATE03);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALNUM05,TOTALNUMCOORDINATE03);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALNUM06,TOTALNUMCOORDINATE03);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALNUM07,TOTALNUMCOORDINATE03);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALNUM08,TOTALNUMCOORDINATE03);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE03);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALNUM00,TOTALNUMCOORDINATE02);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALNUM01,TOTALNUMCOORDINATE02);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALNUM02,TOTALNUMCOORDINATE02);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALNUM03,TOTALNUMCOORDINATE02);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALNUM04,TOTALNUMCOORDINATE02);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALNUM05,TOTALNUMCOORDINATE02);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALNUM06,TOTALNUMCOORDINATE02);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALNUM07,TOTALNUMCOORDINATE02);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALNUM08,TOTALNUMCOORDINATE02);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE02);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                //printIcon(IMAGEICON1,TOTALNUM00,TOTALNUMCOORDINATE01);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALNUM01,TOTALNUMCOORDINATE01);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALNUM02,TOTALNUMCOORDINATE01);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALNUM03,TOTALNUMCOORDINATE01);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALNUM04,TOTALNUMCOORDINATE01);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALNUM05,TOTALNUMCOORDINATE01);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALNUM06,TOTALNUMCOORDINATE01);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALNUM07,TOTALNUMCOORDINATE01);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALNUM08,TOTALNUMCOORDINATE01);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALNUM09,TOTALNUMCOORDINATE01);
                break;
        }
    }
}


/********************************************************************
* Function Name : MianMenuDenomNumDisp()
* Description   : 主菜单面额显示
* Input         : -*pMenu,当前待显菜单结构.
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuDenomNumDisp(void)
{
    INT32U totalnum = 0;

    totalnum = MainMenuDisp[DENOMHIGHBYTE]*256 + MainMenuDisp[DENOMLOWBYTE];

    DenomNum = totalnum;

    if(totalnum>=999)
    {
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALDENOMCOORDINATE03);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALDENOMCOORDINATE02);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALDENOMCOORDINATE01);
    }
    else
    {
        switch(totalnum%10)
        {
            case 0x00:
                printIcon(IMAGEICON1,TOTALOUTLET00,TOTALDENOMCOORDINATE03);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALDENOMCOORDINATE03);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALDENOMCOORDINATE03);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALDENOMCOORDINATE03);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALDENOMCOORDINATE03);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALDENOMCOORDINATE03);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALDENOMCOORDINATE03);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALDENOMCOORDINATE03);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALDENOMCOORDINATE03);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALDENOMCOORDINATE03);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALOUTLET00,TOTALDENOMCOORDINATE02);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALDENOMCOORDINATE02);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALDENOMCOORDINATE02);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALDENOMCOORDINATE02);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALDENOMCOORDINATE02);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALDENOMCOORDINATE02);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALDENOMCOORDINATE02);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALDENOMCOORDINATE02);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALDENOMCOORDINATE02);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALDENOMCOORDINATE02);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                //printIcon(IMAGEICON1,TOTALOUTLET00,TOTALDENOMCOORDINATE01);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALDENOMCOORDINATE01);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALDENOMCOORDINATE01);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALDENOMCOORDINATE01);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALDENOMCOORDINATE01);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALDENOMCOORDINATE01);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALDENOMCOORDINATE01);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALDENOMCOORDINATE01);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALDENOMCOORDINATE01);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALDENOMCOORDINATE01);
                break;
        }
    }
}

/********************************************************************
* Function Name : MianMenuBatchNumDisp()
* Description   : 主菜单批量显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuBatchNumDisp(void)
{
    INT32U totalnum = 0;

    totalnum = BatchNum;

    if(totalnum>=999)
    {
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALBATCHCOORDINATE03);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALBATCHCOORDINATE02);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALBATCHCOORDINATE01);
    }
    else
    {
        switch(totalnum%10)
        {
            case 0x00:
                printIcon(IMAGEICON1,TOTALOUTLET00,TOTALBATCHCOORDINATE03);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALBATCHCOORDINATE03);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALBATCHCOORDINATE03);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALBATCHCOORDINATE03);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALBATCHCOORDINATE03);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALBATCHCOORDINATE03);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALBATCHCOORDINATE03);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALBATCHCOORDINATE03);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALBATCHCOORDINATE03);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALBATCHCOORDINATE03);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALOUTLET00,TOTALBATCHCOORDINATE02);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALBATCHCOORDINATE02);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALBATCHCOORDINATE02);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALBATCHCOORDINATE02);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALBATCHCOORDINATE02);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALBATCHCOORDINATE02);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALBATCHCOORDINATE02);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALBATCHCOORDINATE02);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALBATCHCOORDINATE02);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALBATCHCOORDINATE02);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALBATCHCOORDINATE01);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALBATCHCOORDINATE01);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALBATCHCOORDINATE01);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALBATCHCOORDINATE01);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALBATCHCOORDINATE01);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALBATCHCOORDINATE01);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALBATCHCOORDINATE01);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALBATCHCOORDINATE01);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALBATCHCOORDINATE01);
                break;
        }
    }
}

/********************************************************************
* Function Name : MianMenuOutletNumDisp()
* Description   : 主菜单退钞口显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuOutletNumDisp(void)
{
    INT32U totalnum = 0;

    totalnum = MainMenuDisp[OUTLETHIGHBYTE]*256 + MainMenuDisp[OUTLETLOWBYTE];

    if(totalnum)OUTchangflag = 1;

    if(totalnum>=999)
    {
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALOUTLETCOORDINATE03);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALOUTLETCOORDINATE02);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALOUTLETCOORDINATE01);
    }
    else
    {
        switch(totalnum%10)
        {
            case 0x00:
                printIcon(IMAGEICON1,TOTALOUTLET00,TOTALOUTLETCOORDINATE03);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALOUTLETCOORDINATE03);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALOUTLETCOORDINATE03);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALOUTLETCOORDINATE03);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALOUTLETCOORDINATE03);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALOUTLETCOORDINATE03);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALOUTLETCOORDINATE03);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALOUTLETCOORDINATE03);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALOUTLETCOORDINATE03);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALOUTLETCOORDINATE03);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALOUTLET00,TOTALOUTLETCOORDINATE02);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALOUTLETCOORDINATE02);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALOUTLETCOORDINATE02);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALOUTLETCOORDINATE02);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALOUTLETCOORDINATE02);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALOUTLETCOORDINATE02);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALOUTLETCOORDINATE02);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALOUTLETCOORDINATE02);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALOUTLETCOORDINATE02);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALOUTLETCOORDINATE02);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALOUTLETCOORDINATE01);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALOUTLETCOORDINATE01);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALOUTLETCOORDINATE01);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALOUTLETCOORDINATE01);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALOUTLETCOORDINATE01);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALOUTLETCOORDINATE01);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALOUTLETCOORDINATE01);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALOUTLETCOORDINATE01);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALOUTLETCOORDINATE01);
                break;
        }
    }
}

/********************************************************************
* Function Name : MianMenuInletNumDisp()
* Description   : 主菜单接钞口显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuInletNumDisp(void)
{
    INT32U totalnum = 0;

    totalnum = MainMenuDisp[INLETHIGHBYTE]*256 + MainMenuDisp[INLETLOWBYTE];

    LEDTestFunction(totalnum);
        
    if(totalnum)INchangflag = 1;

    if(totalnum>=999)
    {
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALINLETCOORDINATE03);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALINLETCOORDINATE02);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALINLETCOORDINATE01);
    }
    else
    {
        switch(totalnum%10)
        {
            case 0x00:
                printIcon(IMAGEICON1,TOTALOUTLET00,TOTALINLETCOORDINATE03);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALINLETCOORDINATE03);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALINLETCOORDINATE03);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALINLETCOORDINATE03);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALINLETCOORDINATE03);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALINLETCOORDINATE03);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALINLETCOORDINATE03);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALINLETCOORDINATE03);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALINLETCOORDINATE03);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALINLETCOORDINATE03);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALOUTLET00,TOTALINLETCOORDINATE02);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALINLETCOORDINATE02);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALINLETCOORDINATE02);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALINLETCOORDINATE02);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALINLETCOORDINATE02);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALINLETCOORDINATE02);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALINLETCOORDINATE02);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALINLETCOORDINATE02);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALINLETCOORDINATE02);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALINLETCOORDINATE02);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALOUTLET01,TOTALINLETCOORDINATE01);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALOUTLET02,TOTALINLETCOORDINATE01);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALOUTLET03,TOTALINLETCOORDINATE01);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALOUTLET04,TOTALINLETCOORDINATE01);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALOUTLET05,TOTALINLETCOORDINATE01);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALOUTLET06,TOTALINLETCOORDINATE01);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALOUTLET07,TOTALINLETCOORDINATE01);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALOUTLET08,TOTALINLETCOORDINATE01);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALOUTLET09,TOTALINLETCOORDINATE01);
                break;
        }
    }
}

/********************************************************************
* Function Name : MianMenuAmountNumDisp()
* Description   : 主菜单的总金额或累计金额的显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuAmountNumDisp(void)
{
    INT64U totalnum = 0;

    totalnum = MainMenuDisp[AMOUNTHIGHBYTE]*256*256 + MainMenuDisp[AMOUNTMIDDLEBYTE]*256 + MainMenuDisp[AMOUNTLOWBYTE];

    if(totalnum>=9999999)
    {
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALSUMCOORDINATE07);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALSUMCOORDINATE06);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALSUMCOORDINATE05);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALSUMCOORDINATE04);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALSUMCOORDINATE03);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALSUMCOORDINATE02);
        printIcon(IMAGEICON1,TOTALOUTLET09,TOTALSUMCOORDINATE01);
    }
    else
    {
        switch(totalnum%10)
        {
            case 0x00:
                printIcon(IMAGEICON1,TOTALSUM00,TOTALSUMCOORDINATE07);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALSUM01,TOTALSUMCOORDINATE07);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALSUM02,TOTALSUMCOORDINATE07);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALSUM03,TOTALSUMCOORDINATE07);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALSUM04,TOTALSUMCOORDINATE07);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALSUM05,TOTALSUMCOORDINATE07);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALSUM06,TOTALSUMCOORDINATE07);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALSUM07,TOTALSUMCOORDINATE07);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALSUM08,TOTALSUMCOORDINATE07);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALSUM09,TOTALSUMCOORDINATE07);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALSUM00,TOTALSUMCOORDINATE06);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALSUM01,TOTALSUMCOORDINATE06);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALSUM02,TOTALSUMCOORDINATE06);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALSUM03,TOTALSUMCOORDINATE06);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALSUM04,TOTALSUMCOORDINATE06);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALSUM05,TOTALSUMCOORDINATE06);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALSUM06,TOTALSUMCOORDINATE06);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALSUM07,TOTALSUMCOORDINATE06);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALSUM08,TOTALSUMCOORDINATE06);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALSUM09,TOTALSUMCOORDINATE06);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALSUM00,TOTALSUMCOORDINATE05);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALSUM01,TOTALSUMCOORDINATE05);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALSUM02,TOTALSUMCOORDINATE05);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALSUM03,TOTALSUMCOORDINATE05);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALSUM04,TOTALSUMCOORDINATE05);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALSUM05,TOTALSUMCOORDINATE05);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALSUM06,TOTALSUMCOORDINATE05);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALSUM07,TOTALSUMCOORDINATE05);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALSUM08,TOTALSUMCOORDINATE05);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALSUM09,TOTALSUMCOORDINATE05);
                break;
        }
        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALSUM00,TOTALSUMCOORDINATE04);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALSUM01,TOTALSUMCOORDINATE04);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALSUM02,TOTALSUMCOORDINATE04);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALSUM03,TOTALSUMCOORDINATE04);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALSUM04,TOTALSUMCOORDINATE04);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALSUM05,TOTALSUMCOORDINATE04);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALSUM06,TOTALSUMCOORDINATE04);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALSUM07,TOTALSUMCOORDINATE04);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALSUM08,TOTALSUMCOORDINATE04);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALSUM09,TOTALSUMCOORDINATE04);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALSUM00,TOTALSUMCOORDINATE03);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALSUM01,TOTALSUMCOORDINATE03);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALSUM02,TOTALSUMCOORDINATE03);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALSUM03,TOTALSUMCOORDINATE03);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALSUM04,TOTALSUMCOORDINATE03);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALSUM05,TOTALSUMCOORDINATE03);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALSUM06,TOTALSUMCOORDINATE03);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALSUM07,TOTALSUMCOORDINATE03);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALSUM08,TOTALSUMCOORDINATE03);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALSUM09,TOTALSUMCOORDINATE03);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALSUM00,TOTALSUMCOORDINATE02);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALSUM01,TOTALSUMCOORDINATE02);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALSUM02,TOTALSUMCOORDINATE02);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALSUM03,TOTALSUMCOORDINATE02);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALSUM04,TOTALSUMCOORDINATE02);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALSUM05,TOTALSUMCOORDINATE02);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALSUM06,TOTALSUMCOORDINATE02);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALSUM07,TOTALSUMCOORDINATE02);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALSUM08,TOTALSUMCOORDINATE02);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALSUM09,TOTALSUMCOORDINATE02);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALSUM01,TOTALSUMCOORDINATE01);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALSUM02,TOTALSUMCOORDINATE01);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALSUM03,TOTALSUMCOORDINATE01);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALSUM04,TOTALSUMCOORDINATE01);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALSUM05,TOTALSUMCOORDINATE01);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALSUM06,TOTALSUMCOORDINATE01);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALSUM07,TOTALSUMCOORDINATE01);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALSUM08,TOTALSUMCOORDINATE01);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALSUM09,TOTALSUMCOORDINATE01);
                break;
        }
    }
}

/********************************************************************
* Function Name : MianMenuSignInDisp()
* Description   : 主菜单操作员签到菜单
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuSignInDisp(void)
{
    INT8U i = 0;

    setcolor(0xFFFF,0x0000);
    while(i<10)
    {
        printStrings32(BaseMainMenuSignInCoordinate[i],&SignIn[i]);
        i++;
    }
}

/********************************************************************
* Function Name : MianMenuAnsactionDisp()
* Description   : 主菜单交易号显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MianMenuAnsactionDisp(void)
{
    INT8U i = 0;

    setcolor(0xFFFF,0x0000);
    while(i<10)
    {
        printStrings32(BaseMainMenuAnsactionCoordinate[i],&AnsactionNum[i]);
        i++;
    }
}

/********************************************************************
* Function Name : BatchMenuNumDisp()
* Description   : 批量张数显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void BatchMenuNumDisp(void)
{
    INT32U totalnum = 0;
    totalnum = TemBatchNum;

    printBackImage(gps_CurMenu->FrameID);

    if(totalnum>=999)
    {
        printIcon(IMAGEICON1,TOTALNUM09,BaseBatchParam[2]);
        printIcon(IMAGEICON1,TOTALNUM09,BaseBatchParam[1]);
        printIcon(IMAGEICON1,TOTALNUM09,BaseBatchParam[0]);
    }
    else
    {
        switch(totalnum%10)
        {
            case 0x00:
                printIcon(IMAGEICON1,TOTALNUM00,BaseBatchParam[2]);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALNUM01,BaseBatchParam[2]);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALNUM02,BaseBatchParam[2]);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALNUM03,BaseBatchParam[2]);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALNUM04,BaseBatchParam[2]);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALNUM05,BaseBatchParam[2]);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALNUM06,BaseBatchParam[2]);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALNUM07,BaseBatchParam[2]);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALNUM08,BaseBatchParam[2]);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALNUM09,BaseBatchParam[2]);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                if(totalnum>9)printIcon(IMAGEICON1,TOTALNUM00,BaseBatchParam[1]);
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALNUM01,BaseBatchParam[1]);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALNUM02,BaseBatchParam[1]);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALNUM03,BaseBatchParam[1]);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALNUM04,BaseBatchParam[1]);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALNUM05,BaseBatchParam[1]);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALNUM06,BaseBatchParam[1]);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALNUM07,BaseBatchParam[1]);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALNUM08,BaseBatchParam[1]);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALNUM09,BaseBatchParam[1]);
                break;
        }

        totalnum = totalnum/10;
        switch(totalnum%10)
        {
            case 0x00:
                break;
            case 0x01:
                printIcon(IMAGEICON1,TOTALNUM01,BaseBatchParam[0]);
                break;
            case 0x02:
                printIcon(IMAGEICON1,TOTALNUM02,BaseBatchParam[0]);
                break;
            case 0x03:
                printIcon(IMAGEICON1,TOTALNUM03,BaseBatchParam[0]);
                break;
            case 0x04:
                printIcon(IMAGEICON1,TOTALNUM04,BaseBatchParam[0]);
                break;
            case 0x05:
                printIcon(IMAGEICON1,TOTALNUM05,BaseBatchParam[0]);
                break;
            case 0x06:
                printIcon(IMAGEICON1,TOTALNUM06,BaseBatchParam[0]);
                break;
            case 0x07:
                printIcon(IMAGEICON1,TOTALNUM07,BaseBatchParam[0]);
                break;
            case 0x08:
                printIcon(IMAGEICON1,TOTALNUM08,BaseBatchParam[0]);
                break;
            case 0x09:
                printIcon(IMAGEICON1,TOTALNUM09,BaseBatchParam[0]);
                break;
        }
        totalnum = totalnum/10;
    }
}

/********************************************************************
* Function Name : MixMenuNumDisp()
* Description   : 混点明细菜单内容显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MixMenuNumDisp(void)
{
    INT32U mixtotalpage = 0;
    INT32U MoneyNum = 0;
    INT8U TempNum[4] = {0};
    INT8U i;

    memcpy(TempNum+2,MixedDisp+6,2);
    FetchBackColor(EDITPARAMCOLOR01);

    printMixNums16(BaseMixNumCoordinate[0][5],TempNum);

    memcpy(TempNum+2,MixedDisp+8,2);
    printMixNums16(BaseMixNumCoordinate[0][4],TempNum);

    memcpy(TempNum+2,MixedDisp+10,2);
    printMixNums16(BaseMixNumCoordinate[0][3],TempNum);

    memcpy(TempNum+2,MixedDisp+12,2);
    printMixNums16(BaseMixNumCoordinate[0][2],TempNum);

    memcpy(TempNum+2,MixedDisp+14,2);
    printMixNums16(BaseMixNumCoordinate[0][1],TempNum);

    memcpy(TempNum+2,MixedDisp+16,2);
    printMixNums16(BaseMixNumCoordinate[0][0],TempNum);

    for(i=0;i<9;i++)
    {
        mixtotalpage += MixedDisp[i*2]*256 + MixedDisp[i*2+1];
    }
    TempNum[3] = mixtotalpage&0xFF;
    TempNum[2] = ((mixtotalpage>>8)&0xFF);
    TempNum[1] = ((mixtotalpage>>16)&0xFF);
    TempNum[0] = ((mixtotalpage>>24&0xFF));

    printMixNums16(BaseMixNumCoordinate[0][6],TempNum);

    MoneyNum = ((MixedDisp[6]*256 + MixedDisp[7])*100+(MixedDisp[8]*256 + MixedDisp[9])*50+(MixedDisp[10]*256 + MixedDisp[11])*20+(MixedDisp[12]*256 + MixedDisp[13])*10+(MixedDisp[14]*256 + MixedDisp[15])*5\
                +(MixedDisp[16]*256 + MixedDisp[17])*1);
    TempNum[3] = MoneyNum&0xFF;
    TempNum[2] = ((MoneyNum>>8)&0xFF);
    TempNum[1] = ((MoneyNum>>16)&0xFF);
    TempNum[0] = ((MoneyNum>>24&0xFF));
    printMixNums16(BaseMixNumCoordinate[1][6],TempNum);

    MoneyNum = ((MixedDisp[6]*256 + MixedDisp[7])*100);
    TempNum[3] = MoneyNum&0xFF;
    TempNum[2] = ((MoneyNum>>8)&0xFF);
    TempNum[1] = ((MoneyNum>>16)&0xFF);
    TempNum[0] = ((MoneyNum>>24&0xFF));
    printMixNums16(BaseMixNumCoordinate[1][5],TempNum);

    MoneyNum = ((MixedDisp[8]*256 + MixedDisp[9])*50);
    TempNum[3] = MoneyNum&0xFF;
    TempNum[2] = ((MoneyNum>>8)&0xFF);
    TempNum[1] = ((MoneyNum>>16)&0xFF);
    TempNum[0] = ((MoneyNum>>24&0xFF));
    printMixNums16(BaseMixNumCoordinate[1][4],TempNum);

    MoneyNum = ((MixedDisp[10]*256 + MixedDisp[11])*20);
    TempNum[3] = MoneyNum&0xFF;
    TempNum[2] = ((MoneyNum>>8)&0xFF);
    TempNum[1] = ((MoneyNum>>16)&0xFF);
    TempNum[0] = ((MoneyNum>>24&0xFF));
    printMixNums16(BaseMixNumCoordinate[1][3],TempNum);

    MoneyNum = ((MixedDisp[12]*256 + MixedDisp[13])*10);
    TempNum[3] = MoneyNum&0xFF;
    TempNum[2] = ((MoneyNum>>8)&0xFF);
    TempNum[1] = ((MoneyNum>>16)&0xFF);
    TempNum[0] = ((MoneyNum>>24&0xFF));
    printMixNums16(BaseMixNumCoordinate[1][2],TempNum);

    MoneyNum = ((MixedDisp[14]*256 + MixedDisp[15])*5);
    TempNum[3] = MoneyNum&0xFF;
    TempNum[2] = ((MoneyNum>>8)&0xFF);
    TempNum[1] = ((MoneyNum>>16)&0xFF);
    TempNum[0] = ((MoneyNum>>24&0xFF));
    printMixNums16(BaseMixNumCoordinate[1][1],TempNum);

    MoneyNum = ((MixedDisp[16]*256 + MixedDisp[17]));
    TempNum[3] = MoneyNum&0xFF;
    TempNum[2] = ((MoneyNum>>8)&0xFF);
    TempNum[1] = ((MoneyNum>>16)&0xFF);
    TempNum[0] = ((MoneyNum>>24&0xFF));
    printMixNums16(BaseMixNumCoordinate[1][0],TempNum);
}

/********************************************************************
* Function Name : OutLetDetailDisp()
* Description   : 退钞口菜单内容显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void OutLetDetailDisp(void)
{
    INT8U k;
    INT8U ListNum = 0;
    
    FetchBackColor(EDITPARAMCOLOR01);

    for(k = 0;k < 9;k++)
    {
        if(OUTLETDisp[(OutLetKeyPage*9)+k][0])
        {
//            ListNum[0] = (OutLetKeyPage*9+k)>>8;
//            ListNum[1] = (OutLetKeyPage*9+k)&0xFF;
            printStrings16(BaseOutLetDetailCoordinate[k][0],&ListNum);
            printStrings16(BaseOutLetDetailCoordinate[k][1],&OUTLETDisp[(OutLetKeyPage*9)+k][0]);
            printStrings16(BaseOutLetDetailCoordinate[k][2],&OUTLETDisp[(OutLetKeyPage*9)+k][0]+1);
            printStrings16(BaseOutLetDetailCoordinate[k][3],&OUTLETDisp[(OutLetKeyPage*9)+k][0]+2);
            printOutLetValueStrings16(BaseOutLetDetailCoordinate[k][4],&OUTLETDisp[(OutLetKeyPage*9)+k][0]+3);
            printCrownWordStrings16(BaseOutLetDetailCoordinate[k][5],&OUTLETDisp[(OutLetKeyPage*9)+k][0]+7);
            printStrings16(BaseOutLetDetailCoordinate[k][6],&OUTLETDisp[(OutLetKeyPage*9)+k][0]+5);
        }
    }
}

/********************************************************************
* Function Name : OutLetDetailDisp()
* Description   : 接钞口菜单内容显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void InLetDetailDisp(void)
{    
    INT8U k;
    INT8U ListNum = 0;
    
    FetchBackColor(EDITPARAMCOLOR01);

    for(k = 0;k < 9;k++)
    {
        if(INLETDisp[(InLetKeyPage*9)+k][0])
        {
//            ListNum[0] = (OutLetKeyPage*9+k)>>8;
//            ListNum[1] = (OutLetKeyPage*9+k)&0xFF;
            printStrings16(BaseOutLetDetailCoordinate[k][0],&ListNum);
            printStrings16(BaseOutLetDetailCoordinate[k][1],&INLETDisp[(InLetKeyPage*9)+k][0]);
            printStrings16(BaseOutLetDetailCoordinate[k][2],&INLETDisp[(InLetKeyPage*9)+k][0]+1);
            printStrings16(BaseOutLetDetailCoordinate[k][3],&INLETDisp[(InLetKeyPage*9)+k][0]+2);
            printOutLetValueStrings16(BaseOutLetDetailCoordinate[k][4],&INLETDisp[(InLetKeyPage*9)+k][0]+3);
            printCrownWordStrings16(BaseOutLetDetailCoordinate[k][5],&INLETDisp[(InLetKeyPage*9)+k][0]+7);
            printStrings16(BaseOutLetDetailCoordinate[k][6],&INLETDisp[(InLetKeyPage*9)+k][0]+5);
        }
    }
}

/********************************************************************
* Function Name : CleanLevelDisp()
* Description   : 清分等级菜单内容显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void CleanLevelDisp(void)
{
    printBackImage(gps_CurMenu->FrameID);
    switch(TemClearLevel[0])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseCleanLevelSettingsCoordinate[0][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseCleanLevelSettingsCoordinate[0][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[0][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseCleanLevelSettingsCoordinate[0][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseCleanLevelSettingsCoordinate[0][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[0][2]);
            break;
    }
    switch(TemClearLevel[1])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseCleanLevelSettingsCoordinate[1][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseCleanLevelSettingsCoordinate[1][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[1][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseCleanLevelSettingsCoordinate[1][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseCleanLevelSettingsCoordinate[1][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[1][2]);
            break;
    }
    switch(TemClearLevel[2])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseCleanLevelSettingsCoordinate[2][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseCleanLevelSettingsCoordinate[2][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[2][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseCleanLevelSettingsCoordinate[2][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseCleanLevelSettingsCoordinate[2][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[2][2]);
            break;
    }
    switch(TemClearLevel[3])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseCleanLevelSettingsCoordinate[3][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseCleanLevelSettingsCoordinate[3][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[3][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseCleanLevelSettingsCoordinate[3][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseCleanLevelSettingsCoordinate[3][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[3][2]);
            break;
    }
    switch(TemClearLevel[4])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseCleanLevelSettingsCoordinate[4][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseCleanLevelSettingsCoordinate[4][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[4][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseCleanLevelSettingsCoordinate[4][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseCleanLevelSettingsCoordinate[4][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[4][2]);
            break;
    }
    switch(TemClearLevel[5])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseCleanLevelSettingsCoordinate[5][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseCleanLevelSettingsCoordinate[5][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[5][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseCleanLevelSettingsCoordinate[5][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseCleanLevelSettingsCoordinate[5][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseCleanLevelSettingsCoordinate[5][2]);
            break;
    }
}

/********************************************************************
* Function Name : AuthenticationLevelDisp()
* Description   : 鉴伪等级菜单内容显示
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void AuthenticationLevelDisp(void)
{
    printBackImage(gps_CurMenu->FrameID);
    switch(TemAuthenticationLevel[0])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseAuthenticationSettingsCoordinate[0][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseAuthenticationSettingsCoordinate[0][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[0][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseAuthenticationSettingsCoordinate[0][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseAuthenticationSettingsCoordinate[0][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[0][2]);
            break;
    }
    switch(TemAuthenticationLevel[1])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseAuthenticationSettingsCoordinate[1][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseAuthenticationSettingsCoordinate[1][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[1][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseAuthenticationSettingsCoordinate[1][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseAuthenticationSettingsCoordinate[1][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[1][2]);
            break;
    }
    switch(TemAuthenticationLevel[2])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseAuthenticationSettingsCoordinate[2][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseAuthenticationSettingsCoordinate[2][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[2][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseAuthenticationSettingsCoordinate[2][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseAuthenticationSettingsCoordinate[2][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[2][2]);
            break;
    }
    switch(TemAuthenticationLevel[3])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseAuthenticationSettingsCoordinate[3][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseAuthenticationSettingsCoordinate[3][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[3][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseAuthenticationSettingsCoordinate[3][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseAuthenticationSettingsCoordinate[3][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[3][2]);
            break;
    }
    switch(TemAuthenticationLevel[4])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseAuthenticationSettingsCoordinate[4][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseAuthenticationSettingsCoordinate[4][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[4][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseAuthenticationSettingsCoordinate[4][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseAuthenticationSettingsCoordinate[4][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[4][2]);
            break;
    }
    switch(TemAuthenticationLevel[5])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseAuthenticationSettingsCoordinate[5][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseAuthenticationSettingsCoordinate[5][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[5][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseAuthenticationSettingsCoordinate[5][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseAuthenticationSettingsCoordinate[5][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[5][2]);
            break;
    }
    switch(TemAuthenticationLevel[6])
    {
        case 0x01:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[0],BaseAuthenticationSettingsCoordinate[6][0]);
            break;
        case 0x02:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[1],BaseAuthenticationSettingsCoordinate[6][1]);
            break;
        case 0x03:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[6][2]);
            break;
        case 0x04:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[3],BaseAuthenticationSettingsCoordinate[6][3]);
            break;
        case 0x05:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[4],BaseAuthenticationSettingsCoordinate[6][4]);
            break;
        default:
            printIcon(IMAGEICON2,BaseCleanLevelSettingsIcon[2],BaseAuthenticationSettingsCoordinate[6][2]);
            break;
    }
}

/********************************************************************
* Function Name : KeyBoardDispNum()
* Description   : 显示键盘菜单的原始值
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void KeyBoardDispNum(INT8U *DispNum)
{
    INT8U i = 0;

    setcolor(0xFFFF,0x0000);
    while(i<12)
    {
        printStrings32(BaseSignInCoordinate[i],&DispNum[i]);
        i++;
    }
}

/********************************************************************
* Function Name : KeyBoardDispSignInNum()
* Description   : 显示签到的信息
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void KeyBoardDispSignInNum(INT8U *DispNum)
{
    INT8U i = 0;

    setcolor(0xFFFF,0x0000);
    while(i<10)
    {
        printStrings32(BaseSignInCoordinate[i],&DispNum[i]);
        i++;
    }
}

/********************************************************************
* Function Name : VersionDisp()
* Description   : 显示版本信息
* Input         : INT8U *Version
* Output        : None.
* Return        : None.
********************************************************************/
void VersionDisp(void)
{
    INT8U i = 0;

    char ver_info[30]={0};

    //F400.UMB01.U81.1501.00.03

    getVersion(ver_info);
    memcpy(&Version[6][0],&ver_info[7],18);

    setcolor(0xFFFF,0x0000);
    while(i<7)
    {
        printVersionStrings32(BaseVersionCoordinate[1][i],Version[i]);
        i++;
    }
}

/********************************************************************
* Function Name : OutLetDetailDisp()
* Description   : 马达数据
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void JcSpeedDispMenu(void)
{
    JcSpeedprints(0x55,180,32,JcSpeedDisp);
    JcPwmprints(0x55,294,32,JcSpeedDisp+2);

    JcSpeedprints(0x55,180,76,JcSpeedDisp+3);
    JcPwmprints(0x55,294,76,JcSpeedDisp+5);
}

/********************************************************************
* Function Name : InLetADDispMenu()
* Description   : 进出钞AD采样
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void InLetADDispMenu(void)
{
    INT8U i = 0;
    
    for(i = 0;i<5;i++)
    {
        JcPwmprints(0x55,100,(32+i*44),(InLetADDisp+i));

        JcPwmprints(0x55,200,(32+i*44),(InLetADDisp+i+5));
        
        JcPwmprints(0x55,300,(32+i*44),(InLetADDisp+i+10));
    }
}

/********************************************************************
* Function Name : IRADDispMenu()
* Description   : 红外AD采样
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void IRADDispMenu(void)
{  
    //采样值
    JcPwmprints(0x55,170,32,(IRADDisp));

    JcPwmprints(0x55,170,72,(IRADDisp+1));
    
    JcPwmprints(0x55,170,112,(IRADDisp+2));

    JcPwmprints(0x55,170,152,(IRADDisp+3));
     
    JcPwmprints(0x55,170,192,(IRADDisp+4));

    JcPwmprints(0x55,170,232,(IRADDisp+5));
       
    JcPwmprints(0x55,410,32,(IRADDisp+6));

    JcPwmprints(0x55,410,72,(IRADDisp+7));
        
    JcPwmprints(0x55,410,112,(IRADDisp+8));
       
    JcPwmprints(0x55,410,152,(IRADDisp+9));

    //原始值
    JcPwmprints(0x55,80,32,(IRADDisp+15));

    JcPwmprints(0x55,80,72,(IRADDisp+16));
    
    JcPwmprints(0x55,80,112,(IRADDisp+17));

    JcPwmprints(0x55,80,152,(IRADDisp+18));
     
    JcPwmprints(0x55,80,192,(IRADDisp+19));

    JcPwmprints(0x55,80,232,(IRADDisp+20));
       
    JcPwmprints(0x55,320,32,(IRADDisp+21));

    JcPwmprints(0x55,320,72,(IRADDisp+22));
        
    JcPwmprints(0x55,320,112,(IRADDisp+23));
       
    JcPwmprints(0x55,320,152,(IRADDisp+24));
}

/********************************************************************
* Function Name : MTADDispMenu()
* Description   : 大小磁头
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void MTADDispMenu(void)
{  
    //采样值
    JcPwmprints(0x54,170,32,(MTDisp));

    JcPwmprints(0x54,170,52,(MTDisp+1));
    
    JcPwmprints(0x54,170,72,(MTDisp+2));

    JcPwmprints(0x54,170,92,(MTDisp+3));
     
    JcPwmprints(0x54,170,112,(MTDisp+4));

    JcPwmprints(0x54,170,132,(MTDisp+5));
       
    JcPwmprints(0x54,170,152,(MTDisp+6));

    JcPwmprints(0x54,170,172,(MTDisp+7));
        

    //原始值
    JcPwmprints(0x54,100,32,(MTDisp+10));

    JcPwmprints(0x54,100,52,(MTDisp+11));
    
    JcPwmprints(0x54,100,72,(MTDisp+12));

    JcPwmprints(0x54,100,92,(MTDisp+13));
     
    JcPwmprints(0x54,100,112,(MTDisp+14));

    JcPwmprints(0x54,100,132,(MTDisp+15));
       
    JcPwmprints(0x54,100,152,(MTDisp+16));

    JcPwmprints(0x54,100,172,(MTDisp+17));
        
}

/********************************************************************
* Function Name : UVADDispMenu()
* Description   : 荧光采样
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void UVADDispMenu(void)
{  
    //采样值
    JcPwmprints(0x54,100,32,(UVDisp));

    JcPwmprints(0x54,100,72,(UVDisp+1));
    
    JcPwmprints(0x54,100,112,(UVDisp+2));

    JcPwmprints(0x54,100,152,(UVDisp+3));
     
    JcPwmprints(0x54,100,192,(UVDisp+4));

    JcPwmprints(0x54,100,232,(UVDisp+5));
           

    //原始值
    JcPwmprints(0x54,170,32,(UVDisp+10));

    JcPwmprints(0x54,170,72,(UVDisp+11));
    
    JcPwmprints(0x54,170,112,(UVDisp+12));

    JcPwmprints(0x54,170,152,(UVDisp+13));
     
    JcPwmprints(0x54,170,192,(UVDisp+14));

    JcPwmprints(0x54,170,232,(UVDisp+15));      
        
}

/********************************************************************
* Function Name : InfoIRADDispMenu()
* Description   : 走钞IR
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void InfoIRADDispMenu(void)
{  
    //采样值
    JcPwmprints(0x54,52,40,(InfoIRDisp));

    JcPwmprints(0x54,101,40,(&InfoIRDisp[1]));
    
    JcPwmprints(0x54,150,40,(&InfoIRDisp[2]));

    JcPwmprints(0x54,200,40,(&InfoIRDisp[3]));
     
    JcPwmprints(0x54,250,40,(&InfoIRDisp[4]));

    JcPwmprints(0x54,300,40,(&InfoIRDisp[5]));
       
    JcPwmprints(0x54,350,40,(&InfoIRDisp[6]));
                
}

/********************************************************************
* Function Name : InfoMTADDispMenu()
* Description   : 走钞MT
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void InfoMTADDispMenu(void)
{  
    INT8U i = 1;
    //采样值
    JcPwmprints(0x54,40,50,(&InfoMTDisp[0]));

    while(i<7)
    {
        if(InfoMTDisp[i]!=0)
        {
            JcPwmprints(0x54,240,50,(&InfoMTDisp[i]));
        }
        i++;
    }                
}

/********************************************************************
* Function Name : InfoMGADDispMenu()
* Description   : 走钞MG
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void InfoMGADDispMenu(void)
{  
    INT8U i = 0;

    for(i=0;i<7;i++)
    {            
            JcPwmprints(0x54,80+50*i,72,(&InfoMGDisp[0][i*7]));

            JcPwmprints(0x54,80+50*i,92,(&InfoMGDisp[1][i*7]));
             
            JcPwmprints(0x54,80+50*i,112,(&InfoMGDisp[2][i*7]));

            JcPwmprints(0x54,80+50*i,132,(&InfoMGDisp[3][i*7]));
               
    }
}


/********************************************************************
* Function Name : InfoMTWaveformDispMenu()
* Description   : MT波形数据查看
* Input         :
* Output        : None.
* Return        : None.
********************************************************************/
void InfoMTWaveformDispMenu(void)
{  
    INT8U i,j = 0;

    for(i=0;i<7;i++)
    {            
        for(j=0;j<8;j++)
        {
            JcPwmprints(0x54,65+50*i,82+j*30,(&MTwaveform[i*7+j]));
        }
    }
}

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/
