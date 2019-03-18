#!/bin/sh

# ネットワークを停止する
ifdown eth0


# DHCPから固定IPに変更する
sed -i -e "/^iface\ eth0\ inet\ dhcp/s/dhcp/static/" /etc/network/interfaces

# IPアドレスの設定が残っていれば、それを削除する
sed -i -e "/^\taddress/d" /etc/network/interfaces
sed -i -e "/^\tnetmask/d" /etc/network/interfaces
sed -i -e "/^\tgateway/d" /etc/network/interfaces
sed -i -e "/^\tdns-nameservers/d" /etc/network/interfaces

# IPアドレス識別子を付与する
cat -n /tmp/ip.txt > /tmp/ip_temp.txt
sed -i -e "s/^\ \ \ \ \ 1/\taddress/" /tmp/ip_temp.txt
sed -i -e "s/^\ \ \ \ \ 2/\tnetmask/" /tmp/ip_temp.txt
sed -i -e "s/^\ \ \ \ \ 3/\tgateway/" /tmp/ip_temp.txt
sed -i -e "s/^\ \ \ \ \ 4/\tdns-nameservers/" /tmp/ip_temp.txt
sed -i -e "/^\ \ \ \ \ 5/d" /tmp/ip_temp.txt

# 未設定の行を削除する 
sed -i -e "/^\tgateway\t---.---.---.---/d" /tmp/ip_temp.txt
sed -i -e "/^\tdns-nameservers\t---.---.---.---/d" /tmp/ip_temp.txt

# IPアドレスを一時ファイルから読み込み、設定ファイルに書き込む
sed -i -e "`awk '/^iface\ eth0\ inet/ {print NR+1}' /etc/network/interfaces`ecat\ \/tmp\/ip_temp.txt" /etc/network/interfaces

sync

# DNSサーバアドレスを初期化する
rm /var/run/resolv.conf
touch /var/run/resolv.conf

rm /tmp/ip_temp.txt

# ネットワーク情報をバックアップする 
cp /etc/network/interfaces /etc/network/interfaces.backup
sync

# DNSサーバを設定する
/usr/local/TS-990/bin/utils/setup_dns

# ネットワークを再開する
ifup eth0
