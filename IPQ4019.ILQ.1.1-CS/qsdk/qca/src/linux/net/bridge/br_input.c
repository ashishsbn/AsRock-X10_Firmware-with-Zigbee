/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/neighbour.h>
#include <net/arp.h>
#include <linux/export.h>
#include <linux/rculist.h>
#ifdef CONFIG_BRIDGE_X10_GUEST_NETWORK
#include <linux/inetdevice.h>
#endif
#include "br_private.h"

#ifdef CONFIG_BRIDGE_X10_GUEST_NETWORK
extern int brnf_block_guest;
#endif
#ifdef CONFIG_BRIDGE_X10_DNS_HIJACK
extern int brnf_block_dns_reply;
#endif

/* Hook for brouter */
br_should_route_hook_t __rcu *br_should_route_hook __read_mostly;
EXPORT_SYMBOL(br_should_route_hook);

/* Hook for external Multicast handler */
br_multicast_handle_hook_t __rcu *br_multicast_handle_hook __read_mostly;
EXPORT_SYMBOL_GPL(br_multicast_handle_hook);

/* Hook for external forwarding logic */
br_get_dst_hook_t __rcu *br_get_dst_hook __read_mostly;
EXPORT_SYMBOL_GPL(br_get_dst_hook);

int br_pass_frame_up(struct sk_buff *skb)
{
	struct net_device *indev, *brdev = BR_INPUT_SKB_CB(skb)->brdev;
	struct net_bridge *br = netdev_priv(brdev);
	struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->stats);
	struct net_port_vlans *pv;

	u64_stats_update_begin(&brstats->syncp);
	brstats->rx_packets++;
	brstats->rx_bytes += skb->len;
	u64_stats_update_end(&brstats->syncp);

	/* Bridge is just like any other port.  Make sure the
	 * packet is allowed except in promisc modue when someone
	 * may be running packet capture.
	 */
	pv = br_get_vlan_info(br);
	if (!(brdev->flags & IFF_PROMISC) &&
	    !br_allowed_egress(br, pv, skb)) {
		kfree_skb(skb);
		return NET_RX_DROP;
	}

	indev = skb->dev;
	skb->dev = brdev;
	skb = br_handle_vlan(br, pv, skb);
	if (!skb)
		return NET_RX_DROP;

	return BR_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
		       netif_receive_skb);
}
EXPORT_SYMBOL_GPL(br_pass_frame_up);

static void br_do_proxy_arp(struct sk_buff *skb, struct net_bridge *br,
			    u16 vid, struct net_bridge_port *p)
{
	struct net_device *dev = br->dev;
	struct neighbour *n;
	struct arphdr *parp;
	u8 *arpptr, *sha;
	__be32 sip, tip;

	BR_INPUT_SKB_CB(skb)->proxyarp_replied = false;

	if (dev->flags & IFF_NOARP)
		return;

	if (!pskb_may_pull(skb, arp_hdr_len(dev))) {
		dev->stats.tx_dropped++;
		return;
	}
	parp = arp_hdr(skb);

	if (parp->ar_pro != htons(ETH_P_IP) ||
	    parp->ar_op != htons(ARPOP_REQUEST) ||
	    parp->ar_hln != dev->addr_len ||
	    parp->ar_pln != 4)
		return;

	arpptr = (u8 *)parp + sizeof(struct arphdr);
	sha = arpptr;
	arpptr += dev->addr_len;	/* sha */
	memcpy(&sip, arpptr, sizeof(sip));
	arpptr += sizeof(sip);
	arpptr += dev->addr_len;	/* tha */
	memcpy(&tip, arpptr, sizeof(tip));

	if (ipv4_is_loopback(tip) ||
	    ipv4_is_multicast(tip))
		return;

	n = neigh_lookup(&arp_tbl, &tip, dev);
	if (n) {
		struct net_bridge_fdb_entry *f;

		if (!(n->nud_state & NUD_VALID)) {
			neigh_release(n);
			return;
		}

		f = __br_fdb_get(br, n->ha, vid);
		if (f && ((p->flags & BR_PROXYARP) ||
			  (f->dst && (f->dst->flags & BR_PROXYARP_WIFI)))) {
			arp_send(ARPOP_REPLY, ETH_P_ARP, sip, skb->dev, tip,
				 sha, n->ha, sha);
			BR_INPUT_SKB_CB(skb)->proxyarp_replied = true;
		}

		neigh_release(n);
	}
}

/* note: already called with rcu_read_lock */
int br_handle_frame_finish(struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;
	struct net_bridge_mdb_entry *mdst;
	struct sk_buff *skb2;
	struct net_bridge_port *pdst = NULL;
	br_get_dst_hook_t *get_dst_hook = rcu_dereference(br_get_dst_hook);
	bool unicast = true;
	u16 vid = 0;

	if (!p || p->state == BR_STATE_DISABLED)
		goto drop;

	if (!br_allowed_ingress(p->br, nbp_get_vlan_info(p), skb, &vid))
		goto out;

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
	if (p->flags & BR_LEARNING)
		br_fdb_update(br, p, eth_hdr(skb)->h_source, vid, false);

	if (!is_broadcast_ether_addr(dest) && is_multicast_ether_addr(dest) &&
	    br_multicast_rcv(br, p, skb, vid))
		goto drop;

	if ((p->state == BR_STATE_LEARNING) && skb->protocol != htons(ETH_P_PAE))
		goto drop;

	BR_INPUT_SKB_CB(skb)->brdev = br->dev;

	/* The packet skb2 goes to the local host (NULL to skip). */
	skb2 = NULL;

	if (br->dev->flags & IFF_PROMISC)
		skb2 = skb;

	dst = NULL;

	if (IS_ENABLED(CONFIG_INET) && skb->protocol == htons(ETH_P_ARP))
		br_do_proxy_arp(skb, br, vid, p);

	if (is_broadcast_ether_addr(dest)) {
		skb2 = skb;
		unicast = false;
	} else if (is_multicast_ether_addr(dest)) {
		br_multicast_handle_hook_t *multicast_handle_hook =
			rcu_dereference(br_multicast_handle_hook);
		if (!__br_get(multicast_handle_hook, true, p, skb))
			goto out;

		mdst = br_mdb_get(br, skb, vid);
		if ((mdst || BR_INPUT_SKB_CB_MROUTERS_ONLY(skb)) &&
		    br_multicast_querier_exists(br, eth_hdr(skb))) {
			if ((mdst && mdst->mglist) ||
			    br_multicast_is_router(br))
				skb2 = skb;
			br_multicast_forward(mdst, skb, skb2);
			skb = NULL;
			if (!skb2)
				goto out;
		} else
			skb2 = skb;

		unicast = false;
		br->dev->stats.multicast++;
	} else if ((pdst = __br_get(get_dst_hook, NULL, p, &skb))) {
		if (!skb)
			goto out;
	} else if ((p->flags & BR_ISOLATE_MODE) ||
		   ((dst = __br_fdb_get(br, dest, vid)) && dst->is_local)) {
		skb2 = skb;
		/* Do not forward the packet since it's local. */
		skb = NULL;
	}

	if (skb) {
		if (dst) {
			dst->used = jiffies;
			pdst = dst->dst;
		}

		if (pdst)
			br_forward(pdst, skb, skb2);
		else
			br_flood_forward(br, skb, skb2, unicast);
	}

	if (skb2)
		return br_pass_frame_up(skb2);

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}

/* note: already called with rcu_read_lock */
static int br_handle_local_finish(struct sk_buff *skb)
{
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	if (p->state != BR_STATE_DISABLED) {
		u16 vid = 0;
		/* check if vlan is allowed, to avoid spoofing */
		if (p->flags & BR_LEARNING && br_should_learn(p, skb, &vid))
			br_fdb_update(p->br, p, eth_hdr(skb)->h_source, vid, false);
	}
	return 0;	 /* process further */
}

#ifdef CONFIG_BRIDGE_X10_DNS_HIJACK
typedef struct _header {
	unsigned short int	id;
	unsigned short		u;

	short int	qdcount;
	short int	ancount;
	short int	nscount;
	short int	arcount;
} dnsheader_t;

//KKHuang: SUPPORT_BLOCK_DNS_REPLY is to DROP DNS reply of asrock.router on AP mode
//#define SUPPORT_BLOCK_DNS_REPLY
static int x10_dns_hijack(struct sk_buff *skb)
{
	unsigned char *target_url = "asrock.router";
	unsigned char dns_answer[] = {0xC0, 0x0C, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x03, 0x84, 0x00, 0x04};
	struct iphdr *iph;
	struct udphdr *udph;
	unsigned char *url = NULL;
	unsigned char url2[256], value = 0;
	int len = 0, token_len = 0, offset = 0, i = 0;
#ifdef SUPPORT_BLOCK_DNS_REPLY
	int match = 0;
#endif

	struct net_device *br_dev = NULL;
	struct in_device *br_in_dev = NULL;
	dnsheader_t *dns_pkt;
	unsigned char mac[ETH_ALEN];
	unsigned int ip;
	unsigned short port;
	unsigned char *ptr = NULL;

	iph = (struct iphdr *)skb_network_header(skb);
	udph = (void *)iph + iph->ihl*4;

	if (iph->protocol==IPPROTO_UDP && ntohs(udph->dest) == 53) {
		url = (void *)udph + sizeof(struct udphdr) + sizeof(dnsheader_t);
		len = ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dnsheader_t) - 4;

		while (len) {
			token_len = url[offset];
			if (token_len) {
				memset(url2, 0, sizeof(url2));
				for (i=0; i<token_len; i++) {
					value = *(url+offset+1+i);
					if ((value>=65) && (value<=90)) {
						url2[i] = value+32;
					}
					else {
						url2[i] = value;
					}
				}
				if (memcmp(url2, target_url+offset, token_len)) {
					return 0;
				}
			}
			token_len +=1;
			len -= token_len;
			offset += token_len;
		}

		printk(KERN_INFO "[%s:%d] match target_url[%s]\n", __FUNCTION__, __LINE__, target_url);

		br_dev = dev_get_by_name_rcu(&init_net, "br-lan");
		if (br_dev) {
			br_in_dev = __in_dev_get_rcu(br_dev);
			if (br_in_dev && br_in_dev->ifa_list) {
				dns_pkt = (void *)udph + sizeof(struct udphdr);
				ptr = (void *)udph + ntohs(udph->len);

				skb_put(skb, 16);

				/* swap mac address */
				memcpy(mac, eth_hdr(skb)->h_dest, ETH_ALEN);
				memcpy(eth_hdr(skb)->h_dest, eth_hdr(skb)->h_source, ETH_ALEN);
				memcpy(eth_hdr(skb)->h_source, mac, ETH_ALEN);

				/*swap ip address */
				ip = iph->saddr;
				iph->saddr = iph->daddr;
				iph->daddr = ip;
				iph->tot_len = htons(ntohs(iph->tot_len)+16);

				/* swap udp port */
				port = udph->source;
				udph->source = udph->dest;
				udph->dest = port;
				udph->len = htons(ntohs(udph->len)+16);
				dns_pkt->u = htons(0x8180);
				dns_pkt->qdcount = htons(1);
				dns_pkt->ancount = htons(1);
				dns_pkt->nscount = htons(0);
				dns_pkt->arcount = htons(0);

				/* pad Answers */
				memcpy(ptr, dns_answer, 12);
				memcpy(ptr+12, (unsigned char *)&br_in_dev->ifa_list->ifa_address, 4);

				/* ip checksum */
				skb->ip_summed = htons(CHECKSUM_NONE);
				iph->check = 0;
				iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

				/* udp checksum */
				udph->check = 0;
				udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
												ntohs(udph->len), IPPROTO_UDP,
												csum_partial((char *)udph,
												ntohs(udph->len), 0));
				return 1;
			}
		}
	}

#ifdef SUPPORT_BLOCK_DNS_REPLY
	if (brnf_block_dns_reply==1 && iph->protocol==IPPROTO_UDP && ntohs(udph->source)==53) {
		url = (void *)udph + sizeof(struct udphdr) + sizeof(dnsheader_t);
		len = ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dnsheader_t) - 4;

		offset = 0;
		while (len) {
			token_len = url[offset];
			if (token_len) {
				memset(url2, 0, sizeof(url2));
				for (i=0; i<token_len; i++) {
					value = *(url+offset+1+i);
					if ((value>=65) && (value<=90)) {
						url2[i] = value+32;
					}
					else {
						url2[i] = value;
					}
				}
				if (memcmp(url2, target_url+offset, token_len)) {
					//printk(KERN_INFO "[%s:%d] DNS reply does not match target_url[%s]\n", __FUNCTION__, __LINE__, target_url);
					return 0;
				}
				else {
					match = 1;
				}
			}
			else {
				break;
			}
			token_len +=1;
			len -= token_len;
			offset += token_len;
		}

		if (match) {
			printk(KERN_INFO "[%s:%d] DNS reply match target_url[%s]\n", __FUNCTION__, __LINE__, target_url);
			return -1;
		}
	}
#endif

	return 0;
}
#endif

/*
 * Return NULL if skb is handled
 * note: already called with rcu_read_lock
 */
rx_handler_result_t br_handle_frame(struct sk_buff **pskb)
{
	struct net_bridge_port *p;
	struct sk_buff *skb = *pskb;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	br_should_route_hook_t *rhook;
#ifdef CONFIG_BRIDGE_X10_GUEST_NETWORK
	struct net_device *br_dev = NULL; 
	struct in_device *br_in_dev = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;
#endif
#ifdef CONFIG_BRIDGE_X10_DNS_HIJACK
	int ret = 0;
#endif

	if (unlikely(skb->pkt_type == PACKET_LOOPBACK))
		return RX_HANDLER_PASS;

	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto drop;

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		return RX_HANDLER_CONSUMED;

	p = br_port_get_rcu(skb->dev);

#ifdef CONFIG_BRIDGE_X10_GUEST_NETWORK
	if (brnf_block_guest) {
		if ((skb->dev!=NULL) && (skb->dev->name!=NULL) &&
			(strcmp(skb->dev->name, "ath02")==0 || strcmp(skb->dev->name, "ath12")==0)) {
			iph = (struct iphdr *)skb_network_header(skb);
			br_dev = dev_get_by_name_rcu(&init_net, "br-lan");
			if (br_dev) {
				br_in_dev = __in_dev_get_rcu(br_dev);
				if (br_in_dev) {
					if ((__u32)iph->daddr == (__u32)br_in_dev->ifa_list->ifa_address) {
						if (iph->protocol==IPPROTO_UDP) {
							udph = (void *)iph + iph->ihl*4;
							if (htons(udph->dest) == 445) {
								goto drop;
							}
						}
						else if (iph->protocol==IPPROTO_TCP) {
							tcph = (void *)iph + iph->ihl*4;
							// Block access of HTTP, SAMBA(445), DLNA(8200) and Download Agent(9091)
							if (htons(tcph->dest) == 80 ||
								htons(tcph->dest) == 445 ||
								htons(tcph->dest) == 8200 ||
								htons(tcph->dest) == 9091) {
								goto drop;
							}
						}
					}
				}
			}
		}
	}
#endif

#ifdef CONFIG_BRIDGE_X10_DNS_HIJACK
	ret = x10_dns_hijack(skb);
	if (ret == -1) {
		goto drop;
	}
#endif

	if (unlikely(is_link_local_ether_addr(dest))) {
		/*
		 * See IEEE 802.1D Table 7-10 Reserved addresses
		 *
		 * Assignment		 		Value
		 * Bridge Group Address		01-80-C2-00-00-00
		 * (MAC Control) 802.3		01-80-C2-00-00-01
		 * (Link Aggregation) 802.3	01-80-C2-00-00-02
		 * 802.1X PAE address		01-80-C2-00-00-03
		 *
		 * 802.1AB LLDP 		01-80-C2-00-00-0E
		 *
		 * Others reserved for future standardization
		 */
		switch (dest[5]) {
		case 0x00:	/* Bridge Group Address */
			/* If STP is turned off,
			   then must forward to keep loop detection */
			if (p->br->stp_enabled == BR_NO_STP)
				goto forward;
			break;

		case 0x01:	/* IEEE MAC (Pause) */
			goto drop;

		default:
			/* Allow selective forwarding for most other protocols */
			if (p->br->group_fwd_mask & (1u << dest[5]))
				goto forward;
		}

		/* Deliver packet to local host only */
		if (BR_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev,
			    NULL, br_handle_local_finish)) {
			return RX_HANDLER_CONSUMED; /* consumed by filter */
		} else {
			*pskb = skb;
			return RX_HANDLER_PASS;	/* continue processing */
		}
	}

forward:
	switch (p->state) {
	case BR_STATE_DISABLED:
		if (skb->protocol == htons(ETH_P_PAE)) {
			if (ether_addr_equal(p->br->dev->dev_addr, dest))
				skb->pkt_type = PACKET_HOST;

			if (NF_HOOK(NFPROTO_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev,
				    NULL, br_handle_local_finish))
				break;

			BR_INPUT_SKB_CB(skb)->brdev = p->br->dev;
			br_pass_frame_up(skb);
			break;
		}
		goto drop;
	case BR_STATE_FORWARDING:
		rhook = rcu_dereference(br_should_route_hook);
		if (rhook) {
			if ((*rhook)(skb)) {
				*pskb = skb;
				return RX_HANDLER_PASS;
			}
			dest = eth_hdr(skb)->h_dest;
		}
		/* fall through */
	case BR_STATE_LEARNING:
		if (ether_addr_equal(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;

		BR_HOOK(NFPROTO_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
			br_handle_frame_finish);
		break;
	default:
drop:
		kfree_skb(skb);
	}
	return RX_HANDLER_CONSUMED;
}
