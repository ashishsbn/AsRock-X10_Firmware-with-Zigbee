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

#include <net/sock.h>
#include "nss_tx_rx_common.h"

/*
 * Data structures to store pptp nss debug stats
 */
static DEFINE_SPINLOCK(nss_pptp_session_debug_stats_lock);
static struct nss_stats_pptp_session_debug nss_pptp_session_debug_stats[NSS_MAX_PPTP_DYNAMIC_INTERFACES];

/*
 * nss_pptp_session_debug_stats_sync
 *	Per session debug stats for pptp
 */
void nss_pptp_session_debug_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_pptp_sync_session_stats_msg *stats_msg, uint16_t if_num)
{
	int i;
	spin_lock_bh(&nss_pptp_session_debug_stats_lock);
	for (i = 0; i < NSS_MAX_PPTP_DYNAMIC_INTERFACES; i++) {
		if (nss_pptp_session_debug_stats[i].if_num == if_num) {
			nss_pptp_session_debug_stats[i].stats[NSS_STATS_PPTP_SESSION_RX_DROPPED] += stats_msg->rx_dropped;
			nss_pptp_session_debug_stats[i].stats[NSS_STATS_PPTP_SESSION_TX_DROPPED] += stats_msg->tx_dropped;
			nss_pptp_session_debug_stats[i].stats[NSS_STATS_PPTP_SESSION_RX_PPP_LCP_PKTS] += stats_msg->rx_ppp_lcp_pkts;
			nss_pptp_session_debug_stats[i].stats[NSS_STATS_PPTP_SESSION_RX_EXP_DATA_PKTS] += stats_msg->rx_exception_data_pkts;
			break;
		}
	}
	spin_unlock_bh(&nss_pptp_session_debug_stats_lock);
}

/*
 * nss_pptp_global_session_stats_get()
 *	Get session pptp statitics.
 */
void nss_pptp_session_debug_stats_get(void *stats_mem)
{
	struct nss_stats_pptp_session_debug *stats = (struct nss_stats_pptp_session_debug *)stats_mem;
	int i;

	if (!stats) {
		nss_warning("No memory to copy pptp session stats");
		return;
	}

	spin_lock_bh(&nss_pptp_session_debug_stats_lock);
	for (i = 0; i < NSS_MAX_PPTP_DYNAMIC_INTERFACES; i++) {
		if (nss_pptp_session_debug_stats[i].valid) {
			memcpy(stats, &nss_pptp_session_debug_stats[i], sizeof(struct nss_stats_pptp_session_debug));
			stats++;
		}
	}
	spin_unlock_bh(&nss_pptp_session_debug_stats_lock);
}

/*
 * nss_pptp_handler()
 *	Handle NSS -> HLOS messages for pptp tunnel
 */
static void nss_pptp_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm, __attribute__((unused))void *app_data)
{
	struct nss_pptp_msg *ntm = (struct nss_pptp_msg *)ncm;
	void *ctx;

	nss_pptp_msg_callback_t cb;

	BUG_ON(!(nss_is_dynamic_interface(ncm->interface) || ncm->interface == NSS_PPTP_INTERFACE));

	/*
	 * Is this a valid request/response packet?
	 */
	if (ncm->type >= NSS_PPTP_MSG_MAX) {
		nss_warning("%p: received invalid message %d for PPTP interface", nss_ctx, ncm->type);
		return;
	}

	if (ncm->len > sizeof(struct nss_pptp_msg)) {
		nss_warning("%p: tx request for another interface: %d", nss_ctx, ncm->interface);
		return;
	}

	switch (ntm->cm.type) {

	case NSS_PPTP_MSG_SYNC_STATS:
		/*
		 * session debug stats embeded in session stats msg
		 */
		nss_pptp_session_debug_stats_sync(nss_ctx, &ntm->msg.stats, ncm->interface);
		break;
	}

	/*
	 * Update the callback and app_data for NOTIFY messages, pptp sends all notify messages
	 * to the same callback/app_data.
	 */
	if (ncm->response == NSS_CMM_RESPONSE_NOTIFY) {
		ncm->cb = (uint32_t)nss_ctx->nss_top->pptp_msg_callback;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	/*
	 * Do we have a call back
	 */
	if (!ncm->cb) {
		return;
	}

	/*
	 * callback
	 */
	cb = (nss_pptp_msg_callback_t)ncm->cb;
	ctx =  nss_ctx->nss_top->subsys_dp_register[ncm->interface].ndev;

	/*
	 * call pptp tunnel callback
	 */
	if (!ctx) {
		nss_warning("%p: Event received for pptp tunnel interface %d before registration", nss_ctx, ncm->interface);
		return;
	}

	cb(ctx, ntm);
}

/*
 * nss_pptp_tx()
 *	Transmit a pptp message to NSS firmware
 */
nss_tx_status_t nss_pptp_tx(struct nss_ctx_instance *nss_ctx, struct nss_pptp_msg *msg)
{
	struct nss_pptp_msg *nm;
	struct nss_cmn_msg *ncm = &msg->cm;
	struct sk_buff *nbuf;
	int32_t status;

	NSS_VERIFY_CTX_MAGIC(nss_ctx);
	if (unlikely(nss_ctx->state != NSS_CORE_STATE_INITIALIZED)) {
		nss_warning("%p: pptp msg dropped as core not ready", nss_ctx);
		return NSS_TX_FAILURE_NOT_READY;
	}

	/*
	 * Sanity check the message
	 */
	if (!nss_is_dynamic_interface(ncm->interface)) {
		nss_warning("%p: tx request for non dynamic interface: %d", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE;
	}

	if (ncm->type > NSS_PPTP_MSG_MAX) {
		nss_warning("%p: message type out of range: %d", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	if (ncm->len > sizeof(struct nss_pptp_msg)) {
		nss_warning("%p: message length is invalid: %d", nss_ctx, ncm->len);
		return NSS_TX_FAILURE;
	}

	nbuf = dev_alloc_skb(NSS_NBUF_PAYLOAD_SIZE);
	if (unlikely(!nbuf)) {
		NSS_PKT_STATS_INCREMENT(nss_ctx, &nss_ctx->nss_top->stats_drv[NSS_STATS_DRV_NBUF_ALLOC_FAILS]);
		nss_warning("%p: msg dropped as command allocation failed", nss_ctx);
		return NSS_TX_FAILURE;
	}

	/*
	 * Copy the message to our skb
	 */
	nm = (struct nss_pptp_msg *)skb_put(nbuf, sizeof(struct nss_pptp_msg));
	memcpy(nm, msg, sizeof(struct nss_pptp_msg));

	status = nss_core_send_buffer(nss_ctx, 0, nbuf, NSS_IF_CMD_QUEUE, H2N_BUFFER_CTRL, 0);
	if (status != NSS_CORE_STATUS_SUCCESS) {
		dev_kfree_skb_any(nbuf);
		nss_warning("%p: Unable to enqueue 'pptp message'\n", nss_ctx);
		if (status == NSS_CORE_STATUS_FAILURE_QUEUE) {
			return NSS_TX_FAILURE_QUEUE;
		}
		return NSS_TX_FAILURE;
	}

	nss_hal_send_interrupt(nss_ctx->nmap, nss_ctx->h2n_desc_rings[NSS_IF_CMD_QUEUE].desc_ring.int_bit,
				NSS_REGS_H2N_INTR_STATUS_DATA_COMMAND_QUEUE);

	NSS_PKT_STATS_INCREMENT(nss_ctx, &nss_ctx->nss_top->stats_drv[NSS_STATS_DRV_TX_CMD_REQ]);
	return NSS_TX_SUCCESS;
}

/*
 * nss_register_pptp_if()
 */
struct nss_ctx_instance *nss_register_pptp_if(uint32_t if_num, nss_pptp_callback_t pptp_callback,
			nss_pptp_msg_callback_t event_callback, struct net_device *netdev, uint32_t features)
{

	nss_assert(nss_is_dynamic_interface(if_num));

	nss_top_main.subsys_dp_register[if_num].ndev = netdev;
	nss_top_main.subsys_dp_register[if_num].cb = pptp_callback;
	nss_top_main.subsys_dp_register[if_num].app_data = NULL;
	nss_top_main.subsys_dp_register[if_num].features = features;

	nss_top_main.pptp_msg_callback = event_callback;

	nss_core_register_handler(if_num, nss_pptp_handler, NULL);

	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.pptp_handler_id];
}

/*
 * nss_unregister_pptp_if()
 */
void nss_unregister_pptp_if(uint32_t if_num)
{
	nss_assert(nss_is_dynamic_interface(if_num));

	nss_top_main.subsys_dp_register[if_num].ndev = NULL;
	nss_top_main.subsys_dp_register[if_num].cb = NULL;
	nss_top_main.subsys_dp_register[if_num].app_data = NULL;
	nss_top_main.subsys_dp_register[if_num].features = 0;

	nss_top_main.pptp_msg_callback = NULL;

	nss_core_unregister_handler(if_num);
}

/*
 * nss_get_pptp_context()
 */
struct nss_ctx_instance *nss_pptp_get_context()
{
	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.pptp_handler_id];
}

/*
 * nss_pptp_msg_init()
 *      Initialize nss_pptp msg.
 */
void nss_pptp_msg_init(struct nss_pptp_msg *ncm, uint16_t if_num, uint32_t type,  uint32_t len, void *cb, void *app_data)
{
	nss_cmn_msg_init(&ncm->cm, if_num, type, len, cb, app_data);
}

/* nss_pptp_register_handler()
 *   debugfs stats msg handler received on static pptp interface
 */
void nss_pptp_register_handler(void)
{
	nss_info("nss_pptp_register_handler");
	nss_core_register_handler(NSS_PPTP_INTERFACE, nss_pptp_handler, NULL);
}

EXPORT_SYMBOL(nss_pptp_get_context);
EXPORT_SYMBOL(nss_pptp_tx);
EXPORT_SYMBOL(nss_unregister_pptp_if);
EXPORT_SYMBOL(nss_pptp_msg_init);
EXPORT_SYMBOL(nss_register_pptp_if);
