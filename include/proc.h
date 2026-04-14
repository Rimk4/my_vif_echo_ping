#ifndef _MY_VIF_PROC_H
#define _MY_VIF_PROC_H

#include <linux/netdevice.h>


int my_vif_proc_net_init(void);
void my_vif_proc_net_exit(void);

int my_vif_proc_add_dev(struct net_device *dev);
void my_vif_proc_del_dev(struct net_device *dev);

#endif
