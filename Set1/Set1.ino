#include <ESP8266WiFi.h>
#include <TridentTD_LineNotify.h>
#define LINE_TOKEN "FN5TToF5Fi4r44VqgFKawhr7GKrg6lVCMlRynHuNtpq" //line token
#include "HX711.h"
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient client;
HX711 scale(D7, D8);
const char *ssid = "Acare_Arm"; // replace with your wifi ssid and wpa2 key
const char *pass = "wp221988";
char auth[] = "xdaX6kcCk-GihshC6pBwGiRr2piF_zti";    // You should get Auth Token in the Blynk App.
String apiKey = "9SAQWIB17KV9JR11";
const char* server = "api.thingspeak.com";
int timezone = 7 * 3600;                    //TimeZone ตามเวลาประเทศไทย
int dst = 0;                                
String data;
String text_In;
String text_Ins;
float numeric = V5;
float numerics = V6;
int analogPin = A0;
int val = 0;
float weight;
float calibration_factor = 107006;
 
void setup() {
  
  LINE.setToken(LINE_TOKEN);
  Serial.setDebugOutput(true);
  Serial.begin(115200);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
  Blynk.begin(auth, ssid, pass);
  Wire.begin(D2, D1);
  lcd.begin();
  lcd.setCursor(6,0);
  lcd.print("IOT");
  lcd.setCursor(1,1);
  lcd.print("Weighing Scale");
  delay(1500);
  lcd.clear();
  lcd.print("Connecting Wifi");
  
  WiFi.begin(ssid, pass); {
    
  delay(1500);
  Serial.print(".");
  lcd.clear();
  
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");     //ดึงเวลาจาก Server
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
  Serial.print(".");
  delay(1000);
  }
  Serial.println("");
  
}

BLYNK_WRITE(V1) {
  text_In = param.asStr();  // Text Input Widget - Strings
  Blynk.virtualWrite(V2, text_In);
  
}

BLYNK_WRITE(V3) {
  text_Ins = param.asStr();  // Text Input Widget - Strings
  Blynk.virtualWrite(V4, text_Ins);
  
}

BLYNK_WRITE(V5) {
   numeric = param.asFloat();
   
}

BLYNK_WRITE(V6) {
   numerics = param.asFloat();
   
}

 
void loop() {

  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  Serial.print(p_tm->tm_hour);
  Serial.print(":");
  Serial.print(p_tm->tm_min);
  Serial.print(":");
  Serial.println(p_tm->tm_sec);
  
  float weight = digitalRead(D7);
  val = analogRead(analogPin);

  Blynk.run();
  WidgetLCD lcds(V0);
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  weight = scale.get_units(5); 

  Blynk.virtualWrite(V7, weight);
  lcds.print(0,0,(String)"Weight: "+weight+" Kg");
  delay(100);
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" Kg");
  Serial.println();
  
  if (weight<=numeric) {

  String LineText;
  String string1 = "ปริมาณของบนชั้นวางที่1ใกล้หมดแล้ว ค่าน้ำหนัก: ";
  String string2 = " Kg";
  LineText = string1 + weight + string2;
  Serial.print("Line: ");
  Serial.println(LineText);
  LINE.notify(LineText);
  Blynk.notify(LineText);
  lcds.print(0,1,"Near Out Stock!!");
  delay(900000);
  lcds.clear();
  
  }

  if (weight>=numerics) {

  String LineText2;
  String string3 = "ปริมาณของบนชั้นวางที่1ใกล้เต็มแล้ว ค่าน้ำหนัก: ";
  String string4 = " Kg";
  LineText2 = string3 + weight + string4;
  Serial.print("Line: ");
  Serial.println(LineText2);
  LINE.notify(LineText2);
  Blynk.notify(LineText2);
  lcds.print(0,1,"Full Stock!!");
  delay(900000);
  lcds.clear();
  
  }
  
  if (val>500) { // ค่า เป็น 1 แสดงว่าตรวจพบวัตถุ
    Serial.println("Detect Object");
    lcd.setCursor(0, 0); // กำหนดให้ เคอร์เซอร์ อยู่ตัวอักษรตำแหน่งที่2 แถวที่ 1 เตรียมพิมพ์ข้อความ
    lcd.print(text_In); //พิมพ์ข้อความ
    lcd.setCursor(0, 1); // กำหนดให้ เคอร์เซอร์ อยู่ตัวอักษรกำแหน่งที0 แถวที่ 2 เตรียมพิมพ์ข้อความ
    lcd.print(text_Ins); //พิมพ์ข้อความ
    delay(5000);
    
  }
  
  else {
    
    Serial.println("No Object");
    lcd.setCursor(1, 0);
    lcd.print("Hello Welcome");
    lcd.setCursor(4, 1);
    lcd.print((String)""+weight+" Kg");
    delay(500);
    lcd.clear();
    
  }

  if (client.connect(server,80)) {
    String url = "/update?api_key="+ apiKey ;
           url += "&field1=" + String(weight);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" + 
                   "Connection: close\r\n\r\n");
    Serial.print("Weight from HX711 : ");
    Serial.print(weight);
    Serial.println("Kg");
    client.stop();
    Serial.print("Waiting for send data to thingspeak");
    delay(1500);   
  } 
  else {
    
    Serial.println("Can not send data to thingspeak");
    delay(86400000); 
    
  }
  
}
