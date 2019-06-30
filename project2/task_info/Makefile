ifneq ($(KERNELRELEASE),)
mymodule-objs := task_info.o
obj-m := task_info.o
 
else
PWD := $(shell pwd)
KVER ?= $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build
 
all:
	$(MAKE) -C $(KDIR) M=$(PWD)
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions
endif

