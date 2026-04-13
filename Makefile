KERNEL_DIR ?= /home/roman/linux
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
MY_PWD := $(shell pwd)

MODULE_NAME := my_vif_echo_ping

$(MODULE_NAME)-y := src/main.o src/netdev.o

obj-m += $(MODULE_NAME).o

ccflags-y := -I$(src)/include

all:
	$(MAKE) -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(MY_PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(MY_PWD) clean
