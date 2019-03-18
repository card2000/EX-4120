#!/bin/sh

# ネットワークを停止する
ifdown eth0

# 固定IPからDHCPに変更する
sed -i -e "/^iface\ eth0\ inet\ static/s/static/dhcp/" /etc/network/interfaces
# IPアドレスの設定を削除する
sed -i -e "/^\taddress/d" /etc/network/interfaces
sed -i -e "/^\tnetmask/d" /etc/network/interfaces
sed -i -e "/^\tgateway/d" /etc/network/interfaces
sed -i -e "/^\tdns-nameservers/d" /etc/network/interfaces
sync 

# DNSサーバアドレスを初期化する
rm /var/run/resolv.conf
touch /var/run/resolv.conf

# ネットワーク情報をバックアップする 
cp /etc/network/interfaces /etc/network/interfaces.backup
sync 

# ネットワークを再開する
ifup eth0
