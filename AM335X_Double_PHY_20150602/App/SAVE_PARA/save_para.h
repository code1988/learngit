/***************�ļ���Ϣ********************************************************************************
**��   ��   ��: save_para.h
**˵        ��:
**��   ��   ��: hxj
**��   �� ����: 2015-1-9 15:59
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
//����ͨ�ò��ֵ�SPI FLASH�ĵ�ַ��Ŀǰ�ݶ���2K��
//=======================================================================================================================
//def add ��0��ַ��ʼ��2K����,��ַ�����,�����������޸�(Ϊ���Ժ�ĳ�����������)
#define ADS_RES                         0x0000          //��һ����ַ����                    // 1 byte

#define ADS_LOCAL_IP                    0x0001          //IP��ַ                            // 4 byte
#define ADS_LOCAL_MK                    0x0005          //mask                              // 4 byte
#define ADS_LOCAL_GW                    0x0009          //GW                                // 4 byte
#define ADS_LOCAL_MAC                   0x000D          //MAC                               // 6 byte
#define ADS_LOCAL_PORT                  0x0013          //���ض˿�                          // 2 byte
#define ADS_M_IP                        0x0015          //Զ��IP                            // 4 byte
#define ADS_M_PORT                      0x0019          //Զ�˶˿�                          // 2 byte
//res
#define ADS_LOCAL_RES_ADD               0x001B          //����                              // 21 byte

//-----------------------------------------------------------------------------------------------------------------------
#define ADS_NET_SWITCH                  0x0030          //��������                          // 1 byte
#define ADS_NET_PROTOCOL_TYPE           0x0031          //��������                          // 1 byte
#define ADS_NET_PROTOCOL_MODE           0x0032          //����ģʽ                          // 1 byte
#define ADS_NET_STOP_TIME               0x0033          //������ʱ��                        // 40 byte
//res
#define ADS_NET_RES_ADD                 0x005B          //����                              // 21 byte
//-----------------------------------------------------------------------------------------------------------------------

#define ADS_SD_REMAIN_SIZE_THR          0x0070          //SD��ʣ�������澯ֵ                //  2 byte (����1���ֽ�)
#define ADS_SD_ALARM_EN                 0x0072          //SD��ʣ�������澯ʹ��              //  1 byte
#define ADS_SD_ALARM_TIME               0x0073          //SD��ʣ�������澯����              //  1 byte
#define ADS_SD_SEND_OK_DAY              0x0074          //SD�����ͳɹ����ļ���������        //  4 byte
#define ADS_SD_SEND_ERR_DAY             0x0078          //SD������ʧ�ܵ��ļ���������        //  4 byte

//res
#define ADS_SD_RES_ADD                  0x007C          //����                              //  32 byte
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

