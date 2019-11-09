#include <SoftwareSerial.h>
#include <LobotServoController.h>

//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9,8);             //soft definition serial port communication

LobotServoController Controller(MySerial);  //instantiate secondary development by soft serial communication

void setup() {
  // put your setup code here, to run once:
 MySerial.begin(9600);  //initialized the serial port, the baud rate is 9600
 delay(300);
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint8_t stept = 0;
   static uint32_t timer = 0;
  Controller.receiveHandle();     //receive processing function to retrieve data from the serial receive buffer
  if(timer>millis())
    return;
    
  if(!Controller.isRunning()){
    switch(stept){
      case 0: 
        Controller.moveServo(20,501,1000);//control the No.20 servo on the servo controller,from the range of 500-2500 map to 0-180°，the rotation time is 1000MS
        stept=1;
        timer=millis()+1000;
        break;
        
        case 1:
        Controller.moveServo(20,1000,1000);
        stept=2;
        timer=millis()+1000;
        break;

        case 2:
        Controller.moveServo(20,1500,1000);
        stept=3;
        timer=millis()+1000;
        break;

        case 3:
        Controller.moveServo(20,2000,1000);
        stept=4;
        timer=millis()+1000;
        break;

        case 4:
        Controller.moveServo(20,2499,1000);
        stept=0;
        timer=millis()+1000;
        break;

    }
  }
}
