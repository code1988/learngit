

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
    SYSINFO_GET_ARRAY,
    __SYSINFO_GET_MAX,
};


static const struct blobmsg_policy sysinfo_policy[] = {
    [SYSINFO_GET_ARRAY] = {.name = "get_args", .type = BLOBMSG_TYPE_ARRAY},
};


enum {
    SYSINFO_MODEL_NAME,
    SYSINFO_UNIQUE_ID,
    SYSINFO_DESCRIPTION,
    SYSINFO_SYS_NAME,
    SYSINFO_SYS_LOCATION,
    SYSINFO_MAC_ADDRESS,
    SYSINFO_HARDWARE_VERSION,
    SYSINFO_BOOTLOADER_VERSION,
    SYSINFO_FIRMWARE_VERSION,
    __SYSINFO_MAX
};


static int jw_sysinfo_model_name(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_unique_id(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_description(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_sys_name(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_sys_location(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_mac_address(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_hardware_version(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_bootloader_version(struct blob_buf *buf, int port_idx);
static int jw_sysinfo_firmware_version(struct blob_buf *buf, int port_idx);


static int jw_sysinfo_model_name(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "model_name", "jws2100-4e8o");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_unique_id(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "unique_id", "abcdefghijk0123456789");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_description(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "description", "joyware industry switch, all rights reserved");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_sys_name(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "sys_name", "Linux 3.14.29 + OPENWRT + MV6097F");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_sys_location(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "sys_location", "ZheJiang. HangZhou, WenSan Rd.");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_mac_address(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "mac_address", "00:11:22:33:44:55");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_hardware_version(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "hardware_version", "GE-22107MA_V1.00");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_bootloader_version(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "bootloader_version", "u-boot-2015.04");
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_sysinfo_firmware_version(struct blob_buf *buf, int port_idx)
{
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_sysinfo");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_string(buf, "firmware_version", "OPENWRT-r47008 customized");
    blobmsg_close_table(buf, table);

    return 0;
}


static const struct jw_switch_policy sysinfo_tbl[] = {
    [SYSINFO_MODEL_NAME] = {.name = "model_name", .get_handler = jw_sysinfo_model_name, .set_handler = NULL}, 
    [SYSINFO_UNIQUE_ID] = {.name = "unique_id", .get_handler = jw_sysinfo_unique_id, .set_handler = NULL},
    [SYSINFO_DESCRIPTION] = {.name = "description", .get_handler = jw_sysinfo_description, .set_handler = NULL},
    [SYSINFO_SYS_NAME] = {.name = "sys_name", .get_handler = jw_sysinfo_sys_name, .set_handler = NULL},
    [SYSINFO_SYS_LOCATION] = {.name = "sys_location", .get_handler = jw_sysinfo_sys_location, .set_handler = NULL},
    [SYSINFO_MAC_ADDRESS] = {.name = "mac_address", .get_handler = jw_sysinfo_mac_address, .set_handler = NULL},
    [SYSINFO_HARDWARE_VERSION] = {.name = "hardware_version", .get_handler = jw_sysinfo_hardware_version, .set_handler = NULL},
    [SYSINFO_BOOTLOADER_VERSION] = {.name = "bootloader_version", .get_handler = jw_sysinfo_bootloader_version, .set_handler = NULL},
    [SYSINFO_FIRMWARE_VERSION] = {.name = "firmware_version", .get_handler = jw_sysinfo_firmware_version, .set_handler = NULL},
};


static int sysinfo_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SYSINFO_GET_MAX];

    blobmsg_parse(sysinfo_policy, __SYSINFO_GET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[SYSINFO_GET_ARRAY]) {
        ERROR("invalid params in %s\n", __func__);
        return -1;
    }

	blob_buf_init(&b, 0);

    struct blob_attr *cur = NULL;
    struct blob_attr *_cur = NULL;
    int rem = 0;
    char *name = NULL;

    cur = tb[SYSINFO_GET_ARRAY];
    if (blobmsg_type(cur) != BLOBMSG_TYPE_ARRAY) {
        ERROR("blobmsg type is not array\n");
        return -1;
    }

    void *ret_table = NULL;
    ret_table = blobmsg_open_array(&b, "ret");
    blobmsg_for_each_attr(_cur, cur, rem) {
        if (blobmsg_type(_cur) == BLOBMSG_TYPE_STRING) {
            struct jw_switch_policy *jw_sysinfo_p = NULL;
            name = blobmsg_get_string(_cur);
            jw_sysinfo_p = (struct jw_sysinfo_policy *)jw_switchd_get_context(name, sysinfo_tbl, __SYSINFO_MAX);
            if (jw_sysinfo_p && jw_sysinfo_p->get_handler) {
                printf("%s[%d]: invoke get_handler\n", __func__, __LINE__);
                jw_sysinfo_p->get_handler(&b, 0);
            } else {
                ERROR("undefined keys\n");
            }
            //printf("%s[%d]: array value = %s\n", __func__, __LINE__, name);
        } else {
            printf("%s[%d]: invalid attr type\n", __func__, __LINE__);
        }
    }
    blobmsg_close_array(&b, ret_table);	

    ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}


static const struct ubus_method system_methods[] = {
	UBUS_METHOD("sysinfo_get", sysinfo_handler, sysinfo_policy),
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
