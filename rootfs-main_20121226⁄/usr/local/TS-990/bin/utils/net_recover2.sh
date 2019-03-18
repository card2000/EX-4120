#!/bin/sh

rm /etc/network/interfaces.backup
sync
cp /etc/network/interfaces /etc/network/interfaces.backup
sync


