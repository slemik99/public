#include <LobotServoController.h>
#include <SoftwareSerial.h>
#include <IRremote.h>
#include "Code.h"

const int RECV_PIN = 10;  //infrared receiver pin at NO.10 IO port 

IRrecv irrecv(RECV_PIN); //instantiate infrared remote receiver class
decode_results results; //instantiated infrared decoding result class
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);  //instantiate the secondary development class of the servo controller ,using the soft serial port as the communication interface

bool ledON = true;                      //led lighting marks,lights up when true, false extinguishes
void ledFlash() {
  static uint32_t Timer;                //static variable Timer is used for timing 
  if (Timer > millis())                 //Timer is greater than millis() (the total number of milliseconds run) , returned, //Timer is less than running the total number of milliseconds to continue running the following statement
    return;
  if (ledON) {                          //if light up flag is true , the NO.13 IO is setting high level , and the LED is on board
    digitalWrite(13, HIGH);             //the NO.13 IO is setting high level , and On-board LED is off
    ledON = false;                      //set light up flag is false
    Timer = millis() + 20;              //Timer = Total milliseconds currently running + 20,If implemented,run again after 20 milliseconds
  } else {
    digitalWrite(13, LOW);              //the NO.13 IO is setting low level,On-board LED is off
  }
}

//this function is based on the infrared key value to obtain the number corresponding to the key value. If it is not a number, it returns -1
int getValue(uint32_t value) {
  int Num;
  switch (value) {
    case R_ONE:
      Num = 1;
      break;
    case R_TWO:
      Num = 2;
      break;
    case R_THREE:
      Num = 3;
      break;
    case R_FOUR:
      Num = 4;
      break;
    case R_FIVE:
      Num = 5;
      break;
    case R_SIX:
      Num = 6;
      break;
    case R_SEVEN:
      Num = 7;
      break;
    case R_EIGHT:
      Num = 8;
      break;
    case R_NINE:
      Num = 9;
      break;
    case R_ZERO:
      Num = 0;
      break;
    default: Num = -1;
      break;
  }
  return Num;
}

void IR_Remote() {            //logic realization of infrared remote control
  static uint32_t Timer = 0;  //static variable Timer is used for timing 
  static uint32_t lastRes;    //the last  button key value 
  static uint8_t count;       //static variable is used for counting
  if (Timer > millis())       //Timer is greater than millis() (the total number of milliseconds run) , returned, //Timer is less than running the total number of milliseconds to continue running the following statement
    return;
  if (irrecv.decode(&results)) {
    ledON = true;
    switch (results.value) {
      case R_CH_D:    //if the remote control red CH- button is pressed
        Controller.stopActionGroup(); //stop to run action group
        irrecv.resume();
        return;     //exit this switch statement
    }
    int actionNum;   //eemporary, action group number
    if (results.value == 0xFFFFFFFF)
      actionNum = getValue(lastRes); //use the last valid button to get the corresponding number
    else {  //the read signal is not continuous identification, then it is the new button
      lastRes = results.value;    //save the key value of the new button
      actionNum = getValue(lastRes); //get the button number with the new key value
      if (actionNum >= 0)  //if it is greater than 0, the button is a number in 0-9
        Controller.stopActionGroup();  //Then stop the currently running action group
    }
    if (actionNum >= 0) {  //if it is greater than 0, the button is digital
      Controller.runActionGroup(actionNum, 1); //run actionNum action group 1 times
      Serial.println(actionNum);
    }
    count = 0; //Count is 0
    irrecv.resume(); //receive next one
  } else {  //no button press
    if (count++ > 2) {  //if the count value is greater than 100, it means that the infrared signal is not received for a long time, and the count +1
      lastRes = 0; //the overtime is not pressed, the last pressed button is cleared, and the count is cleared
      count = 0;  //Count is 0
    }
  }
  Timer = millis() + 80; //Timer add 80ms to the total number of milliseconds of operation, run again after 80ms
}

void setup()
{
  Serial.begin(9600);      //initialize serial port 0, the baud rate is 9600
  MySerial.begin(9600);     //initialize serial port, the baud rate is 9600
  pinMode(13, OUTPUT);      //set led NO.13 IO is output port on board
  delay(500);               //delay 500ms, waiting for the servo controller be ready
  irrecv.enableIRIn();      //infrared remote control receiver
  results.decode_type = NEC;//specify the infrared decoding protocol as NEC protocol
  Controller.runActionGroup(0, 1); //run the No. 0 action group and return to the initial position.
  delay(2000);                     //delay 2000ms, ensure that action group 0 finishes running
  Controller.stopActionGroup();    //stop the action group to run, guaranteed to stop
}

void loop() { //main loop
  Controller.receiveHandle(); //receive processing function to retrieve data from the receive buffer
  IR_Remote();                //Infrared receiving processing, running logic implementation
  ledFlash();                 //led shining is used for running status prompts
}
