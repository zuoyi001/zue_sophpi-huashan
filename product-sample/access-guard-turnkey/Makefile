# build flow
export TOP_DIR = $(shell pwd)
ifeq ($(wildcard $(TOP_DIR)/.config),)
$(error  $(TOP_DIR)/.config not exist, please do 'make menuconfig' first!)
else
-include $(TOP_DIR)/.config
endif
##
export SDK_PATH=$(BUILD_PATH)/../
export CUSTOMER = $(shell echo $(CONFIG_CUSTOMER))

ifeq ($(CONFIG_CVI_SOC_CV183X), y)
export KERNEL_INC=$(SDK_PATH)/linux-linaro-stable/build/kernel_output/arm/usr/include
endif
ifeq ($(CONFIG_CVI_SOC_CV182X), y)
export KERNEL_INC=$(SDK_PATH)/linux/build/kernel_output/arm/usr/include
endif
ifeq ($(CONFIG_CVI_SOC_CR182X), y)
export KERNEL_INC=$(SDK_PATH)/linux_5.10/build/kernel_output/riscv/usr/include
endif

#export ALGO_VENDOR=megvii

##
# lib32 or lib64
ifeq ($(CONFIG_MW_SDK_VER_LIB32), y)
export SDK_VER = lib32
endif
ifeq ($(CONFIG_MW_SDK_VER_LIB64), y)
export SDK_VER = lib64
endif
ifeq ($(CONFIG_MW_SDK_VER_UCLIBC), y)
export SDK_VER = uclibc
endif
ifeq ($(CONFIG_MW_SDK_VER_MUSL), y)
export SDK_VER = musl_riscv64
endif
ifeq ($(CONFIG_MW_SDK_VER_LIB32), y)
export MW_SDK_VER = 32bit
endif
ifeq ($(CONFIG_MW_SDK_VER_LIB64), y)
export MW_SDK_VER = 64bit
endif
ifeq ($(CONFIG_MW_SDK_VER_UCLIBC), y)
export MW_SDK_VER = uclibc
endif
ifeq ($(CONFIG_MW_SDK_VER_MUSL), y)
export MW_SDK_VER = musl_riscv64
endif
export CONFIG_CVI_SOC
##
_GCC_TOOLCHAIN_PATH=$(SDK_PATH)/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu
_GCC_TOOLCHAIN_PATH_32=$(SDK_PATH)/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf
_GCC_TOOLCHAIN_PATH_UCLIBC=$(SDK_PATH)/host-tools/gcc/arm-cvitek-linux-uclibcgnueabihf
_GCC_TOOLCHAIN_PATH_MUSL=$(SDK_PATH)/host-tools/gcc/riscv64-linux-musl-x86_64
ifeq ($(CONFIG_MW_SDK_VER_LIB32), y)
export GCC_TOOLCHAIN_PATH = $(_GCC_TOOLCHAIN_PATH_32)/bin/arm-linux-gnueabihf-
export GCC_SYSROOT_PATH = $(_GCC_TOOLCHAIN_PATH_32)/arm-linux-gnueabihf
endif
ifeq ($(CONFIG_MW_SDK_VER_LIB64), y)
export GCC_TOOLCHAIN_PATH = $(_GCC_TOOLCHAIN_PATH)/bin/aarch64-linux-gnu-
export GCC_SYSROOT_PATH = $(_GCC_TOOLCHAIN_PATH)/aarch64-linux-gnu
endif
ifeq ($(CONFIG_MW_SDK_VER_UCLIBC), y)
export GCC_TOOLCHAIN_PATH = $(_GCC_TOOLCHAIN_PATH_UCLIBC)/bin/arm-cvitek-linux-uclibcgnueabihf-
export GCC_SYSROOT_PATH = $(_GCC_TOOLCHAIN_PATH_UCLIBC)/arm-cvitek-linux-uclibcgnueabihf
endif
ifeq ($(CONFIG_MW_SDK_VER_MUSL), y)
export GCC_TOOLCHAIN_PATH = $(_GCC_TOOLCHAIN_PATH_MUSL)/bin/riscv64-unknown-linux-musl-
export GCC_SYSROOT_PATH = $(_GCC_TOOLCHAIN_PATH_MUSL)/sysroot
endif

export CC := $(GCC_TOOLCHAIN_PATH)gcc
export CXX := $(GCC_TOOLCHAIN_PATH)g++
export AR := $(GCC_TOOLCHAIN_PATH)ar
export STRIP := $(GCC_TOOLCHAIN_PATH)strip
#
_NPROC := $(shell nproc)
export NPROC = $(shell $(TOP_DIR)/tools/scripts/nproc_limit.sh $(_NPROC))
#
MCONFG := $(TOP_DIR)/tools/scripts/menuconfig.py
AUTOCONFH := $(TOP_DIR)/tools/gen_autoconf_h
#
$(info TOP_DIR=$(TOP_DIR))
$(info NPROC=$(NPROC))
$(info SDK_VER=$(SDK_VER))
$(info CUSTOMER=$(CUSTOMER))
$(info ALGO_VENDOR=$(ALGO_VENDOR))
$(info SDK_PATH=$(SDK_PATH))
$(info KERNEL_INC=$(KERNEL_INC))
#
ifeq ($(CONFIG_BUILD_APP), y)
BUILD_DIRS = function_lib cpsl gui_engine/lvgl/lib_lvgl customer application
endif
ifeq ($(CONFIG_BUILD_APP_LEGACY), y)
BUILD_DIRS = function_lib cpsl gui_engine/LittleVGL customer application_legacy
endif
ifeq ($(CONFIG_BUILD_APP_SAMPLE), y)
BUILD_DIRS = cpsl gui_engine/awtkcode
BUILD_DIRS += app_sample customer
endif
ifeq ($(CONFIG_BUILD_APP_FRAMEWORK), y)
BUILD_DIRS = cpsl gui_engine/awtkcode application
endif
#
.PHONY: all clean install prepare prepare_clean
default: all
#
all: autoconf
	$(MAKE) -f $(TOP_DIR)/tools/checkconfigfile.mk $@
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) -j$(NPROC) $@" ; \
			if ($(MAKE) -C $$dir -j$(NPROC) $@) ; then true; else exit 1; fi; \
		fi \
	done;
#
clean:
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if ($(MAKE) -C $$dir $@) ; then true; else exit 1; fi; \
		fi \
	done;
#
cleanall: clean
	rm -rf $(TOP_DIR)/customer/$(CUSTOMER)/install
	rm -rf $(TOP_DIR)/customer/$(CUSTOMER)/app_install.tar.gz
	rm -rf $(TOP_DIR)/.config*
#
menuconfig:
	@$(MCONFG) $(TOP_DIR)/Kconfig
#
autoconf:
	@$(AUTOCONFH) $(TOP_DIR)/.config __$@.h
	@mv __$@.h $(TOP_DIR)/include/$@.h
#
prepare:
	$(TOP_DIR)/tools/scripts/get_prebuilt.sh
prepare_clean:
	rm -rf $(TOP_DIR)/prepare

mkinstalldir:
	mkdir -p $(TOP_DIR)/customer/$(CUSTOMER)/install
	mkdir -p $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	mkdir -p $(TOP_DIR)/customer/$(CUSTOMER)/install/cvimodel
#
install: mkinstalldir
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if ($(MAKE) -C $$dir $@) ; then true; else exit 1; fi; \
		fi \
	done;
	@tar -cvf $(TOP_DIR)/customer/$(CUSTOMER)/app_install.tar -C $(TOP_DIR)/customer/$(CUSTOMER) install
#
