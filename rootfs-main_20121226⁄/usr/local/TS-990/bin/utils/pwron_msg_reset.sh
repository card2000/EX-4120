#!/bin/sh

rm /usr/local/TS-990/bin/screen/pwron.800x600.message.screen
cp /usr/local/TS-990/bin/screen/pwron.800x600.hello.screen /usr/local/TS-990/bin/screen/pwron.800x600.message.screen

rm /usr/local/TS-990/bin/screen/pwron.848x480.message.screen
cp /usr/local/TS-990/bin/screen/pwron.848x480.hello.screen /usr/local/TS-990/bin/screen/pwron.848x480.message.screen
 
rm /usr/local/TS-990/bin/screen/pwron.848x480.message.progbar.screen
cp /usr/local/TS-990/bin/screen/pwron.848x480.hello.progbar.screen /usr/local/TS-990/bin/screen/pwron.848x480.message.progbar.screen

rm /usr/local/TS-990/bin/screen/pwron.message.bmp
cp /usr/local/TS-990/bin/screen/pwron.800x480.hello.bmp /usr/local/TS-990/bin/screen/pwron.message.bmp

rmdir /usr/local/TS-990/bin/screen/SETUP
