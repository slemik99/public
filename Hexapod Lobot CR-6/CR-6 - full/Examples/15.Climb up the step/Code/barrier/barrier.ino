#include <SoftwareSerial.h>
#include <LobotServoController.h>
#include <NewPing.h>

//the action group number corresponding to each action group is modified according to the actual situation
#define H_STAND       34    //high posture
#define M_STAND       25    //middle posture
#define L_STAND       0     //low posture,attention

#define H_GO_FORWARD  35    //high posture
#define M_GO_FORWARD  26    //middle posture
#define L_GO_FORWARD  1     //low posture,forward

#define H_GO_BACK     26    //high posture
#define M_GO_BACK     27    //middle posture
#define L_GO_BACK     2     //low posture,backward

#define H_TURN_LEFT   37    //high posture,turn left
#define H_TURN_RIGHT  38    //high posture,turn right 

#define M_MOVE_LEFT   7   //left slide
#define M_MOVE_RIGHT  8   //right slide

#define TRIG    6         /*the TRIG pin of ultraonic is NO.6 IO*/
#define ECHO    7         /*the ECHO pin of ultraonic is NO.7 IO*/
#define MAX_DISTANCE 150   /*the maximum detection distance is 120cm*/

#define BARRIER1 65  //climb a step
#define BARRIER2 66
#define BARRIER3 67
#define BARRIER4 68

#define MIN_DISTANCE 20

SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);  //instantiate the secondary development class of the servo controller ,using the soft serial port as the communication interface 
NewPing Sonar(TRIG, ECHO, MAX_DISTANCE);      //instantiate the ultrasonic measured distance class

int getDistance() {                    //get distance
  uint16_t lEchoTime;                  //variable is used for saving the time of detected pulse high level
  lEchoTime = Sonar.ping_median(6);    //detected ultrasonic 6 times, troubleshoot the wrong result
  int lDistance = Sonar.convert_cm(lEchoTime);  //transfer the time of detected pulse high level is cm 
  return lDistance;                    //return the detected distance
}

void pass_Barrier(void)
{
  static uint32_t timer = 0;  //static variable timer is used for timing
  static uint8_t  stept = 0;   //static variable is used for recording steps
  static uint8_t  stept2 = 0;   //static variable is used for recording steps
  int distance;
  if (timer > millis())  //if the set times is greater than the current running milliseconds, the return otherwise continue
    return;
  if (!Controller.isRunning()) {  //if not action group is running
    distance = getDistance(); //get distance
    switch (stept)  //according to step do branch
    {
      case 0:
        if (distance <= 14 &&distance>0) //determine if the distance is less than 13 cm
        {
          stept = 1;
          timer = millis() + 500;
          Controller.runActionGroup(H_STAND, 1); //run high posture forward action group 
        }
        else
        {
          Controller.runActionGroup(L_GO_FORWARD, 1); //run low posture forward action group
        }
        break;
      case 1:
        switch (stept2) {
          case 0:
            Controller.runActionGroup(BARRIER1, 1); //run climbing ladder NO.1 action group
            stept2 = 1;
            break;

          case 1:
            Controller.runActionGroup(BARRIER2, 2); //run climbing ladder NO.2 action group
            stept2 = 2;
            break;

          case 2:
            Controller.runActionGroup(BARRIER3, 1); //run climbing ladder NO.3 action group
            stept2 = 3;
            break;

          case 3:
            Controller.runActionGroup(BARRIER4, 1); //run climbing ladder NO.4 action group
            stept2 = 0;
            stept = 0;
            break;
        }
        break;
    }
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //configure serial port 0, the baud rate is 9600
  MySerial.begin(9600);  //initialize serial port, the baud rate is 9600
  delay(500);  //delay 500ms, waiting for the servo controller be ready
  Controller.runActionGroup(L_STAND, 1); //run low posture attention action group
  Controller.moveServo(20, 1500, 1000);
  delay(2000);  //delay 2000ms, ensure that action group finishes running
  Controller.stopActionGroup(); //stop the action groups to run
}

void loop() { //main loop
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();  //processing data received from the servo controller
  pass_Barrier();  //logic implementation
}
