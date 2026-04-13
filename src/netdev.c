#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/net_tstamp.h>
#include <linux/u64_stats_sync.h>
#include <linux/if_arp.h>

#include "common.h"

#define IPV4_ADDR(a, b, c, d) ((__be32)((a) | ((b) << 8) | ((c) << 16) | ((d) << 24)))


static netdev_tx_t my_vif_xmit(struct sk_buff *skb, struct net_device *dev)
{
	dev_lstats_add(dev, skb->len);

	skb_tx_timestamp(skb);
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

/* fake multicast ability */
static void set_multicast_list(struct net_device *dev)
{
}

static int my_vif_dev_init(struct net_device *dev)
{
	dev->pcpu_stat_type = NETDEV_PCPU_STAT_LSTATS;

	netdev_lockdep_set_classes(dev);
	return 0;
}

static int my_vif_change_carrier(struct net_device *dev, bool new_carrier)
{
	if (new_carrier)
		netif_carrier_on(dev);
	else
		netif_carrier_off(dev);
	return 0;
}

static void my_vif_get_stats64(struct net_device *dev,
			      struct rtnl_link_stats64 *stats)
{
	dev_lstats_read(dev, &stats->tx_packets, &stats->tx_bytes);
}

static const struct net_device_ops my_vif_netdev_ops = {
	.ndo_init		= my_vif_dev_init,
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
    priv->target_ip = IPV4_ADDR(192, 168, 1, 231);

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
