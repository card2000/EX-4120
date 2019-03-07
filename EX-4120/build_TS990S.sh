# change sdk directory 
source ti-sdk-am3517-evm-05.04.00.00/linux-devkit/environment-setup

cd linux-omap3
make clean
make ARCH=arm ts990_defconfig
make ARCH=arm zImage
../mkimage -A arm -O linux -T kernel -C none -a 0x80008000 -e 0x80008000 -n 'Linux Kernel 2.6.37' -d arch/arm/boot/zImage uImage
make ARCH=arm modules
cd ../
cd u-boot
make clean
make CROSS_COMPILE=arm-arago-linux-gnueabi- am3517_evm
cd ../
cd x-load
make clean
make ARCH=arm CROSS_COMPILE=arm-arago-linux-gnueabi- am3517evm_config
make ARCH=arm CROSS_COMPILE=arm-arago-linux-gnueabi-
./signGP x-load.bin
cd ../

