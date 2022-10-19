
tslib download link:


rm -rf tslib_install
rm tslib_install.tar.gz
mkdir tslib_install

./autogen.sh

32bit lib:

echo "ac_cv_func_malloc_0_nonnull=yes" >arm-linux-gnueabihf.cache
./configure --host=arm-linux-gnueabihf  --prefix=$PWD/tslib_install

make clean
make
make install
tar -czvf tslib_install.tar.gz  tslib_install


64bit lib:


