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
    SWITCH_PORT_CFG_GET_KEYS,
    __SWITCH_PORT_CFG_GET_MAX
};


static const struct blobmsg_policy port_config_get_policy[] = {
    [SWITCH_PORT_CFG_GET_KEYS] = {.name = "get_args", .type = BLOBMSG_TYPE_ARRAY},
};


enum {
    SWITCH_PORT_CFG_GET_TBL_TYPE,
    SWITCH_PORT_CFG_GET_TBL_ADMIN_MODE,
    SWITCH_PORT_CFG_GET_TBL_LINK_STATUS,
    SWITCH_PORT_CFG_GET_TBL_PHYSICAL_MODE,
    SWITCH_PORT_CFG_GET_TBL_PHYSICAL_STATUS,
    SWITCH_PORT_CFG_GET_TBL_FLOW_CONTROL_MODE,
    SWITCH_PORT_CFG_GET_TBL_STATE,
    SWITCH_PORT_CFG_GET_TBL_LINK_TRAP_MODE,
    SWITCH_PORT_CFG_GET_TBL_THRESHOLD_TRAP_MODE,
    SWITCH_PORT_CFG_GET_TBL_TX_THRESHOLD_VALUE,
    SWITCH_PORT_CFG_GET_TBL_RX_THRESHOLD_VALUE,
    __SWITCH_PORT_CFG_GET_TBL_MAX,
};


static int jw_switch_get_type(struct blob_buf *buf, int port_idx);
static int jw_switch_get_admin_mode(struct blob_buf *buf, int port_idx);
static int jw_switch_get_link_status(struct blob_buf *buf, int port_idx);
static int jw_switch_get_physical_mode(struct blob_buf *buf, int port_idx);
static int jw_switch_get_physical_status(struct blob_buf *buf, int port_idx);
static int jw_switch_get_flow_control_mode(struct blob_buf *buf, int port_idx);
static int jw_switch_get_state(struct blob_buf *buf, int port_idx);
static int jw_switch_get_link_trap_mode(struct blob_buf *buf, int port_idx);
static int jw_switch_get_threshold_trap_mode(struct blob_buf *buf, int port_idx);
static int jw_switch_get_tx_threshold_value(struct blob_buf *buf, int port_idx);
static int jw_switch_get_rx_threshold_value(struct blob_buf *buf, int port_idx);


enum {
    SWITCH_PORT_CFG_SET_ARRAY,
    __SWITCH_PORT_CFG_SET_MAX
};


static const struct blobmsg_policy port_config_set_policy[] = {
    [SWITCH_PORT_CFG_SET_ARRAY] = {.name = "set_args", .type = BLOBMSG_TYPE_ARRAY},
};


enum {
    SWITCH_PORT_CFG_SET_TBL_ADMIN_MODE,
    SWITCH_PORT_CFG_SET_TBL_PHYSICAL_MODE,
    SWITCH_PORT_CFG_SET_TBL_FLOW_CONTROL_MODE,
    SWITCH_PORT_CFG_SET_TBL_LINK_TRAP_MODE,
    SWITCH_PORT_CFG_SET_TBL_THRESHOLD_TRAP_MODE,
    SWITCH_PORT_CFG_SET_TBL_TX_THRESHOLD_VALUE,
    SWITCH_PORT_CFG_SET_TBL_RX_THRESHOLD_VALUE,
    __SWITCH_PORT_CFG_SET_TBL_MAX,
};


static int jw_switch_set_admin_mode(int port_idx, void *p);
static int jw_switch_set_physical_mode(int port_idx, void *p);
static int jw_switch_set_flow_control_mode(int port_idx, void *p);
static int jw_switch_set_link_trap_mode(int port_idx, void *p);
static int jw_switch_set_threshold_trap_mode(int port_idx, void *p);
static int jw_switch_set_tx_threshold_value(int port_idx, void *p);
static int jw_switch_set_rx_threshold_value(int port_idx, void *p);


static int jw_switch_get_type(struct blob_buf *buf, int port_idx)
{
    int type = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "type", type);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_admin_mode(struct blob_buf *buf, int port_idx)
{
    int admin_mode = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    admin_mode = 1;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "admin_mode", admin_mode);

    blobmsg_close_table(buf, table);
    return 0;
}


static int jw_switch_get_link_status(struct blob_buf *buf, int port_idx)
{
    int link_status = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    link_status = 2;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "link_status", link_status);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_physical_mode(struct blob_buf *buf, int port_idx)
{
    int phy_mode = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    phy_mode = 3;

    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "physical_mode", phy_mode);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_physical_status(struct blob_buf *buf, int port_idx)
{
    int phy_status = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    phy_status = 4;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "physical_status", phy_status);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_flow_control_mode(struct blob_buf *buf, int port_idx)
{
    int fcm = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    fcm = 5;

    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "flow_control_mode", fcm);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_state(struct blob_buf *buf, int port_idx)
{
    int state = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    state = 6;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    //void *table = NULL;
    //table = blobmsg_open_table(buf, "state_tbl");
    blobmsg_add_u32(buf, "state", state);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_link_trap_mode(struct blob_buf *buf, int port_idx)
{
    int ltm = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    ltm = 7;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "link_trap_mode", ltm);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_threshold_trap_mode(struct blob_buf *buf, int port_idx)
{
    int ttm = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    ttm = 8;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "threshold_trap_mode", ttm);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_tx_threshold_value(struct blob_buf *buf, int port_idx)
{
    int ttv = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    ttv = 9;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "tx_threshold_value", ttv);
    blobmsg_close_table(buf, table);

    return 0;
}


static int jw_switch_get_rx_threshold_value(struct blob_buf *buf, int port_idx)
{
    int rtv = 0;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");

    rtv = 10;
    printf("[%s][%d]: starting get type handler\n", __func__, __LINE__);
    blobmsg_add_u32(buf, "rx_threshold_value", rtv);
    blobmsg_close_table(buf, table);

    return 0;
}


static const struct jw_switch_policy port_config_get_tbl[] = {
    [SWITCH_PORT_CFG_GET_TBL_TYPE] = {.name = "type", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_type},
    [SWITCH_PORT_CFG_GET_TBL_ADMIN_MODE] = {.name = "admin_mode", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_admin_mode}, 
    [SWITCH_PORT_CFG_GET_TBL_LINK_STATUS] = {.name = "link_status", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_link_status},
    [SWITCH_PORT_CFG_GET_TBL_PHYSICAL_MODE] = {.name = "physical_mode", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_physical_mode},
    [SWITCH_PORT_CFG_GET_TBL_PHYSICAL_STATUS] = {.name = "physical_status", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_physical_status},
    [SWITCH_PORT_CFG_GET_TBL_FLOW_CONTROL_MODE] = {.name = "flow_control_mode", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_flow_control_mode},
    [SWITCH_PORT_CFG_GET_TBL_STATE] = {.name = "state", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_state},
    [SWITCH_PORT_CFG_GET_TBL_LINK_TRAP_MODE] = {.name = "link_trap_mode", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_link_trap_mode},
    [SWITCH_PORT_CFG_GET_TBL_THRESHOLD_TRAP_MODE] = {.name = "threshold_trap_mode", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_threshold_trap_mode},
    [SWITCH_PORT_CFG_GET_TBL_TX_THRESHOLD_VALUE] = {.name = "tx_threshold_value", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_tx_threshold_value},
    [SWITCH_PORT_CFG_GET_TBL_RX_THRESHOLD_VALUE] = {.name = "rx_threshold_value", .type = BLOBMSG_TYPE_INT32, .get_handler = jw_switch_get_rx_threshold_value},
};


static int jw_switch_set_admin_mode(int port_idx, void *p)
{
    int am = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, am);

    return 0;
}


static int jw_switch_set_physical_mode(int port_idx, void *p)
{
    int pm = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, pm);

    return 0;
}


static int jw_switch_set_flow_control_mode(int port_idx, void *p)
{
    int fcm = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, fcm);

    return 0;
}


static int jw_switch_set_link_trap_mode(int port_idx, void *p)
{
    int ltm = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, ltm);

    return 0;
}


static int jw_switch_set_threshold_trap_mode(int port_idx, void *p)
{
    int ttm = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, ttm);

    return 0;
}


static int jw_switch_set_tx_threshold_value(int port_idx, void *p)
{
    int ttv = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, ttv);

    return 0;
}


static int jw_switch_set_rx_threshold_value(int port_idx, void *p)
{
    int rtv = *((int*)p);

    printf("[%s][%d]: starting set type handler, pass value [%d]\n", __func__, __LINE__, rtv);

    return 0;
}


static const struct jw_switch_policy port_config_set_tbl[] = {
    [SWITCH_PORT_CFG_SET_TBL_ADMIN_MODE] = {.name = "admin_mode", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_switch_set_admin_mode}, 
    [SWITCH_PORT_CFG_SET_TBL_PHYSICAL_MODE] = {.name = "physical_mode", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_switch_set_physical_mode},
    [SWITCH_PORT_CFG_SET_TBL_FLOW_CONTROL_MODE] = {.name = "flow_control_mode", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_switch_set_flow_control_mode},
    [SWITCH_PORT_CFG_SET_TBL_LINK_TRAP_MODE] = {.name = "link_trap_mode", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_switch_set_link_trap_mode},
    [SWITCH_PORT_CFG_SET_TBL_THRESHOLD_TRAP_MODE] = {.name = "threshold_trap_mode", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_switch_set_threshold_trap_mode},
    [SWITCH_PORT_CFG_SET_TBL_TX_THRESHOLD_VALUE] = {.name = "tx_threshold_value", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_switch_set_tx_threshold_value},
    [SWITCH_PORT_CFG_SET_TBL_RX_THRESHOLD_VALUE] = {.name = "rx_threshold_value", .type = BLOBMSG_TYPE_INT32, .set_handler = jw_switch_set_rx_threshold_value},
};

// 自定义端口参数获取解析 tb是一个三级消息（key="idx-X"，表示端口号; value=数组,表示一系列参数）
static int jw_switchd_parse_port_cfg_get(struct blob_attr *tb, struct blob_buf *buf)
{
    int port_idx = -1;
    int rem = 0;
    struct blob_attr *cur = NULL;   // 指向四级消息
    char *array_var = NULL;
    struct blobmsg_hdr *hdr = NULL;
    char *pidx = NULL;
    struct jw_switch_policy *jw_switch_p = NULL;

    hdr = blob_data(tb);    // 获取消息头
    
    // 判断消息名字符串中是否有"idx-"
    pidx = strstr(hdr->name, "idx-");
    if (pidx) 
    {
        printf("[%s][%d], pidx = [%s]\n", __func__, __LINE__, pidx);
        port_idx = atoi(pidx + 4);  // 获取端口号
        // 端口号范围0~32
        if (port_idx < 0 || port_idx > 32) {
            printf("[%s][%d], invalid port index %d\n", __func__, __LINE__, port_idx);
            return -1;
        }
    } 
    else {
        printf("[%s][%d], invalid port index arg [%s]\n", __func__, __LINE__, hdr->name);
        return -1;
    }

    void *msg_array = NULL;
    void *msg_table = NULL;
    
    msg_table = blobmsg_open_table(buf, hdr->name); // 进入表
    msg_array = blobmsg_open_array(buf, hdr->name); // 进入数组

    // 遍历数组元素,也就是当前端口的参数名,每个元素是一个四级消息
    blobmsg_for_each_attr(cur, tb, rem) 
    {
        // 四级消息必须是字符串格式
        if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING)   // 当数组元素是字符串
        {
            // 获取字符串（）
            array_var = blobmsg_get_string(cur);
            //printf("%s[%d]: array value = %s\n", __func__, __LINE__, array_var);
            // 根据字符串完成索引，找到对应参数名所在的控制块
            jw_switch_p = (struct jw_switch_policy *)jw_switchd_get_context(array_var, port_config_get_tbl, __SWITCH_PORT_CFG_GET_TBL_MAX);
            // 如果索引成功且控制块有效，则执行对应的处理函数
            if (jw_switch_p && jw_switch_p->get_handler) {
                jw_switch_p->get_handler(buf, port_idx);
            }
        }
    }

    blobmsg_close_array(buf, msg_array);
    blobmsg_close_table(buf, msg_table);

    return 0;
}


// 自定义端口参数设置解析 tb是一个三级消息（key="idx-X"，表示端口号; value=数组,表示一系列参数）
static int jw_switchd_parse_port_cfg_set(struct blob_attr *tb, struct blob_buf *buf)
{
    int port_idx = -1;
    int rem = 0;
    struct blob_attr *cur = NULL;   // 指向四级消息
    char *array_var = NULL;
    struct blobmsg_hdr *hdr = NULL;
    char *pidx = NULL;
    struct jw_switch_policy *jw_switch_p = NULL;

    hdr = blob_data(tb);    // 获取消息头

    // 判断消息名字符串中是否有"idx-"
    pidx = strstr(hdr->name, "idx-");
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

    printf("[%s][%d], port [%d]\n", __func__, __LINE__, port_idx);

    // 遍历数组元素,也就是当前端口的参数名,每个元素是一个四级消息,对应一个参数配置
    blobmsg_for_each_attr(cur, tb, rem) 
    {
        // 四级消息必须是table格式
        if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING) {
            array_var = blobmsg_get_string(cur);
            printf("%s[%d]: array value = %s\n", __func__, __LINE__, array_var);
        } 
        else if (blobmsg_type(cur) == BLOBMSG_TYPE_TABLE) 
        {
            printf("%s[%d]: BLOBMSG_TYPE_TABLE\n", __func__, __LINE__);
            // table的每个元素是一个五级消息
            struct blob_attr *t = blobmsg_data(cur);    // t指向五级消息
            int v = -1;
            char *name = NULL;

            // 获取五级消息的消息名，这才是真正的要设置的端口参数名
            name = ((struct blobmsg_hdr *)blob_data(t))->name;  

            // 五级消息必须是整形格式
            v = blobmsg_get_u32(t);

            //printf("%s[%d]: BLOBMSG_TYPE_TABLE, hdr name = [%s], var = [%d]\n", __func__, __LINE__, name, v);
            // 根据五级消息名完成索引，找到对应参数名所在的控制块
            jw_switch_p = (struct jw_switch_policy *)jw_switchd_get_context(name, port_config_set_tbl, __SWITCH_PORT_CFG_SET_TBL_MAX);
            if (jw_switch_p && jw_switch_p->set_handler) {
                // 执行参数对应的处理函数
                jw_switch_p->set_handler(port_idx, (void *)&v);
            }
        }
    }

    return 0;
}

// 处理函数 - [对象:port_config 方法:port_config_get]
static int port_config_get_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    // 一级消息tb指向port_config_get方法的唯一参数"get_args"
    struct blob_attr *tb[__SWITCH_PORT_CFG_GET_MAX];    

    // 根据指定策略对一级消息进行过滤
    blobmsg_parse(port_config_get_policy, __SWITCH_PORT_CFG_GET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[SWITCH_PORT_CFG_GET_KEYS]) {
        ERROR("invalid params in %s\n", __func__);
        return -1;
    }

	blob_buf_init(&b, 0);

    struct blob_attr *cur = NULL;       // cur指向一级消息
    struct blob_attr *_cur = NULL;      // _cur指向二级消息
    int rem = 0;
    char *array_val = NULL;

    cur = tb[SWITCH_PORT_CFG_GET_KEYS];           
    // 一级消息"get_args"必须是数组类型
    if (blobmsg_type(cur) != BLOBMSG_TYPE_ARRAY) {
        ERROR("blobmsg type is not array\n");
        return -1;
    }

    void *ret_table = NULL;
    ret_table = blobmsg_open_array(&b, "ret");      // 进入数组

    // 遍历数组元素，每个元素是一个二级消息
    blobmsg_for_each_attr(_cur, cur, rem) 
    {
        // 二级消息必须是table类型
        if (blobmsg_type(_cur) == BLOBMSG_TYPE_STRING) {
            array_val = blobmsg_get_string(_cur);
            printf("%s[%d]: array value = %s\n", __func__, __LINE__, array_val);
        } 
        else if (blobmsg_type(_cur) == BLOBMSG_TYPE_TABLE) 
        {
            // table的每个元素是一个三级消息"idx-X"
            struct blob_attr *tbl = blobmsg_data(_cur); // tbl指向三级消息
            // 三级消息"idx-X"必须是数组类型
            if (blobmsg_type(tbl) ==  BLOBMSG_TYPE_ARRAY) {
                printf("table value is array\n");
                // 解析三级消息数组中的每个元素
                jw_switchd_parse_port_cfg_get(tbl, &b);
            } 
            else if (blobmsg_type(tbl) == BLOBMSG_TYPE_INT32) {
                printf("%s[%d]: array value = %d\n", __func__, __LINE__, blobmsg_get_u32(tbl));
            }

        } 
        else if (blobmsg_type(_cur) == BLOBMSG_TYPE_ARRAY) {
            printf(">>>>>> msg type array <<<<<<<\n");
        } 
        else {
            printf("%s[%d]: invalid attr type\n", __func__, __LINE__);
        }
    }
    blobmsg_close_array(&b, ret_table);

	ubus_send_reply(ctx, req, b.head);

	return UBUS_STATUS_OK;
}


// 处理函数 - [对象:port_config 方法:port_config_set]
static int port_config_set_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    // 一级消息tb指向port_config_get方法的唯一参数"set_args"
    struct blob_attr *tb[__SWITCH_PORT_CFG_SET_MAX];

	blob_buf_init(&b, 0);

    blobmsg_parse(port_config_set_policy, __SWITCH_PORT_CFG_SET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[SWITCH_PORT_CFG_SET_ARRAY]) {
        ERROR("invalid params in %s\n", __func__);
        blobmsg_add_u32(&b, "ret", 0);    // 0 for fail, 1 for success
        return -1;
    }


    struct blob_attr *cur = NULL;   // cur指向一级消息
    struct blob_attr *_cur = NULL;  // _cur指向二级消息
    int rem = 0;
    char *array_val = NULL;

    cur = tb[SWITCH_PORT_CFG_SET_ARRAY];
    // 一级消息"set_args"必须是数组类型
    if (blobmsg_type(cur) != BLOBMSG_TYPE_ARRAY) {
        ERROR("blobmsg type is not array\n");
        blobmsg_add_u32(&b, "ret", 0);    // 0 for fail, 1 for success
        return -1;
    }

    // 遍历数组元素，每个元素是一个二级消息
    blobmsg_for_each_attr(_cur, cur, rem) 
    {
        // 二级消息必须是table类型
        if (blobmsg_type(_cur) == BLOBMSG_TYPE_TABLE) 
        {
            printf(">>>>>> msg type table <<<<<<<\n");
            // table的每个元素是一个三级消息"idx-X"
            struct blob_attr *tbl = blobmsg_data(_cur); // tbl指向三级消息
            // 三级消息"idx-X"必须是数组类型
            if (blobmsg_type(tbl) ==  BLOBMSG_TYPE_ARRAY) 
            {
                printf("table value is array\n");
                // 解析三级消息数组中的每个元素
                jw_switchd_parse_port_cfg_set(tbl, &b);
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


static const struct ubus_method port_config_methods[] = {
	UBUS_METHOD("port_config_get", port_config_get_handler, port_config_get_policy),
	UBUS_METHOD("port_config_set", port_config_set_handler, port_config_set_policy),
};


static struct ubus_object_type switch_object_type =
	UBUS_OBJECT_TYPE("port_config", port_config_methods);


static struct ubus_object switch_object = {
	.name = "port_config",
	.type = &switch_object_type,
	.methods = port_config_methods,
	.n_methods = ARRAY_SIZE(port_config_methods),
};


void ubus_init_switch(struct ubus_context *ctx)
{
	int ret;

	_ctx = ctx;
	ret = ubus_add_object(ctx, &switch_object);
	if (ret)
		ERROR("Failed to add object port config: %s\n", ubus_strerror(ret));
}
