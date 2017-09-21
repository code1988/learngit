/*
 * hostapd / Configuration definitions and helpers functions
 * Copyright (c) 2003-2009, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef HOSTAPD_CONFIG_H
#define HOSTAPD_CONFIG_H

#include "common/defs.h"
#include "ip_addr.h"
#include "common/wpa_common.h"
#include "wps/wps.h"

#define MAX_STA_COUNT 2007
#define MAX_VLAN_ID 4094

typedef u8 macaddr[ETH_ALEN];

// 黑/白名单条目
struct mac_acl_entry {
	macaddr addr;
	int vlan_id;
};

struct hostapd_radius_servers;
struct ft_remote_r0kh;
struct ft_remote_r1kh;

#define HOSTAPD_MAX_SSID_LEN 32

#define NUM_WEP_KEYS 4
struct hostapd_wep_keys {
	u8 idx;
	u8 *key[NUM_WEP_KEYS];
	size_t len[NUM_WEP_KEYS];
	int keys_set;
	size_t default_len; /* key length used for dynamic key generation */
};

typedef enum hostap_security_policy {
	SECURITY_PLAINTEXT = 0,
	SECURITY_STATIC_WEP = 1,
	SECURITY_IEEE_802_1X = 2,
	SECURITY_WPA_PSK = 3,
	SECURITY_WPA = 4
} secpolicy;

// ssid控制块
struct hostapd_ssid {
	char ssid[HOSTAPD_MAX_SSID_LEN + 1];
	size_t ssid_len;
	int ssid_set;

	char vlan[IFNAMSIZ + 1];
	secpolicy security_policy;

	struct hostapd_wpa_psk *wpa_psk;
	char *wpa_passphrase;
	char *wpa_psk_file;

	struct hostapd_wep_keys wep;

#define DYNAMIC_VLAN_DISABLED 0 // 动态vlan功能禁止
#define DYNAMIC_VLAN_OPTIONAL 1 // 动态vlan功能可选
#define DYNAMIC_VLAN_REQUIRED 2 // 动态vlan功能必须
	int dynamic_vlan;       // 动态vlan功能的标志位，取DYNAMIC_VLAN_*
#ifdef CONFIG_FULL_DYNAMIC_VLAN
	char *vlan_tagged_interface;    // 指定了vlan的宿主设备
#endif /* CONFIG_FULL_DYNAMIC_VLAN */
	struct hostapd_wep_keys **dyn_vlan_keys;
	size_t max_dyn_vlan_keys;
};


#define VLAN_ID_WILDCARD -1
// 单向vlan链表节点
struct hostapd_vlan {
	struct hostapd_vlan *next;
	int vlan_id; /* VLAN ID or -1 (VLAN_ID_WILDCARD) for wildcard entry */
	char ifname[IFNAMSIZ + 1];
	int dynamic_vlan;       // 该动态vlan的引用计数，每增加一个sta加入到该vlan，引用计数加1
#ifdef CONFIG_FULL_DYNAMIC_VLAN

#define DVLAN_CLEAN_BR 	0x1
#define DVLAN_CLEAN_VLAN	0x2
#define DVLAN_CLEAN_VLAN_PORT	0x4
#define DVLAN_CLEAN_WLAN_PORT	0x8
	int clean;
#endif /* CONFIG_FULL_DYNAMIC_VLAN */
};

#define PMK_LEN 32
struct hostapd_wpa_psk {
	struct hostapd_wpa_psk *next;
	int group;
	u8 psk[PMK_LEN];
	u8 addr[ETH_ALEN];
};

#define EAP_USER_MAX_METHODS 8      // 每个用户最多支持的认证方式数量
// 使能EAP认证服务器功能时使用，用户信息表项
struct hostapd_eap_user {
	struct hostapd_eap_user *next;  // 指向下一条用户信息
	u8 *identity;                   // 用户名信息
	size_t identity_len;            // 用户名长度
	struct {
		int vendor;
		u32 method;
	} methods[EAP_USER_MAX_METHODS];
	u8 *password;                   // 用户密码
	size_t password_len;            // 密码长度
	int phase2;
	int force_version;
	unsigned int wildcard_prefix:1;
	unsigned int password_hash:1; /* whether password is hashed with
				       * nt_password_hash() */
	int ttls_auth; /* EAP_TTLS_AUTH_* bitfield */
};


#define NUM_TX_QUEUES 4

struct hostapd_tx_queue_params {
	int aifs;
	int cwmin;
	int cwmax;
	int burst; /* maximum burst time in 0.1 ms, i.e., 10 = 1 ms */
};

struct hostapd_wmm_ac_params {
	int cwmin;
	int cwmax;
	int aifs;
	int txop_limit; /* in units of 32us */
	int admission_control_mandatory;
};


#define MAX_ROAMING_CONSORTIUM_LEN 15

struct hostapd_roaming_consortium {
	u8 len;
	u8 oi[MAX_ROAMING_CONSORTIUM_LEN];
};

/**
 * struct hostapd_bss_config - Per-BSS configuration
 * 每个bss的配置控制块
 */
struct hostapd_bss_config {
	char iface[IFNAMSIZ + 1];       // 此bss名
	char bridge[IFNAMSIZ + 1];      // 此bss所属的网桥名，网桥这个参数这里似乎没有用在wire类型的驱动器中
	char wds_bridge[IFNAMSIZ + 1];

	enum hostapd_logger_level logger_syslog_level, logger_stdout_level;

	unsigned int logger_syslog; /* module bitfield */
	unsigned int logger_stdout; /* module bitfield */

	char *dump_log_name; /* file name for state dump (SIGUSR1) */

	int max_num_sta; /* maximum number of STAs in station table */

	int dtim_period;

	int ieee802_1x; /* use IEEE 802.1X */
	int eapol_version;
	int eap_server; /* Use internal EAP server instead of external
			 * RADIUS server 是否使用本地eap认证服务器标志位*/
	struct hostapd_eap_user *eap_user;      // 当使能了本地EAP服务器时，这里指向一张有效用户信息表
	char *eap_sim_db;
	struct hostapd_ip_addr own_ip_addr;
	char *nas_identifier;
	struct hostapd_radius_servers *radius;  // radius客户端配置的radius服务器信息
	int acct_interim_interval;              // 本地配置的 对于某个确定的会话，中间更新流量信息的时间间隔

	struct hostapd_ssid ssid;               // ssid(服务集标识)控制块，用来区分不同的网络

	char *eap_req_id_text; /* optional displayable message sent with
				* EAP Request-Identity 认证者发送eap-req-id时，附带一个可显示的信息，这是非必须的*/
	size_t eap_req_id_text_len;             // 上面这段可显示信息的长度
	int eapol_key_index_workaround;

	size_t default_wep_key_len;             // 设置WEP用的key长度(不用wep就不用管)
	int individual_wep_key_len;             // 设置WEP用的key周期
	int wep_rekeying_period;
	int broadcast_key_idx_min, broadcast_key_idx_max;
	int eap_reauth_period;                  // 重认证周期，大于0意味着该功能使能(只用于认证系统)

	int ieee802_11f; /* use IEEE 802.11f (IAPP) */
	char iapp_iface[IFNAMSIZ + 1]; /* interface used with IAPP broadcast
					* frames */

	enum {
		ACCEPT_UNLESS_DENIED = 0,
		DENY_UNLESS_ACCEPTED = 1,
		USE_EXTERNAL_RADIUS_AUTH = 2
	} macaddr_acl;
	struct mac_acl_entry *accept_mac;       // 该bss的mac白名单首地址
	int num_accept_mac;                     // 白名单条目数量
	struct mac_acl_entry *deny_mac;         // 该bss的mac黑名单首地址
	int num_deny_mac;                       // 黑名单条目数量
	int wds_sta;
	int isolate;

	int auth_algs; /* bitfield of allowed IEEE 802.11 authentication
			* algorithms, WPA_AUTH_ALG_{OPEN,SHARED,LEAP} */

	int wpa; /* bitfield of WPA_PROTO_WPA, WPA_PROTO_RSN  wpa/wpa2功能选择位*/
	int wpa_key_mgmt;
#ifdef CONFIG_IEEE80211W
	enum mfp_options ieee80211w;
	/* dot11AssociationSAQueryMaximumTimeout (in TUs) */
	unsigned int assoc_sa_query_max_timeout;
	/* dot11AssociationSAQueryRetryTimeout (in TUs) */
	int assoc_sa_query_retry_timeout;
#endif /* CONFIG_IEEE80211W */
	int wpa_pairwise;
	int wpa_group;
	int wpa_group_rekey;
	int wpa_strict_rekey;
	int wpa_gmk_rekey;
	int wpa_ptk_rekey;
	int rsn_pairwise;
	int rsn_preauth;
	char *rsn_preauth_interfaces;
	int peerkey;

#ifdef CONFIG_IEEE80211R
	/* IEEE 802.11r - Fast BSS Transition */
	u8 mobility_domain[MOBILITY_DOMAIN_ID_LEN];
	u8 r1_key_holder[FT_R1KH_ID_LEN];
	u32 r0_key_lifetime;
	u32 reassociation_deadline;
	struct ft_remote_r0kh *r0kh_list;
	struct ft_remote_r1kh *r1kh_list;
	int pmk_r1_push;
	int ft_over_ds;
#endif /* CONFIG_IEEE80211R */

	char *ctrl_interface; /* directory for UNIX domain sockets */
#ifndef CONFIG_NATIVE_WINDOWS
	gid_t ctrl_interface_gid;
#endif /* CONFIG_NATIVE_WINDOWS */
	int ctrl_interface_gid_set;

	char *ca_cert;
	char *server_cert;
	char *private_key;
	char *private_key_passwd;
	int check_crl;
	char *dh_file;
	u8 *pac_opaque_encr_key;
	u8 *eap_fast_a_id;
	size_t eap_fast_a_id_len;
	char *eap_fast_a_id_info;
	int eap_fast_prov;
	int pac_key_lifetime;
	int pac_key_refresh_time;
	int eap_sim_aka_result_ind;
	int tnc;
	int fragment_size;
	u16 pwd_group;

	char *radius_server_clients;
	int radius_server_auth_port;
	int radius_server_ipv6;

	char *test_socket; /* UNIX domain socket path for driver_test */

	int use_pae_group_addr; /* Whether to send EAPOL frames to PAE group
				 * address instead of individual address
				 * (for driver_wired.c).
				 */

	int ap_max_inactivity;      // 定义了每个站表元素的老化时间
	int ignore_broadcast_ssid;

	int wmm_enabled;
	int wmm_uapsd;

	struct hostapd_vlan *vlan, *vlan_tail;  // @vlan:指向该bss支持的vlan集合 @vlan_tail:指向vlan集合的尾节点

	macaddr bssid;

	/*
	 * Maximum listen interval that STAs can use when associating with this
	 * BSS. If a STA tries to use larger value, the association will be
	 * denied with status code 51.
	 */
	u16 max_listen_interval;

	int disable_pmksa_caching;
	int okc; /* Opportunistic Key Caching */

	int wps_state;
#ifdef CONFIG_WPS
	int ap_setup_locked;
	u8 uuid[16];
	char *wps_pin_requests;
	char *device_name;
	char *manufacturer;
	char *model_name;
	char *model_number;
	char *serial_number;
	u8 device_type[WPS_DEV_TYPE_LEN];
	char *config_methods;
	u8 os_version[4];
	char *ap_pin;
	int skip_cred_build;
	u8 *extra_cred;
	size_t extra_cred_len;
	int wps_cred_processing;
	u8 *ap_settings;
	size_t ap_settings_len;
	char *upnp_iface;
	char *friendly_name;
	char *manufacturer_url;
	char *model_description;
	char *model_url;
	char *upc;
	struct wpabuf *wps_vendor_ext[MAX_WPS_VENDOR_EXTENSIONS];
#endif /* CONFIG_WPS */
	int pbc_in_m1;

#define P2P_ENABLED BIT(0)
#define P2P_GROUP_OWNER BIT(1)
#define P2P_GROUP_FORMATION BIT(2)
#define P2P_MANAGE BIT(3)
#define P2P_ALLOW_CROSS_CONNECTION BIT(4)
	int p2p;

	int disassoc_low_ack;

#define TDLS_PROHIBIT BIT(0)
#define TDLS_PROHIBIT_CHAN_SWITCH BIT(1)
	int tdls;
	int disable_11n;

	/* IEEE 802.11v */
	int time_advertisement;
	char *time_zone;

	/* IEEE 802.11u - Interworking */
	int interworking;
	int access_network_type;
	int internet;
	int asra;
	int esr;
	int uesa;
	int venue_info_set;
	u8 venue_group;
	u8 venue_type;
	u8 hessid[ETH_ALEN];

	/* IEEE 802.11u - Roaming Consortium list */
	unsigned int roaming_consortium_count;
	struct hostapd_roaming_consortium *roaming_consortium;
};


/**
 * struct hostapd_config - Per-radio interface configuration
 * 每个接口的配置参数控制块
 */
struct hostapd_config {
    // 每个接口包含的bss配置项通常就只有1个，实际分析时也暂时不考虑多bss的情况
	struct hostapd_bss_config *bss, *last_bss;  // @bss:指向该接口包含的bss配置表(默认就1个表项) @last_bss:指向bss配置表最后一个表项
	size_t num_bss;     // 当前接口下bss的数量，同时也代表了有多少个网桥，默认就是1

	u16 beacon_int;
	int rts_threshold;
	int fragm_threshold;
	u8 send_probe_response;
	u8 channel;
	enum hostapd_hw_mode hw_mode; /* HOSTAPD_MODE_IEEE80211A, .. */
	enum {
		LONG_PREAMBLE = 0,
		SHORT_PREAMBLE = 1
	} preamble;

	int *supported_rates;
	int *basic_rates;

    const struct wpa_driver_ops *driver;    // 指向具体类型驱动程序API集合

	int ap_table_max_size;
	int ap_table_expiration_time;

	char country[3]; /* first two octets: country code as described in
			  * ISO/IEC 3166-1. Third octet:
			  * ' ' (ascii 32): all environments
			  * 'O': Outdoor environemnt only
			  * 'I': Indoor environment only
			  */

	int ieee80211d;

	struct hostapd_tx_queue_params tx_queue[NUM_TX_QUEUES];

	/*
	 * WMM AC parameters, in same order as 802.1D, i.e.
	 * 0 = BE (best effort)
	 * 1 = BK (background)
	 * 2 = VI (video)
	 * 3 = VO (voice)
	 */
	struct hostapd_wmm_ac_params wmm_ac_params[4];

	int ht_op_mode_fixed;
	u16 ht_capab;
	int ieee80211n;
	int secondary_channel;
	int require_ht;
};


int hostapd_mac_comp(const void *a, const void *b);
int hostapd_mac_comp_empty(const void *a);
struct hostapd_config * hostapd_config_defaults(void);
void hostapd_config_defaults_bss(struct hostapd_bss_config *bss);
void hostapd_config_free(struct hostapd_config *conf);
int hostapd_maclist_found(struct mac_acl_entry *list, int num_entries,
			  const u8 *addr, int *vlan_id);
int hostapd_rate_found(int *list, int rate);
int hostapd_wep_key_cmp(struct hostapd_wep_keys *a,
			struct hostapd_wep_keys *b);
const u8 * hostapd_get_psk(const struct hostapd_bss_config *conf,
			   const u8 *addr, const u8 *prev_psk);
int hostapd_setup_wpa_psk(struct hostapd_bss_config *conf);
const char * hostapd_get_vlan_id_ifname(struct hostapd_vlan *vlan,
					int vlan_id);
const struct hostapd_eap_user *
hostapd_get_eap_user(const struct hostapd_bss_config *conf, const u8 *identity,
		     size_t identity_len, int phase2);

#endif /* HOSTAPD_CONFIG_H */
