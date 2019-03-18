#!/bin/sh

ifconfig eth0 | grep "HWaddr" > /tmp/set_mac.txt
