//The number of the action group in this program is different from that in the video, because the CR-6 only use the high posture to avoid obstacles. In the low posture, the front two legs will block the ultrasonic distance measurement.
//Therefore,each action group in this program is a high posture.The 25th is high posture attention,the 26th is high posture forward,the 27th is high posture backward, the 28th is high posture turns left, and the 29th is high posture turns right.
#include <SoftwareSerial.h>
#include <NewPing.h>         /*ultrasound library header file*/
#include <LobotServoController.h>  /*rudder controller secondary development library header file*/
#include "U8glib.h"    /*oled screen library header file*/

#define TRIG   6         /*the pin of ultrasonic TRIG  is NO.6 IO*/
#define ECHO   7         /*the pin of ultrasonic ECHO  is NO.7 IO*/
#define MAX_DISTANCE 200  /*the max detected distance is 200cm*/

#define GO_FORWARD  35     /*straight away action group*/
#define GO_BACK     36     /*backward action group*/
#define TURN_LEFT   37     /*turn left action group*/
#define TURN_RIGHT  38     /*turn right action group*/

#define MIN_DISTANCE_TURN 35 /*obstacle avoidance distance,it means that avoid obstacles when the distance is less than the specified distance*/
#define BIAS  7       /*servo deviation, adjust the size according to the actual situation so that the ultrasonic is facing forward*/
uint16_t Volt;    //battery voltage

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);   //instantiated OLED class
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);  //instantiated the secondary development class of the steering gear control board, using the soft serial port as the communication interface
NewPing Sonar(TRIG, ECHO, MAX_DISTANCE);      //instantiated the ultrasonic class


int gDistance;    //global variable is used for storing the distance measured by the ultrasonic in the middle position
int gLDistance;   //used for storing the distance measured of robot left
int gRDistance;   //used for storing the distance measured of robot right




void draw() {                 //OLED drawing, update
  static uint32_t TimerDraw;  //the static variable for timing
  if (TimerDraw > millis())   //return,if TimerDraw is less than the total number of milliseconds running,otherwise continue to run this function
    return;
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_7x14);            //set the font tou8g_font_unifont
    u8g.setPrintPos(0, 15);         //set the position to (0,15)
    u8g.print("U:");
    u8g.setPrintPos(35, 15);         //set the position to (35,10)
    u8g.print(Volt / 1000.0, 2);  //displayed voltage is
    u8g.print(" v");                         //dispalyed unit is "v"
    u8g.setPrintPos(0, 30);         //set the position is (0,30)
    u8g.print("S:");
    u8g.setPrintPos(35, 15 + 15);    //set the position is (35,30)
    u8g.print(gDistance);                     //displayed detected distance by ultrasonic 
    u8g.print(" cm");                         //displayed unit"cm"

  } while ( u8g.nextPage());
  TimerDraw = millis() + 500;                 //timerDraw is assigned the total number of milliseconds to plus 500, if it implementation to run again after 500 milliseconds.
}

bool ledON = true;            //led shining mark，true is turn on，false is turn off
void ledFlash() {
  static uint32_t Timer;      //define a static variable Timer for timing
  if (Timer > millis())       //timer is greater than  millis（）（the total number of milliseconds running）,return
    return;
  //Timer is less than the total number of milliseconds running,continue
  if (ledON) {
    digitalWrite(13, HIGH);   //if shining mark is true，NO.13 IO is high level,LED is light up on board 
    Timer = millis() + 20;    //timer = total number of milliseconds currently running + 20. if it implementation to run again after 20 milliseconds
    ledON = false;            //set shining mark is false
  } else {
    ledON = false;            //if shining mark is not true，set shining mark is false
    digitalWrite(13, LOW);    //NO.13 IO is low level，LED is off on board
  }
}
int getDistance() {       //get the distance
  unsigned long lEchoTime;     //variable is used to storage detected time of  pulse high level
  lEchoTime = Sonar.ping_median(5);           //detected 5 times of ultrasonic ,troubleshoot wrong result 
  int lDistance = Sonar.convert_cm(lEchoTime);// conversion time to centimeters that detected pulse high level
  return lDistance;                           //return the detected distance
}

void getAllDistance()  //get three direction distance of front,right and left
{
  int tDistance;       //used to detected distance that temporary storage
  Controller.moveServo(20, 1500 + BIAS, 200); //ultrasonic rotational station rotates to 90°is middle position 
  delay(200);                   //delay 200ms,wait for servo rotates to the position 
  gDistance = getDistance();    //measured distances and storaged to gDistance

  Controller.moveServo(20, 1944 + BIAS, 200); //ultrasonic rotational station rotates to 130°is 40°of robot left 
  delay(200);                   //delay,wait for servo rotates to the position
  tDistance = getDistance();    //measured distances and storaged to tDistance
  Controller.moveServo(20, 2450 + BIAS, 200); //rotates to 170°is 80°of robot left 
  delay(200);                   //delay,wait for servo rotates to the position
  gLDistance = getDistance();   //measured distances and storaged to gLDistance
  if (tDistance < gLDistance)   //compare two measured distances from left,take a smaller one and storage to gLDistance as the left distance 
    gLDistance = tDistance;

  Controller.moveServo(20, 1055 + BIAS, 200); //ultrasonic rotational station rotates to 50°is 40°of robot right
  delay(200);                   //delay,wait for servo rotates to the position
  tDistance = getDistance();    //measured distances and storaged to tDistance
  Controller.moveServo(20, 611 + BIAS, 200); //rotates to 10°is 80°of robot right
  delay(200);                   //delay,wait for servo rotates to the position
  gRDistance = getDistance();   //measured distances and storaged to gRDistance
  if (tDistance < gRDistance)   //compare two distance,take a smaller one and storage to gRDistance
    gRDistance = tDistance;

  Controller.moveServo(20, 1500 + BIAS, 200); //ultrasonic rotational station rotates to middle position
}

void sonar()  //Obstacle avoidance logic
{
  static uint32_t timer = 0;  //Static variable for timing
  static uint8_t step = 0;    //Static variable for recording steps
  if (timer > millis())  //If the set time is greater than the current number of milliseconds, then return, no side to continue the subsequent operations
    return;
  switch (step)  //According to the step branch
  {
    case 0:  //step 0
      gDistance = getDistance();   //measured distance and storaged to gDistance
      if (gDistance > MIN_DISTANCE_TURN || gDistance == 0) {  //if the measured distance is greater than the specified obstacle avoidance distance,forward
        if (!Controller.isRunning()) {
          Controller.runActionGroup(GO_FORWARD, 0);  //keep moving forward
          step = 1;  //Move to step 1
          timer = millis() + 300;   //delay 500ms
        }
      } else {  //if the measured distance is less than the specified distance
        step = 2;  //move to step 2
        timer = millis() + 200;  //delay 100ms
      }
      break;  //ending switch statement
    case 1:   //step 1
      gDistance = getDistance();   //measured distance
      if (gDistance < MIN_DISTANCE_TURN && gDistance > 0) {
        //if the measured distance is less than the specified obstacle avoidance distance,stop run all group actions, move to step 2 
        Controller.stopActionGroup();
        step = 2;
      }
      timer = millis() + 200;
      break; //ending switch statement
    case 2:
      if (!Controller.isRunning()) {
        getAllDistance();  //get three directions distance
        //        Serial.println(gDistance);  //Print measured distance
        //        Serial.println(gLDistance);
        //        Serial.println(gRDistance);
        step = 3;  //move to step 3
      } else {
        gDistance = getDistance();  //If the running action group is not running, then get the distance  of direction intermediate and delay 500ms
        timer = millis() + 500;
        break; //ending switch statement
      }
    case 3:
      static bool lastActionIsGoBack = false;   //Static variable , record whether the last action is backward
      if (((gDistance > MIN_DISTANCE_TURN) || (gDistance == 0)) && lastActionIsGoBack == false) {
        // if the intermediate distance are greater than the specified obstacle avoidance distance and the last action is not backward , then return to step 0，
        //here to determine whether the last action is back , is to avoid the program falling back - " forward - " back - " advancing " such an infinite loop
        //when the last step is to retreat , it will not perform the advancement
        step = 0;
        timer = millis() + 200;
        lastActionIsGoBack = false;
        break;
      }
      if ((((gLDistance >= gRDistance) && (gLDistance > MIN_DISTANCE_TURN)) || gLDistance == 0) && gDistance > 15) {
        //ultrasonic measurement shows that the minimum distance on the left side are greater than the minimum distance on the right side is greater than the specified obstacle avoidance distance , and the distance measured in the middle is greater than 15 
        //the purpose of detecting the distance from the middle is to prevent objects of being between the two front legs of the robot , resulting in the robot not being able to turn
        if (!Controller.isRunning()) {   //wait for the action group to finish running
          Controller.runActionGroup(TURN_LEFT, 10);  //turn left 10 times
          lastActionIsGoBack = false;  //identify the last action is not back
          step = 2;  //move to step 2
        }
        timer = millis() + 300;  //delay 500ms
        return;  //return, end function
      }
      if ((((gRDistance > gLDistance) && (gRDistance > MIN_DISTANCE_TURN)) || gRDistance == 0) && gDistance > 15) {
        //Ultrasonic measurement shows that the minimum distance on the left side are greater than the minimum distance on the right side is greater than the specified obstacle avoidance distance , and the distance measured in the middle is greater than 15 
        if (!Controller.isRunning()) {   //wait for the action group to finish running
          Controller.runActionGroup(TURN_RIGHT, 10);  //turn right 10 times
          lastActionIsGoBack = false;  //identify the last action is not backward
          step = 2;  //move to step 2
        }
        gLDistance = gRDistance = 0;
        timer = millis() + 300;   //delay 500ms
        return;    //return, end function
      }
      //when the previous ones are not met , all the returns are not executed
      //The program will execute here
      Controller.runActionGroup(GO_BACK, 3);  //perform the back action groups 3 times
      lastActionIsGoBack = true;  //identify the last action is backward
      step = 2;  //move to step 2
      timer = millis() + 300;  //delay 500ms
  }
}
void updateBatteryState() {      //update status of battery and voltage 
  static uint32_t Timer = 0;     //static variable Timer for timing
  uint16_t veg = 0;
  if (Timer > millis())          //if Timer less than the total number of milliseconds to run, return, otherwise continue to run this function
    return;
  veg = Controller.getBatteryVolt(10); //send the command of get battery and voltage 
  if (veg > 12000) {
    return;
  }
  else
    Volt = veg;
  Timer = millis() + 1000;       //Timer assignment is the total number of milliseconds running + 1000, running again after 1000 milliseconds
}
void setup() {
  // put your setup code here, to run once
  u8g.firstPage();                            //OLED draw Logo
  do {
    u8g.setFont(u8g_font_10x20);            //set font to u8g_font_unifont
    u8g.setPrintPos(40, 23);         //set position to  (0,15)
    u8g.print("LOBOT");

  } while ( u8g.nextPage());

  Serial.begin(9600);                        //Initialize serial port 0
  MySerial.begin(9600);                       //Initialize serial port 
  pinMode(13, OUTPUT);                        //set NO.13 LED IO is output port on board
  delay(500);                                 //delay 500ms，wait for the servo control to be ready
  Controller.runActionGroup(34, 1);           //run the No.34 action group, No.34 is a high posture , return to the initial position
  delay(2000);                                //delay two seconds, simple delay,to ensure that the 0 action group is running
  Controller.stopActionGroup();               //stop the action and make sure to stop
  Controller.moveServo(20, 1500 + BIAS, 200); //ultrasonic rotational station servos rotates to 90,that is the middle position
}


void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t Timer = 0;  //static variable for timing
  updateBatteryState();       //update status of battery and voltage
  Controller.receiveHandle(); //receive processing function to retrieve data from the serial receive buffer
  draw();                     //OLED draw，update
  sonar();                    //voidance
  ledFlash();                 //led shining is used to prompt for running status

}
