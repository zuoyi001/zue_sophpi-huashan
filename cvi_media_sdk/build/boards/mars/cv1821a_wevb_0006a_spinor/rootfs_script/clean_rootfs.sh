#!/bin/bash

SYSTEM_DIR=$1
rm -rf $SYSTEM_DIR/mnt/system/usr
rm -rf $SYSTEM_DIR/mnt/system/lib

rm -rf $SYSTEM_DIR/etc/init.d/S23ntp
rm -rf $SYSTEM_DIR/bin/ntpd

du -sh $SYSTEM_DIR/* |sort -rh
