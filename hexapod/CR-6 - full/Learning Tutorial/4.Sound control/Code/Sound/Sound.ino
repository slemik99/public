#include <LobotServoController.h>
#include <SoftwareSerial.h>

#define SOUND_PIN  A0  /*the IO port of sound sensor*/

//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial); //instantiation secondary development

uint8_t readSound()
{
  uint16_t temp = 0;
  temp = analogRead(SOUND_PIN);
  if (temp > 400)
  {
    //    Serial.println(temp);
    return 1;
  }
  else
    return 0;
}


void sound()
{
  static uint32_t timer = 0;  //static variable be used to timing
  static uint32_t timer_run = 0;  //static variable be used to timing
  static uint8_t stept = 0;        //static variable be used to record step
  if (timer > millis())       //if
    return;
  if (!Controller.isRunning()) { //because the sound of the footsteps of the robot  may cause misjudgment,after the action is finished,delay for a short time to go to the next step.
    switch (stept)   //do branches according to the steps
    {
      case 0:       //step 0
        stept = 1;     //go to the next step1
        timer = millis() + 400;    //delayed 400ms
        break;     //ending switch
      case 1:
        if (readSound() == 1)   //read the status of the pin of sound sensor.If it is 1,there is a sound.
        {
          timer = millis() + 30;//delayed 30ms
          timer_run = timer + 500; //delayed 1000ms
          stept = 2;
          
        }
        break;   //ending switch statement

      case 2:
        if (readSound() == 1)   //read the status of the pin of sound sensor.If it is 1,there is a sound.
        {
          stept = 3;  //two sounds detected within 1000ms
        }
        if (timer_run < millis()) //determine if it exceeds 500ms
        {
          stept = 4;              //Only one sound in 1000ms
        }
        break;   //ending switch statement

      case 3:             //run the action group that detects two sounds
        stept = 0;        //re-enter sound detection
        Controller.runActionGroup(0, 1);
        break;

      case 4:             //run the action group that detects one sound
        stept = 0;        //re-enter sound detection
        Controller.runActionGroup(13, 1);
        break;
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  MySerial.begin(9600);  //Initialize the communication serial port, the baud rate is 9600
  Serial.begin(9600);
  pinMode(SOUND_PIN, INPUT);   //Configured the A0 IO port as the input. Connect the OUT pin of the sound sensor to the 11th IO.
  delay(1000);
  Controller.runActionGroup(0, 1); //run the 0 action group
  delay(300);
}

void loop() {
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();     //Receive processing function to retrieve data from the serial receive buffer
  sound();                        //Sound control implementation

}
