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
#define DATABASE_URL "fir-iot-ec360-default-rtdb.asia-southeast1.firebasedatabase.app/"
//#define SENDUBIDOTS

const char *WIFI_SSID = "你回来了";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "okvx4408";      // Put here your Wi-Fi password
const char* DEVICE_LABEL = "esp8266";      // device label
const char *UBIDOTS_TOKEN = "BBFF-w4V5RHXuY912iDpetacCwu0GCFAfuL";  // Put here your Ubidots TOKEN
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long lastmillis = millis();
int count = 0;
bool signupOK = false;
int prediction = 0;
float probability = 0;


void setup() {
   Serial.begin(115200); /* begin serial for debug */
   ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
   ubidots.setDebug(true);  // Uncomment this line for printing debug  messages                     

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
//   Wire.beginTransmission(8); /* begin with device address 8 */
//   Wire.write("Hello Arduino");  /* sends hello string */
//   Wire.endTransmission();    /* stop transmitting */
  #ifdef SENDUBIDOTS
  if ((millis() - lastmillis) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
    {
     ubidots.add("Is Machine Normal", prediction);
     ubidots.add("Probability", probability);
     bool bufferSent = false;
     bufferSent = ubidots.send(DEVICE_LABEL); // Will send data to a device label that matches the device Id
     if (bufferSent) {
     // Do something if values were sent properly
       Serial.println("Values sent by the device");
     }
     lastmillis = millis();
    }
  #endif
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 7000|| sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.getInt(&fbdo, "/Predictions/Predicted Machine Status")){
        Serial.print("Data type = "); Serial.println(fbdo.dataType());
        prediction = fbdo.intData();
        Serial.println(prediction);
      }
      else {
        //Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      count++;
      if (Firebase.RTDB.getFloat(&fbdo, "/Predictions/Probability")){
        Serial.print("Data type = "); Serial.println(fbdo.dataType());
        probability = fbdo.floatData();
        Serial.println(probability);
      }
      else {
        //Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
}
