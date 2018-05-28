FW_DIR	:= /lib/firmware/rtl_bt/
MDL_DIR	:= /lib/modules/$(KERNEL_VERSION)
DRV_DIR	:= $(MDL_DIR)/kernel/drivers/bluetooth
EXTRA_CFLAGS += -DCONFIG_BT_RTL
EXTRA_CFLAGS += -DCONFIG_BT_INTEL
EXTRA_CFLAGS += -DCONFIG_BT_BCM

ARCH := arm
CROSS_COMPILE ?= 
KVER := $(KERNEL_VERSION)
KSRC := $(KERNEL_PATH)	

ifneq ($(KERNELRELEASE),)

	obj-m := btusb.o btrtl.o btintel.o btbcm.o

else

	KDIR := /lib/modules/$(KVER)/build

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules

clean:
	rm -rf *.o *.mod.c *.mod.o *.ko *.symvers *.order *.a
endif

install:
	@mkdir -p $(FW_DIR)
	@cp -f *_fw.bin $(FW_DIR)/.
	@cp -f *.ko $(DRV_DIR)/.

	@depmod -a
	@echo "installed revised btusb"

uninstall:
	rm -f $(DRV_DIR)/btusb.ko*
	depmod -a $(MDL_DIR)
	echo "uninstalled revised btusb"
