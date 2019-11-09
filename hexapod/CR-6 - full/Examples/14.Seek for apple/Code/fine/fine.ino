
#include <Adafruit_APDS9960.h>

#include <SoftwareSerial.h>
#include <LobotServoController.h> /*servo controller secondary development header file*/
#include <NewPing.h>

#define GO_FORWARD  1     /*straight away action group*/


#define TRIG   6         /*the TRIG pin of ultraonic is NO.6 IO */
#define ECHO   7         /*the ECHO pin of ultraonic is NO.7 IO*/
#define MAX_DISTANCE 120  /*the maximum detection distance is 120cm*/

#define BIAS  7       /*servo deviation, adjust the size according to the actual situation so that the ultrasonic are facing forward*/

#define red 1     //color
#define green 2
#define blue 3

#define ALPHA  0.3 //weight

//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);   //instantiate the secondary development class 
NewPing Sonar(TRIG, ECHO, MAX_DISTANCE);      //instantiate the ultrasonic measured distance class 

Adafruit_APDS9960 apds;

int getDistance() {       //get distance 
  unsigned long lEchoTime;     //Variable is used for saving the time of detected pulse high level 
  lEchoTime = Sonar.ping_median(5);           //detected ultrasonic 5 times, troubleshoot the wrong result
  int lDistance = Sonar.convert_cm(lEchoTime);//transfer the time of detected pulse high level is cm 
  return lDistance;                           //return the detected distance
}


uint8_t color_logic(void) {
  //get the color value for data processing
  static uint16_t r, g, b, c;  //save the color of rgb
  uint16_t temp_r, temp_g, temp_b, temp_c;//temporary variable of rgb
  double PCT_r, PCT_b, PCT_g;              //percentage
  uint16_t mean;                          //the mean of rgb

  //wait for color data to be ready

  while (!apds.colorDataReady()) {
    delay(1);
  }
  //get the data and print the different channels
  apds.getColorData(&temp_r, &temp_g, &temp_b, &temp_c);
  
  //weight smothing
  r = r * (1 - ALPHA) + ALPHA * temp_r;
  g = g * (1 - ALPHA) + ALPHA * temp_g;
  b = b * (1 - ALPHA) + ALPHA * temp_b;
  c = c * (1 - ALPHA) + ALPHA * temp_c;

  mean = (r + b + g) / 3;//mean
  //proportion
  PCT_r = (double)r / mean;
  PCT_g = (double)g / mean;
  PCT_b = (double)b / mean;

  if (PCT_r > 1.40 ) { //judging red
    return red;
  }
  else if (PCT_g > 1.30 ) { //judging green
    return green;
  }
  else if (PCT_b > 1.30 ) { //judging blue
    return blue;
  }
  else
    return 0;
}


void follow(void) {
  uint16_t len = 0;
  len = getDistance();
  if (len > 15) {
    if (!Controller.isRunning()) {
      Controller.runActionGroup(GO_FORWARD, 1); //run low posture forward action group
      //Serial1.println("1");
    }
  }
}

void shake_Head()
{
  Controller.moveServo(20, 700 + BIAS, 200);
  delay(200);
  Controller.moveServo(20, 2300 + BIAS, 400);
  delay(400);
  Controller.moveServo(20, 700 + BIAS, 400);
  delay(400);
  Controller.moveServo(20, 1500 + BIAS, 200);
  delay(200);
}


void fine_Apple() {
  static uint32_t timer = 0;  //static variable timer is used for timing

  static bool Follower = false;//foller flag

  if (Follower) {
    follow();//turn on ultrasonic following
    return;
  }

  if (timer > millis())  //if the set times is greater than the current running milliseconds,then return,otherwise continue
    return;

  if (color_logic() == red)//determine if red is find, apple color
  {
    Controller.runActionGroup(0, 1);//low posture, attention
    Follower = true;
  }
  else if (color_logic() == green || color_logic() == blue) {
    //determine if the detected color is blue and green
    shake_Head();
  }

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //configure serial port 0, the baud rate of 9600
  MySerial.begin(9600); //configure serial port, the baud rate of 9600
  delay(500);           //delay 500ms, waiting for the servo controller be ready

  if (!apds.begin()) { //initialize sensor
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Device initialized!");
  //enable color sensign mode
  apds.enableColor(true);//make sensor can detect color

  Controller.runActionGroup(25, 1);   //run high posture ,attention
  Controller.moveServo(20, 1500 + BIAS, 200); //ultrasonic rotational station turn to that is, the middle position
  delay(2000);                        //delay 2000ms, ensure that action group finishes running
  Controller.stopActionGroup();       //stop to run all action groups
}


void loop() {
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();  //processing data received from the servo controller
  fine_Apple();
}
