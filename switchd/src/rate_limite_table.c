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
    SWITCH_RATE_LIMIT_GET_ARRAY,
    __SWITCH_RATE_LIMIT_GET_MAX
};


static const struct blobmsg_policy rate_limit_get_policy[] = {
    [SWITCH_RATE_LIMIT_GET_ARRAY] = {.name = "get_args", .type = BLOBMSG_TYPE_ARRAY},
};


enum {
    SWITCH_RATE_LIMIT_GET_TBL_INGRESS_RATE_CTRL_MODE,
    SWITCH_RATE_LIMIT_GET_TBL_INGRESS_RATE_LIMIT_VALUE,
    SWITCH_RATE_LIMIT_GET_TBL_EGRESS_RATE_LIMIT_VALUE,
    __SWITCH_RATE_LIMIT_GET_TBL_MAX,
};


static int jw_rate_limit_get_ingress_rate_ctrl_mode(struct blob_buf *buf, int port_idx);
static int jw_rate_limit_get_ingress_rate_limit_value(struct blob_buf *buf, int port_idx);
static int jw_rate_limit_get_egress_rate_limit_value(struct blob_buf *buf, int port_idx);


enum {
    SWITCH_RATE_LIMIT_SET_ARRAY,
    __SWITCH_RATE_LIMIT_SET_MAX,
};


static const struct blobmsg_policy rate_limit_set_policy[] = {
    [SWITCH_RATE_LIMIT_SET_ARRAY] = {.name = "set_args", .type = BLOBMSG_TYPE_ARRAY},
};


enum {
    SWITCH_RATE_LIMIT_SET_TBL_INGRESS_RATE_CTRL_MODE,
    SWITCH_RATE_LIMIT_SET_TBL_INGRESS_RATE_LIMIT_VALUE,
    SWITCH_RATE_LIMIT_SET_TBL_EGRESS_RATE_LIMIT_VALUE,
    __SWITCH_RATE_LIMIT_SET_TBL_MAX,
};


static int jw_rate_limit_get_ingress_rate_ctrl_mode(struct blob_buf *buf, int port_idx)
{
    int ircm = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "ingress_rate_ctrl_mode", ircm);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_rate_limit_get_ingress_rate_limit_value(struct blob_buf *buf, int port_idx)
{
    int irlv = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    irlv = 1;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "ingress_rate_limit_value", irlv);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_rate_limit_get_egress_rate_limit_value(struct blob_buf *buf, int port_idx)
{
    int erlv = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    erlv = 2;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "egress_rate_limit_value", erlv);
    blobmsg_close_table(buf, table);

    return 0;
}


static const struct jw_switch_policy rate_limit_get_tbl[] = {
    [SWITCH_RATE_LIMIT_GET_TBL_INGRESS_RATE_CTRL_MODE] = {.name = "ingress_rate_ctrl_mode", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_rate_limit_get_ingress_rate_ctrl_mode},
    [SWITCH_RATE_LIMIT_GET_TBL_INGRESS_RATE_LIMIT_VALUE] = {.name = "ingress_rate_limit_value", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_rate_limit_get_ingress_rate_limit_value}, 
    [SWITCH_RATE_LIMIT_GET_TBL_EGRESS_RATE_LIMIT_VALUE] = {.name = "egress_rate_limit_value", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_rate_limit_get_egress_rate_limit_value},
};


static int jw_rate_limit_set_ingress_rate_ctrl_mode(int port_idx, void *p)
{
    int ircm = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, ircm);

    return 0;
}


static int jw_rate_limit_set_ingress_rate_limit_value(int port_idx, void *p)
{
    int irlv = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, irlv);

    return 0;
}


static int jw_rate_limit_egress_rate_limit_value(int port_idx, void *p)
{
    int erlv = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, erlv);

    return 0;
}


static const struct jw_switch_policy rate_limit_set_tbl[] = {
    [SWITCH_RATE_LIMIT_SET_TBL_INGRESS_RATE_CTRL_MODE] = {.name = "ingress_rate_ctrl_mode", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_rate_limit_set_ingress_rate_ctrl_mode}, 
    [SWITCH_RATE_LIMIT_SET_TBL_INGRESS_RATE_LIMIT_VALUE] = {.name = "ingress_rate_limit_value", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_rate_limit_set_ingress_rate_limit_value},
    [SWITCH_RATE_LIMIT_SET_TBL_EGRESS_RATE_LIMIT_VALUE] = {.name = "egress_rate_limit_value", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_rate_limit_egress_rate_limit_value},
};


static int jw_rate_limitd_parse_port_cfg_get(struct blob_attr *tb, struct blob_buf *buf)
{
    int port_idx = -1;
    int rem = 0;
    struct blob_attr *cur = NULL;
    char *array_var = NULL;
    struct blobmsg_hdr *hdr = NULL;
    char *pidx = NULL;
    struct jw_switch_policy *jw_rate_limit_p = NULL;

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
            jw_rate_limit_p = (struct jw_switch_policy *)jw_switchd_get_context(array_var, rate_limit_get_tbl, __SWITCH_RATE_LIMIT_GET_TBL_MAX);
            if (jw_rate_limit_p && jw_rate_limit_p->get_handler) {
                jw_rate_limit_p->get_handler(buf, port_idx);
            } else {
                ERROR("undefined keys\n");
            }
        }
    }

    blobmsg_close_array(buf, msg_array);
    blobmsg_close_table(buf, msg_table);

    return 0;
}


static int jw_rate_limitd_parse_port_cfg_set(struct blob_attr *tb, struct blob_buf *buf)
{
    int port_idx = -1;
    int rem = 0;
    struct blob_attr *cur = NULL;
    struct blobmsg_hdr *hdr = NULL;
    char *pidx = NULL;
    struct jw_switch_policy *jw_rate_limit_p = NULL;

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
            printf("%s[%d]: BLOBMSG_TYPE_TABLE\n", __func__, __LINE__);
            struct blob_attr *t = blobmsg_data(cur);
            int v = -1;
            char *name = NULL;
            name = (char *)((struct blobmsg_hdr *)blob_data(t))->name;
            v = blobmsg_get_u32(t);
            printf("%s[%d]: BLOBMSG_TYPE_TABLE, hdr name = [%s], var = [%d]\n", __func__, __LINE__, name, v);
            jw_rate_limit_p = (struct jw_switch_policy *)jw_switchd_get_context(name, rate_limit_set_tbl, __SWITCH_RATE_LIMIT_SET_TBL_MAX);
            if (jw_rate_limit_p && jw_rate_limit_p->set_handler) {
                jw_rate_limit_p->set_handler(port_idx, (void *)&v);
            } else {
                ERROR("undefined keys\n");
            }
        } else {
            ERROR("invalid type to this application\n");
        }
    }

    return 0;
}


static int rate_limit_get_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SWITCH_RATE_LIMIT_GET_MAX];

    blobmsg_parse(rate_limit_get_policy, __SWITCH_RATE_LIMIT_GET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[SWITCH_RATE_LIMIT_GET_ARRAY]) {
        ERROR("invalid params in %s\n", __func__);
        return -1;
    }

	blob_buf_init(&b, 0);

    struct blob_attr *cur = NULL;
    struct blob_attr *_cur = NULL;
    int rem = 0;
    char *array_val = NULL;


    cur = tb[SWITCH_RATE_LIMIT_GET_ARRAY];
    if (blobmsg_type(cur) != BLOBMSG_TYPE_ARRAY) {
        ERROR("blobmsg type is not array\n");
        return -1;
    }

    void *ret_table = NULL;
    ret_table = blobmsg_open_array(&b, "ret");
    blobmsg_for_each_attr(_cur, cur, rem) {
        if (blobmsg_type(_cur) == BLOBMSG_TYPE_STRING) {
            array_val = blobmsg_get_string(_cur);
            printf("%s[%d]: array value = %s\n", __func__, __LINE__, array_val);
        } else if (blobmsg_type(_cur) == BLOBMSG_TYPE_TABLE) {
            struct blob_attr *tbl = blobmsg_data(_cur);
            if (blobmsg_type(tbl) ==  BLOBMSG_TYPE_ARRAY) {
                printf("table value is array\n");
                jw_rate_limitd_parse_port_cfg_get(tbl, &b);
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


static int rate_limit_set_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SWITCH_RATE_LIMIT_SET_MAX];

	blob_buf_init(&b, 0);

    blobmsg_parse(rate_limit_set_policy, __SWITCH_RATE_LIMIT_SET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[SWITCH_RATE_LIMIT_SET_ARRAY]) {
        ERROR("invalid params in %s\n", __func__);
        blobmsg_add_u32(&b, "ret", 0);    // 0 for fail, 1 for success
        return -1;
    }


    struct blob_attr *cur = NULL;
    struct blob_attr *_cur = NULL;
    int rem = 0;

    cur = tb[SWITCH_RATE_LIMIT_SET_ARRAY];
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
                jw_rate_limitd_parse_port_cfg_set(tbl, &b);
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


static const struct ubus_method rate_limit_methods[] = {
	UBUS_METHOD("rate_limit_get", rate_limit_get_handler, rate_limit_get_policy),
	UBUS_METHOD("rate_limit_set", rate_limit_set_handler, rate_limit_set_policy),
};


static struct ubus_object_type rate_limit_object_type =
	UBUS_OBJECT_TYPE("rate_limit", rate_limit_methods);


static struct ubus_object rate_limit_object = {
	.name = "rate_limit",
	.type = &rate_limit_object_type,
	.methods = rate_limit_methods,
	.n_methods = ARRAY_SIZE(rate_limit_methods),
};


void ubus_init_rate_limit(struct ubus_context *ctx)
{
	int ret;

	_ctx = ctx;
	ret = ubus_add_object(ctx, &rate_limit_object);
	if (ret)
		ERROR("Failed to add object port config: %s\n", ubus_strerror(ret));
}
