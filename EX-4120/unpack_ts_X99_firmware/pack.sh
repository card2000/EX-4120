#!/bin/sh

# use : with sudo or as root : ./unpack.sh TS-990_V122.zip
echo " " 
echo " "
echo "use : with sudo or as root : ./pack.sh TS-990_V122"
echo " "
echo " "

if [[ $# -eq 0 ]] ; then
    echo 'no firmware given...'
    exit 1
fi

mod=${1%_*}
model=$(echo ${mod//-} |tr '[:upper:]' '[:lower:]')
ver=${1##*_}
version=${ver:0:-2}.${ver#${ver:0:-2}}

echo $version
echo $model


if [ ! -f ${PWD}/kenwood_tool ]
then 
    make
    test -x ${PWD}/kenwood_tool || exit 0
fi

tooldir=${PWD}
firmware_unzipe_dir=${PWD}/firmware/tmp/$1
firmware_unpacked_dir=${PWD}/firmware/$1/$1
firmware_mod_dir=${PWD}/firmware/out/$1
mkdir -p ${PWD}/firmware/out/$1

if [ -d ${PWD}/firmware/$1/$1 ] 
then
    cd $firmware_unpacked_dir
    rm -rf *.tar.gz #maybee we modify code
    for a in $(ls -d app_*/); do 
        out=$(echo ${a%%/} | cut -c1-5)"_*"
        file=$(echo $out)
        sudo tar cfvz $file.tar.gz $a; 
    done
    sudo cp -rf *.tar.gz $firmware_mod_dir
    sudo cp -rf *.mot    $firmware_mod_dir
    cd $firmware_mod_dir
    sudo zip -r ../$model.zip *
    sudo rm rf *
    cd ../
    sudo ../../kenwood_tool -v $version -p $model.zip
    sudo rm $model.zip
    sudo cp $firmware_unzipe_dir/Readme.txt $firmware_mod_dir
    sudo mv $1.dat $firmware_mod_dir
    cd $firmware_mod_dir 
    
    sudo zip -r ../$1.zip *
    sudo rm *
    sudo mv ../$1.zip $firmware_mod_dir
fi
echo "packed firmware in $firmware_mod_dir"

