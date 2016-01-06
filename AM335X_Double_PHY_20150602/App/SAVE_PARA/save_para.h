/***************文件信息********************************************************************************
**文   件   名: save_para.h
**说        明:
**创   建   人: hxj
**创   建 日期: 2015-1-9 15:59
*******************************************************************************************************/
#ifndef	__INC_SAVE_PARA_H
#define	__INC_SAVE_PARA_H


#include "def_config.h"
#ifdef FZ1500
#include "fLashaddrsconfig.h"
#endif




extern int flash_para_opt_read(uint16 index_id,uint8 *buf);
extern int flash_para_opt_write(uint16 index_id,uint8 *buf);



//=======================================================================================================================
//网络通用部分的SPI FLASH的地址（目前暂定用2K）
//=======================================================================================================================
//def add 用0地址开始的2K长度,地址定义后,不可以随意修改(为了以后的程序升级兼容)
#define ADS_RES                         0x0000          //第一个地址保留                    // 1 byte

#define ADS_LOCAL_IP                    0x0001          //IP地址                            // 4 byte
#define ADS_LOCAL_MK                    0x0005          //mask                              // 4 byte
#define ADS_LOCAL_GW                    0x0009          //GW                                // 4 byte
#define ADS_LOCAL_MAC                   0x000D          //MAC                               // 6 byte
#define ADS_LOCAL_PORT                  0x0013          //本地端口                          // 2 byte
#define ADS_M_IP                        0x0015          //远端IP                            // 4 byte
#define ADS_M_PORT                      0x0019          //远端端口                          // 2 byte
//res
#define ADS_LOCAL_RES_ADD               0x001B          //保留                              // 21 byte

//-----------------------------------------------------------------------------------------------------------------------
#define ADS_NET_SWITCH                  0x0030          //网发开关                          // 1 byte
#define ADS_NET_PROTOCOL_TYPE           0x0031          //网发类型                          // 1 byte
#define ADS_NET_PROTOCOL_MODE           0x0032          //网发模式                          // 1 byte
#define ADS_NET_STOP_TIME               0x0033          //不网发时段                        // 40 byte
//res
#define ADS_NET_RES_ADD                 0x005B          //保留                              // 21 byte
//-----------------------------------------------------------------------------------------------------------------------

#define ADS_SD_REMAIN_SIZE_THR          0x0070          //SD卡剩余容量告警值                //  2 byte (保留1个字节)
#define ADS_SD_ALARM_EN                 0x0072          //SD卡剩余容量告警使能              //  1 byte
#define ADS_SD_ALARM_TIME               0x0073          //SD卡剩余容量告警周期              //  1 byte
#define ADS_SD_SEND_OK_DAY              0x0074          //SD卡发送成功的文件保留天数        //  4 byte
#define ADS_SD_SEND_ERR_DAY             0x0078          //SD卡发送失败的文件保留天数        //  4 byte

//res
#define ADS_SD_RES_ADD                  0x007C          //保留                              //  32 byte
//-----------------------------------------------------------------------------------------------------------------------




extern int write_eeprom_nbyte ( int addr, uint8 *value, int len );
extern int write_eeprom_1byte ( int addr, uint8 value );
extern int write_eeprom_2byte ( int addr, uint16 value );
extern int write_eeprom_4byte ( int addr, uint32 value );

extern int write_eeprom_1byte_p ( int addr, uint8 *value );
extern int write_eeprom_2byte_p ( int addr, uint8 *value );
extern int write_eeprom_4byte_p ( int addr, uint8 *value );

extern int read_eeprom_nbyte ( int addr, uint8 *value, int len );
extern int read_eeprom_1byte ( int addr,uint8 *value );
extern int read_eeprom_2byte ( int addr, uint8 *value );
extern int read_eeprom_4byte ( int addr, uint8 *value );









#endif
/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

