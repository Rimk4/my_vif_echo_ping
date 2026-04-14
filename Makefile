KERNEL_DIR ?= /home/roman/linux
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
MY_PWD := $(shell pwd)
BUILD_DIR := $(MY_PWD)/build

obj-m := my_vif_echo_ping.o
my_vif_echo_ping-y := src/main.o src/netdev.o src/proc.o

ccflags-y := -I$(src)/include

all:
	@mkdir -p $(BUILD_DIR)/src
	@mkdir -p $(BUILD_DIR)/include
	@cp -ru $(MY_PWD)/src $(BUILD_DIR)/
	@cp -ru $(MY_PWD)/include $(BUILD_DIR)/
	@cp $(MY_PWD)/Makefile $(BUILD_DIR)/
	$(MAKE) -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(BUILD_DIR) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(BUILD_DIR) clean
	rm -rf $(BUILD_DIR)
