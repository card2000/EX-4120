#! /bin/sh

date

flash_erase -q /dev/mtd0 0 0
flash_erase -q /dev/mtd1 0 0
flash_erase -q /dev/mtd3 0 0
flash_erase -q /dev/mtd4 0 0

nandwrite -p /dev/mtd0 /media/mmcblk0p1/MLO
nandwrite -p /dev/mtd1 /media/mmcblk0p1/u-boot.bin
nandwrite -p /dev/mtd3 /media/mmcblk0p1/uImage
nandwrite -p /dev/mtd3 -s 0x280000 /media/mmcblk0p1/uImage

ubiattach /dev/ubi_ctrl -m 4 -O 2048
ubimkvol /dev/ubi0 -n 0 -N rootfs -m
if [ $? -eq 0 ]
then
  if [ ! -d /mnt/rootfs ]
  then
    mkdir /mnt/rootfs
  fi
  mount -t ubifs /dev/ubi0_0 /mnt/rootfs
  if [ $? -eq 0 ]
  then
    tar zxf /media/mmcblk0p1/rootfs.tar.gz -C /mnt/rootfs
    fuser -m /mnt/rootfs
    while [ $? -eq 0 ]
    do
      sleep 1
      fuser -m /mnt/rootfs
    done
    umount /mnt/rootfs
    ubidetach -p /dev/mtd4
    date
  fi
fi

