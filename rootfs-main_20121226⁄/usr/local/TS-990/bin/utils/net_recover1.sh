#!/bin/sh

rm /etc/network/interfaces
sync
cp /etc/network/interfaces.backup /etc/network/interfaces
sync


