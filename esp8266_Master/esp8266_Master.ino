#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "Ubidots.h"
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#define PUBLISH_FREQUENCY 5000
#define API_KEY "AIzaSyDEcPCgQuwvFKN88l4GLEG34RA2JUCOqiA"
#define DATABASE_URL "https://fir-iot-ec360-default-rtdb.asia-southeast1.firebasedatabase.app/"
//#define SENDFIREBASE
//#define SENDUBIDOTS

const char* UBIDOTS_TOKEN = "BBFF-w4V5RHXuY912iDpetacCwu0GCFAfuL";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "Guest-PTS";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "Potensi@guestii88";      // Put here your Wi-Fi password
const char* DEVICE_LABEL = "esp8266";      // device label


int is_oil_level_dangerous = 0;

uint8_t magnetic_switch_pin = D5;
int doorState = 0;

byte Re_buf[8], counter = 0;
unsigned char sign = 0;
double distance=0;
double min_distance = 0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long lastmillis = millis();
int count = 0;
bool signupOK = false;


Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

void checkingDistance()
{
  if (sign)
  {
    sign = 0;
    if (Re_buf[0] == 90)
    {
      // Checking checksum, for data corruption
      int sum = 0;
      for (int i = 0; i < 7; i++)
      {
        sum += Re_buf[i];
//        Serial.print(i);
//        Serial.print(". Sum:");
//        Serial.println(sum);
      }
//      Serial.print("Sum overflowed: ");
//      Serial.print(sum);
      while(sum >= 256)
      {
        sum -= 256;
      }
//      Serial.print("\tSum : ");
//      Serial.print(sum);
//      Serial.print("\tChecksum: ");
//      Serial.println(Re_buf[7]);
      if (Re_buf[7] == sum)
      {
        distance = (Re_buf[4] << 8) | Re_buf[5];
        if (distance < min_distance){
          min_distance = distance;
        }
//        Serial.print("Distance: ");
//        Serial.println(distance);
      }
    }
  }
}

void setup() {
   Serial.begin(115200);
   ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
   ubidots.setDebug(true);  // Uncomment this line for printing debug  messages   

  // Software serial 
  //Serial.begin(9600);

//  Serial.write(0xA5);  //continuos output
//  Serial.write(0x45);
//  Serial.write(0xEA);
//
//  Serial.write(0xA5);  //change to high precision mode
//  Serial.write(0x52);
//  Serial.write(0xF7);

  //Firebase
  config.api_key = API_KEY;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

    /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  #ifdef SENDUBIDOTS
    if ((millis() - lastmillis) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
      {
       Wire.requestFrom(8, 1); /* request & read data of size 13 from slave */
       while(Wire.available()){
          is_oil_level_dangerous = Wire.read();
        //Serial.print(c);
       }
      
       checkingDistance();
       doorState = digitalRead(magnetic_switch_pin); // read state
       ubidots.add("Is Oil Level Dangerous", is_oil_level_dangerous);
       ubidots.add("Door is open", doorState);
       ubidots.add("Distance to counterweight", distance);
       ubidots.add("Minimum distance to counterweight", min_distance);
       bool bufferSent = false;
       bufferSent = ubidots.send(DEVICE_LABEL); // Will send data to a device label that matches the device Id
       if (bufferSent) {
       // Do something if values were sent properly
         Serial.println("Values sent by the device");
       }
       lastmillis = millis();
      }
    #endif
    #ifndef SENDFIREBASE
      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 7000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.setInt(&fbdo, "test/Minimum to Counterweight", min_distance)){
        //Serial.println("PASSED");
        //Serial.println("PATH: " + fbdo.dataPath());
        //Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        //Serial.println("FAILED");
        //Serial.println("REASON: " + fbdo.errorReason());
      }
      count++;
      
      // Write an Float number on the database path test/float
      if (Firebase.RTDB.setInt(&fbdo, "test/Disrance to Counterweight", distance)){
        //Serial.println("PASSED");
        //Serial.println("PATH: " + fbdo.dataPath());
        //Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        //Serial.println("FAILED");
        //Serial.println("REASON: " + fbdo.errorReason());
      }
    
      if (Firebase.RTDB.setInt(&fbdo, "test/Is Door Closed", doorState)){
        //Serial.println("PASSED");
        //Serial.println("PATH: " + fbdo.dataPath());
        //Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        //Serial.println("FAILED");
        //Serial.println("REASON: " + fbdo.errorReason());
      }
    
      if (Firebase.RTDB.setInt(&fbdo, "test/Is Oil Level Dangerous", is_oil_level_dangerous)){
        //Serial.println("PASSED");
        //Serial.println("PATH: " + fbdo.dataPath());
        //Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        //Serial.println("FAILED");
        //Serial.println("REASON: " + fbdo.errorReason());
      }
      
    }
    #endif
}


void serialEvent() {
  while (Serial.available()) {
    Re_buf[counter] = Serial.read();
   // Serial.println(Re_buf[counter]);
    if (counter == 0 && Re_buf[0] != 90) return;
    counter++;
    if (counter == 8)
    {
      counter = 0;
      sign = 1;
    }
  }
}
