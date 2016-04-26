

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <libubox/uloop.h>

#include "switchd.h"

static struct blob_buf b;
static int notify;
static struct ubus_context *_ctx;

enum {
    SYSINFO_GET_KEYS,
    __SYSINFO_GET_MAX,
};

enum {
    SYSINFO_SET_ARRAY,
    __SYSINFO_SET_MAX
};

static const struct blobmsg_policy sysinfo_get_policy[] = {
    [SYSINFO_GET_KEYS] = {.name = "get_args", .type = BLOBMSG_TYPE_ARRAY},
};

static const struct blobmsg_policy sysinfo_set_policy[] = {
    [SYSINFO_SET_ARRAY] = {.name = "set_args", .type = BLOBMSG_TYPE_ARRAY},
};

enum {
    SYSINFO_MODEL_NAME,
    SYSINFO_UNIQUE_ID,
    SYSINFO_DESCRIPTION,
    SYSINFO_SYS_NAME,
    SYSINFO_SYS_LOCATION,
    SYSINFO_HARDWARE_VERSION,
    SYSINFO_BOOTLOADER_VERSION,
    SYSINFO_SOFTWARE_VERSION,
    SYSINFO_SAVE_CFG,
    SYSINFO_CLEAR_CFG,
    __SYSINFO_TBL_MAX
};

static int jw_sysinfo_model_name(struct blob_buf *buf);
static int jw_sysinfo_unique_id(struct blob_buf *buf);
static int jw_sysinfo_description(struct blob_buf *buf);
static int jw_sysinfo_sys_name(struct blob_buf *buf);
static int jw_sysinfo_sys_location(struct blob_buf *buf);
static int jw_sysinfo_hardware_version(struct blob_buf *buf);
static int jw_sysinfo_bootloader_version(struct blob_buf *buf);
static int jw_sysinfo_software_version(struct blob_buf *buf);


static int jw_sysinfo_model_name(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "model_name", "jws2100-4e8o");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_unique_id(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "unique_id", "abcdefghijk0123456789");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_description(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "description", "joyware industry switch, all rights reserved");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_sys_name(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "sys_name", "Linux 3.14.29 + OPENWRT + MV6097F");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_sys_location(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "sys_location", "ZheJiang. HangZhou, WenSan Rd.");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_mac_address(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "mac_address", "00:11:22:33:44:55");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_hardware_version(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "hardware_version", "GE-22107MA_V1.00");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_bootloader_version(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "bootloader_version", "u-boot-2015.04");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_software_version(struct blob_buf *buf)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "firmware_version", "OPENWRT-r47008 customized");
    blobmsg_close_table(buf, table);

    return 0;
}

static int JW_SysinfoSaveCfgRet(struct blob_buf *buf)
{
   return 0; 
}

static int JW_SysinfoClearCfgRet(struct blob_buf *buf)
{
   return 0; 
}

static int JW_SysinfoSaveCfg(void *buf)
{
    return 0;
}

static int JW_SysinfoClearCfg(void *buf)
{
    return 0;
}

static int JW_SetSysName(void *buf)
{
    return 0;
}

static int JW_SetSysLocation(void *buf)
{
    return 0;
}

static const struct jw_switch_policy sysinfo_tbl[] = {
    [SYSINFO_MODEL_NAME] = {.name = "model_name", .get_ext_handler = jw_sysinfo_model_name, .set_handler = NULL}, 
    [SYSINFO_UNIQUE_ID] = {.name = "unique_id", .get_ext_handler = jw_sysinfo_unique_id, .set_handler = NULL},
    [SYSINFO_DESCRIPTION] = {.name = "description", .get_ext_handler = jw_sysinfo_description, .set_handler = NULL},
    [SYSINFO_SYS_NAME] = {.name = "sys_name", .get_ext_handler = jw_sysinfo_sys_name, .set_ext_handler = JW_SetSysName},
    [SYSINFO_SYS_LOCATION] = {.name = "sys_location", .get_ext_handler = jw_sysinfo_sys_location, .set_ext_handler = JW_SetSysLocation},
    [SYSINFO_HARDWARE_VERSION] = {.name = "hardware_version", .get_ext_handler = jw_sysinfo_hardware_version, .set_handler = NULL},
    [SYSINFO_BOOTLOADER_VERSION] = {.name = "bootloader_version", .get_ext_handler = jw_sysinfo_bootloader_version, .set_handler = NULL},
    [SYSINFO_SOFTWARE_VERSION] = {.name = "software_version", .get_ext_handler = jw_sysinfo_software_version, .set_handler = NULL},
    [SYSINFO_SAVE_CFG] = {.name = "save_config", .get_ext_handler = JW_SysinfoSaveCfgRet, .set_ext_handler = JW_SysinfoSaveCfg},
    [SYSINFO_CLEAR_CFG] = {.name = "clear_config", .get_ext_handler = JW_SysinfoClearCfgRet, .set_ext_handler = JW_SysinfoClearCfg},
};


static int JW_SysinfoGetHandle(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SYSINFO_GET_MAX];
	struct blob_attr *lvl1 = NULL;  

    blobmsg_parse(sysinfo_get_policy, __SYSINFO_GET_MAX, tb, blob_data(msg), blob_len(msg));
	lvl1 = tb[SYSINFO_GET_KEYS];     
    if (!lvl1) 
    {
        ERROR("invalid params in %s\n",__func__);
        return -1;
    }
    if (blobmsg_type(lvl1) != BLOBMSG_TYPE_ARRAY) 
    {
        ERROR("blobmsg type is not array\n");
        return -1;
    }

    struct blob_attr *lvl2 = NULL; 
    int rem = 0;
    void *ret_table = NULL;
    char *name = NULL;

	blob_buf_init(&b, 0);

    ret_table = blobmsg_open_array(&b, "ret");

    blobmsg_for_each_attr(lvl2, lvl1, rem) 
    {
        if (blobmsg_type(lvl2) == BLOBMSG_TYPE_STRING) 
        {
            struct jw_switch_policy *jw_sysinfo_p = NULL;
            name = blobmsg_get_string(lvl2);
            jw_sysinfo_p = (struct jw_switch_policy *)jw_switchd_get_context(name, sysinfo_tbl, __SYSINFO_TBL_MAX);
            if (jw_sysinfo_p && jw_sysinfo_p->get_ext_handler) 
            {
                printf("%s[%d]: invoke get_handler\n", __func__, __LINE__);
                jw_sysinfo_p->get_ext_handler(&b);
            } 
            else 
            {
                ERROR("undefined keys\n");
            }
            //printf("%s[%d]: array value = %s\n", __func__, __LINE__, name);
        } 
        else 
        {
            printf("%s[%d]: invalid attr type\n", __func__, __LINE__);
        }
    }
    blobmsg_close_array(&b, ret_table);	

    ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}

/* *
 * * @fn JW_SysinfoSetHandle
 * * @brief handle function for the method of "sysinfo_get"
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_SysinfoSetHandle(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SYSINFO_SET_MAX];    
	struct blob_attr *lvl1 = NULL;  

    blobmsg_parse(sysinfo_set_policy, __SYSINFO_SET_MAX, tb, blob_data(msg), blob_len(msg));
	lvl1 = tb[SYSINFO_SET_ARRAY];     
    if (!lvl1) 
	{
        ERROR("invalid params in %s\n", __func__);
        return -1;
    }

	if (blobmsg_type(lvl1) != BLOBMSG_TYPE_ARRAY) 
	{
        ERROR("blobmsg type is not array\n");
        return -1;
    }

    struct blob_attr *lvl2 = NULL; 
    int rem = 0;
    void *ret_table = NULL;
		
	blob_buf_init(&b, 0);

    ret_table = blobmsg_open_array(&b, "ret"); 

    blobmsg_for_each_attr(lvl2,lvl1, rem) 
    {
        if (blobmsg_type(lvl2) == BLOBMSG_TYPE_TABLE) 
        {
            struct blob_attr *lvl3 = blobmsg_data(lvl2);
            struct jw_switch_policy *jw_sysinfo_p = NULL;

            char *name = NULL;
            name = (char *)((struct blobmsg_hdr *)blob_data(lvl3))->name;  
            
            switch(blobmsg_type(lvl3))
            {
                case BLOBMSG_TYPE_STRING:
                    {
                        char *val = blobmsg_get_string(lvl3);
                        
                        printf("[%s][%d]: Lev5: BLOBMSG_TYPE_STRING, name = %s,value = [%s]\n", __func__, __LINE__, name,val);
                        // 根据五级消息名完成索引，找到对应参数名所在的控制块
                        jw_sysinfo_p = (struct jw_switch_policy *)jw_switchd_get_context(name, sysinfo_tbl, __SYSINFO_TBL_MAX);
                        if (jw_sysinfo_p && jw_sysinfo_p->set_ext_handler) 
                        {
                            // 执行参数对应的处理函数
                            jw_sysinfo_p->set_ext_handler((void *)val);
                        }
                    }
                    break;
                case BLOBMSG_TYPE_INT32:
                    {
                        int val = blobmsg_get_u32(lvl3);
                        printf("[%s][%d]: Lev5 BLOBMSG_TYPE_INT32 , name - %s,value = [%d]\n", __func__, __LINE__, name,val); 
                        jw_sysinfo_p = (struct jw_switch_policy *)jw_switchd_get_context(name, sysinfo_tbl, __SYSINFO_TBL_MAX);
                        if (jw_sysinfo_p && jw_sysinfo_p->set_ext_handler) 
                        {
                            // 执行参数对应的处理函数
                            jw_sysinfo_p->set_ext_handler((void *)&val);
                        }
                    }
                    break;
                default:
                    printf("[%s][%d]: Lev5 msg type error!\n",__func__,__LINE__);
                    break;                    
            }
        } 
        else 
		{
            ERROR("%s[%d]: invalid attr type\n", __func__, __LINE__);
        }
    }

    blobmsg_close_array(&b, ret_table);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}


static const struct ubus_method system_methods[] = {
	UBUS_METHOD("sysinfo_get", JW_SysinfoGetHandle, sysinfo_get_policy),
    UBUS_METHOD("sysinfo_set", JW_SysinfoSetHandle, sysinfo_set_policy),
};


static struct ubus_object_type system_object_type =
	UBUS_OBJECT_TYPE("sysinfo", system_methods);


static struct ubus_object system_object = {
	.name = "sysinfo",
	.type = &system_object_type,
	.methods = system_methods,
	.n_methods = ARRAY_SIZE(system_methods),
};


void switchd_bcast_event(char *event, struct blob_attr *msg)
{
	int ret;

	if (!notify)
		return;

	ret = ubus_notify(_ctx, &system_object, event, msg, -1);
	if (ret)
		fprintf(stderr, "Failed to notify log: %s\n", ubus_strerror(ret));
}


void ubus_init_sysinfo(struct ubus_context *ctx)
{
	int ret;

	_ctx = ctx;
	ret = ubus_add_object(ctx, &system_object);
	if (ret)
		ERROR("Failed to add object: %s\n", ubus_strerror(ret));
}
