/**
 * @file common.h
 * @brief Общий заголовочник для модуля
 */

#ifndef COMMON_H
#define COMMON_H

#include <linux/types.h>

/**
 * @struct Приватные данные интерфейса
 */
struct my_vif_priv
{
	u32 target_ip;  // Целевой IP (порядок байтов как в сети - big endian)
	struct proc_dir_entry *proc_entry;	// Сущность в procfs
	struct net_device *dev;	// Устройство, которому принадлежат данные (владелец)
};

/**
 * @brief Функция устанавливающая настройки устройства (виртуального интерфейса)
 * @param [in] dev указатель на устройство, для которого будут установлены настройки
 */
void my_vif_setup(struct net_device *dev);

#endif
