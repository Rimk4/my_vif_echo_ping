/**
 * @file proc.c
 * @brief Реализация procfs интерфейса для управления драйвером my_vif
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/inet.h>
#include <net/net_namespace.h>

#include "proc.h"
#include "common.h"

/*
 *	Names of the proc directory entries
 */
static const char name_root[]	 = "myvif";
/**
 * @brief Указатель на сущность procfs
 */
static struct proc_dir_entry *myvif_dir;


int my_vif_proc_net_init(void)
{
	myvif_dir = proc_net_mkdir(&init_net, name_root, init_net.proc_net);
	if (!myvif_dir)
		return -ENOMEM;

	return 0;
}

void my_vif_proc_net_exit(void)
{
	remove_proc_subtree(name_root, init_net.proc_net);
	pr_info("unloaded successfully\n");
}

/**
 * @brief Чтение текущего целевого IP-адреса из файла в procfs
 */
static ssize_t my_vif_proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct my_vif_priv *priv = pde_data(file_inode(file));
	char tmp[32];
	
	u32 ip;
	ip = READ_ONCE(priv->target_ip);

	int len = scnprintf(tmp, sizeof(tmp), "%pI4\n", &ip);

	return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

/**
 * @brief Запись нового целевого IP-адреса в файл в procfs
 */
static ssize_t my_vif_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct my_vif_priv *priv = pde_data(file_inode(file));
	char kbuf[32];
	u32 new_ip;
	size_t len = min(count, sizeof(kbuf) - 1);

	if (copy_from_user(kbuf, buf, len))
		return -EFAULT;

	kbuf[len] = '\0';

	if (in4_pton(kbuf, -1, (u8 *)&new_ip, -1, NULL) > 0) {
		WRITE_ONCE(priv->target_ip, new_ip);
		pr_debug("%s target IP updated to %pI4\n", priv->dev->name, &new_ip);
	} else {
		return -EINVAL;
	}

	return count;
}

/**
 * @brief Структура с указателями на функции операций над файлами виртуальных интерфейсов в procfs
 */
static const struct proc_ops my_vif_proc_ops = {
	.proc_read  = my_vif_proc_read,
	.proc_write = my_vif_proc_write,
};

int my_vif_proc_add_dev(struct net_device *dev)
{
	struct my_vif_priv *priv = netdev_priv(dev);

	if (!myvif_dir)
		return -ENODEV;

	priv->proc_entry = proc_create_data(dev->name, 0666, myvif_dir, &my_vif_proc_ops, priv);
	if (!priv->proc_entry)
		return -ENOMEM;

	return 0;
}

void my_vif_proc_del_dev(struct net_device *dev)
{
	if (myvif_dir) {
		remove_proc_entry(dev->name, myvif_dir);
	}

	struct my_vif_priv *priv = netdev_priv(dev);
	priv->proc_entry = NULL;
}
