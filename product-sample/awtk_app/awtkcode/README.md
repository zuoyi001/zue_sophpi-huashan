# AWTK = Toolkit AnyWhere
# git clone source:
1.awtk:
source:https://github.com/zlgopen/awtk.git
inital clone from:
branch:remotes/origin/master
commit:dc9fd7fcca4cb1e376158f2747e3481510a1b0ce
2.awtk-linux-fb:
source:https://github.com/zlgopen/awtk-linux-fb.git
initial clone from:
branch:remote/origin/master
commit:a0a64d0138730415667ee8424dca030db938990a

# HOW TO BUILD FOR TARGET CVI BOARD
   cd awtk-linux-fb
   export SDK_PATH=the root path of SDK(ex./home/garry.xin/GWei)
   scons -j3
The target lib is located in awtk-linux-fb/build/lib

# HOW TO BUILD FOR TARGET LINUX(X86_64bit)
   cd awtk
   scons -j3
The target lib is located in awtk/lib
