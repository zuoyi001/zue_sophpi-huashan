1. zbar-0.10 下载地址
   http://zbar.sourceforge.net/download.html
2. Build lib32/lib64 :
./configure --host=arm-linux-gnueabihf --prefix=$PWD/_install --enable-shared -enable-static --disable-video --without-x --without-xshm --without-xv --without-jpeg --without-imagemagick --without-gtk --without-python --without-qt
./configure --host=aarch64-linux-gnu --prefix=$PWD/_install --enable-shared -enable-static --disable-video --without-x --without-xshm --without-xv --without-jpeg --without-imagemagick --without-gtk --without-python --without-qt

"machine `aarch64' not recognized”的解决办法
wget -O config.guess 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
wget -O config.sub 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'

make clean
make 
make install