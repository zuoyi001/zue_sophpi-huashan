rtos: memory-map
	$(call print_target)
	cd ${FREERTOS_PATH}/cvitek && ./build_${CHIP_ARCH_L}.sh

rtos-clean:
ifeq (${CONFIG_ENABLE_FREERTOS},y)
	$(call print_target)
	cd ${FREERTOS_PATH}/cvitek && rm -rf build 
endif
