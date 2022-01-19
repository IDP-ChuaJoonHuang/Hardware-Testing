#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

RTC_DS1307 RTC;     // Setup an instance of DS1307 naming it RTC
char Time[50];
int ss = 0;
int mm = 0;
int hh = 0;
int DD = 0;
int dd = 0;
int MM = 0;
int yyyy = 0;

void RTCSetup(){
  RTC.begin();  // Init RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date is expanded to date and time on your computer at compiletime
  #ifdef DEBUG_RTC
    Serial.print("Time and date set");
  #endif
}


void setup() {
  Serial.begin(9600); // Set serial port speed
  Wire.begin(); // Start the I2C
  RTCSetup();
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
    DateTime now = RTC.now();
    ss = now.second();
    mm = now.minute();
    hh = now.hour();
    DD = now.dayOfWeek();
    dd = now.day();
    MM = now.month();
    yyyy = now.year();
    sprintf(Time,"%d/%d/%d %d:%d:%d \n", yyyy,MM,dd,hh,mm,ss);
    Serial.println(Time);
}
