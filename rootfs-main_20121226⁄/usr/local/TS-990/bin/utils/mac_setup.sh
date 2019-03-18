#!/bin/sh

# MACアドレスの設定が残っていれば、それを削除する
sed -i -e "/^\thwaddress/d" /etc/network/interfaces

# MACアドレス識別子を付与する
cat -n /tmp/mac.txt > /tmp/mac_temp.txt
sed -i -e "s/^\ \ \ \ \ 1/\thwaddress ether/" /tmp/mac_temp.txt
echo "" >> /tmp/mac_temp.txt

# MACアドレスを一時ファイルから読み込み、設定ファイルに書き込む
sed -i -e "`awk '/^iface\ eth0\ inet/ {print NR+1}' /etc/network/interfaces`ecat\ \/tmp\/mac_temp.txt" /etc/network/interfaces

sync

cp /etc/network/interfaces /etc/network/interfaces.backup
sync

rm /tmp/mac_temp.txt
