#ifndef _NET_STP_H
#define _NET_STP_H

// 定义了生成树协议结构
struct stp_proto {
	unsigned char	group_address[ETH_ALEN];        // 该生成树协议使用的组播地址
	void		(*rcv)(const struct stp_proto *, struct sk_buff *,
			       struct net_device *);            // 指向该生成树协议的报文接收函数
	void		*data;
};

int stp_proto_register(const struct stp_proto *proto);
void stp_proto_unregister(const struct stp_proto *proto);

#endif /* _NET_STP_H */
