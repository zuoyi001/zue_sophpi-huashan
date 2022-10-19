#!/bin/sh

ssid=SW-test
psk=012345678

DEVICE=wlan0
BOOTPROTO=dhcp
IPADDR=192.168.1.5
NETMASK=255.255.255.0
BROADCAST=192.168.1.255
GATEWAY=192.168.1.1

echo 0 > /proc/net/rtl8723bs/log_level

if [ -e "/sys/class/net/wlan0" ]; then
    mkdir -p /run/network
    case "$1" in
    start)
        printf "Starting network:"
        /sbin/ifconfig wlan0 up
        echo "ctrl_interface=/tmp/wpa_supplicant" > /etc/network/wpa_supplicant.conf

        /usr/bin/wpa_passphrase $ssid "$psk" >> /etc/network/wpa_supplicant.conf
        /usr/bin/wpa_supplicant -iwlan0 -Dnl80211 -c /etc/network/wpa_supplicant.conf &
        if [ ${BOOTPROTO} = static ]; then
          /sbin/ifconfig wlan0 ${IPADDR} netmask ${NETMASK} up
          #echo "/sbin/ifconfig wlan0 $ip_wifi netmask $netmask_wifi up"
          /sbin/route add default gw ${GATEWAY}
          echo "nameserver 8.8.8.8" > /etc/resolv.conf
        else
          #echo "dhcp"
          /sbin/udhcpc -b -i wlan0 -R &
        fi
        [ $? = 0 ] && echo "OK" || echo "FAIL"
          ;;
    stop)
        printf "Stopping network: "
        /sbin/ifconfig wlan0 down
        [ $? = 0 ] && echo "OK" || echo "FAIL"
          ;;
    restart|reload)
        "$0" stop
        "$0" start
          ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
    esac
fi
exit $?