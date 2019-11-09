#include <SoftwareSerial.h>
#include <LobotServoController.h> /*servo controller secondary development header file*/

#define right_IR_hand  4  //infrared on the right
#define left_IR_hand   2   //infrared on the left
//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);   //instantiating secondary development class


void setup() {
  // put your setup code here, to run once:
 //Serial.begin(9600);  //initialize serial port 0, baud rate 9600
  MySerial.begin(9600); //initialize serial port 0, baud rate 9600
  pinMode(right_IR_hand, INPUT);    
  pinMode(left_IR_hand, INPUT);   
  delay(500);           //delay 500ms waiting for the servo controller ready 
  Controller.runActionGroup(0, 1); //perform NO.0 action group
  delay(1000);    //delay 2ms waiting for the action group to complete
  Controller.stopActionGroup(); //stop all action groups
}

void Move() {  //main logic of the program
  static uint32_t timer = 0;  //static variable is used for timing
  static uint32_t timer_wait = 0;  //static variable is used for timing
  static int stept = 0;        //static variable is used for recording steps
  if (timer > millis())       //if 
    return;
  switch (stept) {  //according to step do a branch
    case 0:
      if (digitalRead(right_IR_hand) == LOW){
        timer_wait=millis()+1000;
        stept = 1;
      }
      else if (digitalRead(left_IR_hand) == LOW){
         stept = 2;
         timer_wait=millis()+1000;
      }
      timer = millis() + 50;
      break;

    case 1:
      if(digitalRead(left_IR_hand) == LOW){
        stept=0;
        Controller.runActionGroup(7, 2); //No.7 action group is left sliding
        
        
      }
      if(timer_wait<millis()){
        stept=0;
      }
      break;
      
    case 2:
      if(digitalRead(right_IR_hand) == LOW){
        stept=0;
        Controller.runActionGroup(8, 2); //No.8 action group is right sliding
      }
      if(timer_wait<millis()){
        stept=0;
      }
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();  ////receive processing function to retrieve data from the serial receive buffer
  Move(); //main processing
}
