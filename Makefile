MODULE_NAME = libkcom
EXTRA_CFLAGS += -I$(PWD) $(CFLAG)
KBUILD_CFLAGS += -Wno-declaration-after-statement

MODULE_SRC := $(notdir $(wildcard $(PWD)/*.c))
MODULE_OBJ = $(MODULE_SRC:c=o)

obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(MODULE_OBJ)

all: 
	@echo objs=$($(MODULE_NAME)-objs)
	@-mkdir -p $(PWD)/out > /dev/null 2>&1
	@-touch $(PWD)/out/Makefile > /dev/null 2>&1
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/out src=$(PWD) modules
	cp $(PWD)/out/*.ko $(PWD)
	cp $(PWD)/out/Module.symvers $(PWD)

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/out clean
	-rm -rf $(PWD)/out
	-rm $(PWD)/*.ko
	-rm $(PWD)/Module.symvers

install:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/out modules_install
	depmod -a
	insmod /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko

uninstall:
	-rmmod $(MODULE_NAME)
	-rm -f /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko
	depmod -a

.PHONY: all clean install uninstall