# packcimg<span>.py</span>

## Execute

execute on build server:

```
usage: packcimg.py [-h] -m pkg_model --ver version -i cimg -b fip.bin -o output_file [-v]

Pack FIP/CIMGs into one bin

optional arguments:
  -h, --help            show this help message and exit
  -m pkg_model, --model pkg_model
                        Package Model Name
  --ver version         Software version
  -i cimg, --input cimg
                        CIMG to pack
  -b fip.bin            fip.bin to pack
  -o output_file, --output_file output_file
                        the output file
  -v, --verbose         increase output verbosity
```

## Example

```python
packcimg.py -m $PROJECT_FULLNAME --ver 1.0.0.0 -o upgrade.bin \
-i out/boot.spinand \
-i out/system.spinand \
-i out/cfg.spinand \
-i out/rootfs.spinand
```
