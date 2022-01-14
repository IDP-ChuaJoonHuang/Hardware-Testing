/* 
* i2c_port_address_scanner
* Scans ports D0 to D7 on an ESP8266 and searches for I2C device. based on the original code
* available on Arduino.cc and later improved by user Krodal and Nick Gammon (www.gammon.com.au/forum/?id=10896)
* D8 throws exceptions thus it has been left out
*
*/
//////////////
///INCLUDES///
//////////////

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
#include <SPI.h>  // not used here, but needed to prevent a RTClib compile error

//////////////
///DEFINES////
//////////////

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DUBUG_RTC
#define DEBUG_DHT22

///////////////
/////Wifi//////
///////////////

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "dljs8888@unifi"
#define WIFI_PASSWORD "djmxs8932"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDEcPCgQuwvFKN88l4GLEG34RA2JUCOqiA"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "fir-iot-ec360-default-rtdb.asia-southeast1.firebasedatabase.app" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

///////////////
////Others/////
///////////////


RTC_DS1307 RTC;     // Setup an instance of DS1307 naming it RTC
char Time[50];
int ss = 0;
int mm = 0;
int hh = 0;
int DD = 0;
int dd = 0;
int MM = 0;
int yyyy = 0;

uint8_t magnetic_switch_pin = D3;
uint8_t DHTPin = D8; 

DHT dht(DHTPin, DHTTYPE);                

float Temperature = 0;
float Humidity = 0;
bool flag = 0;
bool is_water_level_dangerously_low = 0;

////////////////
///FUNCTIONS////
////////////////

void read_temp(){
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
}

void FirebaseSetup(){
  //Firebase
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
  
    /* Assign the api key (required) */
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
  
    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void RTCSetup(){
  RTC.begin();  // Init RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date is expanded to date and time on your computer at compiletime
  #ifdef DEBUG_RTC
    Serial.print("Time and date set");
  #endif
}

void setup() {
  Serial.begin(115200); // Set serial port speed
  Wire.begin(); // Start the I2C
  pinMode(DHTPin, INPUT);
  pinMode(magnetic_switch_pin, INPUT_PULLUP);
  FirebaseSetup();
  RTCSetup();
  
}



void loop() {
  if(flag ==0){
    //RTC
    RTCSetup();
    flag = 1;
  }
  if(flag = 1){
    //Temperature and Humidity
    read_temp();

    //Door
    if (digitalRead(magnetic_switch_pin) == LOW) {
      Serial.println("Switch Closed");
      while (digitalRead(magnetic_switch_pin) == LOW) {}
    }
    else {
      Serial.println("Switch Open");
    }

    //Proximity

    //water level
    while (Serial.available()){
      is_water_level_dangerously_low = Serial.read();
    }

    //RTC
    DateTime now = RTC.now();
    ss = now.second();
    mm = now.minute();
    hh = now.hour();
    DD = now.dayOfTheWeek();
    dd = now.day();
    MM = now.month();
    yyyy = now.year();
    sprintf(Time,"%d/%d/%d %d:%d:%d \n"), yyyy,MM,dd,hh,mm,ss);
    
    //Debugging Code
    #ifdef DEBUG_RTC
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(' ');
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.println();
   #endif
   
   #ifdef DEBUG_DHT22
     Serial.print("Temperature = "), Serial.println((int)Temperature);
     Serial.print("Humidity = "), Serial.println((int)Humidity);
   #endif

  //Send data to firebase
   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.setFloat(&fbdo, "test/Temperature", Temperature)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      
      if (Firebase.RTDB.setFloat(&fbdo, "test/Humidity", Humidity)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setInt(&fbdo, "test/Door", digitalRead(magnetic_switch_pin))){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setString(&fbdo, "test/Time", Time){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.setInt(&fbdo, "test/Water Level", is_water_level_dangerously_low){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
  }
}
