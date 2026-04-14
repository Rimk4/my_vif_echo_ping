# My Virtual Interface (Echo Reply To Ping)

Задача: Модуль на C, который создаёт виртуальный сетевой интерфейс. Посредством procfs модуль позволяет задать IPv4 адрес, который отвечает на ping запросы.

При решении за референс были взяты модули ядра:  
- `drivers/net/dummy.c` - для создания виртуального интерфейса
- `drivers/net/loopback.c` - для примера возвращения ответа через `netif_rx` (внутри `loopback_xmit`)
- `net/8021q/vlanproc.c` и `net/ipv4/ipconfig.c` - для примера работы с procfs

# Дерево проекта
```
my_vif_echo_ping/
├── include
│   ├── common.h
│   └── proc.h
├── Makefile
├── README.md
└── src
	├── main.c
	├── netdev.c
	└── proc.c
```

### Платформа
- **Целевая архитектура**: ARM (Allwinner H2+)
- **Версия ядра**: 6.9.0
- **Кросс-компилятор**: arm-linux-gnueabihf-

# Собрать модуль
```
make
```

# Собрать в дебаге
```
make KCFLAGS="-DDEBUG"
```

# Очистить проект
```
make clean
```

# my_vif_echo_ping - драйвер виртуального сетевого интерфейса

## IP-адрес по умолчанию

По умолчанию драйвер отвечает на ping для IP-адреса **192.168.1.231**

### Загрузка модуля и создание интерфейса
```bash
insmod my_vif_echo_ping.ko
ip link add vping0 type my_vif_echo_ping
ip addr add 192.168.1.230/32 dev vping0
ip link set vping0 up
ip route add 192.168.1.231 dev vping0
```

### Изменение целевого IP через procfs
```
echo "192.168.1.100" > /proc/net/myvif/vping0
cat /proc/net/myvif/vping0
```

### Добавление маршрута для нового IP
```
ip route add 192.168.1.100 dev vping0
ping 192.168.1.100
```

# Пример сценария
```
# mount /dev/mmcblk0p1 /mnt/boot/
[   23.022779] FAT-fs (mmcblk0p1): Volume was not properly unmounted. Some data may be corrupt. Please run fsck.
# insmod /mnt/boot/my_vif_echo_ping.ko 
[   35.658273] my_vif_echo_ping: loading out-of-tree module taints kernel.
# ip link add vping0 type my_vif_echo_ping
# ip addr add 192.168.1.230/32 dev vping0
# ip link set vping0 up
# ip route add 192.168.1.231 dev vping0
# dmesg -c 1>/dev/null
# cat /proc/net/myvif/vping0
192.168.1.231
# ping 192.168.1.231
PING 192.168.1.231 (192.168.1.231): 56 data bytes
64 bytes from 192.168.1.231: seq=0 ttl=64 time=0.254 ms
64 bytes from 192.168.1.231: seq=1 ttl=64 time=0.184 ms
64 bytes from 192.168.1.231: seq=2 ttl=64 time=0.216 ms
^C
--- 192.168.1.231 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max = 0.184/0.218/0.254 ms
# echo "192.168.1.100" > /proc/net/myvif/vping0
# cat /proc/net/myvif/vping0
192.168.1.100
# ping 192.168.1.231
PING 192.168.1.231 (192.168.1.231): 56 data bytes
^C
--- 192.168.1.231 ping statistics ---
5 packets transmitted, 0 packets received, 100% packet loss
# dmesg -c
[  110.555699] my_vif_echo_ping: Received packet to 192.168.1.231
[  110.555747] my_vif_echo_ping: Echo Request
[  111.555968] my_vif_echo_ping: Received packet to 192.168.1.231
[  111.556003] my_vif_echo_ping: Echo Request
[  112.556215] my_vif_echo_ping: Received packet to 192.168.1.231
[  112.556258] my_vif_echo_ping: Echo Request
[  149.384248] my_vif_echo_ping: vping0 target IP updated to 192.168.1.100
[  153.163928] my_vif_echo_ping: Received packet to 192.168.1.231
[  154.164190] my_vif_echo_ping: Received packet to 192.168.1.231
[  155.164350] my_vif_echo_ping: Received packet to 192.168.1.231
[  156.164597] my_vif_echo_ping: Received packet to 192.168.1.231
[  157.164799] my_vif_echo_ping: Received packet to 192.168.1.231
# ip route add 192.168.1.100 dev vping0
# ping 192.168.1.100
PING 192.168.1.100 (192.168.1.100): 56 data bytes
64 bytes from 192.168.1.100: seq=0 ttl=64 time=0.222 ms
64 bytes from 192.168.1.100: seq=1 ttl=64 time=0.180 ms
64 bytes from 192.168.1.100: seq=2 ttl=64 time=0.137 ms
^C
--- 192.168.1.100 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max = 0.137/0.179/0.222 ms
# dmesg -c
[  190.002406] my_vif_echo_ping: Received packet to 192.168.1.100
[  190.002451] my_vif_echo_ping: Echo Request
[  191.009111] my_vif_echo_ping: Received packet to 192.168.1.100
[  191.009148] my_vif_echo_ping: Echo Request
[  192.015207] my_vif_echo_ping: Received packet to 192.168.1.100
```
