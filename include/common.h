#ifndef COMMON_H
#define COMMON_H

#include <linux/mutex.h>
#include <linux/types.h>


struct net_device;

struct my_vif_priv
{
	u32 target_ip;  // big endian
	struct mutex lock;
	struct proc_dir_entry *proc_entry;
	struct net_device *dev;
};

void my_vif_setup(struct net_device *dev);

#endif
