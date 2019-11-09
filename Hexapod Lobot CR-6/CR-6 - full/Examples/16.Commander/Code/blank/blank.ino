#include <LobotServoController.h> /*servo controller secondary development header file*/
#include <SoftwareSerial.h>
//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);   //instantiate the secondary development class 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //configure serial port 0, the baud rate is 9600
  MySerial.begin(9600); //configure serial port, the baud rate is 9600
  delay(500);           //delay 500ms, waiting for the servo controller be ready
  Controller.runActionGroup(0, 1);   //run high posture attention action group
  delay(1000);                        //delay 2000ms, ensure that action group finishes running
  Controller.stopActionGroup();       //stop all action groups
}

void loop() {
  // put your main code here, to run repeatedly:

}
