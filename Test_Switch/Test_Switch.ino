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
#define SENDFIREBASE
#define SENDUBIDOTS


#define PUBLISH_FREQUENCY 1000
#define API_KEY "AIzaSyDEcPCgQuwvFKN88l4GLEG34RA2JUCOqiA"
#define DATABASE_URL "https://fir-iot-ec360-default-rtdb.asia-southeast1.firebasedatabase.app/" 
const char *UBIDOTS_TOKEN = "BBFF-z1BQlo0p0ya6TuRfe7znNAoEqfdWqw";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "ChinKeat MESA PRESIDENT";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "s201e6cb7d1";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "SwitchMonitor";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "Governer Switch State"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL_2 = "Control Panel Run Stop Switch State";
const char *VARIABLE_LABEL_3 = "Output";

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = millis();
int count = 0;
bool signupOK = false;

int prediction = 0;
float probability = 0;


//const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
unsigned long timer;
volatile int check_millis_changed = millis();
Ubidots ubidots(UBIDOTS_TOKEN);

float Temperature = 0;
float Humidity = 0;

int governerSwitchState = 0;
int runStopSwitchState = 0;
int outputState = 0;

unsigned long lastmillis = millis();

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  ubidots.setDebug(true);


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
  governerSwitchState = !digitalRead(D5);
  runStopSwitchState = !digitalRead(D6);
  outputState = analogRead(A0)>100?1:0;
  
   #ifdef SENDUBIDOTS

    if ((millis() - lastmillis) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
    {
      ubidots.add(VARIABLE_LABEL, !digitalRead(D5)); // Insert your variable Labels and the value to be sent
      ubidots.add(VARIABLE_LABEL_2, !digitalRead(D6));
      ubidots.add(VARIABLE_LABEL_3, outputState);
      ubidots.add("Is Machine Normal", prediction);
      ubidots.add("Percentage", probability);

      bool bufferSent = false;
       bufferSent = ubidots.send(DEVICE_LABEL); // Will send data to a device label that matches the device Id
       if (bufferSent) {
       // Do something if values were sent properly
         Serial.println("Values sent by the device");
       }
      lastmillis = millis();
  ////    rev = 0;
    }
  #endif
//  if(flag) {Serial.print("hehe");flag = !flag;}
  #ifdef SENDFIREBASE
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.setFloat(&fbdo, "test/Governer Switch State", governerSwitchState)){
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
      if (Firebase.RTDB.setFloat(&fbdo, "test/Control Panel Run Stop Switch State",   runStopSwitchState)){
  //      Serial.println("PASSED");
  //      Serial.println("PATH: " + fbdo.dataPath());
  //      Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.setFloat(&fbdo, "test/Control Panel Run Stop Switch State", outputState)){
  //      Serial.println("PASSED");
  //      Serial.println("PATH: " + fbdo.dataPath());
  //      Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.getInt(&fbdo, "/Predictions/Predicted Lift Status")){
        Serial.print("Data type = "); Serial.println(fbdo.dataType());
        prediction = fbdo.intData();
        Serial.println(prediction);
      }
      else {
        //Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      count++;
      if (Firebase.RTDB.getFloat(&fbdo, "/Predictions/Percentage")){
        Serial.print("Data type = "); Serial.println(fbdo.dataType());
        probability = fbdo.floatData();
        Serial.println(probability);
      }
      else {
        //Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
    #endif
    
    
}
