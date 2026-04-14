/**
 * @file proc.h
 * @brief Пользовательский интерфейс управления драйвером через procfs
 */

#ifndef _MY_VIF_PROC_H
#define _MY_VIF_PROC_H

#include <linux/netdevice.h>

/**
 * @brief Инициализация корневой директории модуля в procfs
 */
int my_vif_proc_net_init(void);
/**
 * @brief Удаление корневой директории модуля из procfs
 */
void my_vif_proc_net_exit(void);
/**
 * @brief Создание файла для управления конкретным интерфейсом
 */
int my_vif_proc_add_dev(struct net_device *dev);
/**
 * @brief Удаление файла для управления конкретным интерфейсом
 */
void my_vif_proc_del_dev(struct net_device *dev);

#endif
