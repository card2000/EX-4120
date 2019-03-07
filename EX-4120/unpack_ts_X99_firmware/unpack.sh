#!/bin/sh

echo " "
echo " "
echo "use : with sudo or as root : .sh /path/to/your/TS-990_V122.zip"
echo " "
echo " "

if [[ $# -eq 0 ]] ; then
    echo 'no firmware given...'
    exit 1
fi


if [ ! -f ${PWD}/kenwood_tool ]
then 
    make
    test -x ${PWD}/kenwood_tool || exit 0
fi

fw=$(basename -s .zip $1)
dl=$(basename $1)

mod=${fw%_*}
model=$(echo ${mod//-} |tr '[:upper:]' '[:lower:]')
download=$(echo ${mod/-/_} |tr '[:upper:]' '[:lower:]')
ver=${fw##*_}
version=${ver:0:-2}.${ver#${ver:0:-2}}

echo $version
echo $model
echo $dl
echo $download
filetounzip=$1



if [ ! -f $1 ]
then
    wget https://www.kenwood.com/i/products/info/amateur/$download/pdf/$dl
    if [ -f ${PWD}/$dl ]
    then
        filetounzip=${PWD}/$dl
    fi
fi

firmware_unzipe_dir=${PWD}/firmware/tmp/$fw
firmware_unpacked_dir=${PWD}/firmware/$fw/$fw
firmware_mod_dir=${PWD}/firmware/out/$fw
sudo rm -rf $firmware_unpacked_dir
sudo rm -rf $firmware_unzipe_dir

sudo mkdir -p $firmware_unpacked_dir
sudo mkdir -p $firmware_unzipe_dir

#sudo rm -rf ${PWD}/$(basename -s .zip $1)

sudo unzip -o $filetounzip -d $firmware_unzipe_dir
cd $firmware_unzipe_dir
sudo ../../../kenwood_tool -u $(find $firmware_unzipe_dir -name 'TS-*.dat')
sudo unzip -o $(find $firmware_unzipe_dir -name 'ts*.zip') -d $firmware_unpacked_dir
sudo rm -f $(find $firmware_unzipe_dir -name 'ts*.zip')
cd $firmware_unpacked_dir
for a in $(ls -1 *tar.gz); do 
    sudo tar -xvzf $a; 
    touch ${a%%.*}
    sudo rm -f $a; 
done
if [ $model == "ts890" ]
then 
    if [ -f app/rootfs.bin ]
    then 
        mkdir rootfs
        mount -o loop app/rootfs.bin rootfs
        echo "DONT FORGET TO UMOUNT ${PWD}/rootfs before pack.sh execute at with cmd umount rootfs"
    fi  
fi
echo "unpacked firmware in $firmware_unpacked_dir"
