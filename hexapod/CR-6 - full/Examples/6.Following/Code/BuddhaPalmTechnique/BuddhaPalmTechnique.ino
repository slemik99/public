
/******************************************************
   Company:       乐幻索尔
  //作者:深圳市乐幻索尔科技有限公司
  //我们的店铺:lobot-zone.taobao.com

  @@特别说明：控制板的通信口只支持9600波特率@@
 *****************************************************/
#include <SoftwareSerial.h>
#include <LobotServoController.h> /*舵机控制板二次开发头文件*/
#include <NewPing.h>

#define TRIG   6         /*超声波TRIG引脚为 6号IO*/
#define ECHO   7         /*超声波ECHO引脚为 7号IO*/
#define MAX_DISTANCE 120  /*最大检测距离为120cm*/

#define M_STAND     25     //中姿态
#define GO_FORWARD  26     /*直走的动作组*/
#define GO_BACK     27     /*后退动作组*/
#define TURN_LEFT   28     /*左转动作组*/
#define TURN_RIGHT  29     /*右转动作组*/

#define BIAS  7       /*舵机偏差,根据实际情况调整大小以使超声波朝向正前方*/

//SoftwareSerial mySerial(rxPin, txPin)
SoftwareSerial MySerial(9, 8);
LobotServoController Controller(MySerial);   //实例化二次开发类
NewPing Sonar(TRIG, ECHO, MAX_DISTANCE);      //实例化超声波测距类

int gDistance;    //全局变量，用于存储中间位置超声波测得的距离
int gLDistance;   //用于存储机器人左侧测得的距离
int gRDistance;   //用于存储机器人右侧测得的距离


int getDistance() {       //获得距离
  unsigned long lEchoTime;     //变量 ，用于保存检测到的脉冲高电平时间
  lEchoTime = Sonar.ping_median(5);           //检测5次超声波，排除错误的结果
  int lDistance = Sonar.convert_cm(lEchoTime);//转换检测到的脉冲高电平时间为厘米
  return lDistance;                           //返回检测到的距离
}

void getAllDistance()  //获得前及左右三个方向的距离
{
  int tDistance;       //用于暂存测得距离
  Controller.moveServo(20, 1500 + BIAS, 200); //超声波云台舵机转到90度即中间位置
  delay(500);                   //等待200ms，等待舵机转动到位
  gDistance = getDistance();    //测量距离，保存到全局变脸gDistance
  delay(100);
  Controller.moveServo(20, 1944 + BIAS, 200); //超声波云台舵机转到130度位置即机器人左面40度位置
  delay(500);                   //延时，等待舵机转动到位
  tDistance = getDistance();    //测量距离，保存到 tDistance
  delay(100);
  Controller.moveServo(20, 2450 + BIAS, 200); //转动到170度，即机器人左侧80度位置
  delay(500);                   //延时，等待舵机转动到位
  gLDistance = getDistance();   //测量距离，保存到 gLDistance
  delay(100);
  if (tDistance < gLDistance)   //比较左侧测得的两个距离，取小的一个，保存到gLDistance作为左侧距离
    gLDistance = tDistance;


  Controller.moveServo(20, 1055 + BIAS, 200); //超声波云台舵机转到50度位置即机器人右面40度位置
  delay(500);                   //延时，等待舵机转动到位
  tDistance = getDistance();    //测量距离，保存到tDistance
  delay(100);
  Controller.moveServo(20, 611 + BIAS, 200); //转到10度，即机器人右面80度位置
  delay(500);                   //延时，等待舵机转动到位
  gRDistance = getDistance();   //测量距离，保存到gRDistance
  delay(100);
  
  if (tDistance < gRDistance)   //比较两个距离，将较小的一个保存到gRDistance
    gRDistance = tDistance;
  Controller.moveServo(20, 1500 + BIAS, 300); //超声波云台舵机转回中间位置
}


void follow(void)
{
  static uint32_t timer = 0;  //定义静态变量timer， 用于计时
  uint16_t len = 0;
  if (timer > millis())  //如果设定时间大于当前毫秒数，则返回，否则继续
    return;

  len = getDistance();
  if (len > 4 && len < 22) {
    if (!Controller.isRunning()) {
      Controller.runActionGroup(GO_BACK, 2); //执行底姿态后退动作组
      //Serial1.println("1");
    }
    timer = millis() + 200;
  }
  else if (len > 40 && len < 80)
  {
    if (!Controller.isRunning())
    {
      Controller.runActionGroup(GO_FORWARD, 2); //执行底姿态前进动作组
      //Serial1.println("2");
    }
    timer = millis() + 200;
  }
  else if (len == 0 && len < 100)
  {
    if (!Controller.isRunning()) {
      getAllDistance();
      if ((gLDistance < gRDistance) &&  (gLDistance != 0) || (gLDistance > 0 && gRDistance == 0)) {
        //左边距离小于右边
        //如果右边距离等于0时表示超出最大检测距离
        Controller.runActionGroup(TURN_LEFT, 3);//左转3次
      }
      else if ((gLDistance > gRDistance) &&  (gRDistance != 0) ||  (gRDistance > 0 && gLDistance == 0)) {
        //右边距离小于左边
        //如果左边距离等于0时表示超出最大检测距离
        Controller.runActionGroup(TURN_RIGHT, 3);//右转3次
      }
      else if (gLDistance == 0 && gRDistance == 0) {
        Controller.runActionGroup(M_STAND, 1);
      }
      timer = millis() + 200;
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  //Serial1.begin(9600);
  MySerial.begin(9600); //初始化串口，波特率9600
  delay(1000);
  Controller.runActionGroup(M_STAND, 1); //执行底姿态立正动作组
}

void loop() {
  // put your main code here, to run repeatedly:
  follow();
  Controller.receiveHandle();
}
