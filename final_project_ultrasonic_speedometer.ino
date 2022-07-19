// 1912091 하승갑 기말프로젝트 - 초음파 센서를 이용한 근접 물체의 속도 측정 및 표시
// sensor - URM07
// datasheet : https://wiki.dfrobot.com/URM07-UART_Ultrasonic_Sensor_SKU__SEN0153

#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>


// URM07 address - I'll use 0x13
#define URM1 0x13

byte segValue[10][7] = {
   {1,1,1,0,1,1,1}, //0
   {0,0,1,0,0,0,1}, //1
   {0,1,1,1,1,1,0}, //2
   {0,1,1,1,0,1,1}, //3
   {1,0,1,1,0,0,1}, //4
   {1,1,0,1,0,1,1}, //5
   {1,0,0,1,1,1,1}, //6
   {0,1,1,0,0,0,1}, //7
   {1,1,1,1,1,1,1}, //8
   {1,1,1,1,0,0,1}  //9   
 };

SoftwareSerial urm07(2, 3); // RX, TX
Adafruit_NeoPixel np = Adafruit_NeoPixel(56, 10, NEO_GRB + NEO_KHZ800);


void setNPSeg(int number, byte R, byte G, byte B){
  int firstSegNum = number / 10;
  int lastSegNum = (number % 10) - 1;

  Serial.print(firstSegNum); Serial.println(lastSegNum);
  
  np.clear();

  for(int pixelAddrIdx = 0; pixelAddrIdx < 28; pixelAddrIdx += 4){
    // 첫 번째 숫자 설정
    for(int i = 0; i < 4; i++){
      // 입력된 수의 앞 자리[firstSegNum] segValue가 true 이면
      if(segValue[firstSegNum][pixelAddrIdx / 4]){
        // 해당 세그먼트 LED 켜기
        np.setPixelColor(pixelAddrIdx + i, np.Color(R, G, B));
      }
    }
  }
  
  for(int pixelAddrIdx = 28; pixelAddrIdx < 56; pixelAddrIdx += 4){
    // 두 번째 숫자 설정
    for(int i = 0; i < 4; i++){
      // 입력된 수의 앞 자리[firstSegNum] segValue가 true 이면
      if(segValue[lastSegNum][pixelAddrIdx / 4]){
        // 해당 세그먼트 LED 켜기
        np.setPixelColor(pixelAddrIdx + i, np.Color(R, G, B));
      }
    }
  }

  np.show();

}

int urm07GetDistance(unsigned char address) {
    unsigned int distance = 0;
    // {frameHeader_H, frameHeader_L, address, data_length, command, distance_H, distance_L, checksum}
    // -> Rx_DATA length = 8
    unsigned char Rx_DATA[8];
    unsigned char checksum = (0x55+0xAA+address+0x00+0x02);
    // {header_H, header_L, device_addr, data_length, command, checksum}
    // -> Tx_DATA length = 6
    unsigned char CMD[6]={0x55,0xAA,address,0x00,0x02,checksum};
    
    // transmit 6 bytes to urm07
    for(int i = 0; i < 6; i++) { urm07.write(CMD[i]); }
    delay(70); //Measurement Period: <60ms
    
    int count = 0;
    // Serial 버퍼에 데이터가 있을 동안-
    while(urm07.available()){
      Rx_DATA[count++] = urm07.read();
    }

    //Rx_DATA[5] - distance High
    //Rx_DATA[6] - distance Low
    distance = Rx_DATA[5] + Rx_DATA[6];
    
    // ??
    if(Rx_DATA[7] < 20)
    {
      distance = -2;
    }

    // distance (cm)
    return distance;
}

void testNPSeg(){
  for(int i = 0; i < 100; i++){
    setNPSeg(i, 10, 10, 10);
    delay(30);
  }
}

void setup()
{
	  pinMode(2, INPUT); //RX
    pinMode(3, OUTPUT); //TX
    pinMode(10, OUTPUT); // neoPixel

    Serial.begin(115200);
    urm07.begin(19200);

    testNPSeg();
}

void loop()
{
	int distance_1 = urm07GetDistance(URM1);
  delay(1000);
  int distance_2 = urm07GetDistance(URM1);

  int difference = distance_1 - distance_2;
  
  // cm per 100ms -> km per hour => (distance / 100) * 36
  int speed = (difference * 0.001) * 36;
  // Serial.print(distance_1); Serial.print(" "); Serial.println(distance_2);
  Serial.print("difference = "); Serial.println(difference);
  Serial.print("speed : "); Serial.println(speed);
  
  setNPSeg(abs(speed), 10, 10, 0);
}