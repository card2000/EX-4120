#!/bin/sh

cat /sys/bus/platform/devices/davinci_emac/net/eth0/operstate > /tmp/link.log
ifconfig eth0 | grep "inet addr" > /tmp/given_ip.log
