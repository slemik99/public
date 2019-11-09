//本例的协议为 命令,参数,参数结束符
//命令为一个英文字母从 a-z, A-Z。
//参数为十六位的有符号整形， -32768 ~ 32767
//命令和参数间，参数和参数间用一个非数字、非字母符号分隔，如 逗号',' 
//两个参数之后要以一个非数字、非字母符号结束，如逗号','
//命令发送者应该保证发送命令的正确性，符合协议要求，否则无法正确解析命令。
//如果命令不需要参数，应该用0填充
//示例命令: A,100,-100,
//            C,2222,9098,
#define Serial Serial1
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int step = 0;
  char cmd = 'N';
  int  p1=0,p2=0;
  while(Serial.available() > 0)
  {
    switch(step){
      case 0:
        cmd = Serial.read();
        if(cmd >= 'A' && cmd <= 'z')          //看收到的是否是英文字母， 如果是就是命令开始
        {
          delay(5);  //小延时，保证命令被完整接收
          Serial.read();  //读取一个字节， 英文字母之后应该是一个分隔符， 读取，相当于把他从缓冲区去除。
          step = 1;   //进入下一步
        }
        break;
     case 1:
        p1 = Serial.parseInt();   //解析第一个参数
        Serial.read();           //读取一个字节，第一个参数之后是一个分隔符， 把他从缓冲区去除
        p2 = Serial.parseInt();  //解析第二个参数
        step = 2;         //进入下一步，因为第二个参数之后还有一个结束符， 所以 Serail.availabale() 返回值还是大于0
        break;
      case 2:
      //这里加入你要根据 接收到的命令进行的操作
        Serial.print(cmd);       
        Serial.print(':');
        Serial.print(p1);
        Serial.print(':');
        Serial.println(p2);
        switch(cmd){
          case 'a':
            //命令a 要进行的对应操作
            digitalWrite(13,HIGH); //例如将板载LED灯点亮
            break;
          case 'b':
            //命令b 要进行的对应操作
            digitalWrite(13, LOW); //例如将板载LED灯熄灭
            break;
          case 'c':
            //命令c 要进行的对应操作
          default:
            break;
        }
        step = 0;
        break;
      default:
        step = 0;
        break;
    }
  }
}
