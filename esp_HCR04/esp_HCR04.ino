/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-hc-sr04-ultrasonic-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "Ubidots.h"
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#define PUBLISH_FREQUENCY 5000
#define API_KEY "AIzaSyDEcPCgQuwvFKN88l4GLEG34RA2JUCOqiA"
#define DATABASE_URL "https://fir-iot-ec360-default-rtdb.asia-southeast1.firebasedatabase.app/" 
const char* UBIDOTS_TOKEN = "BBFF-w4V5RHXuY912iDpetacCwu0GCFAfuL";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "ChinKeat MESA PRESIDENT";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "s201e6cb7d1";      // Put here your Wi-Fi password
const char* DEVICE_LABEL = "Proximity";      // device label
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);
#define SENDFIREBASE

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = millis();
int count = 0;
bool signupOK = false;


//const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
unsigned long timer;
volatile int check_millis_changed = millis();

const int trigPin = 12;
const int echoPin = 14;
float min_distance = 1000;
unsigned long lastmillis = millis();

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm = 1000;
//float distanceInch;

void readDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;
  if(distanceCm < min_distance){
    min_distance = distanceCm;
  }
  
  // Convert to inches
  //distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
}

void setup() {
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
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

  timer = millis();
}

void loop() {
  // Clears the trigPin
  if ((millis() - lastmillis) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    readDistance();
     ubidots.add("Distance", distanceCm);
     ubidots.add("Minimum distance to counterweight", min_distance);
     bool bufferSent = false;
     bufferSent = ubidots.send(DEVICE_LABEL); // Will send data to a device label that matches the device Id
     if (bufferSent) {
     // Do something if values were sent properly
       Serial.println("Values sent by the device");
     }
     lastmillis = millis();
  }
  #ifdef SENDFIREBASE
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.setFloat(&fbdo, "test/Distance to counterweight", distanceCm)){
  //      Serial.println("PASSED");
  //      Serial.println("PATH: " + fbdo.dataPath());
  //      Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo.errorReason());
      }
      count++;
      
      // Write an Float number on the database path test/float
      if (Firebase.RTDB.setFloat(&fbdo, "test/Minimum tdistance to CounterWeight", min_distance)){
  //      Serial.println("PASSED");
  //      Serial.println("PATH: " + fbdo.dataPath());
  //      Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo.errorReason());
      }

    

    }
    #endif
//  Serial.print("Distance (inch): ");
//  Serial.println(distanceInch);
  
  //delay(1000);
}
