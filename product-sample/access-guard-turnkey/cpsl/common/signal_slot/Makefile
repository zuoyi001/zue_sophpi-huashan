SRCTREE ?= ../../../
PROJECT_NAME := cvi_signal_slot

include $(SRCTREE)/build/base.mk

.PHONY : clean all install

all:
	cmake $(CURDIR) -G Ninja -Bbuild -DCMAKE_TOOLCHAIN_FILE=$(SRCTREE)/cmake/toolchain.cmake \
	-D$(PROJECT_NAME)_BUILD_STATIC_LIB=on
	ninja -C $(CURDIR)/build
	ninja -C $(CURDIR)/build install
	mkdir -p $(CURDIR)/lib
	cp $(CURDIR)/install/lib/lib$(PROJECT_NAME)* $(CURDIR)/lib/

clean:
	rm $(CURDIR)/build -rf
	rm $(CURDIR)/install -rf

$(INSTALL_PATH)/lib:
	mkdir -p $@

install: | $(INSTALL_PATH)/lib
	cp $(CURDIR)/lib/lib$(PROJECT_NAME)* $(INSTALL_PATH)/lib/
