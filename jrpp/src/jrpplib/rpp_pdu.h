
#ifndef _RPP_PDU_H_
#define _RPP_PDU_H_

#include "rpp_mgmt.h"

#define RPP_MSG_DATA_SIZE		36

typedef enum {
	RING_FAULT					= 0,
	RING_HEALTH					= 1
} eRingState;

typedef enum {
	PACKET_AUTHENTICATION		= 1,
	PACKET_BALLOT				= 2,
    PACKET_HELLO				= 5,
    PACKET_COMPLETE				= 6,
    PACKET_COMMON				= 7,
    PACKET_LINKDOWN				= 8,
    PACKET_EDGE_HELLO			= 10,
    PACKET_MAJOR_FAULT			= 11,
    PACKET_COMMAND				= 20
} ePacketType;

typedef enum {
	NODE_STATE_IDLE				= 0,
    NODE_STATE_COMPLETE			= 1,
    NODE_STATE_FAIL				= 2,
    NODE_STATE_LINK_UP			= 3,
    NODE_STATE_LINK_DOWN		= 4,
    NODE_STATE_PRE_FORWARDING	= 5  
} eNodeState;

typedef enum {
	PORT_IDLE					= 0,
	PORT_DOWN					= 1,
	PORT_AUTH_REQ				= 2,
	PORT_AUTH_FAIL				= 3,
	PORT_AUTH_PASS				= 4,
	PORT_BALLOT_ACTIVE			= 5,
	PORT_BALLOT_FINISH			= 7
} ePortRunState;

/*************************************************************
	Ring message data type define
 *************************************************************/
typedef enum {
	MSG_AUTH_REQ					= 0x01,	/* Auth Request */
	MSG_AUTH_ACK					= 0x02	/* Auth Ack */
} eMsgAuthType;

typedef enum {
	MSG_BALLOT_INITIATIVE           = 0x01,	/* Start ballot timer */
	MSG_BALLOT_PASSIVE				= 0x02,
	MSG_BALLOT_LOOPBACK				= 0x03
} eMsgBallotType;

typedef enum {
	MSG_COMPLETE_UPDATE_NODE		= 0x01,	/* Update NodeState */
	MSG_COMPLETE_WITH_FLUSH_FDB		= 0x02	/* Update NodeState and StpState */
} eMsgCompleteType;

typedef enum {
	MSG_LINKDOWN_REQ_UPDATE_NODE	= 0x01,	/* Request update NodeState */
	MSG_LINKDOWN_REQ_FLUSH_FDB		= 0x02	/* Request update NodeState and FlushFDB */
} eMsgLinkDownType;

typedef enum {
	MSG_COMMON_UPDATE_NODE			= 0x01,
	MSG_COMMON_WITH_FLUSH_FDB		= 0x02,
	MSG_COMMON_RING_CHAIN_READY		= 0x03
} eMsgCommonType;

typedef enum {
	CMD_GET_NODE_REQ				= 0x01,
	CMD_GET_NODE_RSP				= 0x81,
	CMD_RING_DISABLE_REQ			= 0x02,
	CMD_RING_DISABLE_RSP			= 0x82,
	CMD_RING_ENABLE_REQ				= 0x03,
	CMD_RING_ENABLE_RSP				= 0x83,
	CMD_RING_REBOOT_REQ				= 0x04,
	CMD_RING_REBOOT_RSP				= 0x84
} eMsgCmdCode;

typedef enum {
	CMD_RET_CODE_CONTINUE			= 0x01,
	CMD_RET_CODE_END				= 0x02,
	CMD_RET_CODE_TIMEOUT			= 0x03
} eMsgCmdRetCode;

typedef enum {
	CMD_RET_NODE_NOT_LAST			= 0x00,
	CMD_RET_LAST_NODE_CHAIN			= 0x01,
	CMD_RET_LAST_NODE_RING			= 0x02
} eMsgCmdRetLastNode;

/*************************************************************
	                Message body struct
 *************************************************************/
/* Authentication message */
typedef struct {
	unsigned char	type; 
	BALLOT_ID_T		ballot_id;
} tRMsgAuth;
 
/* Ballot message */
typedef struct {
	unsigned char	type;
	unsigned char	port;
	BALLOT_ID_T		id;
	unsigned char	run_state;
} tRMsgBallot;

/* Hello message */
typedef struct {
	unsigned char	tick[4];
	unsigned char   master_secondary_state;
	unsigned char   txport_state;
	unsigned char	block_line_num[2];
	unsigned char   ring_state;
} tRMsgHello;

/* Complete message */
typedef struct {
	unsigned char	type;
} tRMsgComplete;

/* Common message */
typedef struct {
	unsigned char	type;
} tRMsgCommon;

/* LinkDown message */
typedef struct {
	unsigned char	type;
	unsigned char	ext_neighbor_mac[MAC_LEN];
	unsigned char	ext_neighbor_port;
} tRMsgLinkDown;

/* Message Body */
typedef union {
    unsigned char 	data[RPP_MSG_DATA_SIZE]; 
	tRMsgAuth		auth;
	tRMsgBallot		ballot;
	tRMsgHello		hello;
	tRMsgComplete	complete;
	tRMsgCommon		common;
	tRMsgLinkDown	linkdown;
	//tRMsgCmd		command;
} __attribute__((packed)) tRingMsgBody;

/*************************************************************
	JRPP Frame struct
	802.3 LLC header + Switch Tag + VLAN Tag + 64 Data
 *************************************************************/
// 30bytes
typedef struct rpp_header_t {
	unsigned char	dst_mac[6];
	unsigned char	src_mac[6];
	unsigned char	dsa_tag[4];
	unsigned char	eth_type[2];
	unsigned char	vlan_tag[2];
	unsigned char	len8023[2];
	unsigned char	dsap;
	unsigned char	ssap;
	unsigned char	llc;
	unsigned char	oui_code[3];
	unsigned char	pid[2];
} RPP_HEADER_T;

// 28bytes + 36bytes
typedef struct rpp_body_t {
  	unsigned char	res1[2];
	unsigned char	length[2];
	unsigned char	version;
	unsigned char	type;
	unsigned char	domain_id[2];
	unsigned char	ring_id[2];
	unsigned char	res2[2];
	unsigned char	sys_mac[6];
	unsigned char	hello_time[2];
	unsigned char	fail_time[2];
	unsigned char	port_id;
	unsigned char	ring_level;
	unsigned char	hello_seq[2];
	unsigned char	res3[2];
	/* Message data, 36 bytes */
    tRingMsgBody    msg;
	//unsigned char	msg[RPP_MSG_DATA_SIZE];
} RPP_BODY_T;

typedef struct rpp_pdu_t {
	RPP_HEADER_T	hdr;
	RPP_BODY_T		body;
} RPP_PDU_T;

#endif

