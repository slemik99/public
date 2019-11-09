使用软件串口或是硬件串口与舵机控制板通信，即是 SoftwareSerial 或是 HardwareSerial
接线：舵机控制板的Tx接Arduino的Rx，舵机控制板的Rx接Arduino的Tx，舵机控制板的GND接Arduino的GND
      若使用软件串口通讯则也是相应的接到队友的 tx rx引脚

使用库前请初始化串口为波特率9600  不支持其他波特率
实例化LobotServoController类后调用，必须显式的传入对应的串口
对于Arduino Leonardo等 atmega32u4的板子上的rx，tx对应的Serial1， usb口是Serail

具体使用方法请打开arduino 开发环境 -》菜单栏-》文件-》示例-》LobotServoController-》对应的示例
可以查看使用示例


电池电压单位为毫伏mv
