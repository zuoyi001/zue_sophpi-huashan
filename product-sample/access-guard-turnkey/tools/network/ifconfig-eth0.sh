#!/bin/sh

DEVICE=eth0
BOOTPROTO=static
IPADDR=192.168.1.3
NETMASK=255.255.255.0
BROADCAST=192.168.1.255
GATEWAY=192.168.1.1

/sbin/ifconfig lo 127.0.0.1

if [ -e "/sys/class/net/eth0" ]; then
    mkdir -p /run/network
    case "$1" in
    start)
        printf "Starting network: "
        if [ ${BOOTPROTO} = static ];then
        start-stop-daemon -K -q -n udhcp 
        /sbin/ifconfig eth0 down
        /sbin/ifconfig eth0 ${IPADDR} netmask ${NETMASK} up
        /sbin/route add default gw ${GATEWAY}
        else
        echo "error !!"
        fi
        [ $? = 0 ] && echo "OK" || echo "FAIL"
        ;;
    stop)
        printf "Stopping network: "
        /sbin/ifdown -a
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