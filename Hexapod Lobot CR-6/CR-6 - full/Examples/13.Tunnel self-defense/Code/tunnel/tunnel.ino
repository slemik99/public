#include <SoftwareSerial.h>
#include <LobotServoController.h> /*servo controller secondary development header file*/

#define sound A0 //sound sensor
#define light A2 //photosensitive sensor
//there is a discrepancy between the number of the action group and the explanation video. Please refer to the code. You can also modify the number according to the actual situation
#define GO_WAR 10   //fighting action group, group number

//low posture
#define L_STANG     0   //attention
#define GO_FORWARD  1   /*forward action group number*/
#define GO_BACK     2   /*backward action group number*/
#define TURN_LEFT   3   /*turn left action group number*/
#define TURN_RIGHT  4   /*turn right action group number*/
//middle posture
#define M_STANG 25         //attention
#define M_GO_FORWARD  26   /*forward action group number*/
#define M_GO_BACK     27   /*backward action group number*/
#define M_TURN_LEFT   28   /*turn left action group number*/
#define M_TURN_RIGHT  28   /*turn right action group number*/

#define L_SHRINK      13   /*fear state*/

//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);   //instantiate the secondary development class

uint8_t readLight()
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
uint8_t readSound()
{
  uint16_t temp_sound = 0;
  temp_sound = analogRead(sound);
  Serial.println(temp_sound);
  if (temp_sound > 100) {
    return 1;
  }
  else
    return 0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //configure serial port 0,the baud rate is 9600
  MySerial.begin(9600); //configure serial port,the baud rate is 9600
  delay(500);           //delay 500ms, waiting for the servo controller be ready
  Controller.runActionGroup(25, 1);   //run high posture attention
  delay(2000);                        //delay 2000ms, ensure that action group finishes running
  Controller.stopActionGroup();       //stop all action groups 
}

void realize() {   //logical implementation
  static uint32_t  timer = 0;  //static variable is used for timing
  static uint8_t stept = 0;     //static variable is used for recording steps
  static bool shrink = true;    //prevent duplicates from being stuck in a standing state
  if (timer > millis())  //if the set time is greater than the current number of milliseconds, return, if it is less than the current number of milliseconds, continue
    return;

  if (!Controller.isRunning()) {  //No action group is running
    if (readLight())             //the intensity of light is less than the set value
    {
      if (readSound() && !shrink)
      {
        Controller.runActionGroup(GO_FORWARD, 5);//forward
        shrink = true;
        
      }
      else if (shrink)
      {
        Controller.runActionGroup(L_SHRINK, 1); //，
        shrink = false;
        timer=millis()+1100;
      }
    }
    else
    {
      Controller.runActionGroup(M_GO_FORWARD, 2);//forward
      shrink = true;
    }
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();  //Processing data received from the servo controller
  realize();                     //light-tracking logic
}
