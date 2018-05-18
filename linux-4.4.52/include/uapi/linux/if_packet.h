#ifndef __LINUX_IF_PACKET_H
#define __LINUX_IF_PACKET_H

#include <linux/types.h>

struct sockaddr_pkt {
	unsigned short spkt_family;
	unsigned char spkt_device[14];
	__be16 spkt_protocol;
};

// 链路层地址结构
struct sockaddr_ll {
	unsigned short	sll_family; // 地址族固定为AF_PACKET
	__be16		sll_protocol;   // 标识上层承载的802.3标准以太网协议类型
	int		sll_ifindex;        // 接口索引号（0匹配任何接口，但只允许用于bind时）
	unsigned short	sll_hatype; // 硬件地址类型ARPHDR_*（optional,只有接收时有意义）
	unsigned char	sll_pkttype;// 包类型（optional,只有接收时有意义）
	unsigned char	sll_halen;  // MAC地址长度（optional,只有接收时有意义）
	unsigned char	sll_addr[8];// 目的MAC地址（optional,只有接收时有意义）
};

/* Packet types 
 * 包类型(可以用来设置skb->pkt_type字段)
 *
 * 备注： 以太网驱动程序都会在接收路径中调用eth_type_trans来确定包类型,所以包类型只对接收到的数据包有意义
 *        这里定义的包类型和以太网协议类型的区别在于进行分类的视角不同
 * */
#define PACKET_HOST		0		    /* To us    标示目标地址是本机的数据包		*/
#define PACKET_BROADCAST	1		/* To all	标示物理层广播包	*/
#define PACKET_MULTICAST	2		/* To group	标示物理层组播包	*/
#define PACKET_OTHERHOST	3		/* To someone else 	    标示目标地址是其他主机的数据包 */
#define PACKET_OUTGOING		4		/* Outgoing of any type 标示本地主机的环回包(这个似乎是发送时的包类型)*/
#define PACKET_LOOPBACK		5		/* MC/BRD frame looped back  标示这是个loopback包 */
#define PACKET_USER		6		/* To user space	*/
#define PACKET_KERNEL		7		/* To kernel space	*/
/* Unused, PACKET_FASTROUTE and PACKET_LOOPBACK are invisible to user space */
#define PACKET_FASTROUTE	6		/* Fastrouted frame	*/

/* Packet socket options 
 * SOL_PACKET层可以设置的选项
 * */
#define PACKET_ADD_MEMBERSHIP		1   // 往PROMISC/MULTICAST/ALLMULTI中添加成员，对应的参数结构:struct packet_mreq
#define PACKET_DROP_MEMBERSHIP		2   // 从PROMISC/MULTICAST/ALLMULTI中退出成员
#define PACKET_RECV_OUTPUT		3
/* Value 4 is still used by obsolete turbo-packet. */
#define PACKET_RX_RING			5   // 创建环形接收缓冲区
#define PACKET_STATISTICS		6   // 获取环形缓冲区的统计信息，对应的参数结构：struct tpacket_stats_v3
#define PACKET_COPY_THRESH		7                                                                                                  
#define PACKET_AUXDATA			8                                                                                                  
#define PACKET_ORIGDEV			9                                                                                                  
#define PACKET_VERSION			10  // 设置/获取环形缓冲区版本，目前有3个版本可选
#define PACKET_HDRLEN			11  // 获取指定版本环形缓冲区帧头长(通常也用该选项来判断内核是否支持指定版本的环形缓冲区)
#define PACKET_RESERVE			12  // 设置环形接收缓冲区中每个帧的保留空间长度(作为额外的headroom)，通常用于预留4字节VLAN tag空间
#define PACKET_TX_RING			13  // 创建环形发送缓冲区
#define PACKET_LOSS			14
#define PACKET_VNET_HDR			15
#define PACKET_TX_TIMESTAMP		16
#define PACKET_TIMESTAMP		17
#define PACKET_FANOUT			18
#define PACKET_TX_HAS_OFF		19
#define PACKET_QDISC_BYPASS		20
#define PACKET_ROLLOVER_STATS		21
#define PACKET_FANOUT_DATA		22

#define PACKET_FANOUT_HASH		0
#define PACKET_FANOUT_LB		1
#define PACKET_FANOUT_CPU		2
#define PACKET_FANOUT_ROLLOVER		3
#define PACKET_FANOUT_RND		4
#define PACKET_FANOUT_QM		5
#define PACKET_FANOUT_CBPF		6
#define PACKET_FANOUT_EBPF		7
#define PACKET_FANOUT_FLAG_ROLLOVER	0x1000
#define PACKET_FANOUT_FLAG_DEFRAG	0x8000

/* 环形缓冲区TPACKET_V1、TPACKET_V2、TPACKET_V3之间的差别如下
 *      TPACKET_V1 : 这是缺省的环形缓冲区版本
 *      TPACKET_V2 : 相比V1的改进有以下几点
 *                          32位的用户空间环形缓冲区可以基于64位内核工作;
 *                          时间戳的精度从ms提升到ns;
 *                          支持携带VLAN信息(这意味着通过V1接收到的vlan包将会丢失vlan信息)；
 *      TPACKET_V3 : 相比V2的改进有以下几点
 *                          内存块可以配置成可变帧长(V1、V2的帧长都是tpacket_req.tp_frame_size固定值);
 *                          read/poll基于block-level(V1、V2基于packet_level);
 *                          开始支持poll超时参数；
 *                          新增了用户可配置选项：tp_retire_blk_tov、tpkt_hdr::sk_rxhash;
 *                          RX Hash数据可以被用户使用；
 *                   需要注意的是，V3当前只支持接收环形缓冲区
 */

struct tpacket_stats {
	unsigned int	tp_packets;
	unsigned int	tp_drops;
};

// PACKET_STATISTICS选项对应的参数结构
struct tpacket_stats_v3 {
	unsigned int	tp_packets;     // 统计收到的包数量
	unsigned int	tp_drops;       // 统计丢弃的包数量
	unsigned int	tp_freeze_q_cnt;// 统计冻结的包数量
};

struct tpacket_rollover_stats {
	__aligned_u64	tp_all;
	__aligned_u64	tp_huge;
	__aligned_u64	tp_failed;
};

union tpacket_stats_u {
	struct tpacket_stats stats1;
	struct tpacket_stats_v3 stats3;
};

struct tpacket_auxdata {
	__u32		tp_status;
	__u32		tp_len;
	__u32		tp_snaplen;
	__u16		tp_mac;
	__u16		tp_net;
	__u16		tp_vlan_tci;
	__u16		tp_vlan_tpid;
};

/* Rx ring - header status 
 * 这部分专门定义了接收环形缓冲区内存块的状态，用于设置tpacket_hdr_v1.block_status
 * */
#define TP_STATUS_KERNEL		      0     // 标识内存块正在被内核使用
#define TP_STATUS_USER			(1 << 0)    // 标识内存块可以提供给用户读取数据
#define TP_STATUS_COPY			(1 << 1)
#define TP_STATUS_LOSING		(1 << 2)
#define TP_STATUS_CSUMNOTREADY		(1 << 3)
#define TP_STATUS_VLAN_VALID		(1 << 4) /* auxdata has valid tp_vlan_tci */
#define TP_STATUS_BLK_TMO		(1 << 5)
#define TP_STATUS_VLAN_TPID_VALID	(1 << 6) /* auxdata has valid tp_vlan_tpid */
#define TP_STATUS_CSUM_VALID		(1 << 7)

/* Tx ring - header status 
 * 这部分专门定义了发送环形缓冲区内存块的状态，用于设置tpacket_hdr_v1.block_status
 * */
#define TP_STATUS_AVAILABLE	      0
#define TP_STATUS_SEND_REQUEST	(1 << 0)
#define TP_STATUS_SENDING	(1 << 1)
#define TP_STATUS_WRONG_FORMAT	(1 << 2)

/* Rx and Tx ring - header status */
#define TP_STATUS_TS_SOFTWARE		(1 << 29)
#define TP_STATUS_TS_SYS_HARDWARE	(1 << 30) /* deprecated, never set */
#define TP_STATUS_TS_RAW_HARDWARE	(1 << 31)

/* Rx ring - feature request bits */
#define TP_FT_REQ_FILL_RXHASH	0x1

// TPACKET_V1环形缓冲区每个帧的头部结构
struct tpacket_hdr {
	unsigned long	tp_status;
	unsigned int	tp_len;
	unsigned int	tp_snaplen;
	unsigned short	tp_mac;
	unsigned short	tp_net;
	unsigned int	tp_sec;
	unsigned int	tp_usec;
};

/* V1帧结构基本如下：
 *          struct tpacket_hdr + padding + struct sockaddr_ll + frame_data
 * V2帧结构基本如下(V3类似)：
 *          struct tpacket_hdr + struct sockaddr_ll + padding + frame_data
 */
#define TPACKET_ALIGNMENT	16
#define TPACKET_ALIGN(x)	(((x)+TPACKET_ALIGNMENT-1)&~(TPACKET_ALIGNMENT-1))
#define TPACKET_HDRLEN		(TPACKET_ALIGN(sizeof(struct tpacket_hdr)) + sizeof(struct sockaddr_ll))    // V1环形缓冲区帧头长度

// TPACKET_V2环形缓冲区每个帧的头部结构
struct tpacket2_hdr {
	__u32		tp_status;
	__u32		tp_len;
	__u32		tp_snaplen;
	__u16		tp_mac;
	__u16		tp_net;
	__u32		tp_sec;
	__u32		tp_nsec;
	__u16		tp_vlan_tci;
	__u16		tp_vlan_tpid;
	__u8		tp_padding[4];
};

// TPACKET_V3环形缓冲区帧头子结构，主要包含了VLAN信息
struct tpacket_hdr_variant1 {
	__u32	tp_rxhash;
	__u32	tp_vlan_tci;    // 低12bit为vid
	__u16	tp_vlan_tpid;   // 缺省就是0x8100
	__u16	tp_padding;
};

// TPACKET_V3环形缓冲区每个帧的头部结构
struct tpacket3_hdr {
	__u32		tp_next_offset; // 指向同一个内存块中的下一个帧
	__u32		tp_sec;         // 时间戳(s)
	__u32		tp_nsec;        // 时间戳(ns)
	__u32		tp_snaplen;     // 捕获到的帧实际长度
	__u32		tp_len;         // 帧的理论长度
	__u32		tp_status;                                       
	__u16		tp_mac;         // 以太网MAC字段距离帧头的偏移量
	__u16		tp_net;
	/* pkt_hdr variants */
	union {
		struct tpacket_hdr_variant1 hv1;
	};
	__u8		tp_padding[8];
};

struct tpacket_bd_ts {
	unsigned int ts_sec;
	union {
		unsigned int ts_usec;
		unsigned int ts_nsec;
	};
};

// tpacket内存块头部子结构(V1、V2、V3共用)
struct tpacket_hdr_v1 {
	__u32	block_status;   // 主要用来标识该内存块当前是否正在被内核使用(内存块被内核填充期间是无法提供给用户使用的)
	__u32	num_pkts;       // 该内存块中的帧数量
	__u32	offset_to_first_pkt;    // 该内存块中第一个帧距离内存块起始地址的偏移量

	/* Number of valid bytes (including padding)
	 * blk_len <= tp_block_size
	 */
	__u32	blk_len;

	/*
	 * Quite a few uses of sequence number:
	 * 1. Make sure cache flush etc worked.
	 *    Well, one can argue - why not use the increasing ts below?
	 *    But look at 2. below first.
	 * 2. When you pass around blocks to other user space decoders,
	 *    you can see which blk[s] is[are] outstanding etc.
	 * 3. Validate kernel code.
	 */
	__aligned_u64	seq_num;

	/*
	 * ts_last_pkt:
	 *
	 * Case 1.	Block has 'N'(N >=1) packets and TMO'd(timed out)
	 *		ts_last_pkt == 'time-stamp of last packet' and NOT the
	 *		time when the timer fired and the block was closed.
	 *		By providing the ts of the last packet we can absolutely
	 *		guarantee that time-stamp wise, the first packet in the
	 *		next block will never precede the last packet of the
	 *		previous block.
	 * Case 2.	Block has zero packets and TMO'd
	 *		ts_last_pkt = time when the timer fired and the block
	 *		was closed.
	 * Case 3.	Block has 'N' packets and NO TMO.
	 *		ts_last_pkt = time-stamp of the last pkt in the block.
	 *
	 * ts_first_pkt:
	 *		Is always the time-stamp when the block was opened.
	 *		Case a)	ZERO packets
	 *			No packets to deal with but atleast you know the
	 *			time-interval of this block.
	 *		Case b) Non-zero packets
	 *			Use the ts of the first packet in the block.
	 *
	 */
	struct tpacket_bd_ts	ts_first_pkt, ts_last_pkt;
};

union tpacket_bd_header_u {
	struct tpacket_hdr_v1 bh1;
};

// 每个tpacket内存块的头部结构
struct tpacket_block_desc {
	__u32 version;
	__u32 offset_to_priv;       // 该内存块私有空间距离内存块起始地址的偏移量
	union tpacket_bd_header_u hdr;
};

#define TPACKET2_HDRLEN		(TPACKET_ALIGN(sizeof(struct tpacket2_hdr)) + sizeof(struct sockaddr_ll))   // V2环形缓冲区帧头长度
#define TPACKET3_HDRLEN		(TPACKET_ALIGN(sizeof(struct tpacket3_hdr)) + sizeof(struct sockaddr_ll))   // V3环形缓冲区帧头长度

// 环形缓冲区版本枚举
enum tpacket_versions {
	TPACKET_V1,
	TPACKET_V2,
	TPACKET_V3
};

/*
   Frame structure:

   - Start. Frame must be aligned to TPACKET_ALIGNMENT=16
   - struct tpacket_hdr
   - pad to TPACKET_ALIGNMENT=16
   - struct sockaddr_ll
   - Gap, chosen so that packet data (Start+tp_net) alignes to TPACKET_ALIGNMENT=16
   - Start+tp_mac: [ Optional MAC header ]
   - Start+tp_net: Packet data, aligned to TPACKET_ALIGNMENT=16.
   - Pad to align to TPACKET_ALIGNMENT=16
 */

// 创建TPACKET_V1/TPACKET_V2环形缓冲区对应的配置参数结构
struct tpacket_req {
	unsigned int	tp_block_size;	/* Minimal size of contiguous block     每个连续内存块的最小尺寸(必须是 PAGE_SIZE * 2^n ) */
	unsigned int	tp_block_nr;	/* Number of blocks                     内存块的数量 */
	unsigned int	tp_frame_size;	/* Size of frame                        每个帧的大小
                                       (必须大于TPACKET_HDRLEN，并且TPACKET_ALIGNMENT对齐，并且最好保证tp_block_size能够整除tp_frame_size) */
	unsigned int	tp_frame_nr;	/* Total number of frames               帧的总个数(必须等于 每个内存块中的帧数量*内存块数量) */
};

/* 创建TPACKET_V3环形缓冲区对应的配置参数结构
 *
 * 备注：显然tpacket_req3结构是tpacket_req结构的超集，实际可以统一使用本结构去设置所有版本的环形缓冲区，V1/V2版本会自动忽略多余的字段 
 */
struct tpacket_req3 {
	unsigned int	tp_block_size;	/* Minimal size of contiguous block */
	unsigned int	tp_block_nr;	/* Number of blocks */
	unsigned int	tp_frame_size;	/* Size of frame    (虽然V3中的帧长是可变的，但创建时还是会传入一个最大的允许值) */
	unsigned int	tp_frame_nr;	/* Total number of frames */
	unsigned int	tp_retire_blk_tov; /* timeout in msecs  超时时间(ms)，超时后即使内存块没有被数据填入也会被内核停用，0意味着不设超时 */
	unsigned int	tp_sizeof_priv; /* offset to private data area  每个内存块中私有空间大小，0意味着不设私有空间 */
	unsigned int	tp_feature_req_word;    // 标志位集合(目前就1个标志 TP_FT_REQ_FILL_RXHASH)
};

union tpacket_req_u {
	struct tpacket_req	req;
	struct tpacket_req3	req3;
};

// PACKET_ADD_MEMBERSHIP和PACKET_DROP_MEMBERSHIP选项对应的参数结构
struct packet_mreq {
	int		mr_ifindex;
	unsigned short	mr_type;    // 具体的类型，就是下面的PACKET_MR_*
	unsigned short	mr_alen;
	unsigned char	mr_address[8];
};

#define PACKET_MR_MULTICAST	0
#define PACKET_MR_PROMISC	1
#define PACKET_MR_ALLMULTI	2
#define PACKET_MR_UNICAST	3

#endif
