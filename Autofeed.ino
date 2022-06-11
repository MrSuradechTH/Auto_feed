#include <Wire.h>
#include <Nextion.h>
#include <RTClib.h>
#include <HX711.h>
#include <Adafruit_MCP23017.h>
Adafruit_MCP23017 mcp;

//servo
void servo_open() {
  Serial.println("servo open");
  ledcWrite(1, 400);
  delay(100);
}

void servo_close() {
  Serial.println("servo close");
  ledcWrite(1, 950);
  delay(100);
}

//esc
void esc_run() {
  Serial.println("esc run");
  ledcWrite(2, 400);
  delay(3000);
  ledcWrite(2, 950);
  delay(3000);
}

void esc_stop() {
  Serial.println("esc stop");
  ledcWrite(2, 0);
  delay(1000);
}

//stepmotor
void stepmotor_forward() {
  mcp.digitalWrite(7, LOW);
  mcp.digitalWrite(8, HIGH);
  ledcWrite(3, 950);
  delay(1000);
  ledcWrite(3, 0);
}
void stepmotor_reward() {
  mcp.digitalWrite(7, HIGH);
  mcp.digitalWrite(8, LOW);
  ledcWrite(3, 1023);
  delay(1000);
  ledcWrite(3, 0);
}

//rgb
void red() { //ฟังก์ชั่นไฟสีแดงติด
  mcp.digitalWrite(0, LOW);//red
  mcp.digitalWrite(1, LOW);
  mcp.digitalWrite(2, LOW);
  mcp.digitalWrite(3, LOW);
  mcp.digitalWrite(4, HIGH);
  mcp.digitalWrite(5, LOW);
  delay(100);
}

void yellow() { //ฟังก์ชั่นไฟเหลืองติด
  mcp.digitalWrite(0, LOW);
  mcp.digitalWrite(1, LOW);
  mcp.digitalWrite(2, LOW);//yellow
  mcp.digitalWrite(3, HIGH);
  mcp.digitalWrite(4, LOW);
  mcp.digitalWrite(5, LOW);
  delay(100);
}

void green() { //ฟังก์ชั่นไฟสีเขียวติด
  mcp.digitalWrite(0, LOW);
  mcp.digitalWrite(1, LOW);
  mcp.digitalWrite(2, HIGH);//green
  mcp.digitalWrite(3, LOW);
  mcp.digitalWrite(4, LOW);
  mcp.digitalWrite(5, LOW);
  delay(100);
}

void white() { //ฟังก์ชั่นปิดไฟ
  mcp.digitalWrite(0, LOW);
  mcp.digitalWrite(1, LOW);
  mcp.digitalWrite(2, LOW);//white
  mcp.digitalWrite(3, LOW);
  mcp.digitalWrite(4, LOW);
  mcp.digitalWrite(5, LOW);
  delay(100);
}

//limit
int8_t limitcheck;
void limit() {
  limitcheck = mcp.digitalRead(6);
  limitcheck = 1;
}

//RTC
RTC_DS3231 rtc;
uint32_t H, M;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int8_t changetime[] = {0,0};
int8_t page;

//loadcell
#define DOUT  23
#define CLK  5
HX711 scale(DOUT,CLK);
int g;
int zero;
float calibration_factor = 7050; //-7050 worked for my 440lb max scale setup

void loadcell() {
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  Serial.print("Reading: ");
  g = int(((((scale.get_units() * 2.204629722417076)/100) * 1000)) - zero);
  if (g < 0) {
    g = 0;
  }
  Serial.print(g);
  Serial.println(" g"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
}

//ledboard
int LED = 2;

//ultrasonics
uint8_t UltrasonicPin[] = {18,19}; //ใส่ขา trig encho ตามลำดับบ
long duration,cm; //ตัวแปรเก็บค่าระยะเวลาและระยะทาง ของความถีที่ Ultrasonic ปล่อยไปแล้วสะท้อนกลับมา

long tocm(long microseconds) {
  return microseconds/29/2;
  //เนื่องความเร็วของเเสงอยู่ที่ 340เมตร ต่อ วินาที หรือ 29ไมโครวินาที ต่อ เซนติเมตร จึงนำเวลาที่มีพัลส์เข้าที่ขา echo หารด้วย 29 จะได้ระยะทางไปกลับ นำมาหาร 2 เพื่อหาระยะทางไปอย่างเดียว
}

void ultrasonic() {
  pinMode(UltrasonicPin[0], OUTPUT);
  digitalWrite(UltrasonicPin[0], LOW);
  delayMicroseconds(2);
  digitalWrite(UltrasonicPin[0], HIGH);
  delayMicroseconds(5);
  digitalWrite(UltrasonicPin[0], LOW);
  pinMode(UltrasonicPin[1], INPUT);
  duration = pulseIn(UltrasonicPin[1], HIGH);
  cm = tocm(duration);
  Serial.print(cm);
  Serial.println("cm");
  Serial.println();
}

//any
long millisnext,millisnow;
int8_t password[] = {1,2,4,8};
int8_t input_password[] = {0,0,0,0};
boolean test,started,zerofeedstand,modes,timestack,pagecheck,feedingcheck;
int8_t feed[] = {0,0,0,0};
int8_t fishmode,rounds,timechange,ofstack;
int8_t timefeedfish1H[] = {7,9,11,15,17,0,0,0};
int8_t timefeedfish1M[] = {0,0,0,0,0,0,0,0};
int8_t timefeedfish2H[] = {8,10,12,16,18,0,0,0};
int8_t timefeedfish2M[] = {0,0,0,0,0,0,0,0};
int8_t timefeedfish3H[] = {0,0,0,0,0,0,0,0};
int8_t timefeedfish3M[] = {0,0,0,0,0,0,0,0};
int8_t timefeedchange[] = {0,0};
uint32_t dual_state;
int8_t ofcheck[] = {1,1,1,1,1,1,1,1};
float percentfeed;
//**************************************************PAGE 0**************************************************//
NexPage p0 = NexPage(0, 0, "main_menu");
NexNumber p0_nH = NexNumber(0, 7, "p0_nH");
NexNumber p0_nM = NexNumber(0, 6, "p0_nM");
NexButton p0_b0 = NexButton(0, 1, "p0_b0");
NexButton p0_b1 = NexButton(0, 8, "p0_b1");
NexButton p0_b2 = NexButton(0, 9, "p0_b2");
//**************************************************PAGE 1**************************************************//
NexPage p1 = NexPage(1, 0, "fish_menu");
NexNumber p1_nH = NexNumber(1, 4, "p1_nH");
NexNumber p1_nM = NexNumber(1, 3, "p1_nM");
NexButton p1_b0 = NexButton(1, 1, "p1_b0");
NexButton p1_b1 = NexButton(1, 5, "p1_b1");
NexButton p1_b2 = NexButton(1, 6, "p1_b2");
NexButton p1_b3 = NexButton(1, 9, "p1_b3");
NexButton p1_b4 = NexButton(1, 8, "p1_b4");
//**************************************************PAGE 2**************************************************//
NexPage p2 = NexPage(2, 0, "auto_round");
NexNumber p2_nH = NexNumber(2, 52, "p2_nH");
NexNumber p2_nM = NexNumber(2, 53, "p2_nM");
NexButton p2_b0 = NexButton(2, 5, "p2_b0");
NexButton p2_b1 = NexButton(2, 6, "p2_b1");
NexButton p2_b2 = NexButton(2, 7, "p2_b2");
NexButton p2_b3 = NexButton(2, 9, "p2_b3");
NexButton p2_b4 = NexButton(2, 12, "p2_b4");
NexButton p2_b5 = NexButton(2, 13, "p2_b5");
NexButton p2_b6 = NexButton(2, 3, "p2_b6");
NexButton p2_b7 = NexButton(2, 4, "p2_b7");
NexButton p2_b8 = NexButton(2, 1, "p2_b8");
NexButton p2_b9 = NexButton(2, 2, "p2_b9");
NexButton p2_b10 = NexButton(2, 10, "p2_b10");
NexButton p2_b11 = NexButton(2, 11, "p2_b11");
NexButton p2_b12 = NexButton(2, 53, "p2_b12");
NexButton p2_b13 = NexButton(2, 54, "p2_b13");
NexButton p2_b14 = NexButton(2, 55, "p2_b14");
NexButton p2_b15 = NexButton(2, 56, "p2_b15");
NexButton p2_b16 = NexButton(2, 57, "p2_b16");
NexButton p2_b17 = NexButton(2, 58, "p2_b17");
NexButton p2_b18 = NexButton(2, 59, "p2_b18");
NexButton p2_b19 = NexButton(2, 60, "p2_b19");
NexDSButton p2_bt0 = NexDSButton(2, 23, "p2_bt0");
NexDSButton p2_bt1 = NexDSButton(2, 24, "p2_bt1");
NexDSButton p2_bt2 = NexDSButton(2, 25, "p2_bt2");
NexDSButton p2_bt3 = NexDSButton(2, 26, "p2_bt3");
NexDSButton p2_bt4 = NexDSButton(2, 27, "p2_bt4");
NexDSButton p2_bt5 = NexDSButton(2, 28, "p2_bt5");
NexDSButton p2_bt6 = NexDSButton(2, 29, "p2_bt6");
NexDSButton p2_bt7 = NexDSButton(2, 30, "p2_bt7");
NexNumber p2_n0 = NexNumber(2, 31, "p2_n0");
NexNumber p2_n1 = NexNumber(2, 32, "p2_n1");
NexNumber p2_n2 = NexNumber(2, 33, "p2_n2");
NexNumber p2_n3 = NexNumber(2, 34, "p2_n3");
NexNumber p2_n4 = NexNumber(2, 35, "p2_n4");
NexNumber p2_n5 = NexNumber(2, 36, "p2_n5");
NexNumber p2_n6 = NexNumber(2, 37, "p2_n6");
NexNumber p2_n7 = NexNumber(2, 38, "p2_n7");
NexNumber p2_n8 = NexNumber(2, 39, "p2_n8");
NexNumber p2_n9 = NexNumber(2, 40, "p2_n9");
NexNumber p2_n10 = NexNumber(2, 41, "p2_n10");
NexNumber p2_n11 = NexNumber(2, 42, "p2_n11");
NexNumber p2_n12 = NexNumber(2, 43, "p2_n12");
NexNumber p2_n13 = NexNumber(2, 44, "p2_n13");
NexNumber p2_n14 = NexNumber(2, 45, "p2_n14");
NexNumber p2_n15 = NexNumber(2, 46, "p2_n15");
NexNumber p2_n16 = NexNumber(2, 47, "p2_n16");
NexNumber p2_n17 = NexNumber(2, 48, "p2_n17");
NexNumber p2_n18 = NexNumber(2, 49, "p2_n18");
NexNumber p2_n19 = NexNumber(2, 64, "p2_n19");
NexText p2_t13 = NexText(2, 66, "p2_t13");
//**************************************************PAGE 3**************************************************//
NexPage p3 = NexPage(3, 0, "manual_round");
NexNumber p3_nH = NexNumber(3, 9, "p3_nH");
NexNumber p3_nM = NexNumber(3, 8, "p3_nM");
NexNumber p3_n0 = NexNumber(3, 6, "p3_n0");
NexButton p3_b0 = NexButton(3, 1, "p3_b0");
NexButton p3_b1 = NexButton(3, 2, "p3_b1");
NexButton p3_b2 = NexButton(3, 3, "p3_b2");
NexButton p3_b3 = NexButton(3, 4, "p3_b3");
NexButton p3_b4 = NexButton(3, 5, "p3_b4");
//**************************************************PAGE 4**************************************************//
NexPage p4 = NexPage(4, 0, "start");
NexNumber p4_nH = NexNumber(4, 6, "p4_nH");
NexNumber p4_nM = NexNumber(4, 5, "p4_nM");
NexNumber p4_n0 = NexNumber(4, 7, "p4_n0");
NexButton p4_b0 = NexButton(4, 1, "p4_b0");
NexButton p4_b1 = NexButton(4, 2, "p4_b1");
NexText p4_t1 = NexText(2, 3, "p4_t1");
NexText p4_t2 = NexText(2, 9, "p4_t2");
//**************************************************PAGE 5**************************************************//
NexPage p5 = NexPage(5, 0, "feedstand");
NexNumber p5_nH = NexNumber(5, 6, "p5_nH");
NexNumber p5_nM = NexNumber(5, 5, "p5_nM");
NexNumber p5_n0 = NexNumber(5, 7, "p5_n0");
NexButton p5_b0 = NexButton(5, 1, "p5_b0");
NexButton p5_b1 = NexButton(5, 2, "p5_b1");
//**************************************************PAGE 6**************************************************//
NexPage p6 = NexPage(6, 0, "feeding");
NexNumber p6_nH = NexNumber(6, 6, "p6_nH");
NexNumber p6_nM = NexNumber(6, 5, "p6_nM");
NexNumber p6_n0 = NexNumber(6, 7, "p6_n0");
NexButton p6_b0 = NexButton(6, 1, "p6_b0");
NexButton p6_b1 = NexButton(6, 2, "p6_b1");
//**************************************************PAGE 7**************************************************//
NexPage p7 = NexPage(7, 0, "service_pass");
NexNumber p7_nH = NexNumber(7, 5, "p7_nH");
NexNumber p7_nM = NexNumber(7, 4, "p7_nM");
NexNumber p7_n0 = NexNumber(7, 9, "p7_n0");
NexNumber p7_n1 = NexNumber(7, 10, "p7_n1");
NexNumber p7_n2 = NexNumber(7, 11, "p7_n2");
NexNumber p7_n3 = NexNumber(7, 12, "p7_n3");
NexButton p7_b0 = NexButton(7, 1, "p7_b0");
NexButton p7_b1 = NexButton(7, 2, "p7_b1");
NexButton p7_b2 = NexButton(7, 6, "p7_b2");
NexButton p7_b3 = NexButton(7, 13, "p7_b3");
NexButton p7_b4 = NexButton(7, 14, "p7_b4");
NexButton p7_b5 = NexButton(7, 15, "p7_b5");
NexButton p7_b6 = NexButton(7, 16, "p7_b6");
NexButton p7_b7 = NexButton(7, 17, "p7_b7");
NexButton p7_b8 = NexButton(7, 18, "p7_b8");
NexButton p7_b9 = NexButton(7, 19, "p7_b9");
NexButton p7_b10 = NexButton(7, 20, "p7_b10");
NexText p7_t2 = NexText(7, 8, "p7_t2");
//**************************************************PAGE 8**************************************************//
NexPage p8 = NexPage(8, 0, "service_menu");
NexNumber p8_nH = NexNumber(8, 10, "p8_nH");
NexNumber p8_nM = NexNumber(8, 9, "p8_nM");
NexButton p8_b0 = NexButton(8, 1, "p8_b0");
NexButton p8_b1 = NexButton(8, 2, "p8_b1");
NexButton p8_b2 = NexButton(8, 18, "p8_b2");
NexButton p8_b3 = NexButton(8, 4, "p8_b3");
NexButton p8_b4 = NexButton(8, 3, "p8_b4");
NexButton p8_b5 = NexButton(8, 5, "p8_b5");
NexButton p8_b6 = NexButton(8, 13, "p8_b6");
NexButton p8_b7 = NexButton(8, 12, "p8_b7");
NexButton p8_b8 = NexButton(8, 14, "p8_b8");
//**************************************************PAGE 9**************************************************//
NexPage p9 = NexPage(9, 0, "time");
NexNumber p9_nH = NexNumber(9, 5, "p9_nH");
NexNumber p9_nM = NexNumber(9, 4, "p9_nM");
NexNumber p9_n0 = NexNumber(9, 6, "p9_n0");
NexNumber p9_n1 = NexNumber(9, 7, "p9_n1");
NexButton p9_b0 = NexButton(9, 1, "p9_b0");
NexButton p9_b1 = NexButton(9, 2, "p9_b1");
NexButton p9_b2 = NexButton(9, 12, "p9_b2");
NexButton p9_b3 = NexButton(9, 8, "p9_b3");
NexButton p9_b4 = NexButton(9, 9, "p9_b4");
NexButton p9_b5 = NexButton(9, 10, "p9_b5");
NexButton p9_b6 = NexButton(9, 11, "p9_b6");
//**************************************************PAGE 10**************************************************//
NexPage p10 = NexPage(10, 0, "step_motor");
NexNumber p10_nH = NexNumber(10, 5, "p10_nH");
NexNumber p10_nM = NexNumber(10, 4, "p10_nM");
NexButton p10_b0 = NexButton(10, 1, "p10_b0");
NexButton p10_b1 = NexButton(10, 2, "p10_b1");
NexButton p10_b2 = NexButton(10, 6, "p10_b2");
NexButton p10_b3 = NexButton(10, 7, "p10_b3");
//**************************************************PAGE 11**************************************************//
NexPage p11 = NexPage(11, 0, "loadcell");
NexNumber p11_nH = NexNumber(11, 6, "p11_nH");
NexNumber p11_nM = NexNumber(11, 5, "p11_nM");
NexButton p11_b0 = NexButton(11, 1, "p11_b0");
NexButton p11_b1 = NexButton(11, 2, "p11_b1");
NexButton p11_b2 = NexButton(11, 8, "p11_b2");
NexNumber p11_n0 = NexNumber(11, 7, "p11_n0");
//**************************************************PAGE 12**************************************************//
NexPage p12 = NexPage(12, 0, "servo");
NexNumber p12_nH = NexNumber(12, 5, "p12_nH");
NexNumber p12_nM = NexNumber(12, 4, "p12_nM");
NexButton p12_b0 = NexButton(12, 1, "p12_b0");
NexButton p12_b1 = NexButton(12, 2, "p12_b1");
NexButton p12_b2 = NexButton(12, 6, "p12_b2");
NexButton p12_b3 = NexButton(12, 7, "p12_b3");
//**************************************************PAGE 13**************************************************//
NexPage p13 = NexPage(13, 0, "rgb");
NexNumber p13_nH = NexNumber(13, 5, "p13_nH");
NexNumber p13_nM = NexNumber(13, 4, "p13_nM");
NexButton p13_b0 = NexButton(13, 1, "p13_b0");
NexButton p13_b1 = NexButton(13, 2, "p13_b1");
NexButton p13_b2 = NexButton(13, 6, "p13_b2");
NexButton p13_b3 = NexButton(13, 7, "p13_b3");
NexButton p13_b4 = NexButton(13, 8, "p13_b4");
//**************************************************PAGE 14**************************************************//
NexPage p14 = NexPage(14, 0, "ultrasonic");
NexNumber p14_nH = NexNumber(14, 6, "p14_nH");
NexNumber p14_nM = NexNumber(14, 5, "p14_nM");
NexNumber p14_n0 = NexNumber(14, 7, "p14_n0");
NexButton p14_b0 = NexButton(14, 1, "p14_b0");
NexButton p14_b1 = NexButton(14, 2, "p14_b1");
//**************************************************PAGE 15**************************************************//
NexPage p15 = NexPage(15, 0, "esc");
NexNumber p15_nH = NexNumber(15, 5, "p15_nH");
NexNumber p15_nM = NexNumber(15, 4, "p15_nM");
NexButton p15_b0 = NexButton(15, 1, "p15_b0");
NexButton p15_b1 = NexButton(15, 2, "p15_b1");
NexButton p15_b2 = NexButton(15, 6, "p15_b2");

NexTouch *nex_listen_list[] = {
  //**************************************************PAGE 0**************************************************//
  &p0_b0,&p0_b1,&p0_b2,
  //**************************************************PAGE 1**************************************************//
  &p1_b0,&p1_b1,&p1_b2,&p1_b3,&p1_b4,
  //**************************************************PAGE 2**************************************************//
  &p2_b0,&p2_b1,&p2_b2,&p2_b3,&p2_b4,&p2_b5,&p2_b6,&p2_b7,&p2_b8,&p2_b9,&p2_b10,&p2_b11,&p2_b12,&p2_b13,&p2_b14,&p2_b15,&p2_b16,&p2_b17,&p2_b18,&p2_b19,&p2_bt0,&p2_bt1,&p2_bt2,&p2_bt3,&p2_bt4,&p2_bt5,&p2_bt6,&p2_bt7,
  //**************************************************PAGE 3**************************************************//
  &p3_b0,&p3_b1,&p3_b2,&p3_b3,&p3_b4,
  //**************************************************PAGE 4**************************************************//
  &p4_b0,&p4_b1,
  //**************************************************PAGE 5**************************************************//
  &p5_b0,&p5_b1,
  //**************************************************PAGE 6**************************************************//
  &p6_b0,&p6_b1,
  //**************************************************PAGE 7**************************************************//
  &p7_b0,&p7_b1,&p7_b2,&p7_b3,&p7_b4,&p7_b5,&p7_b6,&p7_b7,&p7_b8,&p7_b9,&p7_b10,
  //**************************************************PAGE 8**************************************************//
  &p8_b0,&p8_b1,&p8_b2,&p8_b3,&p8_b4,&p8_b5,&p8_b6,&p8_b7,&p8_b8,
  //**************************************************PAGE 9**************************************************//
  &p9_b0,&p9_b1,&p9_b2,&p9_b3,&p9_b4,&p9_b5,&p9_b6,
  //**************************************************PAGE 10**************************************************//
  &p10_b0,&p10_b1,&p10_b2,&p10_b3,
  //**************************************************PAGE 11**************************************************//
  &p11_b0,&p11_b1,&p11_b2,
  //**************************************************PAGE 12**************************************************//
  &p12_b0,&p12_b1,&p12_b2,&p12_b3,
  //**************************************************PAGE 13**************************************************//
  &p13_b0,&p13_b1,&p13_b2,&p13_b3,&p13_b4,
  //**************************************************PAGE 14**************************************************//
  &p14_b0,&p14_b1,
  //**************************************************PAGE 15**************************************************//
  &p15_b0,&p15_b1,&p15_b2,NULL
};

void showtime() {
  DateTime now = rtc.now();
   
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  
  H = now.hour();
  M = now.minute();
  
  if (page == 0) {
    p0_nH.setValue(H);
    p0_nM.setValue(M);
  }else if (page == 1) {
    p1_nH.setValue(H);
    p1_nM.setValue(M);
  }else if (page == 2) {
    p2_nH.setValue(H);
    p2_nM.setValue(M);
  }else if (page == 3) {
    p3_nH.setValue(H);
    p3_nM.setValue(M);
  }else if (page == 4) {
    p4_nH.setValue(H);
    p4_nM.setValue(M);
  }else if (page == 5) {
    p5_nH.setValue(H);
    p5_nM.setValue(M);
  }else if (page == 6) {
    p6_nH.setValue(H);
    p6_nM.setValue(M);
  }else if (page == 7) {
    p7_nH.setValue(H);
    p7_nM.setValue(M);
  }else if (page == 8) {
    p8_nH.setValue(H);
    p8_nM.setValue(M);
  }else if (page == 9) {
    p9_nH.setValue(H);
    p9_nM.setValue(M);
  }else if (page == 10) {
    p10_nH.setValue(H);
    p10_nM.setValue(M);
  }else if (page == 11) {
    p11_nH.setValue(H);
    p11_nM.setValue(M);
  }else if (page == 12) {
    p12_nH.setValue(H);
    p12_nM.setValue(M);
  }else if (page == 13) {
    p13_nH.setValue(H);
    p13_nM.setValue(M);
  }else if (page == 14) {
    p14_nH.setValue(H);
    p14_nM.setValue(M);
  }else if (page == 15) {
    p15_nH.setValue(H);
    p15_nM.setValue(M);
  }
  
  //เครื่องคิดเลขในการคำนวณวันเวลา เป็นคำสั่งเฉพาะในไลบรารี่ ห้ามเปลี่ยน
  DateTime future (now + TimeSpan(7,12,30,6));
}

void checktime() {
  showtime();
  if(fishmode == 0) { 
    for (int i = 0;i <= 7 && started == true;i++) {
      nlisten();
      if (ofcheck[i] == 1) {
        if (timefeedfish1H[i] == H && timefeedfish1M[i] == M && timestack == false) {
          Serial.print(String(timefeedfish1H[i]) + ":" + String(timefeedfish1M[i]) + ":");
          Serial.println("is time!!!");
          timestack = true;
          checkfeeding();
        }else if (timefeedfish1H[i] == H && timefeedfish1M[i] == M && timestack == true) {
          Serial.print(String(timefeedfish1H[i]) + ":" + String(timefeedfish1M[i]) + ":");
          Serial.println("is cooldown!!!");
        }else if (timefeedfish1H[i] == H && timefeedfish1M[i] != M) {
          Serial.print(String(timefeedfish1H[i]) + ":" + String(timefeedfish1M[i]) + ":");
          Serial.println("not is time!!!");
          timestack = false;
         }else if (timefeedfish1H[i] != H && timefeedfish1M[i] != M) {
          Serial.print(String(timefeedfish1H[i]) + ":" + String(timefeedfish1M[i]) + ":");
          Serial.println("not is time!!!");
          timestack = false;
         }
        }else if (ofcheck[i] == 0){
          Serial.print(String(timefeedfish1H[i]) + ":" + String(timefeedfish1M[i]) + ":");
          Serial.println("off!!!");
        }
    }
  }else if(fishmode == 1) { 
    for (int i = 0;i <= 7;i++) {
      if (ofcheck[i] == 1) {
        if (timefeedfish2H[i] == H && timefeedfish2M[i] == M && timestack == false) {
          Serial.print(String(timefeedfish2H[i]) + ":" + String(timefeedfish2M[i]) + ":");
          Serial.println("is time!!!");
          timestack = true;
          checkfeeding();
        }else if (timefeedfish2H[i] == H && timefeedfish2M[i] == M && timestack == true) {
          Serial.print(String(timefeedfish2H[i]) + ":" + String(timefeedfish2M[i]) + ":");
          Serial.println("is cooldown!!!");
        }else if (timefeedfish2H[i] == H && timefeedfish2M[i] != M) {
          Serial.print(String(timefeedfish2H[i]) + ":" + String(timefeedfish2M[i]) + ":");
          Serial.println("not is time!!!");
          timestack = false;
         }else if (timefeedfish2H[i] != H && timefeedfish2M[i] != M) {
          Serial.print(String(timefeedfish2H[i]) + ":" + String(timefeedfish2M[i]) + ":");
          Serial.println("not is time!!!");
          timestack = false;
         }
        }else if (ofcheck[i] == 0){
          Serial.print(String(timefeedfish2H[i]) + ":" + String(timefeedfish2M[i]) + ":");
          Serial.println("off!!!");
        }
    }
  }else if(fishmode == 2) { 
    for (int i = 0;i <= 7;i++) {
      if (ofcheck[i] == 1) {
        if (timefeedfish3H[i] == H && timefeedfish3M[i] == M && timestack == false) {
          Serial.print(String(timefeedfish3H[i]) + ":" + String(timefeedfish3M[i]) + ":");
          Serial.println("is time!!!");
          timestack = true;
          checkfeeding();
        }else if (timefeedfish3H[i] == H && timefeedfish3M[i] == M && timestack == true) {
          Serial.print(String(timefeedfish3H[i]) + ":" + String(timefeedfish3M[i]) + ":");
          Serial.println("is cooldown!!!");
        }else if (timefeedfish3H[i] == H && timefeedfish3M[i] != M) {
          Serial.print(String(timefeedfish3H[i]) + ":" + String(timefeedfish3M[i]) + ":");
          Serial.println("not is time!!!");
          timestack = false;
         }else if (timefeedfish3H[i] != H && timefeedfish3M[i] != M) {
          Serial.print(String(timefeedfish3H[i]) + ":" + String(timefeedfish3M[i]) + ":");
          Serial.println("not is time!!!");
          timestack = false;
         }
        }else if (ofcheck[i] == 0){
          Serial.print(String(timefeedfish3H[i]) + ":" + String(timefeedfish3M[i]) + ":");
          Serial.println("off!!!");
        }
    }
  }
}

void debugbt() {
  ofstack = 0;
  for (int i = 0;i <= 7;i++) {
    ofstack += ofcheck[i];
    }
  if(ofstack != rounds) {
    if (fishmode == 0) {
    p2.show();
    page = 2;
    showtime();
    fishmode = 0;
    rounds = 8;
    ofcheck[0] = 1;
    ofcheck[1] = 1;
    ofcheck[2] = 1;
    ofcheck[3] = 1;
    ofcheck[4] = 1;
    ofcheck[5] = 1;
    ofcheck[6] = 1;
    ofcheck[7] = 1;
    timechange = 8;
    p2_t13.setText("-");
    p2_n0.setValue(timefeedfish1H[0]);
    p2_n1.setValue(timefeedfish1M[0]);
    p2_n2.setValue(timefeedfish1H[1]);
    p2_n3.setValue(timefeedfish1M[1]);
    p2_n4.setValue(timefeedfish1H[2]);
    p2_n5.setValue(timefeedfish1M[2]);
    p2_n6.setValue(timefeedfish1H[3]);
    p2_n7.setValue(timefeedfish1M[3]);
    p2_n8.setValue(timefeedfish1H[4]);
    p2_n9.setValue(timefeedfish1M[4]);
    p2_n10.setValue(timefeedfish1H[5]);
    p2_n11.setValue(timefeedfish1M[5]);
    p2_n12.setValue(timefeedfish1H[6]);
    p2_n13.setValue(timefeedfish1M[6]);
    p2_n14.setValue(timefeedfish1H[7]);
    p2_n15.setValue(timefeedfish1M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
  }else if (fishmode == 1) {
    p2.show();
    page = 2;
    showtime();
    fishmode = 1;
    rounds = 8;
    ofcheck[0] = 1;
    ofcheck[1] = 1;
    ofcheck[2] = 1;
    ofcheck[3] = 1;
    ofcheck[4] = 1;
    ofcheck[5] = 1;
    ofcheck[6] = 1;
    ofcheck[7] = 1;
    timechange = 8;
    p2_t13.setText("-");
    p2_n0.setValue(timefeedfish2H[0]);
    p2_n1.setValue(timefeedfish2M[0]);
    p2_n2.setValue(timefeedfish2H[1]);
    p2_n3.setValue(timefeedfish2M[1]);
    p2_n4.setValue(timefeedfish2H[2]);
    p2_n5.setValue(timefeedfish2M[2]);
    p2_n6.setValue(timefeedfish2H[3]);
    p2_n7.setValue(timefeedfish2M[3]);
    p2_n8.setValue(timefeedfish2H[4]);
    p2_n9.setValue(timefeedfish2M[4]);
    p2_n10.setValue(timefeedfish2H[5]);
    p2_n11.setValue(timefeedfish2M[5]);
    p2_n12.setValue(timefeedfish2H[6]);
    p2_n13.setValue(timefeedfish2M[6]);
    p2_n14.setValue(timefeedfish2H[7]);
    p2_n15.setValue(timefeedfish2M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
  }else if (fishmode == 2) {
    p2.show();
    page = 2;
    showtime();
    fishmode = 2;
    rounds = 8;
    ofcheck[0] = 1;
    ofcheck[1] = 1;
    ofcheck[2] = 1;
    ofcheck[3] = 1;
    ofcheck[4] = 1;
    ofcheck[5] = 1;
    ofcheck[6] = 1;
    ofcheck[7] = 1;
    timechange = 8;
    p2_t13.setText("-");
    p2_n0.setValue(timefeedfish3H[0]);
    p2_n1.setValue(timefeedfish3M[0]);
    p2_n2.setValue(timefeedfish3H[1]);
    p2_n3.setValue(timefeedfish3M[1]);
    p2_n4.setValue(timefeedfish3H[2]);
    p2_n5.setValue(timefeedfish3M[2]);
    p2_n6.setValue(timefeedfish3H[3]);
    p2_n7.setValue(timefeedfish3M[3]);
    p2_n8.setValue(timefeedfish3H[4]);
    p2_n9.setValue(timefeedfish3M[4]);
    p2_n10.setValue(timefeedfish3H[5]);
    p2_n11.setValue(timefeedfish3M[5]);
    p2_n12.setValue(timefeedfish3H[6]);
    p2_n13.setValue(timefeedfish3M[6]);
    p2_n14.setValue(timefeedfish3H[7]);
    p2_n15.setValue(timefeedfish3M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
  }
  }
}
//**************************************************PAGE 0**************************************************//
void p0_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p1.show();
  page = 1;
  showtime();
  modes = true;
}

void p0_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p3.show();
  page = 3;
  showtime();
  fishmode = 3;
  p3_n0.setValue(feed[3]);
  modes = false;
}

void p0_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p7.show();
  page = 7;
  showtime();
  for (int i = 0;i <= 3;i++) {
    input_password[i] = 0;
  }
  p7_n0.setValue(input_password[0]);
  p7_n1.setValue(input_password[1]);
  p7_n2.setValue(input_password[2]);
  p7_n3.setValue(input_password[3]);
}
//**************************************************PAGE 1**************************************************//
void p1_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2.show();
  page = 2;
  showtime();
  fishmode = 0;
  rounds = 8;
  ofcheck[0] = 1;
  ofcheck[1] = 1;
  ofcheck[2] = 1;
  ofcheck[3] = 1;
  ofcheck[4] = 1;
  ofcheck[5] = 1;
  ofcheck[6] = 1;
  ofcheck[7] = 1;
  timechange = 8;
  p2_t13.setText("-");
  p2_n0.setValue(timefeedfish1H[0]);
  p2_n1.setValue(timefeedfish1M[0]);
  p2_n2.setValue(timefeedfish1H[1]);
  p2_n3.setValue(timefeedfish1M[1]);
  p2_n4.setValue(timefeedfish1H[2]);
  p2_n5.setValue(timefeedfish1M[2]);
  p2_n6.setValue(timefeedfish1H[3]);
  p2_n7.setValue(timefeedfish1M[3]);
  p2_n8.setValue(timefeedfish1H[4]);
  p2_n9.setValue(timefeedfish1M[4]);
  p2_n10.setValue(timefeedfish1H[5]);
  p2_n11.setValue(timefeedfish1M[5]);
  p2_n12.setValue(timefeedfish1H[6]);
  p2_n13.setValue(timefeedfish1M[6]);
  p2_n14.setValue(timefeedfish1H[7]);
  p2_n15.setValue(timefeedfish1M[7]);
  p2_n16.setValue(feed[fishmode]);
  p2_n19.setValue(rounds);
}

void p1_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2.show();
  page = 2;
  showtime();
  fishmode = 1;
  rounds = 8;
  ofcheck[0] = 1;
  ofcheck[1] = 1;
  ofcheck[2] = 1;
  ofcheck[3] = 1;
  ofcheck[4] = 1;
  ofcheck[5] = 1;
  ofcheck[6] = 1;
  ofcheck[7] = 1;
  timechange = 8;
  p2_t13.setText("-");
  p2_n0.setValue(timefeedfish2H[0]);
  p2_n1.setValue(timefeedfish2M[0]);
  p2_n2.setValue(timefeedfish2H[1]);
  p2_n3.setValue(timefeedfish2M[1]);
  p2_n4.setValue(timefeedfish2H[2]);
  p2_n5.setValue(timefeedfish2M[2]);
  p2_n6.setValue(timefeedfish2H[3]);
  p2_n7.setValue(timefeedfish2M[3]);
  p2_n8.setValue(timefeedfish2H[4]);
  p2_n9.setValue(timefeedfish2M[4]);
  p2_n10.setValue(timefeedfish2H[5]);
  p2_n11.setValue(timefeedfish2M[5]);
  p2_n12.setValue(timefeedfish2H[6]);
  p2_n13.setValue(timefeedfish2M[6]);
  p2_n14.setValue(timefeedfish2H[7]);
  p2_n15.setValue(timefeedfish2M[7]);
  p2_n16.setValue(feed[fishmode]);
  p2_n19.setValue(rounds);
}

void p1_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2.show();
  page = 2;
  showtime();
  fishmode = 2;
  rounds = 8;
  ofcheck[0] = 1;
  ofcheck[1] = 1;
  ofcheck[2] = 1;
  ofcheck[3] = 1;
  ofcheck[4] = 1;
  ofcheck[5] = 1;
  ofcheck[6] = 1;
  ofcheck[7] = 1;
  timechange = 8;
  p2_t13.setText("-");
  p2_n0.setValue(timefeedfish3H[0]);
  p2_n1.setValue(timefeedfish3M[0]);
  p2_n2.setValue(timefeedfish3H[1]);
  p2_n3.setValue(timefeedfish3M[1]);
  p2_n4.setValue(timefeedfish3H[2]);
  p2_n5.setValue(timefeedfish3M[2]);
  p2_n6.setValue(timefeedfish3H[3]);
  p2_n7.setValue(timefeedfish3M[3]);
  p2_n8.setValue(timefeedfish3H[4]);
  p2_n9.setValue(timefeedfish3M[4]);
  p2_n10.setValue(timefeedfish3H[5]);
  p2_n11.setValue(timefeedfish3M[5]);
  p2_n12.setValue(timefeedfish3H[6]);
  p2_n13.setValue(timefeedfish3M[6]);
  p2_n14.setValue(timefeedfish3H[7]);
  p2_n15.setValue(timefeedfish3M[7]);
  p2_n16.setValue(feed[fishmode]);
  p2_n19.setValue(rounds);
}

void p1_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p1_b4_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}
//**************************************************PAGE 2**************************************************//
void p2_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p2_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p1.show();
  page = 1;
  showtime();
}

void p2_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_t13.setText("-");
  rounds = 8;
  timechange = 8;
  ofcheck[0] = 1;
  ofcheck[1] = 1;
  ofcheck[2] = 1;
  ofcheck[3] = 1;
  ofcheck[4] = 1;
  ofcheck[5] = 1;
  ofcheck[6] = 1;
  ofcheck[7] = 1;
  if (fishmode == 0) {
    p2.show();
    page = 2;
    showtime();
    feed[fishmode] = 0;
    timefeedfish1H[0] = 7;
    timefeedfish1H[1] = 9;
    timefeedfish1H[2] = 11;
    timefeedfish1H[3] = 15;
    timefeedfish1H[4] = 17;
    timefeedfish1H[5] = 0;
    timefeedfish1H[6] = 0;
    timefeedfish1H[7] = 0;
    timefeedfish1M[0] = 0;
    timefeedfish1M[1] = 0;
    timefeedfish1M[2] = 0;
    timefeedfish1M[3] = 0;
    timefeedfish1M[4] = 0;
    timefeedfish1M[5] = 0;
    timefeedfish1M[6] = 0;
    timefeedfish1M[7] = 0;
    p2_n0.setValue(timefeedfish1H[0]);
    p2_n1.setValue(timefeedfish1M[0]);
    p2_n2.setValue(timefeedfish1H[1]);
    p2_n3.setValue(timefeedfish1M[1]);
    p2_n4.setValue(timefeedfish1H[2]);
    p2_n5.setValue(timefeedfish1M[2]);
    p2_n6.setValue(timefeedfish1H[3]);
    p2_n7.setValue(timefeedfish1M[3]);
    p2_n8.setValue(timefeedfish1H[4]);
    p2_n9.setValue(timefeedfish1M[4]);
    p2_n10.setValue(timefeedfish1H[5]);
    p2_n11.setValue(timefeedfish1M[5]);
    p2_n12.setValue(timefeedfish1H[6]);
    p2_n13.setValue(timefeedfish1M[6]);
    p2_n14.setValue(timefeedfish1H[7]);
    p2_n15.setValue(timefeedfish1M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
  }else if (fishmode == 1) {
    p2.show();
    page = 2;
    showtime();
    feed[fishmode] = 0;
    timefeedfish2H[0] = 8;
    timefeedfish2H[1] = 10;
    timefeedfish2H[2] = 12;
    timefeedfish2H[3] = 16;
    timefeedfish2H[4] = 18;
    timefeedfish2H[5] = 0;
    timefeedfish2H[6] = 0;
    timefeedfish2H[7] = 0;
    timefeedfish2M[0] = 0;
    timefeedfish2M[1] = 0;
    timefeedfish2M[2] = 0;
    timefeedfish2M[3] = 0;
    timefeedfish2M[4] = 0;
    timefeedfish2M[5] = 0;
    timefeedfish2M[6] = 0;
    timefeedfish2M[7] = 0;
    p2_n0.setValue(timefeedfish2H[0]);
    p2_n1.setValue(timefeedfish2M[0]);
    p2_n2.setValue(timefeedfish2H[1]);
    p2_n3.setValue(timefeedfish2M[1]);
    p2_n4.setValue(timefeedfish2H[2]);
    p2_n5.setValue(timefeedfish2M[2]);
    p2_n6.setValue(timefeedfish2H[3]);
    p2_n7.setValue(timefeedfish2M[3]);
    p2_n8.setValue(timefeedfish2H[4]);
    p2_n9.setValue(timefeedfish2M[4]);
    p2_n10.setValue(timefeedfish2H[5]);
    p2_n11.setValue(timefeedfish2M[5]);
    p2_n12.setValue(timefeedfish2H[6]);
    p2_n13.setValue(timefeedfish2M[6]);
    p2_n14.setValue(timefeedfish2H[7]);
    p2_n15.setValue(timefeedfish2M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
  }else if (fishmode == 2) {
    p2.show();
    page = 2;
    showtime();
    feed[fishmode] = 0;
    timefeedfish3H[0] = 0;
    timefeedfish3H[1] = 0;
    timefeedfish3H[2] = 0;
    timefeedfish3H[3] = 0;
    timefeedfish3H[4] = 0;
    timefeedfish3H[5] = 0;
    timefeedfish3H[6] = 0;
    timefeedfish3H[7] = 0;
    timefeedfish3M[0] = 0;
    timefeedfish3M[1] = 0;
    timefeedfish3M[2] = 0;
    timefeedfish3M[3] = 0;
    timefeedfish3M[4] = 0;
    timefeedfish3M[5] = 0;
    timefeedfish3M[6] = 0;
    timefeedfish3M[7] = 0;
    p2_n0.setValue(timefeedfish3H[0]);
    p2_n1.setValue(timefeedfish3M[0]);
    p2_n2.setValue(timefeedfish3H[1]);
    p2_n3.setValue(timefeedfish3M[1]);
    p2_n4.setValue(timefeedfish3H[2]);
    p2_n5.setValue(timefeedfish3M[2]);
    p2_n6.setValue(timefeedfish3H[3]);
    p2_n7.setValue(timefeedfish3M[3]);
    p2_n8.setValue(timefeedfish3H[4]);
    p2_n9.setValue(timefeedfish3M[4]);
    p2_n10.setValue(timefeedfish3H[5]);
    p2_n11.setValue(timefeedfish3M[5]);
    p2_n12.setValue(timefeedfish3H[6]);
    p2_n13.setValue(timefeedfish3M[6]);
    p2_n14.setValue(timefeedfish3H[7]);
    p2_n15.setValue(timefeedfish3M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
  }
}

void p2_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  debugbt();
  p4.show();
  page = 4;
  showtime();
  started = true;
  start();
}


void p2_b4_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  feed[fishmode] += 1;
  if (feed[fishmode] > 99) {
    feed[fishmode] = 0;
  }
  p2_n16.setValue(feed[fishmode]);
}


void p2_b5_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  feed[fishmode] -= 1;
  if (feed[fishmode] < 0) {
    feed[fishmode] = 99;
  }
  p2_n16.setValue(feed[fishmode]);
}


void p2_b6_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timefeedchange[0] += 1;
  if (timefeedchange[0] > 23) {
    timefeedchange[0] = 0;
  }
  p2_n17.setValue(timefeedchange[0]);
}


void p2_b7_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timefeedchange[0] -= 1;
  if (timefeedchange[0] < 0) {
    timefeedchange[0] = 23;
  }
  p2_n17.setValue(timefeedchange[0]);
}


void p2_b8_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timefeedchange[1] += 1;
  if (timefeedchange[1] > 59) {
    timefeedchange[1] = 0;
  }
  p2_n18.setValue(timefeedchange[1]);
}


void p2_b9_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timefeedchange[1] -= 1;
  if (timefeedchange[1] < 0) {
    timefeedchange[1] = 59;
  }
  p2_n18.setValue(timefeedchange[1]);
}


void p2_b10_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_t13.setText("-");
  if (fishmode == 0) {
    timefeedfish1H[timechange] = timefeedchange[0];
    timefeedfish1M[timechange] = timefeedchange[1];
    timefeedchange[0] = 0;
    timefeedchange[1] = 0;
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
    if (timechange == 0) {
      p2_n0.setValue(timefeedfish1H[timechange]);
      p2_n1.setValue(timefeedfish1M[timechange]);
    }else if (timechange == 1) {
      p2_n2.setValue(timefeedfish1H[timechange]);
      p2_n3.setValue(timefeedfish1M[timechange]);
    }else if (timechange == 2) {
      p2_n4.setValue(timefeedfish1H[timechange]);
      p2_n5.setValue(timefeedfish1M[timechange]);
    }else if (timechange == 3) {
      p2_n6.setValue(timefeedfish1H[timechange]);
      p2_n7.setValue(timefeedfish1M[timechange]);
    }else if (timechange == 4) {
      p2_n8.setValue(timefeedfish1H[timechange]);
      p2_n9.setValue(timefeedfish1M[timechange]);
    }else if (timechange == 5) {
      p2_n10.setValue(timefeedfish1H[timechange]);
      p2_n11.setValue(timefeedfish1M[timechange]);
    }else if (timechange == 6) {
      p2_n12.setValue(timefeedfish1H[timechange]);
      p2_n13.setValue(timefeedfish1M[timechange]);
    }else if (timechange == 7) {
      p2_n14.setValue(timefeedfish1H[timechange]);
      p2_n15.setValue(timefeedfish1M[timechange]);
    }
  }else if (fishmode == 1) {
    timefeedfish2H[timechange] = timefeedchange[0];
    timefeedfish2M[timechange] = timefeedchange[1];
    timefeedchange[0] = 0;
    timefeedchange[1] = 0;
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
    if (timechange == 0) {
      p2_n0.setValue(timefeedfish2H[timechange]);
      p2_n1.setValue(timefeedfish2M[timechange]);
    }else if (timechange == 1) {
      p2_n2.setValue(timefeedfish2H[timechange]);
      p2_n3.setValue(timefeedfish2M[timechange]);
    }else if (timechange == 2) {
      p2_n4.setValue(timefeedfish2H[timechange]);
      p2_n5.setValue(timefeedfish2M[timechange]);
    }else if (timechange == 3) {
      p2_n6.setValue(timefeedfish2H[timechange]);
      p2_n7.setValue(timefeedfish2M[timechange]);
    }else if (timechange == 4) {
      p2_n8.setValue(timefeedfish2H[timechange]);
      p2_n9.setValue(timefeedfish2M[timechange]);
    }else if (timechange == 5) {
      p2_n10.setValue(timefeedfish2H[timechange]);
      p2_n11.setValue(timefeedfish2M[timechange]);
    }else if (timechange == 6) {
      p2_n12.setValue(timefeedfish2H[timechange]);
      p2_n13.setValue(timefeedfish2M[timechange]);
    }else if (timechange == 7) {
      p2_n14.setValue(timefeedfish2H[timechange]);
      p2_n15.setValue(timefeedfish2M[timechange]);
    }
  }else if (fishmode == 2) {
    timefeedfish3H[timechange] = timefeedchange[0];
    timefeedfish3M[timechange] = timefeedchange[1];
    timefeedchange[0] = 0;
    timefeedchange[1] = 0;
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
    if (timechange == 0) {
      p2_n0.setValue(timefeedfish3H[timechange]);
      p2_n1.setValue(timefeedfish3M[timechange]);
    }else if (timechange == 1) {
      p2_n2.setValue(timefeedfish3H[timechange]);
      p2_n3.setValue(timefeedfish3M[timechange]);
    }else if (timechange == 2) {
      p2_n4.setValue(timefeedfish3H[timechange]);
      p2_n5.setValue(timefeedfish3M[timechange]);
    }else if (timechange == 3) {
      p2_n6.setValue(timefeedfish3H[timechange]);
      p2_n7.setValue(timefeedfish3M[timechange]);
    }else if (timechange == 4) {
      p2_n8.setValue(timefeedfish3H[timechange]);
      p2_n9.setValue(timefeedfish3M[timechange]);
    }else if (timechange == 5) {
      p2_n10.setValue(timefeedfish3H[timechange]);
      p2_n11.setValue(timefeedfish3M[timechange]);
    }else if (timechange == 6) {
      p2_n12.setValue(timefeedfish3H[timechange]);
      p2_n13.setValue(timefeedfish3M[timechange]);
    }else if (timechange == 7) {
      p2_n14.setValue(timefeedfish3H[timechange]);
      p2_n15.setValue(timefeedfish3M[timechange]);
    }
  }
  timechange = 8;
}

void p2_b11_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_t13.setText("-");
  timechange = 8;
  timefeedchange[0] = 0;
  timefeedchange[1] = 0;
  p2_n17.setValue(timefeedchange[0]);
  p2_n18.setValue(timefeedchange[1]);
}


void p2_b12_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 0;
  p2_t13.setText("round1");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}


void p2_b13_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 1;
  p2_t13.setText("round2");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}

void p2_b14_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 2;
  p2_t13.setText("round3");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}

void p2_b15_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 3;
  p2_t13.setText("round4");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}

void p2_b16_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 4;
  p2_t13.setText("round5");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}

void p2_b17_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 5;
  p2_t13.setText("round6");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}

void p2_b18_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 6;
  p2_t13.setText("round7");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}

void p2_b19_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  timechange = 7;
  p2_t13.setText("round8");
  if (fishmode == 0) {
    timefeedchange[0] = timefeedfish1H[timechange];
    timefeedchange[1] = timefeedfish1M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 1) {
    timefeedchange[0] = timefeedfish2H[timechange];
    timefeedchange[1] = timefeedfish2M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }else if (fishmode == 2) {
    timefeedchange[0] = timefeedfish3H[timechange];
    timefeedchange[1] = timefeedfish3M[timechange];
    p2_n17.setValue(timefeedchange[0]);
    p2_n18.setValue(timefeedchange[1]);
  }
}

void p2_b20_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}

void p2_b21_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}

void p2_b22_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}

void p2_bt0_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt0.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[0] = 0;
        p2_bt0.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[0] = 1;
        p2_bt0.setText("on");
        p2_n19.setValue(rounds);
    }
    debugbt();
}

void p2_bt1_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt1.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[1] = 0;
        p2_bt1.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[1] = 1;
        p2_bt1.setText("on");
        p2_n19.setValue(rounds);
    }
  debugbt();
}

void p2_bt2_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt2.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[2] = 0;
        p2_bt2.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[2] = 1;
        p2_bt2.setText("on");
        p2_n19.setValue(rounds);
    }
    debugbt();
}

void p2_bt3_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt3.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[3] = 0;
        p2_bt3.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[3] = 1;
        p2_bt3.setText("on");
        p2_n19.setValue(rounds);
    }
    debugbt();
}

void p2_bt4_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt4.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[4] = 0;
        p2_bt4.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[4] = 1;
        p2_bt4.setText("on");
        p2_n19.setValue(rounds);
    }
    debugbt();
}

void p2_bt5_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt5.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[5] = 0;
        p2_bt5.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[5] = 1;
        p2_bt5.setText("on");
        p2_n19.setValue(rounds);
    }
    debugbt();
}

void p2_bt6_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt6.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[6] = 0;
        p2_bt6.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[6] = 1;
        p2_bt6.setText("on");
        p2_n19.setValue(rounds);
    }
    debugbt();
}

void p2_bt7_PopCallback(void *ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p2_bt7.getValue(&dual_state);
    if(dual_state) {
        rounds -= 1;
        ofcheck[7] = 0;
        p2_bt7.setText("off");
        p2_n19.setValue(rounds);
    }
    else {
        rounds += 1;
        ofcheck[7] = 1;
        p2_bt7.setText("on");
        p2_n19.setValue(rounds);
    }
    debugbt();
}
//**************************************************PAGE 3**************************************************//
void p3_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p3_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p3_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p4.show();
  page =4;
  showtime();
  started = true;
  start();
}

void p3_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  feed[3] += 1;
  if (feed[3] > 99) {
    feed[3] = 0;
  }
  p3_n0.setValue(feed[3]);
}

void p3_b4_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  feed[3] -= 1;
  if (feed[3] < 0) {
    feed[3] = 99;
  }
  p3_n0.setValue(feed[3]);
}
//**************************************************PAGE 4**************************************************//
void p4_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  started = false;
  p0.show();
  page = 0;
  showtime();
}

void p4_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  started = false;
}
//**************************************************PAGE 5**************************************************//
void p5_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  started = false;
  p0.show();
  page = 0;
  showtime();
}

void p5_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  started = false;
}
//**************************************************PAGE 6**************************************************//
void p6_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  started = false;
  p0.show();
  page = 0;
  showtime();
}

void p6_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  started = false;
}
//**************************************************PAGE 7**************************************************//
void p7_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p7_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p7_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  if ((input_password[0] == password[0]) && (input_password[1] == password[1]) && (input_password[2] == password[2]) && (input_password[3] == password[3])) {
    p8.show();
    page = 8;
    showtime();
  }else {
    p7_t2.setText("Password is incorrect");
    for (int i = 0;i <= 3;i++) {
    input_password[i] = 0;
    }
    p7_n0.setValue(input_password[0]);
    p7_n1.setValue(input_password[1]);
    p7_n2.setValue(input_password[2]);
    p7_n3.setValue(input_password[3]);
  }
}

void p7_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[0] += 1;
  if (input_password[0] > 9) {
    input_password[0] = 0;
  }
  p7_n0.setValue(input_password[0]);
}

void p7_b4_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[0] -= 1;
  if (input_password[0] < 0) {
    input_password[0] = 9;
  }
  p7_n0.setValue(input_password[0]);
}

void p7_b5_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[1] += 1;
  if (input_password[1] > 9) {
    input_password[1] = 0;
  }
  p7_n1.setValue(input_password[1]);
}

void p7_b6_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[1] -= 1;
  if (input_password[1] < 0) {
    input_password[1] = 9;
  }
  p7_n1.setValue(input_password[1]);
}

void p7_b7_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[2] += 1;
  if (input_password[2] > 9) {
    input_password[2] = 0;
  }
  p7_n2.setValue(input_password[2]);
}

void p7_b8_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[2] -= 1;
  if (input_password[2] < 0) {
    input_password[2] = 9;
  }
  p7_n2.setValue(input_password[2]);
}

void p7_b9_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[3] += 1;
  if (input_password[3] > 9) {
    input_password[3] = 0;
  }
  p7_n3.setValue(input_password[3]);
}

void p7_b10_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  input_password[3] -= 1;
  if (input_password[3] < 0) {
    input_password[3] = 9;
  }
  p7_n3.setValue(input_password[3]);
}
//**************************************************PAGE 8**************************************************//
void p8_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p8_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p7.show();
  page = 7;
  showtime();
  for (int i = 0;i <= 3;i++) {
    input_password[i] = 0;
  }
  p7_n0.setValue(input_password[0]);
  p7_n1.setValue(input_password[1]);
  p7_n2.setValue(input_password[2]);
  p7_n3.setValue(input_password[3]);
}

void p8_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p9.show();
  page = 9;
  showtime();
  changetime[0] = H;
  changetime[1] = M;
  p9_n0.setValue(changetime[0]);
  p9_n1.setValue(changetime[1]);
}

void p8_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p10.show();
  page = 10;
  showtime();
}

void p8_b4_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p11.show();
  page = 11;
  showtime();
  test = true;
  while (test == true) {
    loadcell();
    p11_n0.setValue(g);
    nlisten();
  }
}

void p8_b5_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p12.show();
  page = 12;
  showtime();
}

void p8_b6_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p13.show();
  page = 13;
  showtime();
}

void p8_b7_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p14.show();
  page = 14;
  showtime();
  test = true;
  while (test == true) {
    ultrasonic();
    p14_n0.setValue(cm);
    nlisten();
  }
}

void p8_b8_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p15.show();
  page = 15;
  showtime();
}
//**************************************************PAGE 9**************************************************//
void p9_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p9_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p8.show();
  page = 8;
  showtime();
}

void p9_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  rtc.adjust(DateTime(2020, 12, 1, changetime[0], changetime[1], 40)); //เอาไว้ตั้งเวลา RTC
  showtime();
}

void p9_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  changetime[0] += 1;
  if (changetime[0] > 23) {
    changetime[0] = 0;
  }
  p9_n0.setValue(changetime[0]);
  Serial.print(changetime[0]);
}

void p9_b4_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  changetime[0] -= 1;
  if (changetime[0] < 0) {
    changetime[0] = 23;
  }
  p9_n0.setValue(changetime[0]);
  Serial.print(changetime[0]);
}

void p9_b5_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  changetime[1] += 1;
  if (changetime[1] > 59) {
    changetime[1] = 0;
  }
  p9_n1.setValue(changetime[1]);
  Serial.print(changetime[1]);
}

void p9_b6_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  changetime[1] -= 1;
  if (changetime[1] < 0) {
    changetime[1] = 59;
  }
  p9_n1.setValue(changetime[1]);
  Serial.print(changetime[1]);
}
//**************************************************PAGE 10**************************************************//
void p10_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p10_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p8.show();
  page = 8;
  showtime();
}

void p10_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  stepmotor_forward();
}

void p10_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  stepmotor_reward();
}
//**************************************************PAGE 11**************************************************//
void p11_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
  test = false;
}

void p11_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p8.show();
  page = 8;
  showtime();
  test = false;
}

void p11_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  zero =+ g;
}
//**************************************************PAGE 12**************************************************//
void p12_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p12_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p8.show();
  page = 8;
  showtime();
}

void p12_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  servo_open();
}

void p12_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  servo_close();
}
//**************************************************PAGE 13**************************************************//
void p13_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void p13_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p8.show();
  page = 8;
  showtime();
}

void p13_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  red();
  delay(1000);
  white();
}

void p13_b3_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  yellow();
  delay(1000);
  white();
}

void p13_b4_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  green();
  delay(1000);
  white();
}
//**************************************************PAGE 14**************************************************//
void p14_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
  test = false;
}

void p14_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p8.show();
  page = 8;
  showtime();
  test = false;
}
//**************************************************PAGE 15**************************************************//
void  p15_b0_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p0.show();
  page = 0;
  showtime();
}

void  p15_b1_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  p8.show();
  page = 8;
  showtime();
}

void  p15_b2_Press(void*ptr) {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  esc_run();
  esc_stop(); 
  
}

void setup() {
  Serial.begin(115200);
  #ifndef ESP32
   while (!Serial);
  #endif

  //nextion
  nexInit();
  //**************************************************PAGE 0**************************************************//
  p0_b0.attachPush(p0_b0_Press, &p0_b0);
  p0_b1.attachPush(p0_b1_Press, &p0_b1);
  p0_b2.attachPush(p0_b2_Press, &p0_b2);
  //**************************************************PAGE 1**************************************************//
  p1_b0.attachPush(p1_b0_Press, &p1_b0);
  p1_b1.attachPush(p1_b1_Press, &p1_b1);
  p1_b2.attachPush(p1_b2_Press, &p1_b2);
  p1_b3.attachPush(p1_b3_Press, &p1_b3);
  p1_b4.attachPush(p1_b4_Press, &p1_b4);
  //**************************************************PAGE 2**************************************************//
  p2_b0.attachPush(p2_b0_Press, &p2_b0);
  p2_b1.attachPush(p2_b1_Press, &p2_b1);
  p2_b2.attachPush(p2_b2_Press, &p2_b2);
  p2_b3.attachPush(p2_b3_Press, &p2_b3);
  p2_b4.attachPush(p2_b4_Press, &p2_b4);
  p2_b5.attachPush(p2_b5_Press, &p2_b5);
  p2_b6.attachPush(p2_b6_Press, &p2_b6);
  p2_b7.attachPush(p2_b7_Press, &p2_b7);
  p2_b8.attachPush(p2_b8_Press, &p2_b8);
  p2_b9.attachPush(p2_b9_Press, &p2_b9);
  p2_b10.attachPush(p2_b10_Press, &p2_b10);
  p2_b11.attachPush(p2_b11_Press, &p2_b11);
  p2_b12.attachPush(p2_b12_Press, &p2_b12);
  p2_b13.attachPush(p2_b13_Press, &p2_b13);
  p2_b14.attachPush(p2_b14_Press, &p2_b14);
  p2_b15.attachPush(p2_b15_Press, &p2_b15);
  p2_b16.attachPush(p2_b16_Press, &p2_b16);
  p2_b17.attachPush(p2_b17_Press, &p2_b17);
  p2_b18.attachPush(p2_b18_Press, &p2_b18);
  p2_b19.attachPush(p2_b19_Press, &p2_b19);
  p2_bt0.attachPush(p2_bt0_PopCallback, &p2_bt0);
  p2_bt1.attachPush(p2_bt1_PopCallback, &p2_bt1);
  p2_bt2.attachPush(p2_bt2_PopCallback, &p2_bt2);
  p2_bt3.attachPush(p2_bt3_PopCallback, &p2_bt3);
  p2_bt4.attachPush(p2_bt4_PopCallback, &p2_bt4);
  p2_bt5.attachPush(p2_bt5_PopCallback, &p2_bt5);
  p2_bt6.attachPush(p2_bt6_PopCallback, &p2_bt6);
  p2_bt7.attachPush(p2_bt7_PopCallback, &p2_bt7);
  //**************************************************PAGE 3**************************************************//
  p3_b0.attachPush(p3_b0_Press, &p3_b0);
  p3_b1.attachPush(p3_b1_Press, &p3_b1);
  p3_b2.attachPush(p3_b2_Press, &p3_b2);
  p3_b3.attachPush(p3_b3_Press, &p3_b3);
  p3_b4.attachPush(p3_b4_Press, &p3_b4);
  //**************************************************PAGE 4**************************************************//
  p4_b0.attachPush(p4_b0_Press, &p4_b0);
  p4_b1.attachPush(p4_b1_Press, &p4_b1);
  //**************************************************PAGE 5**************************************************//
  p5_b0.attachPush(p5_b0_Press, &p5_b0);
  p5_b1.attachPush(p5_b1_Press, &p5_b1);
  //**************************************************PAGE 6**************************************************//
  p6_b0.attachPush(p6_b0_Press, &p6_b0);
  p6_b1.attachPush(p6_b1_Press, &p6_b1);
  //**************************************************PAGE 7**************************************************//
  p7_b0.attachPush(p7_b0_Press, &p7_b0);
  p7_b1.attachPush(p7_b1_Press, &p7_b1);
  p7_b2.attachPush(p7_b2_Press, &p7_b2);
  p7_b3.attachPush(p7_b3_Press, &p7_b3);
  p7_b4.attachPush(p7_b4_Press, &p7_b4);
  p7_b5.attachPush(p7_b5_Press, &p7_b5);
  p7_b6.attachPush(p7_b6_Press, &p7_b6);
  p7_b7.attachPush(p7_b7_Press, &p7_b7);
  p7_b8.attachPush(p7_b8_Press, &p7_b8);
  p7_b9.attachPush(p7_b9_Press, &p7_b9);
  p7_b10.attachPush(p7_b10_Press, &p7_b10);
  //**************************************************PAGE 8**************************************************//
  p8_b0.attachPush(p8_b0_Press, &p8_b0);
  p8_b1.attachPush(p8_b1_Press, &p8_b1);
  p8_b2.attachPush(p8_b2_Press, &p8_b2);
  p8_b3.attachPush(p8_b3_Press, &p8_b3);
  p8_b4.attachPush(p8_b4_Press, &p8_b4);
  p8_b5.attachPush(p8_b5_Press, &p8_b5);
  p8_b6.attachPush(p8_b6_Press, &p8_b6);
  p8_b7.attachPush(p8_b7_Press, &p8_b7);
  p8_b8.attachPush(p8_b8_Press, &p8_b8);
  //**************************************************PAGE 9**************************************************//
  p9_b0.attachPush(p9_b0_Press, &p9_b0);
  p9_b1.attachPush(p9_b1_Press, &p9_b1);
  p9_b2.attachPush(p9_b2_Press, &p9_b2);
  p9_b3.attachPush(p9_b3_Press, &p9_b3);
  p9_b4.attachPush(p9_b4_Press, &p9_b4);
  p9_b5.attachPush(p9_b5_Press, &p9_b5);
  p9_b6.attachPush(p9_b6_Press, &p9_b6);
  //**************************************************PAGE 10**************************************************//
  p10_b0.attachPush(p10_b0_Press, &p10_b0);
  p10_b1.attachPush(p10_b1_Press, &p10_b1);
  p10_b2.attachPush(p10_b2_Press, &p10_b2);
  p10_b3.attachPush(p10_b3_Press, &p10_b3);
  //**************************************************PAGE 11**************************************************//
  p11_b0.attachPush(p11_b0_Press, &p11_b0);
  p11_b1.attachPush(p11_b1_Press, &p11_b1);
  p11_b2.attachPush(p11_b2_Press, &p11_b2);
  //**************************************************PAGE 12**************************************************//
  p12_b0.attachPush(p12_b0_Press, &p12_b0);
  p12_b1.attachPush(p12_b1_Press, &p12_b1);
  p12_b2.attachPush(p12_b2_Press, &p12_b2);
  p12_b3.attachPush(p12_b3_Press, &p12_b3);
  //**************************************************PAGE 13**************************************************//
  p13_b0.attachPush(p13_b0_Press, &p13_b0);
  p13_b1.attachPush(p13_b1_Press, &p13_b1);
  p13_b2.attachPush(p13_b2_Press, &p13_b2);
  p13_b3.attachPush(p13_b3_Press, &p13_b3);
  p13_b4.attachPush(p13_b4_Press, &p13_b4);
  //**************************************************PAGE 14**************************************************//
  p14_b0.attachPush(p14_b0_Press, &p14_b0);
  p14_b1.attachPush(p14_b1_Press, &p14_b1);
  //**************************************************PAGE 15**************************************************//
  p15_b0.attachPush(p15_b0_Press, &p15_b0);
  p15_b1.attachPush(p15_b1_Press, &p15_b1);
  p15_b2.attachPush(p15_b2_Press, &p15_b2);

  //servo
  ledcAttachPin(12, 1);
  ledcSetup(1, 490, 10);

  //esc
  ledcAttachPin(13, 2);
  ledcSetup(2, 490, 10);
  
  //stepmotor
  ledcAttachPin(25, 3);
  ledcSetup(3, 490, 10);
  
  //RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // ตั้งเวลา(ปปปป,ดด,วว,ชช,นน,วว)
  rtc.adjust(DateTime(2020, 12, 1, 9, 28, 40)); //เอาไว้ตั้งเวลา RTC

  //ultrasonics
  pinMode(UltrasonicPin[0],OUTPUT);
  pinMode(UltrasonicPin[1],INPUT);

  //loadcell
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  //rgb
  mcp.begin(0x27);
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(5, OUTPUT);

  //limit
  mcp.pinMode(6, INPUT);
  
  //ledboard
  mcp.pinMode(7, OUTPUT);
  mcp.pinMode(8, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(25,OUTPUT);

  //any
  millisnow = millis();
  showtime();
  servo_close();
  Serial.println("start!!!");
}

//nextion
void nlisten() {
  for (int i = 0;i <= 25;i++) {
      nexLoop(nex_listen_list);
      delay(10);
    }
}

//runing
void start() {
  feedingcheck = false;
  pagecheck = false;
  while (started == true) {
    nlisten();
    checkfeedstand();
    limit();
    if (fishmode != 3 && started == true && limitcheck == 1) {
      if (zerofeedstand == false) {
        checktime();
        limit();
      }else if (zerofeedstand == true) {
        start();
      }
    }else if (fishmode == 3 && started == true && limitcheck == 1) {
      checkfeeding();
    }
  }
  if (page == 0 && pagecheck == false) {
    pagecheck = true;
  }else if (modes == true && pagecheck == false) {
    if (fishmode == 0 && pagecheck == false) {
    p2.show();
    page = 2;
    showtime();
    fishmode = 0;
    rounds = 8;
    ofcheck[0] = 1;
    ofcheck[1] = 1;
    ofcheck[2] = 1;
    ofcheck[3] = 1;
    ofcheck[4] = 1;
    ofcheck[5] = 1;
    ofcheck[6] = 1;
    ofcheck[7] = 1;
    timechange = 8;
    p2_t13.setText("-");
    p2_n0.setValue(timefeedfish1H[0]);
    p2_n1.setValue(timefeedfish1M[0]);
    p2_n2.setValue(timefeedfish1H[1]);
    p2_n3.setValue(timefeedfish1M[1]);
    p2_n4.setValue(timefeedfish1H[2]);
    p2_n5.setValue(timefeedfish1M[2]);
    p2_n6.setValue(timefeedfish1H[3]);
    p2_n7.setValue(timefeedfish1M[3]);
    p2_n8.setValue(timefeedfish1H[4]);
    p2_n9.setValue(timefeedfish1M[4]);
    p2_n10.setValue(timefeedfish1H[5]);
    p2_n11.setValue(timefeedfish1M[5]);
    p2_n12.setValue(timefeedfish1H[6]);
    p2_n13.setValue(timefeedfish1M[6]);
    p2_n14.setValue(timefeedfish1H[7]);
    p2_n15.setValue(timefeedfish1M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
    pagecheck = true;
  }else if (fishmode == 1 && pagecheck == false) {
    p2.show();
    page = 2;
    showtime();
    fishmode = 1;
    rounds = 8;
    ofcheck[0] = 1;
    ofcheck[1] = 1;
    ofcheck[2] = 1;
    ofcheck[3] = 1;
    ofcheck[4] = 1;
    ofcheck[5] = 1;
    ofcheck[6] = 1;
    ofcheck[7] = 1;
    timechange = 8;
    p2_t13.setText("-");
    p2_n0.setValue(timefeedfish2H[0]);
    p2_n1.setValue(timefeedfish2M[0]);
    p2_n2.setValue(timefeedfish2H[1]);
    p2_n3.setValue(timefeedfish2M[1]);
    p2_n4.setValue(timefeedfish2H[2]);
    p2_n5.setValue(timefeedfish2M[2]);
    p2_n6.setValue(timefeedfish2H[3]);
    p2_n7.setValue(timefeedfish2M[3]);
    p2_n8.setValue(timefeedfish2H[4]);
    p2_n9.setValue(timefeedfish2M[4]);
    p2_n10.setValue(timefeedfish2H[5]);
    p2_n11.setValue(timefeedfish2M[5]);
    p2_n12.setValue(timefeedfish2H[6]);
    p2_n13.setValue(timefeedfish2M[6]);
    p2_n14.setValue(timefeedfish2H[7]);
    p2_n15.setValue(timefeedfish2M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
    pagecheck = true;
  }else if (fishmode == 2 && pagecheck == false) {
    p2.show();
    page = 2;
    showtime();
    fishmode = 2;
    rounds = 8;
    ofcheck[0] = 1;
    ofcheck[1] = 1;
    ofcheck[2] = 1;
    ofcheck[3] = 1;
    ofcheck[4] = 1;
    ofcheck[5] = 1;
    ofcheck[6] = 1;
    ofcheck[7] = 1;
    timechange = 8;
    p2_t13.setText("-");
    p2_n0.setValue(timefeedfish3H[0]);
    p2_n1.setValue(timefeedfish3M[0]);
    p2_n2.setValue(timefeedfish3H[1]);
    p2_n3.setValue(timefeedfish3M[1]);
    p2_n4.setValue(timefeedfish3H[2]);
    p2_n5.setValue(timefeedfish3M[2]);
    p2_n6.setValue(timefeedfish3H[3]);
    p2_n7.setValue(timefeedfish3M[3]);
    p2_n8.setValue(timefeedfish3H[4]);
    p2_n9.setValue(timefeedfish3M[4]);
    p2_n10.setValue(timefeedfish3H[5]);
    p2_n11.setValue(timefeedfish3M[5]);
    p2_n12.setValue(timefeedfish3H[6]);
    p2_n13.setValue(timefeedfish3M[6]);
    p2_n14.setValue(timefeedfish3H[7]);
    p2_n15.setValue(timefeedfish3M[7]);
    p2_n16.setValue(feed[fishmode]);
    p2_n19.setValue(rounds);
    pagecheck = true;
  }
  }else if (modes == false && pagecheck == false) {
    p3.show();
    page = 3;
    showtime();
    p3_n0.setValue(feed[3]);
    pagecheck = true;
  }
}

void percent() {
  percentfeed = 100 - ((float(cm) / 50) * 100);
  Serial.print(percentfeed);
  Serial.print("%");
  limit();
  if (page == 4 && started == true && limitcheck == 1) {
    p4_n0.setValue(percentfeed);
  }else if (page == 5 && started == true) {
    p5_n0.setValue(percentfeed);
  }else if (page == 6 && started == true) {
    p6_n0.setValue(percentfeed);
  }
}

void checkfeedstand() {
  ultrasonic();
  limit();
  nlisten();
  if (cm > 25 && cm < 45 && started == true && limitcheck == 1) {
    yellow();
    if (zerofeedstand == true && started == true && limitcheck == 1) {
      p4_t1.setText("started");
      zerofeedstand = false;
      percent();
      showtime();
      nlisten();
    }
  }else if (cm >= 45 && started == true && limitcheck == 1) {
    red();
    p4_t1.setText("don't have feed stand");
    showtime();
    percent();
    zerofeedstand = true;
    nlisten();
    start();
  }else if (cm <= 25 && started == true && limitcheck == 1){
    green();
    limit();
    if (zerofeedstand == true && started == true && limitcheck == 1) {
      p4_t1.setText("started");
      zerofeedstand = false;
      percent();
      showtime();
      nlisten();
    }
  }
}

void checkfeeding() {
  loadcell();
  limit();
  if ((g < feed[fishmode]) && started == true && limitcheck == 1) {
    feedingcheck == true;
    nlisten();
    if (started == true && zerofeedstand == false)  {
    stepmotor_forward();
    }
    nlisten();
    if (started == true && zerofeedstand == false)  {
    stepmotor_reward();
    }
    if (fishmode == 3) {
      start();
    }else {
      checkfeeding();
    }
  }
  limit();
  while ((g > zero + 5 ) && started == true && limitcheck == 1) {
    nlisten();
    if (started == true)  {
    servo_open();
    }
    nlisten();
    if (started == true)  {
    esc_run();
    }
    loadcell();
  }
  esc_stop();
  servo_close();
  limit();
  if (fishmode == 3) {
    started = false;
    p3.show();
    page = 3;
    showtime();
    p3_n0.setValue(feed[fishmode]);
  }else if (fishmode != 3) {
    start();
  }
}

void only_check() {
  ultrasonic();
  if (cm > 25 && cm < 45) {
    yellow();
  }else if (cm >= 45) {
    red();
  }else if (cm <= 25){
    green();
  }
}

void loop() {
  delay(10);
  nexLoop(nex_listen_list);
  millisnow = millis();
  if ((millisnow - millisnext) > 5000) {
    Serial.println("working");
    millisnext = millisnow;
    showtime();
    only_check();
  }
}

// 0.O
