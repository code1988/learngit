#ifndef _ET850_DATA_H_
#define _ET850_DATA_H_


#include "utility.h"

/********************************************************************************/
/*						消息分发管理接口										*/
/********************************************************************************/
#define SYSMSG_NET1_STATUS_ID			0x1000
#define SYSMSG_NET2_STATUS_ID			0x1001
#define SYSMSG_NET1_MAC_ID				0x1002
#define SYSMSG_NET2_MAC_ID				0x1003

#define SysPostMessage(a, b)			do {}while(0)
/********************************************************************************/
/* $describer:			黑名单管理接口											*/
/* $file				black.c													*/
/********************************************************************************/
status_t blacklist_init(void);
u32_t black_count_get(s32_t flag);
status_t black_insert(s8_t *black, s32_t flag);
status_t black_delete(s8_t *black);
status_t blacklist_clear(s32_t flag);
status_t blacklist_insert(s8_t *ptr, u32_t size, s32_t flag);
status_t blacklist_delete(s8_t *ptr, u32_t size);
status_t blacklist_update(s8_t *ptr, u32_t size, s32_t flag);
u32_t black_select(s8_t *ptr, u32_t start, u32_t size, s32_t flag);
/********************************************************************************/
/* $describer:			用户管理接口											*/
/* $file				user.c													*/
/********************************************************************************/
status_t active_user_get(s8_t *user, s32_t *permission);
status_t active_user_set(s8_t *user, s32_t permission);
status_t userlist_init(void);
u32_t user_count_get(void);
status_t user_insert(s8_t *user, s8_t *password, s32_t permission);
status_t user_delete(s8_t *user);
status_t userlist_clear(void);
status_t userlist_insert(s8_t *ptr, u32_t size);
status_t userlist_delete(s8_t *ptr, u32_t size);
status_t user_check(s8_t *user, s8_t *password);
s32_t user_select_get(s8_t *ptr, u32_t start, s32_t size);
/********************************************************************************/
/* $describer:			MCU配置参数管理接口										*/
/* $file				mcu_config.c											*/
/********************************************************************************/
#define MCU_CONFIG_FLUORESCENCE1_INDEX			0x1100
#define MCU_CONFIG_COLORINK1_INDEX				0x1101
#define MCU_CONFIG_PASTE1_INDEX					0x1102
#define MCU_CONFIG_TRICKNESS1_INDEX				0x1103
#define MCU_CONFIG_100TOGETHER1_INDEX			0x1104
#define MCU_CONFIG_50TOGETHER1_INDEX			0x1105
#define MCU_CONFIG_MIDMAGNETIC1_INDEX			0x1106
#define MCU_CONFIG_SIDEMAGNETIC1_INDEX			0x1107
#define MCU_CONFIG_IMAGE1_INDEX					0x1108
#define MCU_CONFIG_FLUORESCENCE2_INDEX			0x1200
#define MCU_CONFIG_COLORINK2_INDEX				0x1201
#define MCU_CONFIG_PASTE2_INDEX					0x1202
#define MCU_CONFIG_TRICKNESS2_INDEX				0x1203
#define MCU_CONFIG_100TOGETHER2_INDEX			0x1204
#define MCU_CONFIG_50TOGETHER2_INDEX			0x1205
#define MCU_CONFIG_MIDMAGNETIC2_INDEX			0x1206
#define MCU_CONFIG_SIDEMAGNETIC2_INDEX			0x1207
#define MCU_CONFIG_IMAGE2_INDEX					0x1208
#define MCU_CONFIG_FLUORESCENCE3_INDEX			0x1300
#define MCU_CONFIG_COLORINK3_INDEX				0x1301
#define MCU_CONFIG_PASTE3_INDEX					0x1302
#define MCU_CONFIG_TRICKNESS3_INDEX				0x1303
#define MCU_CONFIG_100TOGETHER3_INDEX			0x1304
#define MCU_CONFIG_50TOGETHER3_INDEX			0x1305
#define MCU_CONFIG_MIDMAGNETIC3_INDEX			0x1306
#define MCU_CONFIG_SIDEMAGNETIC3_INDEX			0x1307
#define MCU_CONFIG_IMAGE3_INDEX					0x1308
#define MCU_CONFIG_FLUORESCENCE4_INDEX			0x1400
#define MCU_CONFIG_COLORINK4_INDEX				0x1401
#define MCU_CONFIG_PASTE4_INDEX					0x1402
#define MCU_CONFIG_TRICKNESS4_INDEX				0x1403
#define MCU_CONFIG_100TOGETHER4_INDEX			0x1404
#define MCU_CONFIG_50TOGETHER4_INDEX			0x1405
#define MCU_CONFIG_MIDMAGNETIC4_INDEX			0x1406
#define MCU_CONFIG_SIDEMAGNETIC4_INDEX			0x1407
#define MCU_CONFIG_IMAGE4_INDEX					0x1408
#define MCU_CONFIG_AUDIO_INDEX					0xA100
#define MCU_CONFIG_OLD_INDEX					0xA101
#define MCU_CONFIG_100MAGNETIC_INDEX			0xA102
#define MCU_CONFIG_50MAGNETIC_INDEX				0xA103
#define MCU_CONFIG_FALSEBILL_INDEX				0xA104
#define MCU_CONFIG_LASTSAVE_INDEX				0xA105
#define MCU_CONFIG_ALERTAUTO_INDEX				0xA106
#define MCU_CONFIG_ALERTKEEP_INDEX				0xA107
#define MCU_CONFIG_PRESET100_INDEX				0xA108
#define MCU_CONFIG_SDCARDFIRST_INDEX			0xA109
#define MCU_CONFIG_NOPROPAGANDA_INDEX			0xA10A
#define MCU_CONFIG_1DOLLOR_INDEX				0xA10B
#define MCU_CONFIG_LOGIN_INDEX					0xA10C
#define MCU_CONFIG_STANDBY_INDEX				0xF100
#define MCU_CONFIG_FACTORY_INDEX				0xF101
#define MCU_CONFIG_DAMAGED_INDEX				0xF102
typedef struct tagMcuParamSetS {
	u32_t			key;
	u32_t			type;
	s8_t			title[32];
	s8_t			vice0[16];
	s8_t			vice1[16];
	u8_t			value;
	u8_t			min;
	u8_t			max;
}MCU_PARAMSET_S;
status_t mcu_config_init(void);
status_t mcu_param_set(u32_t key, u8_t value);
status_t mcu_param_get(u32_t key, u8_t *value);
status_t mcu_param_save(void);
status_t mcu_param_getex(u32_t key, MCU_PARAMSET_S *h);
/********************************************************************************/
/* $describer:			MCU运行参数管理接口										*/
/* $file				param.c													*/
/********************************************************************************/
#define BUSINESS_TYPE_MIN		0
#define BUSINESS_TYPE_MAX		2
/* 0：为智能 1：混点 2：计数 3：版别清分 4：套别清分 5：残钞清分 */
#define FUNCTION_TYPE_MIN		0
#define FUNCTION_TYPE_MAX		5

#define SENSITIVE_LEVEL_MIN		1
#define SENSITIVE_LEVEL_MAX		4
#define PRESET_VALUE_MIN		-1
#define PRESET_VALUE_MAX		9999

void mculink_status_set(bool_t status);
bool_t mculink_status_get(void);
void business_type_set(u16_t type);
u16_t business_type_get(void);
void function_type_set(u16_t type);
u16_t function_type_get(void);
void sensitive_level_set(u16_t level);
u16_t sensitive_level_get(void);
void preset_value_set(s16_t value);
s16_t preset_value_get(void);
status_t device_name_get(s8_t *name);
status_t device_name_set(s8_t *name);
status_t device_organize_get(s8_t *branch, s8_t *branch_net);
status_t device_organize_set(s8_t *branch, s8_t *branch_net);
status_t fluorescence_param_save(void);
status_t spi_keyalert(void);
//status_t dsp_reset(void);
//status_t spi_warn_clear(void);

/********************************************************************************/
/* $describer:			网络参数接口											*/
/* $file				network.c												*/
/********************************************************************************/
status_t net1_interface_get(s32_t *type, s32_t *ip, s32_t *mask, s32_t *gate);
status_t net1_interface_save(s32_t type, s32_t ip, s32_t mask, s32_t gate);
void net1_status_set(bool_t flag);
bool_t net1_status_get(void);
void net1_status_check(void);
status_t net1_mac_set(s8_t *mac);
status_t net1_mac_get(s8_t *mac);

status_t net2_interface_get(s32_t *type, s32_t *ip, s32_t *mask, s32_t *gate);
status_t net2_interface_save(s32_t type, s32_t ip, s32_t mask, s32_t gate);
void net2_status_set(bool_t flag);
bool_t net2_status_get(void);
void net2_status_check(void);
status_t net2_mac_set(s8_t *mac);
status_t net2_mac_get(s8_t *mac);

status_t tcpdsp_cntl_port_set(u16_t port);
status_t tcpdsp_cntl_port_get(u16_t *port);
status_t tcpdsp_data_port_set(u16_t port);
status_t tcpdsp_data_port_get(u16_t *port);
status_t tcpmonitor_cntl_port_set(u16_t port);
status_t tcpmonitor_cntl_port_get(u16_t *port);
status_t tcpmonitor_data_port_set(u16_t port);
status_t tcpmonitor_data_port_get(u16_t *port);
status_t tcpfsn_upload_port_set(s32_t ip, u16_t port);
status_t tcpfsn_upload_port_get(s32_t *ip, u16_t *port);

/********************************************************************************/
/* $describer:			SD卡接口												*/
/* $file				sdcard.c												*/
/********************************************************************************/
bool_t sdcard_meminfo_get(u32_t *total, u32_t *free);
void sdcard_check(void);

/********************************************************************************/
/* $describer:			纸币显示接口											*/
/* $file				moneydisp.c												*/
/********************************************************************************/
typedef struct tagMoneyDispS {
	u16_t		curpages;
	u16_t		prepages;
	u32_t		money_sum;
	s8_t		crown[16];
	u8_t		errcode;
	u8_t		enomination;
	u8_t		reserve[2];
	u16_t		errnum;
	u32_t		Valuta[6];
	u32_t		total_sum;
}MONEYDISP_S;

status_t moneydisp_init(void);
status_t moneydisp_info_set(u8_t *ptr);
status_t moneydisp_info_get(MONEYDISP_S *ptr);
status_t moneydisp_info_clear(void);
bool_t moneydisp_info_status(void);

/********************************************************************************/
/* $describer:			错误信息接口											*/
/* $file				errorcode.c												*/
/********************************************************************************/
void deverror_id_set(u8_t id);
u8_t deverror_id_get(void);
s8_t* deverror_str_get(u8_t id);

void moneyerror_id_set(u8_t id);
u8_t moneyerror_id_get(void);
s8_t* moneyerror_str_get(u8_t id);

status_t mcu_upgrade_create(s8_t *filename);
status_t mcu_upgrade_destroy(void);
status_t mcu_upgrade_request(void);
status_t mcu_upgrade_start(void);
status_t mcu_upgrade_data(u32_t block);
status_t mcu_upgrade_end(void);
status_t mcu_upgrade_failed(u32_t sum);
s32_t mcu_upgrade_status_get(void);

/********************************************************************************/
/* $describer:			静态调试接口											*/
/* $file				debugstatic.c											*/
/********************************************************************************/
typedef struct tagStaticDebugInfoS {
	u8_t		ir[8];
	u8_t		tape[6];
	u8_t		fluore[4];
	u8_t		magnetic[4];
	u16_t		encode;
}STATICDEBUG_S;
status_t debugstatic_init(void);
status_t debugstatic_release(void);
status_t debugstatic_open(void);
status_t debugstatic_close(void);
status_t debugstatic_write(u8_t *ptr, s32_t size);
u32_t debugstatic_read(u8_t *ptr, u32_t start, u32_t size);
u32_t debugstatic_count_get(void);

/********************************************************************************/
/* $describer:			动态调试接口											*/
/* $file				debugdynamic.c											*/
/********************************************************************************/
typedef struct tagDynamicDebugInfoS {
	u8_t		pulse[6];
	u8_t		ir[6];
	u8_t		count[2];
	u8_t		tape[6];
	u8_t		magnetic[4];
	u8_t		face;
	u8_t		version;
	u16_t		width;
}DYNAMICDEBUG_S;
status_t debugdynamic_init(void);
status_t debugdynamic_release(void);
status_t debugdynamic_open(void);
status_t debugdynamic_close(void);
status_t debugdynamic_write(u8_t *ptr, s32_t size);
u32_t debugdynamic_read(u8_t *ptr, u32_t start, u32_t size);
u32_t debugdynamic_count_get(void);


/********************************************************************************/
/* $describer:			AD采样调试接口											*/
/* $file				debugadsamp.c											*/
/********************************************************************************/
typedef struct tagADSampDebugInfoS {
	u8_t		data[23];
}ADSAMPDEBUG_S;
status_t debugadsamp_init(void);
status_t debugadsamp_release(void);
status_t debugadsamp_open(void);
status_t debugadsamp_close(void);
status_t debugadsamp_write(u8_t *ptr, s32_t size);
u32_t debugadsamp_read(u8_t *ptr, u32_t start, u32_t size);
u32_t debugadsamp_count_get(void);

/********************************************************************************/
/* $describer:			AD采样调试接口											*/
/* $file				imageinfo.c												*/
/********************************************************************************/
status_t imageinfo_init(void);
status_t imageinfo_release(void);
status_t imageinfo_open(void);
status_t imageinfo_close(void);
status_t imageinfo_write(u8_t *ptr);
status_t imageinfo_read(u8_t *ptr);
status_t imageinfo_refresh(void);

/********************************************************************************/
/* $describer:			mcu_dsp通信调试管理接口									*/
/* $file				imagecom.c												*/
/********************************************************************************/
typedef struct tagMcuDspCommS {
	u32_t				id;
	u16_t				cmd;
	bool_t				direct;
	u8_t				data[100];
	u32_t				size;
}MCU_DSP_COMM_S;
status_t mcudsp_comm_init(void);
status_t mcudsp_comm_release(void);
status_t mcudsp_comm_open(void);
status_t mcudsp_comm_close(void);
status_t mcudsp_comm_data_write(u16_t cmd,  bool_t direct, u8_t *ptr, u32_t size);
u32_t mcudsp_comm_data_read(u8_t *ptr, u32_t start, u32_t size);
u32_t mcudsp_comm_count_get(void);

/********************************************************************************/
/* $describer:			版本管理接口											*/
/* $file				version.c												*/
/********************************************************************************/
void arm_hardware_version_get(s8_t *version);
void arm_hardware_version_set(s8_t *version);
void arm_software_version_get(s8_t *version);
void arm_software_version_set(s8_t *version);
void mcu_hardware_version_get(s8_t *version);
void mcu_hardware_version_set(s8_t *version);
void mcu_software_version_get(s8_t *version);
void mcu_software_version_set(s8_t *version);
void dsp_hardware_version_get(s8_t *version);
void dsp_hardware_version_set(s8_t *version);
void dsp_software_version_get(s8_t *version);
void dsp_software_version_set(s8_t *version);
void dsp_fpga_version_get(s8_t *version);
void dsp_fpga_version_set(s8_t *version);
void dsp_module_version_get(s8_t *version);
void dsp_module_version_set(s8_t *version);
void dsp_param_version_get(s8_t *version);
void dsp_param_version_set(s8_t *version);

/********************************************************************************/
/*						SPI通信接口函数											*/
/* $file				spi.c													*/
/********************************************************************************/
status_t spi_protocol_init(void);
status_t spi_protocol_release(void);
status_t mcu_comm_send(u16_t cmd, u8_t *ptr, u16_t len);
/********************************************************************************/
/*						SPI解析接口函数											*/
/* $file				analyze.c												*/
/********************************************************************************/
status_t mcu_param_init(void);
status_t data_analyze(u16_t cmd, u8_t *buf, u16_t size);
/********************************************************************************/
/*						fsn消息接口函数											*/
/* $file				fsnmessage.c											*/
/********************************************************************************/
status_t fsn_message_init(void);
status_t fsn_message_release(void);
status_t fsn_message_send(s8_t *ptr, u32_t size);

/********************************************************************************/
/*						fsn数据库管理接口函数									*/
/* $file				fsn.c													*/
/********************************************************************************/
typedef struct tagFSNHeaderS {
	u16_t		HeadStart[4];
	u16_t		HeadString[6];
	u32_t		Counter;
	u16_t		HeadEnd[4];
}FSNHEAD_S;

typedef struct tagTIMGSNODATA {
	u32_t		Data[32];
}TIMGSNODATA;

typedef struct tagTIMAGESNO {
	u16_t		Num;
	u16_t		height;
	u16_t		width;
	u16_t		Reserves;
	TIMGSNODATA	SNo[12];
}TIMAGESNO;

typedef struct tagFSN_S {
	u16_t		Date;
	u16_t		Time;
	u16_t		tfFlag;
	u16_t		ErrorCode[3];
	u16_t		MoneyFlag[4];
	u16_t		Ver;
	u16_t		Valuta;
	u16_t		CharNum;
	u16_t		SNo[12];
	u16_t		MachineSNo[24];
	u16_t		Reservel;
	TIMAGESNO	ImageSNo;
}FSN_S;

typedef struct tagFSNUnitS {
	s32_t				id;
	s8_t				direct[64];
	s8_t				filename[128];
	s8_t				username[16];
	s32_t				upload;
	s32_t				write;
	s32_t				datetime;
}FSN_UNIT_S;

/********************************************************************************/
/*						fsn单位纸币管理接口函数									*/
/* $file				fsnunit.c												*/
/********************************************************************************/
FSN_S* fsnunit_create(u8_t *ptr);
status_t fsnunit_destroy(FSN_S *hFSN);
u32_t fsnunit_count_get(s8_t *direct, s8_t *filename);
status_t fsnunit_write(s8_t *direct, s8_t *filename, FSN_S *hFSN);
u32_t fsnunit_read_bydown(s8_t *direct, s8_t *filename, FSN_S *hFSN, u32_t start, u32_t size);
u32_t fsnunit_read_byup(s8_t *direct, s8_t *filename, u8_t *ptr, u32_t start, u32_t size);

status_t fsnfile_init(void);
status_t fsndb_init(void);
status_t fsnfile_create(s8_t *direct, s8_t *filename);
status_t fsnfile_destroy(s8_t *filename);
status_t fsnfile_insert(s8_t *direct, s8_t *filename, s8_t *username);
status_t fsnfile_delete(u32_t id);
u32_t fsnfile_count_get(s32_t upload, s32_t write);
u32_t fsnfile_read(FSN_UNIT_S *ptr, u32_t start, u32_t size, s32_t upload, s32_t write);
status_t fsnfile_write_get(FSN_UNIT_S *hFSN);

/********************************************************************************/
/*						图形比较参数接口函数									*/
/* $file				fsnunit.c												*/
/********************************************************************************/
status_t image_identify_init(void);
status_t image_identify_release(void);
u8_t* image_identify_ptr_get(u8_t *h, u8_t level, u8_t ver, u8_t type, u8_t dir);
u8_t* image_identify_load(void);
status_t image_identify_unload(u8_t *h);
status_t image_identify_save(u8_t *h);

/********************************************************************************/
/*						fota升级接口											*/
/* $file				fota/upgrade.c											*/
/********************************************************************************/
#define UPGRADE_NULL_STATUS			-1
#define UPGRADE_START_STATUS		0
#define UPGRADE_SEND_STATUS			1
#define UPGRADE_WAIT_STATUS			2
#define UPGRADE_END_STATUS			3
#define UPGRADE_BURN_STATUS			4
#define UPGRADE_CHECK_STATUS		5
#define UPGRADE_FINISH_STATUS		6
status_t fota_upgrade_init(void);
status_t fota_upgrade_release(void);
status_t fota_upgrade_create(u8_t type, s8_t *filename, void *func);
status_t fota_upgrade_destroy(void);
status_t fota_upgrade_set(s32_t status, s32_t errorcode);
s32_t fota_upgrade_status(s32_t* errorcode, u8_t *percent);

/********************************************************************************/
/*						图像校验接口											*/
/* $file				fota/image_verification.c								*/
/********************************************************************************/
#define VERIFICATION_NULL_STATUS		-1
#define VERIFICATION_REQUEST_STATUS		0
#define VERIFICATION_START_STATUS		1
#define VERIFICATION_STEP1_STATUS		2
#define VERIFICATION_STEP2_STATUS		3
#define VERIFICATION_FINISH_STATUS		4
#define VERIFICATION_FAILED_STATUS		5
status_t image_verification_start(void);
status_t image_verification_set(s32_t status);
s32_t image_verfication_get(void);


/********************************************************************************/
/*						图像通信命令接口										*/
/* $file				fota/cmdsend.c											*/
/********************************************************************************/
status_t fota_image_identify_request(s32_t valid, u8_t *ptr, s32_t size);

/********************************************************************************/
/*						未整理函数												*/
/* $file																		*/
/********************************************************************************/
status_t FotaBlackIndication(void);

#define MCU_NORMAL_MODE				0
#define MCU_BURN_IN_MODE			1
#define MCU_STATIC_DEBUG_MODE		2
#define MCU_DYNAMIC_DEBUG_MODE		3

status_t mcu_mode_set(s32_t mode);
s32_t mcu_mode_get(void);
/********************************************************************/
/********************************************************************/
typedef struct tagMoneyUnitInfoS {
	u16_t			seq;
	u8_t			reserve[2];
	u8_t			pic[180];
	u8_t			crown[12];
	u8_t			Valuta;
	u8_t			ver;
	u8_t			flag;
	u8_t			type;
}MONEY_UNITINFO_S;

status_t moneybunch_init(void);
status_t moneybunch_release(void);
status_t moneybunch_add(u8_t *ptr);
status_t moneybunch_free(void);
u32_t moneybunch_count_get(void);
status_t moneybunch_get(u32_t index, MONEY_UNITINFO_S *pData);

#define MAX_NAME_SIZE       80

typedef struct _KEY_VALUE {
	int 				intval;
	char 				key[MAX_NAME_SIZE];
	char				value[BUFSIZ];
	struct _KEY_VALUE	*prev;
	struct _KEY_VALUE	*next;
}KEY_VALUE, *PKEY_VALUE;

typedef struct _SECTION {
	char 				name[MAX_NAME_SIZE];
	struct _SECTION		*prev;
	struct _SECTION		*next;
	KEY_VALUE			*head;
}SECTION, *PSECTION;


status_t upload_param_save(s8_t *name, s32_t value);
s32_t upload_param_load(s8_t *name, s32_t def);

typedef struct tagUpgradeStatusS {
	s32_t		arm;
	s32_t		param;
	s32_t		dsp;
	s32_t		fpga;
	s32_t		module;
	s32_t		mcu;
	s32_t		total;
}UPGRADE_STATUS_S;
status_t upgrade_create(s8_t *direct);
status_t upgrade_status_get(UPGRADE_STATUS_S *h);

u8_t motor_status_get(void);
void fota_reset_factory(void);
status_t fota_startstop(void);
status_t fota_clearzero(void);
int fota_paramsave(u8_t value);


u8_t fota_sensitive_normal_get(void);
status_t fota_sensitive_normal_set(u8_t level);
status_t fota_sensitive_image_set(u8_t level);
u8_t fota_sensitive_image_get(void);
status_t fota_sensitive_tape_set(u8_t level);
u8_t fota_sensitive_tape_get(void);

status_t fota_money4_support_set(u8_t level);
u8_t fota_money4_support_get(void);
status_t fota_damaged_support_set(u8_t level);
u8_t fota_damaged_support_get(void);

status_t image_transport_open(void);
status_t image_transport_close(void);
status_t image_transport_clear(void);
s32_t image_transport_status(u8_t *total, u8_t *curnum);

status_t image_identify_export(void);
status_t image_identify_import(void);
status_t image_identify_init(void);
status_t image_identify_release(void);
u8_t* image_identify_ptr_get(u8_t *h, u8_t level, u8_t ver, u8_t type, u8_t dir);
u8_t* image_identify_load(void);
status_t image_identify_unload(u8_t *h);
status_t image_identify_save(u8_t *h);
status_t image_identify_sync(void);




#endif





