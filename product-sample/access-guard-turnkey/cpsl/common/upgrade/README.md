# sample_upgrade

This sample will demonstrate the basic usage of upgrade lib.

# ModelName and SoftVer

upgrade lib will compare **ModelName** and **SoftVer** between `CVI_UPGRADE_DEV_INFO_S` and package header to decide if upgrade allow-able.

**ModelName** **MUST** be the same and **SoftVer** comparsion according `strcmp()` rule with a flag, which able to disable check.

Currently, `sample_upgrade` **DOES NOT** do version check; modify 3rd argument of `CVI_UPGRADE_CheckPkg()` to enable it.

`sample_upgrade` uses `$PROJECT_FULLNAME` and `$PROJECT_SOFTVER` environment variable as **ModelName** and **SoftVer**.

It's just for demonstration purpose; You can change to any rule you want.


Output is under `sample/upgrade/sample_upgrade`

## Execute

execute on soc board:

```
./sample_upgrade <pkg>
```
### P.S. SPI NAND require **nandwrite**/**flash_erase** under the same path for now.


## Additional information

For those who want to directly upgrade partition in use, please make sure those partitions are unmounted or read-only.

And all executables used during upgrade processes **MUST** be independent of those partitions, ex: static-link version under ramdisk.

Below is an  instrument example for directly upgrade on our board.


```
sync
# For CFG partition
umount /mnt/cfg/param/;umount /mnt/cfg/
# For rootfs
mount / -o ro,remount
# For overlayfs
mount /overlay/rwdata -o ro,remount
```

**Please choose properly steps for your own file system.**
