/***************文件信息********************************************************************************
**文   件   名: save_para.c
**说        明:
**创   建   人: hxj
**创   建 日期: 2015-1-9 15:59
*******************************************************************************************************/
#include "save_para.h"
#include "def_config.h"
#include "bsp_flash.h"




#if FZ2000
#if 0==DBG_SAVE_A8_PARA_USE_ADD_EN

/****************************************************************************************************
**名称:int flash_para_opt_read(uint16 index_id,uint8 *buf)
**功能:通过id,读参数
* 入口:无
* 出口:成功返回1,出错返回0
**auth:hxj, date: 2015-1-30 16:39
*****************************************************************************************************/
int flash_para_opt_read(uint16 index_id,uint8 *buf)
{
    return 0;

}


/****************************************************************************************************
**名称:int flash_para_opt_write(uint16 index_id,uint8 *buf)
**功能:通过id,写参数
* 入口:无
* 出口:成功返回1,出错返回0
**auth:hxj, date: 2015-1-30 16:39
*****************************************************************************************************/
int flash_para_opt_write(uint16 index_id,uint8 *buf)
{
    return 0;

}

#endif
#endif











int write_eeprom_nbyte ( int addr, uint8 *value, int len )
{
    if(NULL==value) return 0;

    #if 0==DBG_NOT_SAVE_PARA_EN
        if(0==BSP_WriteDataToAT(addr,value,len))
        {
            return 0;
        }
    #endif

    return len;
}



int write_eeprom_1byte ( int addr, uint8 value )
{
	int retval;
	retval = write_eeprom_nbyte ( addr, &value, 1 );
	return retval;
}

int write_eeprom_1byte_p ( int addr, uint8 *value )
{
	int retval;
	retval = write_eeprom_nbyte ( addr, value, 1 );
	return retval;
}


int write_eeprom_2byte ( int addr, uint16 value )
{
	int retval;
	retval = write_eeprom_nbyte ( addr, ( unsigned char * ) &value, 2 );
	return retval;
}


int write_eeprom_2byte_p ( int addr, uint8 *value )
{
	int retval;
	retval = write_eeprom_nbyte ( addr,value, 2 );
	return retval;
}



int write_eeprom_4byte ( int addr, uint32 value )
{
	int retval;
    retval = write_eeprom_nbyte (addr, (unsigned char *)&value, 4 );
	return retval;
}



int write_eeprom_4byte_p ( int addr, uint8 *value )
{
	int retval;
    retval = write_eeprom_nbyte (addr,value, 4 );
	return retval;
}




int read_eeprom_nbyte ( int addr, uint8 *value, int len )
{
    if(NULL==value) return 0;

    #if 0==DBG_NOT_SAVE_PARA_EN
        if(0 == BSP_ReadDataFromAT(addr,value,len))
        {
            return 0;
        }
    #endif

	return len;

}


int read_eeprom_1byte ( int addr, uint8 *value )
{
	int retval;
	retval = read_eeprom_nbyte ( addr, value, 1 );

	return retval;
}

int read_eeprom_2byte ( int addr, uint8 *value )
{
	int retval;
	retval = read_eeprom_nbyte ( addr, value, 2 );

	return retval;
}

int read_eeprom_4byte ( int addr, uint8 *value )
{
	int retval;
	retval = read_eeprom_nbyte ( addr, value, 4 );

	return retval;
}



























/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

