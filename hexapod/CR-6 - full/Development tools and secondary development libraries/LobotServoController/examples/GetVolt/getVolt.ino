#include <LobotServoController.h>

#define rxPin 10
#define txPin 11

SoftwareSerial mySerial(rxPin, txPin);
LobotServoController myse(mySerial);

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
}

void loop() {
  Serial.print("Volt:");
  int v;
  v = myse.getBatteryVolt(1000); //1000为超时时间，单位为毫秒，超时而没有接收到返回的时候函数会返回 -1（无符号数65535）
                                 //使用此方法会阻塞程序，直到获得返回或是超时，才会继续后面的程序
  Serial.print(v);
  Serial.println("mv");
  delay(500);
}
