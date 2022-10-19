本文件为atsha204a驱动的说明文件(采用芯片授权方案需要关注该部分内容)

1. 驱动实现则要保证hal层接口不要变。
2. 驱动代码当中需要根据实际电路设计修改i2c总线编号。例如芯片挂在i2c总线2，那么修改方法如下：
   方法一：驱动编译成模块，插入内核时传入参数，例如 insmod tmel_drive.ko adapter_id=2
   方法二：修改驱动源码，static int adapter_id = 2; ，然后直接编译进内核
3. test中的2个测试函数，一个为读取sn号，获取到的sn号需要跟CryptoAuth-ATSHA204A-Datasheet.pdf中第10页<2.1.2 Configration Zone>的Table 2-2中SN比对下，看驱动能否跟204a通讯成功。
4. 如果问题请及时联系。
