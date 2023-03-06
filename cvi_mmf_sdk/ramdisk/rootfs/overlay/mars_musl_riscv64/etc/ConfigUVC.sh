#!/bin/sh

cd /tmp/usb/usb_gadget/cvitek/functions/uvc.usb0
mkdir control/header/h/
echo "0x0100" > control/header/h/bcdUVC
echo "48000000" > control/header/h/dwClockFrequency
ln -s control/header/h/ control/class/fs/
ln -s control/header/h/ control/class/ss/

echo 2304 > streaming_maxpacket

#MJPEG
mkdir streaming/mjpeg/m/
mkdir streaming/mjpeg/m/2160p/
echo "400000" > streaming/mjpeg/m/2160p/dwFrameInterval
echo "400000" > streaming/mjpeg/m/2160p/dwDefaultFrameInterval
echo "3981312000" > streaming/mjpeg/m/2160p/dwMaxBitRate
echo "16588800" > streaming/mjpeg/m/2160p/dwMaxVideoFrameBufferSize
echo "663552000" > streaming/mjpeg/m/2160p/dwMinBitRate
echo "2160" > streaming/mjpeg/m/2160p/wHeight
echo "3840" > streaming/mjpeg/m/2160p/wWidth

mkdir streaming/mjpeg/m/1080p/
echo "400000" > streaming/mjpeg/m/1080p/dwFrameInterval
echo "400000" > streaming/mjpeg/m/1080p/dwDefaultFrameInterval
echo "3981312000" > streaming/mjpeg/m/1080p/dwMaxBitRate
echo "16588800" > streaming/mjpeg/m/1080p/dwMaxVideoFrameBufferSize
echo "663552000" > streaming/mjpeg/m/1080p/dwMinBitRate
echo "1080" > streaming/mjpeg/m/1080p/wHeight
echo "1920" > streaming/mjpeg/m/1080p/wWidth

mkdir streaming/mjpeg/m/720p/
echo "400000" > streaming/mjpeg/m/720p/dwFrameInterval
echo "400000" > streaming/mjpeg/m/720p/dwDefaultFrameInterval
echo "3981312000" > streaming/mjpeg/m/720p/dwMaxBitRate
echo "16588800" > streaming/mjpeg/m/720p/dwMaxVideoFrameBufferSize
echo "663552000" > streaming/mjpeg/m/720p/dwMinBitRate
echo "720" > streaming/mjpeg/m/720p/wHeight
echo "1280" > streaming/mjpeg/m/720p/wWidth

mkdir streaming/mjpeg/m/360p/
echo "400000" > streaming/mjpeg/m/360p/dwFrameInterval
echo "400000" > streaming/mjpeg/m/360p/dwDefaultFrameInterval
echo "3981312000" > streaming/mjpeg/m/360p/dwMaxBitRate
echo "16588800" > streaming/mjpeg/m/360p/dwMaxVideoFrameBufferSize
echo "663552000" > streaming/mjpeg/m/360p/dwMinBitRate
echo "360" > streaming/mjpeg/m/360p/wHeight
echo "640" > streaming/mjpeg/m/360p/wWidth

#H264
mkdir streaming/framebased/fb/
mkdir streaming/framebased/fb/2160p/
echo "400000" > streaming/framebased/fb/2160p/dwFrameInterval
echo "400000" > streaming/framebased/fb/2160p/dwDefaultFrameInterval
echo "15360000" > streaming/framebased/fb/2160p/dwMaxBitRate
echo "15360000" > streaming/framebased/fb/2160p/dwMinBitRate
echo "2160" > streaming/framebased/fb/2160p/wHeight
echo "3840" > streaming/framebased/fb/2160p/wWidth

mkdir streaming/framebased/fb/1080p/
echo "400000" > streaming/framebased/fb/1080p/dwFrameInterval
echo "400000" > streaming/framebased/fb/1080p/dwDefaultFrameInterval
echo "15360000" > streaming/framebased/fb/1080p/dwMaxBitRate
echo "15360000" > streaming/framebased/fb/1080p/dwMinBitRate
echo "1080" > streaming/framebased/fb/1080p/wHeight
echo "1920" > streaming/framebased/fb/1080p/wWidth

mkdir streaming/framebased/fb/720p/
echo "400000" > streaming/framebased/fb/720p/dwFrameInterval
echo "400000" > streaming/framebased/fb/720p/dwDefaultFrameInterval
echo "15360000" > streaming/framebased/fb/720p/dwMaxBitRate
echo "15360000" > streaming/framebased/fb/720p/dwMinBitRate
echo "720" > streaming/framebased/fb/720p/wHeight
echo "1280" > streaming/framebased/fb/720p/wWidth

mkdir streaming/framebased/fb/360p/
echo "400000" > streaming/framebased/fb/360p/dwFrameInterval
echo "400000" > streaming/framebased/fb/360p/dwDefaultFrameInterval
echo "15360000" > streaming/framebased/fb/360p/dwMaxBitRate
echo "15360000" > streaming/framebased/fb/360p/dwMinBitRate
echo "360" > streaming/framebased/fb/360p/wHeight
echo "640" > streaming/framebased/fb/360p/wWidth

mkdir streaming/header/h/
ln -s streaming/mjpeg/m/ streaming/header/h/
ln -s streaming/framebased/fb/ streaming/header/h/

ln -s streaming/header/h/ streaming/class/fs/
ln -s streaming/header/h/ streaming/class/hs/
ln -s streaming/header/h/ streaming/class/ss/

#-Create and setup configuration
cd ../../
echo "0x01" > bDeviceProtocol
echo "0x02" > bDeviceSubClass
echo "0xEF" > bDeviceClass

