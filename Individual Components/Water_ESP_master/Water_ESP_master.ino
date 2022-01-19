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

bool is_water_level_dangerously_low = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Set serial port speed
  
}

void loop() {
  // put your main code here, to run repeatedly:
    if (Serial.available()){
      Serial.println((unsigned char)Serial.read());
    }
    Serial.println(15);
    //Serial.print("is water level low= ");Serial.println(is_water_level_dangerously_low);
}
