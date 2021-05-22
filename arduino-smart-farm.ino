/*
 * 회로 연결
 * - 아두이노 <---> LCD (I2C)
 *     5V    ----    VCC
 *    GND    ----    GND
 *           ----    SDA
 *           ----    SCL
 * - 아두이노 <---> DHT11
 *    3.3V   ----  VCC
 *    GND    ----  GND
 *     A1    ----   S
 * - 아두이노 <---> DS1302
 *     5V    ----   VCC
 *    GND    ----   GND
 *                 CLK
 * 
 * 토양 수분 센서의 입력값 (다이소 원예용 상토)
 * - 완전히 매마른 흙: 370 (0%)
 * - 물을 주지 않은 부슬부슬한 흙: 520 (60%)
 * - 물을 완전히 적신 축축한 흙: 620 (100%)
 * 
 * LED 스트립 설치 시 참고
 * - 딸기를 기준으로 광보상점은 45 PPFD, 광포화점은 320 PPFD
 * - 조명도로는 광보상점은 507 lx, 광포화점은 3606 lx
 */

/* 라이브러리 불러오기 */
#include <DHT.h> // DHT 센서(온습도 센서)를 위한 라이브러리를 불러옵니다.
#include <Wire.h> // I2C 통신에 사용되는 라이브러리를 불러옵니다.
#include <LiquidCrystal_I2C.h> // I2C를 사용하는 LCD를 위한 라이브러리를 불러옵니다.
#include <DS1302.h> // RTC 모듈을 위한 라이브러리를 불러옵니다.
#include <Adafruit_NeoPixel.h> // LED 스트립 제어를 위한 네오픽셀 라이브러리를 불러옵니다.

int temperature; // 온도 값을 저장할 변수를 선언합니다.
int humidity; // 습도 값을 저장할 변수를 선언합니다.

int moistureSensor=A0; // 아날로그 0번 핀을 토양 수분 센서로 지정합니다.
int LEDStrip=9; // 디지털 4번 핀을 LED 스트립으로 지정합니다.

DHT dht(A1, DHT11); // 아날로그 0번 핀에 연결되어 있는 DHT 센서를 사용합니다.
LiquidCrystal_I2C lcd(0x27,16,2); // I2C 주소가 0x27이고 16*2 사이즈인 LCD를 사용합니다.
Adafruit_NeoPixel strip=Adafruit_NeoPixel(25, LEDStrip, NEO_GRB+NEO_KHZ800); // 네오픽셀 25개를 LED 스트립으로 지정합니다.
DS1302 rtc(7,6,5); // 5~7번 핀에 연결되어있는 RTC 모듈을 지정합니다.

int LEDStartTime=8; // LED 스트립을 켤 시간을 정합니다.
int LEDEndTime=18; // LED 스트립을 끌 시간을 정합니다.

int moistureUp=620;
int moistureDown=370;

/* 최초 1번만 실행되는 코드 */
void setup() {
  dht.begin(); // DHT 센서 사용을 시작합니다.

  lcd.init(); // LCD를 초기화합니다.
  lcd.backlight(); // LCD의 백 라이트를 켭니다.
  
  rtc.halt(false); // RTC 모듈의 작동 중지를 사용하지 않습니다.
  rtc.writeProtect(true); // RTC 모듈의 시간 수정 보호를 사용합니다.
  
  strip.begin(); // LED 스트립 작동을 시작합니다.

  Time t=rtc.getTime(); // RTC 모듈에 저장되어있는 시간을 시간 변수 t에 저장합니다.
  
  if(t.hour>=LEDStartTime && t.hour<LEDEndTime) { turnStrip(1); } // LED 스트립 작동 시간 내이면 LED 스트립을 켭니다.
  else { turnStrip(0); } // LED 스트립 작동 시간 외이면 LED 스트립을 끕니다.
}

/* 반복 실행되는 코드 */
void loop() {
  Time t=rtc.getTime(); // RTC 모듈에 저장되어있는 시간을 시간 변수 t에 저장합니다.

  /* 매시 30분마다 */
  if(t.min==30) {
    int moistureInput=analogRead(moistureSensor); // 토양 수분 센서의 입력값을 moistureInput 변수에 저장합니다.
    if(moistureInput<=moistureDown) { moistureInput=moistureDown; } // 비정상적으로 낮은 값에 대해 하한값으로 지정합니다.
    else if(moistureInput>=moistureUp) { moistureInput=moistureUp; } // 비정상적으로 높은 값에 대해 상한값으로 지정합니다.
    int moisture=map(moistureInput, moistureDown, moistureUp, 0, 100); // 토양 수분 센서의 입력값을 0~100(%)으로 바꿔서 moisture 변수에 저장합니다.

    /* 토양 수분 센서의 입력값이 600 미만이라면 */
    while(moisture<20) {
      // 워터 펌프 모터를 켜는 함수 추가
      moistureInput=analogRead(moistureSensor); // 토양 수분 센서의 입력값을 moistureInput 변수에 저장합니다.
    }
  }

  /* 매시 3분 이하마다 */
  if(t.min<=3) {
    if(t.hour==LEDStartTime) { turnStrip(1); } // LED 스트립을 켤 시간이 되면 LED 스트립을 켭니다.
    else if(t.hour==LEDEndTime) { turnStrip(0); } // LED 스트립을 끌 시간이 되면 LED 스트립을 끕니다.
  }

  temperature=int(dht.readTemperature()); // DHT 센서로 받아온 온도 값을 변수에 저장합니다.
  humidity=int(dht.readHumidity()); // DHT 센서로 받아온 상대 습도 값을 변수에 저장합니다.

  lcd.setCursor(0,0); // LCD의 커서를 1열 1행에 둡니다.
  lcd.print("Temp: "); lcd.print(temperature); lcd.print("C"); // "Temp: (온도)C"와 같이 출력합니다.

  lcd.setCursor(0,1); // LCD의 커서를 1열 2행에 둡니다.
  lcd.print("Humi: "); lcd.print(humidity); lcd.print("%"); // "Humi: (상대 습도)%"와 같이 출력합니다.
}

/* LED 스트립을 켜고 끄는 함수 */
void turnStrip(boolean onoff) {
  /* onoff 매개변수가 1이라면 */
  if(onoff==1) {
    for(int i=0; i<25; i+=2) { strip.setPixelColor(i,255,0,0,255); } // 홀수번째의 LED는 빨간색으로 지정합니다.
    for(int i=1; i<25; i+=2) { strip.setPixelColor(i,0,70,255,255); } // 짝수번째의 LED는 파란색으로 지정합니다.
    strip.show(); // LED 스트립을 지정된 색으로 켭니다.
  }

  /* onoff 매개변수가 0이라면 */
  else if(onoff==0) {
    for(int i=0; i<25; i+=2) { strip.setPixelColor(i,0,0,0,0); } // 홀수번째의 LED는 빨간색으로 지정합니다.
    for(int i=1; i<25; i+=2) { strip.setPixelColor(i,0,0,0,0); } // 짝수번째의 LED는 파란색으로 지정합니다.
    strip.show(); // LED 스트립을 지정된 색으로 켭니다.
  }
}
