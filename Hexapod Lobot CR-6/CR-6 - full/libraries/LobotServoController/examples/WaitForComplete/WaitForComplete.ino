#include <LobotServoController.h>

//非阻塞的方法读取电压

#define rxPin 10
#define txPin 11

SoftwareSerial mySerial(rxPin, txPin);
LobotServoController myse(mySerial);

void setup() {
  pinMode(13,OUTPUT);
  mySerial.begin(9600);
  Serial.begin(9600);
}
void loop() {
  digitalWrite(13, HIGH);
  myse.runActionGroup(1,1);
  if(myse.waitForStop(10000))  //等待动作组执行完成或被强制停止，或是直到超时。参数为超时时间，单位为毫秒。动作组执行完成返回true，超时返回false
                               //此函数阻塞运行，即只有等到超时或是动作中执行完成后才会返回，才会执行后面的程序
  {
    Serial.println("FINISH");
    digitalWrite(13, LOW);
  }
  else
    Serial.println("NOT FINISH");
  delay(1000);
}
