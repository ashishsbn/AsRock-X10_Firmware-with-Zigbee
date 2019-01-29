/*
 * Copyright (c) 2014 - 2015, The Linux Foundation. All rights reserved.
 *
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
 */

/*
 * rfs_wxt.c
 *	Receiving Flow Streering - Wireless Extension
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/socket.h>
#include <linux/delay.h>
#include <linux/wireless.h>
#include <linux/irq.h>
#include <linux/irqnr.h>
#include <linux/cpumask.h>
#include <linux/irqdesc.h>
#include <net/iw_handler.h>
#include "rfs.h"
#include "rfs_rule.h"
#include "rfs_ess.h"

/*
 * Per-module structure.
 */
struct rfs_wxt {
	struct task_struct *thread;
	struct socket *sock;
};

static struct rfs_wxt __rwn;

/*
 * rfs_wxt_get_parent
 *	Get parent interface of a VAP
 *	This API actually depends on QCA WLAN implmentation
 */
static int rfs_wxt_get_parent(int ifindex)
{
	struct net_device *dev;
	iw_handler  iwhandler;
	const struct iw_priv_args *private_arg;
	int num_private_args;
	int pifindex = -1;
	int param = 0;
	int i;

	dev = dev_get_by_index(&init_net, ifindex);

	if (!dev)
		return -1;

	if (!dev->wireless_handlers ||
		!dev->wireless_handlers->private)
	{
		goto exit1;
	}

	/*SIOCIWFIRSTPRIV + 1 iwpriv */
	iwhandler = dev->wireless_handlers->private[1];
	num_private_args = dev->wireless_handlers->num_private_args;

	/*iwpriv get_parent*/
	for (i = 0; i < num_private_args; i ++) {
		private_arg =
			&dev->wireless_handlers->private_args[i];
		if (!memcmp(private_arg->name, "get_parent", 11)) {
			param = private_arg->cmd;
			break;
		}
	}

	if (!param)
		goto exit1;

	RFS_DEBUG("get_parent cmd id %d\n", param);

	if (iwhandler(dev, NULL, NULL, (char *)&param) < 0)
	{
		goto exit1;
	}

	pifindex = param;
exit1:
	dev_put(dev);
	return pifindex;

}


/*
 * rfs_wxt_get_irq
 *	Get IRQ number of a interface
 */
static int rfs_wxt_get_irq(int ifindex)
{
	struct net_device *dev;
	int irq;

	dev = dev_get_by_index(&init_net, ifindex);
	if (!dev)
		return -1;

	irq = dev->irq;
	dev_put(dev);

	return irq;
}


/*
 * rfs_wxt_get_cpu_by_irq
 */
static int rfs_wxt_get_cpu_by_irq(int irq)
{
	struct irq_desc *desc;
	const struct cpumask *mask;
	int cpu;

	desc = irq_to_desc(irq);
	mask = desc->irq_data.affinity;
	cpu = cpumask_first(mask);
	if (cpu >= nr_cpu_ids) {
		return -1;
	}

	if (cpumask_next(cpu, mask) < nr_cpu_ids) {
		RFS_INFO("IRQ is bound to more than one CPU\n");
		return -1;
	}

	return cpu;
}


/*
 * rfs_wxt_get_cpu
 */
int rfs_wxt_get_cpu(int ifindex)
{
	int pifi;
	int irq;
	int cpu;

	/*
	 * get physic interface wifix
	 */
	pifi = rfs_wxt_get_parent(ifindex);
	RFS_DEBUG("vif %d, parent %d\n", ifindex, pifi);
	if (pifi < 0)
		return RPS_NO_CPU;

	/*
	 * get irq of physic interface
	 */
	irq = rfs_wxt_get_irq(pifi);
	RFS_DEBUG("irq %d\n", irq);
	if (pifi < 0)
		return RPS_NO_CPU;

	/*
	 * get irq affinity
	 */
	cpu = rfs_wxt_get_cpu_by_irq(irq);
	RFS_DEBUG("cpu %d\n", cpu);
	if (cpu < 0)
		return RPS_NO_CPU;

	return cpu;
}


#ifdef RFS_USE_IW_EVENT
/*
 * rfs_wxt_iwevent
 *	wireless event handler
 */
static int rfs_wxt_iwevent(int ifindex, unsigned char *buf, size_t len)
{
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end;
	int cpu;

	pos = buf;
	end = buf + len;
	while (pos + IW_EV_LCP_LEN <= end) {
		memcpy(&iwe_buf, pos, sizeof(struct iw_event));
		if (iwe->len <= IW_EV_LCP_LEN)
			return -1;

		if (iwe->cmd == IWEVREGISTERED) {
			RFS_DEBUG("STA %pM joining\n", (unsigned char*) iwe->u.addr.sa_data);
			cpu = rfs_wxt_get_cpu(ifindex);
			if (cpu < 0 )
				return -1;
			rfs_rule_create_mac_rule((unsigned char*) iwe->u.addr.sa_data, (uint16_t)cpu, 0, 0);
		}
		else if (iwe->cmd == IWEVEXPIRED) {
			RFS_DEBUG("STA %pM leaving\n", (unsigned char*) iwe->u.addr.sa_data);
			rfs_rule_destroy_mac_rule((unsigned char*) iwe->u.addr.sa_data, 0);
		}

		pos += iwe->len;

	}

	return 0;
}


/*
 * rfs_wxt_newlink
 *	Link event handler
 */
static int rfs_wxt_newlink(struct ifinfomsg *ifi, unsigned char *buf, size_t len)
{
	struct rtattr *attr;
	int attrlen, rta_len;

	RFS_TRACE("Event from interface %d\n", ifi->ifi_index);

	attrlen = len;
	attr = (struct rtattr *) buf;
	rta_len = RTA_ALIGN(sizeof(struct rtattr));

	while (RTA_OK(attr, attrlen)) {
		if (attr->rta_type == IFLA_WIRELESS) {
			rfs_wxt_iwevent(ifi->ifi_index, ((char *) attr) + rta_len, attr->rta_len - rta_len);
		}
		attr = RTA_NEXT(attr, attrlen);
	}

	return 0;
}
#endif

/*
 * rfs_wxt_handler
 *	Netlink event handler
 */
static int rfs_wxt_handler(unsigned char *buf, int len)
{
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifi;
	int left;

	nlh = (struct nlmsghdr *) buf;
	left = len;

	while (NLMSG_OK(nlh, left)) {
		switch (nlh->nlmsg_type) {
		case RTM_NEWLINK:
		case RTM_DELLINK:
			if (NLMSG_PAYLOAD(nlh, 0) < sizeof(struct ifinfomsg)) {
				RFS_DEBUG("invalid netlink message");
				break;
			}

			ifi = NLMSG_DATA(nlh);
			if (ifi->ifi_family == AF_BRIDGE) {
				if (nlh->nlmsg_type == RTM_NEWLINK)
					rfs_ess_add_brif(ifi->ifi_index);
				else
					rfs_ess_del_brif(ifi->ifi_index);
			}
#ifdef RFS_USE_IWEVENT
			else {
				rfs_wxt_newlink(ifi, (u8 *)ifi + NLMSG_ALIGN(sizeof(struct ifinfomsg)),
					NLMSG_PAYLOAD(nlh, sizeof(struct ifinfomsg)));
			}
#endif
			break;

		}

		nlh = NLMSG_NEXT(nlh, left);

	}

	return 0;
}


/*
 * rfs_wxt_rx
 *	Receive netlink message from socket
 */
static int rfs_wxt_rx(struct socket *sock, struct sockaddr_nl *addr, unsigned char *buf, int len)
{
	struct msghdr msg;
	struct iovec  iov;
	mm_segment_t oldfs;
	int size;

	iov.iov_base = buf;
	iov.iov_len  = len;

	msg.msg_flags = 0;
	msg.msg_name  = addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_iov   = &iov;
	msg.msg_iovlen = 1;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	size = sock_recvmsg(sock, &msg, len, msg.msg_flags);
	set_fs(oldfs);

	return size;
}


/*
 * rfs_wxt_thread
 */
static void rfs_wxt_thread(void)
{
	int err;
	int size;
	struct sockaddr_nl saddr;
	unsigned char buf[512];
	int len = sizeof(buf);

	allow_signal(SIGKILL|SIGSTOP);
	err = sock_create(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE, &__rwn.sock);
	if (err < 0) {
		RFS_ERROR("failed to create sock\n");
		goto exit1;
	}

	memset(&saddr, 0, sizeof(struct sockaddr));
	saddr.nl_family = AF_NETLINK;
	saddr.nl_groups = RTNLGRP_LINK;
	saddr.nl_pid    = current->pid;

	err = __rwn.sock->ops->bind(__rwn.sock, (struct sockaddr*)&saddr, sizeof(struct sockaddr));
	if (err < 0) {
		RFS_ERROR("failed to bind sock\n");
		goto exit2;
	}

	RFS_DEBUG("rfs_wxt thread started\n");
	while (!kthread_should_stop()) {
		size = rfs_wxt_rx(__rwn.sock, &saddr, buf, len);
		RFS_TRACE("got a netlink msg with len %d\n", size);

		if (signal_pending(current))
			break;

		if (size < 0) {
			RFS_WARN("netlink rx error\n");
		} else {
			rfs_wxt_handler(buf, size);
		}
	}

	RFS_DEBUG("rfs_wxt thread stopped\n");
exit2:
	sock_release(__rwn.sock);
	__rwn.sock = NULL;

exit1:
	return;
}

int rfs_wxt_start(void)
{
	if (__rwn.thread) {
		return 0;
	}

	__rwn.thread = kthread_run((void*)rfs_wxt_thread, NULL, "rfs_wxt");
	if (IS_ERR(__rwn.thread)) {
		RFS_ERROR("Unable to start kernel thread\n");
		return -ENOMEM;
	}


	return 0;
}


int rfs_wxt_stop(void)
{
	int err;

	if (__rwn.thread == NULL) {
		return 0;
	}

	RFS_DEBUG("kill rfs_wxt thread");
	force_sig(SIGKILL, __rwn.thread);
	err = kthread_stop(__rwn.thread);
	__rwn.thread = NULL;

	return err;
}

/*
 * rfs_wxt_init
 */
int rfs_wxt_init(void)
{
	return 0;
}


/*
 * rfs_wxt_exit
 */
void rfs_wxt_exit(void)
{
	RFS_DEBUG("leaving wxt\n");
	rfs_wxt_stop();
}


