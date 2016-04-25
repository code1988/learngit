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
static struct ubus_context *_ctx;

enum {
    SWITCH_PORT_MIRROR_GET_ARRAY,
    __SWITCH_PORT_MIRROR_GET_MAX
};


static const struct blobmsg_policy port_mirror_get_policy[] = {
    [SWITCH_PORT_MIRROR_GET_ARRAY] = {.name = "get_args", .type = BLOBMSG_TYPE_ARRAY},
};


enum {
    SWITCH_PORT_MIRROR_GET_TBL_DESTINATION_PORT,
    SWITCH_PORT_MIRROR_GET_TBL_ADMIN_MODE,
    SWITCH_PORT_MIRROR_GET_TBL_DIRECTION,
    __SWITCH_PORT_MIRROR_GET_TBL_MAX,
};


static int jw_port_mirror_get_destination_port(struct blob_buf *buf, int port_idx);
static int jw_port_mirror_get_admin_mode(struct blob_buf *buf, int port_idx);
static int jw_port_mirror_get_direction(struct blob_buf *buf, int port_idx);


enum {
    SWITCH_PORT_MIRROR_SET_ARRAY,
    __SWITCH_PORT_MIRROR_SET_MAX,
};


static const struct blobmsg_policy port_mirror_set_policy[] = {
    [SWITCH_PORT_MIRROR_SET_ARRAY] = {.name = "set_args", .type = BLOBMSG_TYPE_ARRAY},
};


enum {
    SWITCH_PORT_MIRROR_SET_TBL_DESTINATION_PORT,
    SWITCH_PORT_MIRROR_SET_TBL_ADMIN_MODE,
    SWITCH_PORT_MIRROR_SET_TBL_DIRECTION,
    __SWITCH_PORT_MIRROR_SET_TBL_MAX,
};


static int jw_port_mirror_set_destination_port(int port_idx, void *p);
static int jw_port_mirror_set_admin_mode(int port_idx, void *p);
static int jw_port_mirror_set_direction(int port_idx, void *p);


static int jw_port_mirror_get_destination_port(struct blob_buf *buf, int port_idx)
{
    int ircm = 0;

    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "ingress_rate_ctrl_mode", ircm);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_port_mirror_get_admin_mode(struct blob_buf *buf, int port_idx)
{
    int irlv = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    irlv = 1;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "ingress_port_mirror_value", irlv);
    blobmsg_close_table(buf, table);
    
    return 0;
}


static int jw_port_mirror_get_direction(struct blob_buf *buf, int port_idx)
{
    int erlv = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");    erlv = 2;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "egress_port_mirror_value", erlv);
    blobmsg_close_table(buf, table);

    return 0;
}


static const struct jw_switch_policy port_mirror_tbl[] = {
    [SWITCH_PORT_MIRROR_GET_TBL_DESTINATION_PORT] = {.name = "destination_port", .get_handler = jw_port_mirror_get_destination_port, .set_handler = jw_port_mirror_set_destination_port}, 
    [SWITCH_PORT_MIRROR_GET_TBL_ADMIN_MODE] = {.name = "admin_mode", .get_handler = jw_port_mirror_get_admin_mode, .set_handler = jw_port_mirror_set_admin_mode},
    [SWITCH_PORT_MIRROR_GET_TBL_DIRECTION] = {.name = "direction", .get_handler = jw_port_mirror_get_direction, .set_handler = jw_port_mirror_set_direction},
};


static int jw_port_mirror_set_destination_port(int port_idx, void *p)
{
    int ircm = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, ircm);

    return 0;
}


static int jw_port_mirror_set_admin_mode(int port_idx, void *p)
{
    int irlv = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, irlv);

    return 0;
}


static int jw_port_mirror_set_direction(int port_idx, void *p)
{
    int erlv = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, erlv);

    return 0;
}


static int jw_port_mirror_parse_port_cfg_get(struct blob_attr *tb, struct blob_buf *buf)
{
    int port_idx = -1;
    int rem = 0;
    struct blob_attr *cur = NULL;
    char *array_var = NULL;
    struct blobmsg_hdr *hdr = NULL;
    char *pidx = NULL;
    struct jw_switch_policy *jw_port_mirror_p = NULL;

    hdr = blob_data(tb);

    pidx = strstr((char *)hdr->name, "idx-");
    if (pidx) {
        printf("[%s][%d], pidx = [%s]\n", __func__, __LINE__, pidx);
        port_idx = atoi(pidx + 4);
        if (port_idx < 0 || port_idx > 32) {
            printf("[%s][%d], invalid port index %d\n", __func__, __LINE__, port_idx);
            return -1;
        }
    } else {
        printf("[%s][%d], invalid port index arg [%s]\n", __func__, __LINE__, hdr->name);
        return -1;
    }

    void *msg_array = NULL;
    void *msg_table = NULL;
    
    msg_table = blobmsg_open_table(buf, (char *)hdr->name);
    msg_array = blobmsg_open_array(buf, (char *)hdr->name);

    blobmsg_for_each_attr(cur, tb, rem) {
        if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING) {
            array_var = blobmsg_get_string(cur);
            printf("%s[%d]: array value = %s\n", __func__, __LINE__, array_var);
            jw_port_mirror_p = (struct jw_switch_policy *)jw_switchd_get_context(array_var, port_mirror_tbl, __SWITCH_PORT_MIRROR_GET_TBL_MAX);
            if (jw_port_mirror_p && jw_port_mirror_p->get_handler) {
                jw_port_mirror_p->get_handler(buf, port_idx);
            } else {
                ERROR("undefined keys\n");
            }
        }
    }

    blobmsg_close_array(buf, msg_array);
    blobmsg_close_table(buf, msg_table);

    return 0;
}

static int jw_port_mirror_parse_port_cfg_set(struct blob_attr *tb, struct blob_buf *buf)
{
    int port_idx = -1;
    int rem = 0;
    int v = -1;
    struct blob_attr *cur = NULL;
    //char *array_var = NULL;
    char *name = NULL;
    struct blobmsg_hdr *hdr = NULL;
    char *pidx = NULL;
    struct jw_switch_policy *jw_port_mirror_p = NULL;

    hdr = blob_data(tb);

    pidx = strstr((char *)hdr->name, "idx-");
    if (pidx) {
        printf("[%s][%d], pidx = [%s]\n", __func__, __LINE__, pidx);
        port_idx = atoi(pidx + 4);
        if (port_idx < 0 || port_idx > 32) {
            printf("[%s][%d], invalid port index %d\n", __func__, __LINE__, port_idx);
            return -1;
        }
    } else {
        printf("[%s][%d], invalid port index arg [%s]\n", __func__, __LINE__, hdr->name);
        return -1;
    }

    blobmsg_for_each_attr(cur, tb, rem) {
        if (blobmsg_type(cur) == BLOBMSG_TYPE_TABLE) {
            struct blob_attr *t = blobmsg_data(cur);
            name = (char *)((struct blobmsg_hdr *)blob_data(t))->name;
            v = blobmsg_get_u32(t);
            printf("%s[%d]: BLOBMSG_TYPE_TABLE, hdr name = [%s], var = [%d]\n", __func__, __LINE__, name, v);
            jw_port_mirror_p = (struct jw_switch_policy *)jw_switchd_get_context(name, port_mirror_tbl, __SWITCH_PORT_MIRROR_SET_TBL_MAX);
            if (jw_port_mirror_p && jw_port_mirror_p->set_handler) {
                jw_port_mirror_p->set_handler(port_idx, (void *)&v);
            } else {
                ERROR("undefined keys %s\n", name);
            }
        } else if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING) {
            printf("table value is array\n");
        } else {
            ERROR("invalid type to this application\n");
        }
    }

    return 0;
}


static int port_mirror_get_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SWITCH_PORT_MIRROR_GET_MAX];

    blobmsg_parse(port_mirror_get_policy, __SWITCH_PORT_MIRROR_GET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[SWITCH_PORT_MIRROR_GET_ARRAY]) {
        ERROR("invalid params in %s\n", __func__);
        return -1;
    }

	blob_buf_init(&b, 0);

    struct blob_attr *cur = NULL;
    struct blob_attr *_cur = NULL;
    int rem = 0;
    char *name = NULL;

    cur = tb[SWITCH_PORT_MIRROR_GET_ARRAY];
    if (blobmsg_type(cur) != BLOBMSG_TYPE_ARRAY) {
        ERROR("blobmsg type is not array\n");
        return -1;
    }

    void *ret_table = NULL;
    ret_table = blobmsg_open_array(&b, "ret");
    blobmsg_for_each_attr(_cur, cur, rem) {
        if (blobmsg_type(_cur) == BLOBMSG_TYPE_STRING) {
            //array_val = blobmsg_get_string(_cur); 
            //int v = -1;
            struct jw_switch_policy *jw_port_mirror_p = NULL;
            name = blobmsg_get_string(_cur);
            jw_port_mirror_p = (struct jw_switch_policy *)jw_switchd_get_context(name, port_mirror_tbl, __SWITCH_PORT_MIRROR_GET_TBL_MAX);
            if (jw_port_mirror_p && jw_port_mirror_p->get_handler) {
                jw_port_mirror_p->get_handler(&b, 0);
            } else {
                ERROR("undefined keys\n");
            }
            printf("%s[%d]: array value = %s\n", __func__, __LINE__, name);
        } else if (blobmsg_type(_cur) == BLOBMSG_TYPE_TABLE) {
            printf(">>>>>> msg type table <<<<<<<\n");
            struct blob_attr *tbl = blobmsg_data(_cur);
            if (blobmsg_type(tbl) ==  BLOBMSG_TYPE_ARRAY) {
                printf("table value is array\n");
                jw_port_mirror_parse_port_cfg_get(tbl, &b);
            } else if (blobmsg_type(tbl) == BLOBMSG_TYPE_INT32) {
                printf("%s[%d]: array value = %d\n", __func__, __LINE__, blobmsg_get_u32(tbl));
            }

        } else if (blobmsg_type(_cur) == BLOBMSG_TYPE_ARRAY) {
            printf(">>>>>> msg type array <<<<<<<\n");

        } else {
            printf("%s[%d]: invalid attr type\n", __func__, __LINE__);
        }
    }
    blobmsg_close_array(&b, ret_table);	

    ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}


static int port_mirror_set_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SWITCH_PORT_MIRROR_SET_MAX];

	blob_buf_init(&b, 0);

    blobmsg_parse(port_mirror_set_policy, __SWITCH_PORT_MIRROR_SET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[SWITCH_PORT_MIRROR_SET_ARRAY]) {
        ERROR("invalid params in %s\n", __func__);
        blobmsg_add_u32(&b, "ret", 0);    // 0 for fail, 1 for success
        return -1;
    }


    struct blob_attr *cur = NULL;
    struct blob_attr *_cur = NULL;
    int rem = 0;
    //char *array_val = NULL;

    cur = tb[SWITCH_PORT_MIRROR_SET_ARRAY];
    if (blobmsg_type(cur) != BLOBMSG_TYPE_ARRAY) {
        ERROR("blobmsg type is not array\n");
        blobmsg_add_u32(&b, "ret", 0);    // 0 for fail, 1 for success
        return -1;
    }

    blobmsg_for_each_attr(_cur, cur, rem) {
        if (blobmsg_type(_cur) == BLOBMSG_TYPE_TABLE) {
            printf(">>>>>> msg type table <<<<<<<\n");
            struct blob_attr *tbl = blobmsg_data(_cur);
            if (blobmsg_type(tbl) ==  BLOBMSG_TYPE_ARRAY) {
                printf("table value is array\n");
                jw_port_mirror_parse_port_cfg_set(tbl, &b);
            } else {
                printf("%s[%d]: not defined application\n", __func__, __LINE__);
            }
        } else {
            printf("%s[%d]: invalid attr type, not defined for application\n", __func__, __LINE__);
        }
    }

    blobmsg_add_u32(&b, "ret", 1);
	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}


static const struct ubus_method port_mirror_methods[] = {
	UBUS_METHOD("port_mirror_get", port_mirror_get_handler, port_mirror_get_policy),
	UBUS_METHOD("port_mirror_set", port_mirror_set_handler, port_mirror_set_policy),
};


static struct ubus_object_type port_mirror_object_type =
	UBUS_OBJECT_TYPE("port_mirror", port_mirror_methods);


static struct ubus_object port_mirror_object = {
	.name = "port_mirror",
	.type = &port_mirror_object_type,
	.methods = port_mirror_methods,
	.n_methods = ARRAY_SIZE(port_mirror_methods),
};


void ubus_init_port_mirror(struct ubus_context *ctx)
{
	int ret;

	_ctx = ctx;
	ret = ubus_add_object(ctx, &port_mirror_object);
	if (ret)
		ERROR("Failed to add object port config: %s\n", ubus_strerror(ret));
}
