
export EXTRA_CFLAGS := -std=gnu99

obj-m += cnodes.o

KVERSION := $(shell uname -r)


ifneq ("$(wildcard $(/lib/modules/$(KVERSION)/build/include/generated/uapi/linux/version.h))","")
	EXTRA_CFLAGS := -I/lib/modules/$(KVERSION)/build/include/generated/uapi/
endif

all:
	make -C /lib/modules/$(KVERSION)/build $(INCLUDE) M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build $(INCLUDE) M=$(PWD) clean
