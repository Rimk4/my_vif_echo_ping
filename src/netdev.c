/**
 * @file netdev.c
 * @brief Реализация виртуального сетевого устройства my_vif
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/icmp.h>
#include <linux/net_tstamp.h>
#include <linux/u64_stats_sync.h>
#include <linux/if_arp.h>

#include "proc.h"
#include "common.h"

#define IPV4_ADDR(a, b, c, d) ((__be32)((a) | ((b) << 8) | ((c) << 16) | ((d) << 24)))

/**
 * @brief Дефолтный IP
 */
const static __be32 my_vif_default_ip = IPV4_ADDR(192, 168, 1, 231);

/**
 * @brief Transmit функция обратного вызова для виртуального интерфейса
 */
static netdev_tx_t my_vif_xmit(struct sk_buff *skb, struct net_device *dev)
{
	skb_set_network_header(skb, 0);
	struct iphdr *iph = ip_hdr(skb);
	pr_debug("Received packet to %pI4\n", &iph->daddr);

	if (skb->len < sizeof(struct iphdr) || iph->protocol != IPPROTO_ICMP) {
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	struct my_vif_priv *priv = netdev_priv(dev);
	if (iph->daddr != priv->target_ip)
	{
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	if (skb_cow_head(skb, 0) != 0) {
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	iph = ip_hdr(skb);
	skb_set_transport_header(skb, ip_hdrlen(skb));

	if (skb->len < ip_hdrlen(skb) + sizeof(struct icmphdr))
	{
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	struct icmphdr *icmph = icmp_hdr(skb);

	if (icmph->type == ICMP_ECHO) {
		pr_debug("Echo Request");

		swap(iph->saddr, iph->daddr);
		icmph->type = ICMP_ECHOREPLY;

		ip_send_check(iph);

		icmph->checksum = 0;
		icmph->checksum = ip_compute_csum(icmph, skb->len - ip_hdrlen(skb));

		skb->protocol = htons(ETH_P_IP);
		skb->dev = dev;

		skb_reset_network_header(skb);
		skb->ip_summed = CHECKSUM_NONE;

		dev_lstats_add(dev, skb->len);
		netif_rx(skb);
		if (netif_rx(skb) != NET_RX_SUCCESS) {
			dev->stats.rx_dropped++;
		}

		return NETDEV_TX_OK;
	}

	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

/**
 * @brief Заглушка для ndo_set_rx_mode
 */
static void set_multicast_list(struct net_device *dev)
{
}

/**
 * @brief Инициализация устройства
 */
static int my_vif_dev_init(struct net_device *dev)
{
	dev->pcpu_stat_type = NETDEV_PCPU_STAT_LSTATS;

	int err = my_vif_proc_add_dev(dev);
	if (err) {
		pr_err("failed to create proc entry for %s (err %d)\n", dev->name, err);
		return err;
	}

	netdev_lockdep_set_classes(dev);

	return 0;
}

/**
 * @brief Деинициализация устройства
 */
static void my_vif_dev_uninit(struct net_device *dev)
{
	my_vif_proc_del_dev(dev);
}

/**
 * @brief Управление состоянием устройства
 */
static int my_vif_change_carrier(struct net_device *dev, bool new_carrier)
{
	if (new_carrier)
		netif_carrier_on(dev);
	else
		netif_carrier_off(dev);
	return 0;
}

/**
 * @brief Сбор статистики
 */
static void my_vif_get_stats64(struct net_device *dev,
				  struct rtnl_link_stats64 *stats)
{
	dev_lstats_read(dev, &stats->tx_packets, &stats->tx_bytes);
}

/**
 * @brief Структура с указателями на ф-ии Network Device Operations
 */
static const struct net_device_ops my_vif_netdev_ops = {
	.ndo_init		= my_vif_dev_init,
	.ndo_uninit     = my_vif_dev_uninit,
	.ndo_start_xmit		= my_vif_xmit,
	.ndo_set_rx_mode	= set_multicast_list,
	.ndo_get_stats64	= my_vif_get_stats64,
	.ndo_change_carrier	= my_vif_change_carrier,
};

void my_vif_setup(struct net_device *dev)
{
	dev->type = ARPHRD_NONE;
	/* Initialize the device structure. */
	dev->netdev_ops = &my_vif_netdev_ops;
	dev->needs_free_netdev = true;

	dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
	struct my_vif_priv *priv = netdev_priv(dev);
	WRITE_ONCE(priv->target_ip, my_vif_default_ip);
	priv->dev = dev;

	dev->priv_flags |= IFF_LIVE_ADDR_CHANGE | IFF_NO_QUEUE;
	dev->features	|= NETIF_F_SG | NETIF_F_FRAGLIST;
	dev->features	|= NETIF_F_GSO_SOFTWARE;
	dev->features	|= NETIF_F_HW_CSUM | NETIF_F_HIGHDMA | NETIF_F_LLTX;
	dev->features	|= NETIF_F_GSO_ENCAP_ALL;
	dev->hw_features |= dev->features;
	dev->hw_enc_features |= dev->features;

	dev->mtu = 1500;
	dev->min_mtu = 68;
	dev->max_mtu = 65535;
}
