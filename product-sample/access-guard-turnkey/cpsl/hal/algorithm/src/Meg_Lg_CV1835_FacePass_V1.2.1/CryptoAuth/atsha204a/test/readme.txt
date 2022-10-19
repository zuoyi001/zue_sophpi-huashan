#export PATH=$PATH:/home/data/cv1835/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin
#export PATH=$PATH:/home/data/cv1835/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin

#aarch64-linux-gnu-gcc  test_main.c ../hal/hal_linux_i2c_userspace.c -I ../hal   -o test_main_64
arm-linux-gnueabihf-gcc test_main.c ../hal/hal_linux_i2c_userspace.c -I ../hal   -o test_main_32

