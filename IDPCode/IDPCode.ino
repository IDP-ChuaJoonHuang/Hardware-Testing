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
//#define DEBUG_DHT22
//#define DEBUG_DOOR
#define DEBUG_FIREBASE

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
#define DATABASE_URL "https://fir-iot-ec360-default-rtdb.asia-southeast1.firebasedatabase.app/" 

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
char c_time[50] = {'0','\n','\0'};
String Time = "nothing";
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

float t = 0;
float h = 0;
bool is_water_level_dangerously_low = 0;
int doorState=0;

////////////////
///FUNCTIONS////
////////////////

void read_temp(){
  t = dht.readTemperature(); // Gets the values of the temperature
  h = dht.readHumidity(); // Gets the values of the humidity 
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  #ifdef DEBUG_DHT22
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
  #endif
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
  Serial.begin(9600); // Set serial port speed
  Wire.begin(); // Start the I2C
  pinMode(DHTPin, INPUT);
  pinMode(magnetic_switch_pin, INPUT_PULLUP);
  RTCSetup();
  FirebaseSetup();

  dht.begin();
}



void loop() {
    //Temperature and Humidity


    //Door
    doorState = digitalRead(magnetic_switch_pin); // read state
    #ifdef DEBUG_DOOR
    if (doorState == HIGH) {
      Serial.println("The door is open");
    } else {
      Serial.println("The door is closed");
    }
    #endif

    //Proximity

    //water level
    

    //RTC
    DateTime now = RTC.now();
    ss = now.second();
    mm = now.minute();
    hh = now.hour();
    DD = now.dayOfWeek();
    dd = now.day();
    MM = now.month();
    yyyy = now.year();
    sprintf(c_time,"%d/%d/%d %d:%d:%d", yyyy,MM,dd,hh,mm,ss);
    Time = c_time;
    
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
     Serial.print("Temperature = "), Serial.println(t);
     Serial.print("Humidity = "), Serial.println(h);
   #endif

   if(millis()%2000 == 0){
    read_temp();
   }

  //Send data to firebase
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    #ifndef DEBUG_FIREBASE
      Firebase.RTDB.setInt(&fbdo, "test/Door", doorState);
      Firebase.RTDB.setFloat(&fbdo, "test/Temperature", t);
      Firebase.RTDB.setFloat(&fbdo, "test/Humidity", h);
      Firebase.RTDB.setString(&fbdo, "test/Time", Time);
    
    #else
      if (Firebase.RTDB.setInt(&fbdo, "test/Door", doorState)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      count++;
      
      // Write an Float number on the database path test/float
      if (Firebase.RTDB.setFloat(&fbdo, "test/Temperature", t)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
  
      if (Firebase.RTDB.setFloat(&fbdo, "test/Humidity", h)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
  
      if (Firebase.RTDB.setString(&fbdo, "test/Time", Time)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "test/Low_Oil_Level", 0)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    #endif
  }
}
