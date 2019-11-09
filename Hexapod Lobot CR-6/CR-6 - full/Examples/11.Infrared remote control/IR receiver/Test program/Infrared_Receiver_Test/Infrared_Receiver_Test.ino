/********Infrared remote test program*******
 * Arduino model：Arduino UNO
 * Please use the IRremote folder in this directory
 * Copy documents to computer library (not E disk)
 * --Arduino--libraries
 **************************/
 
#include <IRremote.h>

#define RECV_PIN   12

#define R_ZERO     0x00FF6897     //IR sensor decoding value 
#define R_ONE      0x00FF30CF
#define R_TWO      0x00FF18E7
#define R_THREE    0x00FF7A85
#define R_FOUR     0x00FF10EF
#define R_FIVE     0x00FF38C7
#define R_SIX      0x00FF5AA5
#define R_SEVEN    0x00FF42BD
#define R_EIGHT    0x00FF4AB5
#define R_NINE     0x00FF52AD
#define R_Previous 0x00FF22DD
#define R_LAST     0x00FF02FD

uint8_t result;

IRrecv irrecv(RECV_PIN); //Set an example of IRrecv 
decode_results results;  //Set an example of decode_results， Results is used to save the decoding results

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  irrecv.enableIRIn();
  delay(1500);
}

int CheckButton(uint32_t code)
{
  int ret;
  switch (code) {
    case R_ZERO:
      ret = 0;
      break;
    case R_ONE:
      ret = 1;
      break;
    case R_TWO:
      ret = 2;
      break;
    case R_THREE:
      ret = 3;
      break;
    case R_FOUR:
      ret = 4;
      break;
    case R_FIVE:
      ret = 5;
      break;
    case R_SIX:
      ret = 6;
      break;
    case R_SEVEN:
      ret = 7;
      break;
    case R_EIGHT:
      ret = 8;
      break;
    case R_NINE:
      ret = 9;
      break;
    case R_Previous:
      ret = -2;
      break;
    case R_LAST:
      ret = -3;
      break;
    default:
      ret = -1;
      break;
  }
  return ret;
}

void loop()
{
  int n;
  if (irrecv.decode(&results)) // If there is data received
  {
    n = CheckButton(results.value);  
    irrecv.resume();
    Serial.println(n);
  }
  delay(200);
}
