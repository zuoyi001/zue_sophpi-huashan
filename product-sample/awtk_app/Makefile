CUR_DIR     := $(shell pwd)
TOP_DIR     := $(CUR_DIR)/../..
INSTALL_DIR ?= $(TOP_DIR)/install

.PHONY : clean all install

all:
	$(CUR_DIR)/build_awtk.sh
	make -C touchpad all
	make -C ui all

clean:
	make -C touchpad clean
	make -C ui clean
	
install:
	mkdir -p $(CUR_DIR)/install/lib
	cp $(CUR_DIR)/awtkcode/awtk-linux-fb/build/lib/*.so $(CUR_DIR)/install/lib
	make -C touchpad install
	make -C ui install
