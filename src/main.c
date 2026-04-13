#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>

#include "common.h"


#define DRV_NAME	"my_vif_echo_ping"


static int my_vif_validate(struct nlattr *tb[], struct nlattr *data[],
			  struct netlink_ext_ack *extack)
{
	if (tb[IFLA_ADDRESS]) {
		NL_SET_ERR_MSG(extack, "L3 device does not support MAC addresses");
		return -EOPNOTSUPP;
	}
	return 0;
}

static struct rtnl_link_ops my_vif_link_ops __read_mostly = {
	.kind		= DRV_NAME,
	.setup		= my_vif_setup,
	.validate	= my_vif_validate,
};


static int __init my_vif_init_module(void)
{
	int err = 0;

	err = rtnl_link_register(&my_vif_link_ops);
	if (err < 0)
		return err;

    /*
    err = my_vif_proc_init();
    if (err < 0) {
        // ВОТ ЗДЕСЬ нам нужно будет сделать отмену Шага 1
        rtnl_link_unregister(&my_vif_link_ops);
        return err;
    }
    */

    return 0;
}

static void __exit my_vif_cleanup_module(void)
{
	rtnl_link_unregister(&my_vif_link_ops);
}

module_init(my_vif_init_module);
module_exit(my_vif_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Dummy netdevice driver which discards all packets sent to it");
MODULE_ALIAS_RTNL_LINK(DRV_NAME);
