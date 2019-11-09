#include <LobotServoController.h> /*servo controller secondary development header file*/
#include <SoftwareSerial.h>

#define RFR1 12   /*front right infrared sensor*/
#define RFL1 6   /*front left infrared sensor*/
#define RFR2 2   /*back right infrared sensor*/
#define RFL2 4   /*back left infrared sensor*/

#define GO_FORWARD  1   /*forward action group*/
#define GO_BACK     2   /*backward action group*/
#define TURN_LEFT   3   /*turn left action group*/
#define TURN_RIGHT  4   /*turn right action group*/

SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);  //instantiate the secondary development class of the servo controller, using the soft serial port as the communication interface

void body()   //main logic implementation of the program
{
  static uint32_t timer = 0;  //static variable is used for timing
  static uint8_t  stept = 0;   //static variable is used for recording steps
  static uint8_t  laststept = 0; //static variable,to record the last step of step
  if (timer > millis())  //return,if the set times is greater than the current running milliseconds
    return;
  switch (stept) {  //according to stept do switch
    case 0:        //step 0
      if (!Controller.isRunning()) {   //if not group action is running 
        if ((!digitalRead(RFR1)) && (!digitalRead(RFL1))) { //if the front left front right infrared sensor is not suspended
          Controller.runActionGroup(51, 1);    //the first step in running forward 
          stept = 1;                            //step,move to step 1
          timer = millis() + 300;              //delay 300ms
        } else {                               //if any one of the infrared sensors on the front left or front right does not detect the desktop
          laststept = 0;                        //the final step is assigned a value of 0
          stept = 3;                            //move to step 3
          timer = millis() + 200;              //delay 200ms
        }
      }
      break;   //turn off switch statement
    case 1:    //step 1
      if (!Controller.isRunning()) {   //if not action group is running 
        if ((!digitalRead(RFR1)) && (!digitalRead(RFL1))) {  //if the front right front right infrared sensor is not suspended
          Controller.runActionGroup(71, 1);    //the second step in running forward 
          stept = 0;                            //move to step 0
          timer = millis() + 300;              //delay 300 ms
        } else {                               //if any one of the infrared sensors on the front left or front right does not detect the desktop
          laststept = 1;                        //the final step is assigned a value of 1
          stept = 3;                            //move to step 3
          timer = millis() + 200;              //delay 200ms
        }
      }
      break;
    case 3:  //step 3
      Controller.stopActionGroup();      //send command of stop action group
      stept = 4;                          //move to step 4
      timer = millis() + 200;            //delay 200ms
      break;                             //turn off switch statement
    case 4:  //step 4
      if (laststept == 0) {                //if the last step is 0
        Controller.runActionGroup(81, 1); //run the NO.81 action group, NO.81  forward 1 action group is reverse action of NO.71 action group
      }
      if (laststept == 1) {                //if the last step is 1
        Controller.runActionGroup(61, 1); //run the NO.61 action group, NO.61  forward 2 action group is reverse action of NO.51 action group
      }
      timer = millis() + 300;     //delay 300ms
      stept = 5;                   //move to step 5
      break;                      //turn off switch statement
    case 5:
      if (!Controller.isRunning()) {  //if not action group is running 
        Controller.runActionGroup(2, 2); //run NO.2 action group 2 times, NO.2 is back action
         timer = millis() + 300;    //demo 600 milliseconds and then execute
        stept = 6;                  //move to step 6
      }
      break;                     //turn off switch statement
    case 6:
      if (!Controller.isRunning()) { //if not action group is running 
        if (laststept == 0) {       //if the last step is 0
          Controller.runActionGroup(4, 7); //run NO.4 action group 7 times, NO.4 is turn right action,can be modified as needed
        }
        if (laststept == 1) {      //if the last step is 1
          Controller.runActionGroup(3, 7);  //run NO.3 action group 7 times, NO.3 is turn left action
        }
        laststept = 0; //the last step resets 0
        stept = 0;     //step move back to 0
      }
      timer = millis() + 300; //delay 300ms
      break;          //turn off switch statement
  }
}

void setup()
{
  //Serial.begin(9600);      //Initialize serial port 0, the baud rate is 9600
  MySerial.begin(9600);     //Initialize serial soft port, the baud rate is 9600

  pinMode(13, OUTPUT);      //set led NO.13 IO is output port on board

  pinMode(RFR1, INPUT);     //set IO of front right infrared sensor is input port 
  pinMode(RFL1, INPUT);     //set IO of front left infrared sensor is input port  
  pinMode(RFR2, INPUT);     //set IO of back right infrared sensor is input port  
  pinMode(RFL2, INPUT);     //set IO of back left infrared sensor is input port   
  
  delay(500);               //delay 500ms, waiting for the servo controller be ready
  
  Controller.runActionGroup(0, 1); //run the No. 0 action group and return to the initial position
  delay(2000);                     //delay 2000ms, ensure that action group 0 finishes running
  Controller.stopActionGroup();    //stop the action group to run, guaranteed to stop
}

void loop() { //main loop 
  Controller.receiveHandle(); //receive processing function to retrieve data from the receive buffer
  body();    //subject logic
}
