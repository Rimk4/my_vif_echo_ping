my_vif_echo_ping/
├── include
│   └── common.h
├── Makefile
├── README.md
└── src
    ├── main.c
    ├── netdev.c
    └── proc_config.c


make KCFLAGS="-DDEBUG"

insmod /mnt/boot/my_vif_echo_ping.ko

ip link add vping0 type my_vif_echo_ping
ip addr add 192.168.1.230/32 dev vping0
ip link set vping0 up
ip route add 192.168.1.231 dev vping0
ping 192.168.1.231
