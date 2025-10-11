MODULE_NAME = libkcom
EXTRA_CFLAGS += -I$(PWD) $(CFLAG)
KBUILD_CFLAGS += -Wno-declaration-after-statement

MODULE_SRC := $(notdir $(wildcard $(PWD)/*.c))
MODULE_OBJ = $(MODULE_SRC:c=o)

obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(MODULE_OBJ)

all:
	@echo objs=$($(MODULE_NAME)-objs)
	@-mkdir -p $(PWD)/tmp > /dev/null 2>&1
	@-touch $(PWD)/tmp/Makefile > /dev/null 2>&1
	@-ln -s $(PWD)/*.c $(PWD)/tmp > /dev/null 2>&1
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/tmp src=$(PWD) modules
	cp $(PWD)/tmp/*.ko $(PWD)
	cp $(PWD)/tmp/Module.symvers $(PWD)
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/tmp clean
	-rm -rf $(PWD)/tmp
	-rm $(PWD)/*.ko
	-rm $(PWD)/Module.symvers

install:
	@-mkdir /lib/modules/$(shell uname -r)/extra
	make INSTALL_MOD_DIR=extra -C /lib/modules/$(shell uname -r)/build M=$(PWD)/tmp modules_install
	depmod -a
	insmod /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko

uninstall:
	-rmmod $(MODULE_NAME)
	-rm -f /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko
	depmod -a

.PHONY: all clean install uninstall