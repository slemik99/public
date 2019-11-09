#include <SoftwareSerial.h>
#include <LobotServoController.h> /*servo controller secondary development header file*/

#define light A2
//there is a discrepancy between the number of the action group and the explanation video. Please refer to the code. You can also modify the number according to the actual situation
#define GO_WAR 10   //fighting action group, group number


//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);   //instantiate the secondary development class


uint8_t readLight()//returns 1 when the light is dark , and returns 0 when the light is bright
{
  uint16_t temp_light = 0;
  temp_light = analogRead(light);
  //Serial.println(temp_light);
  if (temp_light > 900) {
    return 1;
  }
  else
    return 0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //configure serial port 0, the baud rate of 9600
  MySerial.begin(9600); //configure serial port, the baud rate of 9600
  pinMode(light, INPUT);  //configure the IO of the touch sensor as input port

  delay(500);           //delay 500ms, waiting for the servo controller be ready
  Controller.runActionGroup(25, 1);   //run high posture ,attention
  delay(2000);                        //delay 2000ms, ensure that action group finishes running
  Controller.stopActionGroup();       //stop to run all the action groups
}

void realize() {   //logical implementation
  static uint32_t  timer = 0;  //static variable is used for timing
  static uint8_t stept = 0;     //static variable is used for recording steps
  if (timer > millis())  //return,if the set times is greater than the current running milliseconds.if it is less than the current number of milliseconds, continue
    return;

  if (!Controller.isRunning()) {  //No action group is running
    if (readLight()==1) {    //Detecting that the touch sensor is low level, that is, detecting a touch
      Controller.runActionGroup(GO_WAR, 1); //fighting,
      timer = millis() + 1000;
    }
  }
  timer = millis() + 200;
}
void loop() {
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();  //Processing data received from the servo controller
  realize();                     //light-tracking logic
}
