#include <SoftwareSerial.h>
#include <LobotServoController.h>
#include <NewPing.h>

//action group corresponding number is modified accroding to the actual situation
#define H_STAND       34    //high posture
#define M_STAND       25    //middle posture
#define L_STAND       0     //low posture，attention

#define H_GO_FORWARD  35    //high posture
#define M_GO_FORWARD  26    //middle posture,
#define L_GO_FORWARD  1     //low posture，forward

#define H_GO_BACK     26    //high posture
#define M_GO_BACK     27    //middle posture
#define L_GO_BACK     2     //low posture，backward

#define H_TURN_LEFT   37    //high posture, turn left
#define H_TURN_RIGHT  38    //high posture,turn right

#define M_MOVE_LEFT   7   //Left slide
#define M_MOVE_RIGHT  8   //right slide

#define TRIG    6         /*the pin of ultrasonic TRIG  is NO.6 IO*/
#define ECHO    7         /*the pin of ultrasonic ECHO  is NO.7 IO*/
#define MAX_DISTANCE 150   /*The maximum detection distance is 150cm*/

#define MIN_DISTANCE 20

SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);  //instantiating secondary development and use a soft serial port as the communication interface
NewPing Sonar(TRIG, ECHO, MAX_DISTANCE);      //instantating ultrasonic distance measurement

int getDistance() {                    //get the distance
  uint16_t lEchoTime;                  //variable is used to storage time that detected by pulse high level
  lEchoTime = Sonar.ping_median(6);    //detected 6 times of ultrasonic,troubleshoot the wrong result
  int lDistance = Sonar.convert_cm(lEchoTime);  //Conversion the time of pulse high level that be detected to cm
  return lDistance;                    //return the detected distance
}

void cf()
{
  static uint32_t timer = 0;  //Defining static variables timer is used for timing
  static uint8_t  stept = 0;   //Static variable is uesd for recording steps
  int distance;  
  if (timer > millis())  //if the set time is greater than the current number of milliseconds, then return, otherwise continue
    return;
  distance = getDistance(); //get the distance
  switch (stept)  //do the branch according to the step 
  {
    case 0:  //step 0
      if (distance > MIN_DISTANCE || distance == 0) {  //if the distance is greater than the specified distance
        Controller.runActionGroup(H_GO_FORWARD, 0); //run high posture forward action group all the time
      }
      stept = 1;  //transfer to step 1
      timer = millis() + 200;  //delay 200ms
      break; //turn off switch statement
    case 1:
      if (distance < MIN_DISTANCE  && distance > 0) {  //if the distance is less than the specified distance
        Controller.stopActionGroup();  //stop the running action group
        stept = 2; //transfer to step 2
      }
      timer = millis() + 200;  //delay 200ms
      break;  //turn off switch statement
    case 2:   //step 2
      if (!Controller.isRunning()) {  //if there is no action group running, it means wait for the action group to finish running
        Controller.runActionGroup(L_STAND, 1); //run the low posture attention action group
        stept = 3; //transfer to step 3
      }  
      timer = millis() + 200; //delay 200ms
      break;  //turn off switch statement
    case 3: //step 3
      if (!Controller.isRunning()) {  //if there is no action group running，
        distance = getDistance();   //get current distance
        if (distance > MIN_DISTANCE  || distance == 0) { //if the distance is greater than the specified distance
          Controller.runActionGroup(L_GO_FORWARD, 8);    //performing low posture forward action group
          stept = 6;  //transfer to step 6
        } else {     //if the distance is less than the specified distance,transfer to step 8
          stept = 8;
          //The distance measured by the low posture is still less than the specified distance, so it is impossible to pass the obstacle in the low posture
          //So try to move the side,whether it can pass,step 8 is used for side shifting
        }
      }
      timer = millis() + 200; //delay 200ms
      break;  //turn off switch statement
    case 6: //step 6
      if (!Controller.isRunning()) { //if there is no action group running, it means wait for the action group to finish running
        Controller.runActionGroup(H_STAND, 1);  //run high posture attention action group
        stept =  0;   //transfer to step 0
      }
      timer = millis() + 200; //delay 200ms
      break; //turn off switch statement
    case 8:
      if (!Controller.isRunning()) {   //waiting for the action group that is already running to finish running
        Controller.runActionGroup(M_MOVE_LEFT, 20);  //run the action group of  high posture and move to left
        stept = 0; //transfer to step 0
      }
      timer = millis() + 200; //delay 200ms
      break; //turn off switch statement
    default:
      break;
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //Configure serial port 0, the baud rate is 9600.
   MySerial.begin(9600);  //Initialize the serial port, the baud rate is 9600
  delay(500);  //delay 500ms, waiting for the servo controller ready
  Controller.runActionGroup(H_STAND, 1); //run high posture attention action group
  delay(2000);  //delay 2000ms,wait for the action group to finish running
  Controller.stopActionGroup(); //Stop all action groups that is running
}

void loop() { //main loop
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();  //receive processing for processing data received from the servo controller
  cf();  //logical realization of crossing the fire line
}
