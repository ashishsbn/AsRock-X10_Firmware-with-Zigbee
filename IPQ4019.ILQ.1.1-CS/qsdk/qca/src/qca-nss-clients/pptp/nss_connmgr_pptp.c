/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/rwlock_types.h>
#include <linux/hashtable.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <net/route.h>
#include <linux/if_pppox.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#endif

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include "nss_connmgr_pptp.h"

#define PPP_HDR_LEN        4

/*
 * NSS pptp debug macros
 */
#if (NSS_PPTP_DEBUG_LEVEL < 1)
#define nss_connmgr_pptp_assert(fmt, args...)
#else
#define nss_connmgr_pptp_assert(c)  BUG_ON(!(c));
#endif

#if (NSS_PPTP_DEBUG_LEVEL < 2)
#define nss_connmgr_pptp_info(s, ...)
#else
#define nss_connmgr_pptp_info(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#define NUM_PPTP_CHANNELS_IN_PPP_NETDEVICE  1
#define HASH_BUCKET_SIZE 2  /* ( 2^ HASH_BUCKET_SIZE ) == 4 */

static DEFINE_HASHTABLE(pptp_session_table, HASH_BUCKET_SIZE);

/*
 * nss_connmgr_pptp_get_session()
 *	Retrieve pptp session associated with this netdevice if any
 */
static int nss_connmgr_pptp_get_session(struct net_device *dev, struct pptp_opt *opt)
{
	struct ppp_channel *channel[1] = {NULL};
	int px_proto;
	int ppp_ch_count;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		nss_connmgr_pptp_info("netdevice is not a PPP tunnel type\n");
		return -1;
	}

	if (ppp_is_multilink(dev)) {
		nss_connmgr_pptp_info("channel is multilink PPP\n");
		return -1;
	}

	ppp_ch_count = ppp_hold_channels(dev, channel, 1);
	nss_connmgr_pptp_info("PPP hold channel ret %d\n", ppp_ch_count);
	if (ppp_ch_count != 1) {
		nss_connmgr_pptp_info("hold channel for netdevice failed\n");
		return -1;
	}

	if (!channel[0]) {
		nss_connmgr_pptp_info("channel don't have a ppp_channel\n");
		return -1;
	}

	px_proto = ppp_channel_get_protocol(channel[0]);
	if (px_proto != PX_PROTO_PPTP) {
		nss_connmgr_pptp_info("session socket is not of type PX_PROTO_PPTP\n");
		ppp_release_channels(channel, 1);
		return -1;
	}

	pptp_channel_addressing_get(opt, channel[0]);

	ppp_release_channels(channel, 1);
	return 0;
}

/*
 * nss_connmgr_add_pptp_session()
 *	Add PPTP session entry into Hash table
 */
static struct nss_connmgr_pptp_session_entry *nss_connmgr_add_pptp_session(struct net_device *dev, struct pptp_opt *session)

{
	struct nss_connmgr_pptp_session_entry *pptp_session_data = NULL;
	struct nss_connmgr_pptp_session_info *data;

	pptp_session_data = kmalloc(sizeof(struct nss_connmgr_pptp_session_entry),
				      GFP_KERNEL);
	if (!pptp_session_data) {
		nss_connmgr_pptp_info("failed to allocate pptp_session_data\n");
		return NULL;
	}

	data = &pptp_session_data->data;

	/*
	 * Get session info
	 */
	data->src_call = session->src_addr.call_id;
	data->dst_call = session->dst_addr.call_id;
	data->src_ip = session->src_addr.sin_addr.s_addr;
	data->dst_ip = session->dst_addr.sin_addr.s_addr;

	nss_connmgr_pptp_info("src_call_id=%u peer_call_id=%u\n", data->src_call, data->dst_call);

	/*
	 * This netdev hold will be released when netdev
	 * down event arrives and session goes down.
	 */
	dev_hold(dev);
	pptp_session_data->dev = dev;

	/*
	 * There is no need for protecting simultaneous addition &
	 * deletion of pptp_session_data as all netdev notifier chain
	 * call back is called with rtln mutex lock.
	 */
	hash_add_rcu(pptp_session_table,
		&pptp_session_data->hash_list,
		dev->ifindex);

	return pptp_session_data;
}

/*
 * nss_connmgr_pptp_event_receive()
 *	Event Callback to receive events from NSS
 */
static void nss_connmgr_pptp_event_receive(void *if_ctx, struct nss_pptp_msg *tnlmsg)
{
	struct net_device *netdev = if_ctx;
	struct nss_pptp_sync_session_stats_msg *sync_stats;

	switch (tnlmsg->cm.type) {
	case NSS_PPTP_MSG_SYNC_STATS:
		if (!netdev) {
			return;
		}

		nss_connmgr_pptp_info("Update PPP stats\n");
		sync_stats = (struct nss_pptp_sync_session_stats_msg *)&tnlmsg->msg.stats;
		dev_hold(netdev);

		/*
		 * Update ppp stats
		 */
		ppp_update_stats(netdev,
				 (unsigned long)sync_stats->node_stats.rx_packets,
				 (unsigned long)sync_stats->node_stats.rx_bytes,
				 (unsigned long)sync_stats->node_stats.tx_packets,
				 (unsigned long)sync_stats->node_stats.tx_bytes,
				  0, 0, sync_stats->rx_dropped, sync_stats->tx_dropped);

		dev_put(netdev);
		break;

	default:
		nss_connmgr_pptp_info("Unknown Event from NSS\n");
		break;
	}
}

/*
 * nss_connmgr_pptp_exception()
 *	Exception handler registered to NSS for handling pptp pkts
 */
static void nss_connmgr_pptp_exception(struct net_device *dev,
				       struct sk_buff *skb,
				       __attribute__((unused)) struct napi_struct *napi)

{
	const struct iphdr *iph_outer, *iph_inner;
	struct nss_connmgr_pptp_session_entry *session_info;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
	struct hlist_node *node;
#endif
	struct nss_pptp_gre_hdr *gre_hdr;
	__be32 tunnel_local_ip;
	__be32 tunnel_peer_ip;
	struct rtable *rt;
	struct net_device *in_dev;
	uint8_t gre_version;
	uint8_t gre_flags;
	int gre_hdr_sz = sizeof(struct nss_pptp_gre_hdr);

	/* discard L2 header */
	skb_pull(skb, sizeof(struct ethhdr));
	skb_reset_mac_header(skb);

	iph_outer = (const struct iphdr *)skb->data;
	rcu_read_lock();
	hash_for_each_possible_rcu(pptp_session_table, session_info,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
				   node,
#endif
				   hash_list, dev->ifindex) {
		if (session_info->dev == dev) {
			tunnel_local_ip = session_info->data.src_ip;
			tunnel_peer_ip = session_info->data.dst_ip;
			rcu_read_unlock();
			if ((iph_outer->version == 4) && (iph_outer->protocol == IPPROTO_GRE) &&
				(iph_outer->saddr == tunnel_local_ip) && (iph_outer->daddr == tunnel_peer_ip)) { /*pkt is encapsulated */
				skb_pull(skb, sizeof(struct iphdr));
				gre_hdr = (struct nss_pptp_gre_hdr *)skb->data;
				if ((ntohs(gre_hdr->protocol) != NSS_PPTP_GRE_PROTO) &&
						(gre_hdr->flags_ver == NSS_PPTP_GRE_VER)) {
					nss_connmgr_pptp_info("Not PPTP_GRE_PROTO, so freeing\n");
					dev_kfree_skb_any(skb);
					return;
				}

				gre_flags = gre_hdr->flags;
				gre_version = gre_hdr->flags_ver;

				/*
				 * Check if seq present or not in GRE Header, If seq is not present,
				 * reduce the GRE header size.
				 */
				if (!(gre_flags & NSS_PPTP_GRE_HAS_SEQ)) {
					gre_hdr_sz -= sizeof(gre_hdr->seq);
				}

				/*
				 * Check if ack present or not in GRE Header, If Ack is not present,
				 * reduce the GRE header size.
				 */
				if (!(gre_version & NSS_PPTP_GRE_HAS_ACK)) {
					gre_hdr_sz -= sizeof(gre_hdr->ack);
				}

				skb_pull(skb, gre_hdr_sz); /* pull pptp header */
				skb_pull(skb, PPP_HDR_LEN);  /* pull ppp header */

				iph_inner = (const struct iphdr *)skb->data;
				if (iph_inner->version == 4) {
					skb->protocol = htons(ETH_P_IP);
				} else if (iph_inner->version == 6) {
					skb->protocol = htons(ETH_P_IPV6);
				} else {
					nss_connmgr_pptp_info("not able to handle this pkt, so freeing\n");
					dev_kfree_skb_any(skb);
					return;
				}

				skb_reset_network_header(skb);
				skb_set_transport_header(skb, iph_inner->ihl*4);
				skb->ip_summed = CHECKSUM_NONE;
				skb->pkt_type = PACKET_HOST;
				skb->dev = dev;

				/*
				 * set skb_iif
				 */
				rt = ip_route_output(&init_net, iph_inner->saddr, 0, 0, 0);
				if (unlikely(IS_ERR(rt))) {
					nss_connmgr_pptp_info("Martian packets");
				} else {
					in_dev = rt->dst.dev;
					ip_rt_put(rt);
					if (likely(in_dev)) {
						skb->skb_iif = in_dev->ifindex;
					} else {
						nss_connmgr_pptp_info("could not find incoming interface\n");
					}
				}

				nss_connmgr_pptp_info("send packet to dev_queue_xmit\n");
				dev_queue_xmit(skb);
				return;

			} else  { /* pkt is decapsulated */
				if (iph_outer->version == 4) {
					skb->protocol = htons(ETH_P_IP);
				} else if (iph_outer->version == 6) {
					skb->protocol = htons(ETH_P_IPV6);
				} else {
					nss_connmgr_pptp_info("pkt may be a control packet\n");
				}
				skb_reset_network_header(skb);
				skb->pkt_type = PACKET_HOST;
				skb->skb_iif = dev->ifindex;
				skb->ip_summed = CHECKSUM_NONE;
				skb->dev = dev;
				nss_connmgr_pptp_info("send packet to netif_receive_skb");
				netif_receive_skb(skb);
				return;
			}
		}
	}
	rcu_read_unlock();
	nss_connmgr_pptp_info("not able to handle this pkt from %s, so freeing\n", dev->name);
	dev_kfree_skb_any(skb);
}

/*
 * nss_connmgr_pptp_dev_up()
 *	pppopptp interface's up event handler
 */
static int nss_connmgr_pptp_dev_up(struct net_device *dev)
{
	struct pptp_opt opt;
	struct nss_connmgr_pptp_session_entry *session_info = NULL;
	struct nss_connmgr_pptp_session_info *data;
	nss_tx_status_t status;
	struct nss_ctx_instance *nss_ctx;
	uint32_t features = 0;
	uint32_t if_number;
	struct nss_pptp_msg  pptpmsg;
	struct nss_pptp_session_configure_msg *pptpcfg;
	int ret;

	ret = nss_connmgr_pptp_get_session(dev, &opt);
	if (ret < 0) {
		return NOTIFY_DONE;
	}

	/*
	 * Create nss dynamic interface and register
	 */
	if_number = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PPTP);
	if (if_number == -1) {
		nss_connmgr_pptp_info("Request interface number failed\n");
		return NOTIFY_DONE;
	}

	nss_connmgr_pptp_info("nss_dynamic_interface_alloc_node() sucessful. if_number = %d\n", if_number);
	/*
	 * Register pptp  tunnel with NSS
	 */
	nss_ctx = nss_register_pptp_if(if_number,
				       nss_connmgr_pptp_exception,
				       nss_connmgr_pptp_event_receive,
				       dev,
				       features);

	if (!nss_ctx) {
		status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_PPTP);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_pptp_info("Unable to dealloc the node[%d] in the NSS fw!\n", if_number);
		}
		nss_connmgr_pptp_info("nss_register_pptp_if failed\n");
		return NOTIFY_BAD;
	}

	nss_connmgr_pptp_info("%p: nss_register_pptp_if() successful\n", nss_ctx);

	session_info = nss_connmgr_add_pptp_session(dev, &opt);
	if (session_info == NULL) {
		nss_unregister_pptp_if(if_number);
		status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_PPTP);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_pptp_info("%p: Unable to dealloc the node[%d] in the NSS fw!\n", nss_ctx, if_number);
		}
		return NOTIFY_DONE;
	}

	data = &session_info->data;

	memset(&pptpmsg, 0, sizeof(struct nss_pptp_msg));
	pptpcfg = &pptpmsg.msg.session_configure_msg;

	/*
	 * The call id is already in host byte order,
	 * therefore no need to do ntohs() for call id.
	 */
	pptpcfg->src_call_id = data->src_call;
	pptpcfg->dst_call_id = data->dst_call;

	/*
	 * Convert IP addresses from nework byte order
	 * to host byte order before posting to firmware.
	 */
	pptpcfg->sip = ntohl(data->src_ip);
	pptpcfg->dip = ntohl(data->dst_ip);

	nss_connmgr_pptp_info("%p: pptp info\n", nss_ctx);
	nss_connmgr_pptp_info("%p: local_call_id %d peer_call_id %d\n", nss_ctx,
									pptpcfg->src_call_id,
									pptpcfg->dst_call_id);
	nss_connmgr_pptp_info("%p: saddr 0x%x daddr 0x%x \n", nss_ctx, pptpcfg->sip, pptpcfg->dip);
	nss_connmgr_pptp_info("Sending pptp i/f up command to NSS %x\n", (int)nss_ctx);

	nss_pptp_msg_init(&pptpmsg, if_number, NSS_PPTP_MSG_SESSION_CONFIGURE, sizeof(struct nss_pptp_session_configure_msg), NULL, NULL);

	status = nss_pptp_tx(nss_ctx, &pptpmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_unregister_pptp_if(if_number);
		status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_PPTP);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_pptp_info("%p: Unable to dealloc the node[%d] in the NSS fw!\n", nss_ctx, if_number);
		}
		nss_connmgr_pptp_info("%p: nss pptp session creation command error %d\n", nss_ctx, status);
		return NOTIFY_BAD;
	}
	nss_connmgr_pptp_info("%p: nss_pptp_tx() successful\n", nss_ctx);

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_pptp_dev_down()
 *	pppopptp interface's down event handler
 */
static int nss_connmgr_pptp_dev_down(struct net_device *dev)
{
	struct nss_connmgr_pptp_session_entry *session_info;
	struct hlist_node *tmp;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
	struct hlist_node *node;
#endif

	struct nss_pptp_msg pptpmsg;
	struct nss_pptp_session_deconfigure_msg *pptpcfg;
	nss_tx_status_t status;
	uint32_t if_number;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		nss_connmgr_pptp_info("netdevice is not a pptp tunnel type\n");
		return NOTIFY_DONE;
	}

	/*
	 * Check if pptp is registered ?
	 */
	if_number = nss_cmn_get_interface_number_by_dev(dev);
	if (if_number < 0) {
		nss_connmgr_pptp_info("Net device:%p is not registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	hash_for_each_possible_safe(pptp_session_table, session_info,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
				    node,
#endif
				    tmp, hash_list, dev->ifindex) {
		if (session_info->dev == dev) {
			dev_put(dev);
			hash_del_rcu(&session_info->hash_list);
			synchronize_rcu();

			memset(&pptpmsg, 0, sizeof(struct nss_pptp_msg));
			pptpcfg = &pptpmsg.msg.session_deconfigure_msg;
			pptpcfg->src_call_id = session_info->data.src_call;

			nss_pptp_msg_init(&pptpmsg, if_number, NSS_PPTP_MSG_SESSION_DECONFIGURE, sizeof(struct nss_pptp_session_deconfigure_msg), NULL, NULL);
			status = nss_pptp_tx(nss_pptp_get_context(), &pptpmsg);
			if (status != NSS_TX_SUCCESS) {
				nss_connmgr_pptp_info("pptp session destroy command failed, if_number = %d\n", if_number);
				kfree(session_info);
				return NOTIFY_BAD;
			}
			nss_unregister_pptp_if(if_number);
			status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_PPTP);
			if (status != NSS_TX_SUCCESS) {
				nss_connmgr_pptp_info("pptp dealloc node failure for if_number=%d\n", if_number);
				kfree(session_info);
				return NOTIFY_BAD;
			}
			nss_connmgr_pptp_info("deleting pptpsession, if_number %d, local_call_id %d, peer_call_id %d\n",
									dev->ifindex, session_info->data.src_call,  session_info->data.dst_call);
			kfree(session_info);
			break;
		}
	}

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_pptp_dev_event()
 *	Net device notifier for nss pptp module
 */
static int nss_connmgr_pptp_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *dev)
{
	struct net_device *netdev;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 10, 0))
	netdev = (struct net_device *)dev;
#else
	netdev = netdev_notifier_info_to_dev(dev);
#endif

	switch (event) {
	case NETDEV_UP:
		nss_connmgr_pptp_info("netdevice '%s' UP event\n", netdev->name);
		return nss_connmgr_pptp_dev_up(netdev);

	case NETDEV_DOWN:
		nss_connmgr_pptp_info("netdevice '%s' Down event\n", netdev->name);
		return nss_connmgr_pptp_dev_down(netdev);

	default:
		break;
	}

	return NOTIFY_DONE;
}

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_connmgr_pptp_notifier = {
	.notifier_call = nss_connmgr_pptp_dev_event,
};

/*
 * nss_connmgr_pptp_init_module()
 *	Tunnel pptp module init function
 */
int __init nss_connmgr_pptp_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif

	register_netdevice_notifier(&nss_connmgr_pptp_notifier);
	return 0;
}

/*
 * nss_connmgr_pptp_exit_module
 *	Tunnel pptp module exit function
 */
void __exit nss_connmgr_pptp_exit_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

	unregister_netdevice_notifier(&nss_connmgr_pptp_notifier);
}

module_init(nss_connmgr_pptp_init_module);
module_exit(nss_connmgr_pptp_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS pptp over ppp offload manager");
