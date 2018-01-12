/*
 * ndpi_util.h
 *
 * Copyright (C) 2011-16 - ntop.org
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * This module contains routines to help setup a simple nDPI program.
 *
 * If you concern about performance or have to integrate nDPI in your
 * application, you could need to reimplement them yourself.
 *
 * WARNING: this API is unstable! Use it at your own risk!
 */
#ifndef __NDPI_UTIL_H__
#define __NDPI_UTIL_H__

#include <pcap.h>

#define MAX_NUM_READER_THREADS     16
#define IDLE_SCAN_PERIOD           10 /* msec (use TICK_RESOLUTION = 1000) 超过该间隔时间就需要清理一下空闲的数据流 */
#define MAX_IDLE_TIME           30000   // msec 用来判断数据流是否已经处于空闲状态的时间间隔
#define IDLE_SCAN_BUDGET         1024
#define NUM_ROOTS                 512
#define MAX_NDPI_FLOWS      200000000   // 可以容纳的数据流上限
#define TICK_RESOLUTION          1000
#define MAX_NUM_IP_ADDRESS          5  /* len of ip address array */
#define UPDATED_TREE                1
#define AGGRESSIVE_PERCENT      95.00
#define DIR_SRC                    10
#define DIR_DST                    20

/* flow tracking 定义了数据流的跟踪信息(作为二叉树的节点数据来进行管理)
 * 
 * 备注：这是一个属于demo的结构，而ndpi_flow_struct是一个属于nDPI核心库的结构，包含了真正的数据流信息
 *       这个结构可以看做是数据流结构的一层壳，其目的是方便对真正数据流的管理
 */
typedef struct ndpi_flow_info {
  u_int32_t hashval;            // (protocol + vlan_id + lower_ip + upper_id + lower_port + upper_port) % 512
  u_int32_t lower_ip;           // 记录了该数据流的源IP
  u_int32_t upper_ip;           // 记录了该数据流的目的IP
  u_int16_t lower_port;         // 记录了该数据流的源端口
  u_int16_t upper_port;         // 记录了该数据流的目的端口
  u_int8_t detection_completed; // 标识是否完成对该数据流的探测
  u_int8_t protocol;            // 记录了该数据流的传输层协议号(比如TCP)
  u_int8_t bidirectional;       // 标识是否是一条双向流
  u_int16_t vlan_id;            // 记录了该数据流关联的VLAN ID
  struct ndpi_flow_struct *ndpi_flow;   // 指向nDPI核心库中的数据流结构
  char lower_name[48], upper_name[48];  // 分别记录了该数据流的源IP名和目的IP名
  u_int8_t ip_version;
  u_int64_t last_seen;  // 记录了最近收到包的时间
  u_int64_t bytes;      // 记录了收到的包字节总数(前提是识别为有效流)
  u_int32_t packets;    // 记录了收到的包数量(前提是识别为有效流)
  
  // result only, not used for flow identification 记录了该数据流的识别结果
  ndpi_protocol detected_protocol;

  char info[96];
  char host_server_name[192];
  char bittorent_hash[41];

  struct {
    char client_info[48], server_info[48];
  } ssh_ssl;

  void *src_id, *dst_id;
} ndpi_flow_info_t;


// flow statistics info 定义了对应工作流的统计信息
typedef struct ndpi_stats {
  u_int32_t guessed_flow_protocols;
  u_int64_t raw_packet_count;           // 记录了从libpcap收到的原始报文数量
  u_int64_t ip_packet_count;            // 记录了收到的IP报文数量
  u_int64_t total_wire_bytes, total_ip_bytes, total_discarded_bytes;
  u_int64_t protocol_counter[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];     // 这张表记录了每种使能的协议收到的报文数量
  u_int64_t protocol_counter_bytes[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];   // 这张表记录了每种使能的协议收到的字节数量
  u_int32_t protocol_flows[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];       // 这张表记录了每种使能的协议收到的流数量
  u_int32_t ndpi_flow_count;        // 记录了数据流的数量 
  u_int64_t tcp_count, udp_count;   // 分别记录了收到的TCP、UDP报文数量
  u_int64_t mpls_count;
  u_int64_t pppoe_count;
  u_int64_t vlan_count;             // 记录了收到的vlan报文数量
  u_int64_t fragmented_count;       // 记录了收到的IP分片报文数量
  u_int64_t packet_len[6];          // 收到的不同长度(基于传输层长度)的包统计
                                    // 6个元素依次对应：<64、64<= && <128、128<= && <256、256<= && <1024、1204<= && <1500、1500<=
  u_int16_t max_packet_len;         // 记录了收到的最长包
} ndpi_stats_t;


// flow preferences 定义了工作流的参数配置块
typedef struct ndpi_workflow_prefs {
  u_int8_t decode_tunnels;  // 标识是否使能解析GTP隧道功能
  u_int8_t quiet_mode;      // 标识是否使能安静模式
  u_int32_t num_roots;      // 固定为NUM_ROOTS
  u_int32_t max_ndpi_flows; // 固定为MAX_NDPI_FLOWS
} ndpi_workflow_prefs_t;

struct ndpi_workflow;

/** workflow, flow, user data */
typedef void (*ndpi_workflow_callback_ptr) (struct ndpi_workflow *, struct ndpi_flow_info *, void *);


// workflow main structure 定义了工作流的主结构
typedef struct ndpi_workflow {
  u_int64_t last_time;      // 该工作流最近一次收到包的时间

  struct ndpi_workflow_prefs prefs; // 记录了该工作流关联的参数配置块
  struct ndpi_stats stats;

  ndpi_workflow_callback_ptr __flow_detected_callback;  // 完成探测后的回调函数 (on_protocol_discovered)
  void * __flow_detected_udata;                         // 传递给__flow_detected_callback回调函数的参数
  ndpi_workflow_callback_ptr __flow_giveup_callback;
  void * __flow_giveup_udata;

  /* outside referencies */
  pcap_t *pcap_handle;      // 指向该工作流关联的pcap句柄

  /* allocated by prefs */
  void **ndpi_flows_root;   // 一个prefs.num_roots长度的hash数组，每个成员是二叉树的根节点，所以整个组织形式为 hash + 二叉树结构，存放了所有的数据流
  struct ndpi_detection_module_struct *ndpi_struct; // 指向该工作流专属的探测模块
} ndpi_workflow_t;


/* TODO: remove wrappers parameters and use ndpi global, when their initialization will be fixed... */
struct ndpi_workflow * ndpi_workflow_init(const struct ndpi_workflow_prefs * prefs, pcap_t * pcap_handle);


 /* workflow main free function */
void ndpi_workflow_free(struct ndpi_workflow * workflow);


/** Free flow_info ndpi support structures but not the flow_info itself
 *
 *  TODO remove! Half freeing things is bad!
 */
void ndpi_free_flow_info_half(struct ndpi_flow_info *flow);


/* Process a packet and update the workflow  */
struct ndpi_proto ndpi_workflow_process_packet(struct ndpi_workflow * workflow,
					       const struct pcap_pkthdr *header,
					       const u_char *packet);


/* flow callbacks for complete detected flow
   (ndpi_flow_info will be freed right after) 
   为指定的工作流注册探测完成后的回调函数
   */
static inline void ndpi_workflow_set_flow_detected_callback(struct ndpi_workflow * workflow, ndpi_workflow_callback_ptr callback, void * udata) {
  workflow->__flow_detected_callback = callback;
  workflow->__flow_detected_udata = udata;
}

/* flow callbacks for sufficient detected flow
   (ndpi_flow_info will be freed right after) */
static inline void ndpi_workflow_set_flow_giveup_callback(struct ndpi_workflow * workflow, ndpi_workflow_callback_ptr callback, void * udata) {
  workflow->__flow_giveup_callback = callback;
  workflow->__flow_giveup_udata = udata;
}

 /* compare two nodes in workflow */
int ndpi_workflow_node_cmp(const void *a, const void *b);
void process_ndpi_collected_info(struct ndpi_workflow * workflow, struct ndpi_flow_info *flow);
u_int32_t ethernet_crc32(const void* data, size_t n_bytes);
void ndpi_flow_info_freer(void *node);
#endif
