/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-hc-sr04-ultrasonic-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "Ubidots.h"
#define PUBLISH_FREQUENCY 5000
const char* UBIDOTS_TOKEN = "BBFF-w4V5RHXuY912iDpetacCwu0GCFAfuL";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "iPhone (10)";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "aaaaaaaa";      // Put here your Wi-Fi password
const char* DEVICE_LABEL = "Proximity";      // device label
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

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

//  Serial.print("Distance (inch): ");
//  Serial.println(distanceInch);
  
  //delay(1000);
}
