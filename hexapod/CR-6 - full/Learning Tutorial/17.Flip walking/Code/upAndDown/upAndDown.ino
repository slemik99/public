#include <I2Cdev.h>
#include <MPU6050.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LobotServoController.h>


//the action group number corresponding to each action group is modified according to the actual situation

#define L_STAND       0     //low posture,attention
#define L_GO_FORWARD  1     //low posture,forward
#define L_GO_BACK     2     //low posture,backward

#define U_STAND       90     //anti-posture,attention
#define U_GO_FORWARD  91     //anti-posture,forward
#define U_GO_BACK     92     //anti-posture,backward

#define weight 0.1

SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);  //instantiate the secondary development class of the servo controller ,using the soft serial port as the communication interface 

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
float ax0, ay0, az0;
float gx0, gy0, gz0;
float ax1, ay1, az1;
float gx1, gy1, gz1;

int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;

float radianY;
float radianY_last; //get Y-axis inclination,finally 
//update the tilt sensor data
void update_mpu6050()
{
  static uint32_t timer_u;
  if (timer_u < millis())
  {
    // put your main code here, to run repeatedly:
    timer_u = millis() + 20;
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    ax0 = ((float)(ax)) * 0.3 + ax0 * 0.7;  //do a filtering for reading value
    ay0 = ((float)(ay)) * 0.3 + ay0 * 0.7;
    az0 = ((float)(az)) * 0.3 + az0 * 0.7;

    ax1 = (ax0 - ax_offset) /  8192.0;  // calibrated,，and converted to a multiple of the gravitational acceleration
    ay1 = (ay0 - ay_offset) /  8192.0;
    az1 = (az0 - az_offset) /  8192.0;

    gx0 = ((float)(gx)) * 0.3 + gx0 * 0.7;  //do a filtering for reading angular velocity
    gy0 = ((float)(gy)) * 0.3 + gy0 * 0.7;
    gz0 = ((float)(gz)) * 0.3 + gz0 * 0.7;
    gx1 = (gx0 - gx_offset);  //calibrated angular velocity
    gy1 = (gy0 - gy_offset);
    gz1 = (gz0 - gz_offset);


    //    //complementary calculation of x-axis inclination
    //    radianX = atan2(ay1, az1);
    //    radianX = radianX * 180.0 / 3.1415926;
    //    float radian_temp = (float)(gx1) / 16.4 * 0.02;
    //    radianX_last = 0.8 * (radianX_last + radian_temp) + (-radianX) * 0.2;

    //complementary calculation of y-axis inclination
    radianY = atan2(ax1, az1);
    radianY = radianY * 180.0 / 3.1415926;
    float radian_temp = (float)(gy1) / 16.4 * 0.01;
    radianY_last = 0.8 * (radianY_last + radian_temp) + (-radianY) * 0.2;
  }
  //  Serial.print(", GY:"); Serial.println(radianY_last);
  //  delay(500);
}




void upDown()
{
  static uint32_t timer = 0;//static variable timer is used for timing
  static uint8_t  stept = 0;   //static variable is used for recording steps
  update_mpu6050();


  if (timer > millis())  //if the set times is greater than the current running milliseconds, the return otherwise continue
    return;
  if (!Controller.isRunning()) {  //if not action group is running
    switch (stept)  //according to step do branch
    {
      case 0:
        if (radianY_last > 140 || radianY_last < -140) {
          timer = millis() + 500;
          stept = 1;
        }
        else
          Controller.runActionGroup(L_GO_FORWARD, 1);//forward
        break;

      case 1:
        if (radianY_last > 140 || radianY_last < -140) {
          Controller.runActionGroup(U_STAND, 1);//anti-attention
          timer = millis() + 1500;
          stept = 2;
        }
        else
          stept = 0;
        break;

      case 2:
        if (radianY_last < 40 && radianY_last > -40) {
          //up = true;
          timer = millis() + 500;
          stept = 3;
        }
        else
          Controller.runActionGroup(U_GO_FORWARD, 1);//anti-forward
        break;

      case 3:
        if (radianY_last < 40 && radianY_last > -40){
          Controller.runActionGroup(L_STAND, 1);//attention
          timer = millis() + 1500;
          stept = 0;
        }
        else
          stept = 2;
        break;
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //configure serial port 0, the baud rate is 9600
  MySerial.begin(9600);  //initialize serial port, the baud rate is 9600

  //MPU6050 configure
  Wire.begin();
  accelgyro.initialize();
  accelgyro.setFullScaleGyroRange(3); //Set angular velocity range
  accelgyro.setFullScaleAccelRange(1); //Set acceleration range
  delay(200);
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);  //get current all of axis data to calibrate
  ax_offset = ax;  //the calibration data of x-axis acceleration 
  ay_offset = ay;  //the calibration data of y-axis acceleration 
  az_offset = az - 8192;  //the calibration data of z-axis acceleration 
  gx_offset = gx; //the calibration data of x-axis angular velocity
  gy_offset = gy; //the calibration data of y-axis angular velocity
  gz_offset = gz; //the calibration data of z-axis angular velocity

  delay(200);  //delay 200ms, waiting for the servo controller be ready
  Controller.runActionGroup(L_STAND, 1); //run low posture attention action group
  Controller.moveServo(20, 1500, 1000);
  delay(1000);  //delay 1000ms, ensure that action group finishes running
  Controller.stopActionGroup(); //stop the action groups to run
}

void loop() { //main loop
  // put your main code here, to run repeatedly:
  Controller.receiveHandle();  //processing data that received from the servo controller
  upDown();  //logic implementation
}
