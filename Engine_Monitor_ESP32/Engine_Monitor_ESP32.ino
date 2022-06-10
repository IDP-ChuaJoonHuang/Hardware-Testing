#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "EmonLib.h"   //https://github.com/openenergymonitor/EmonLib
#include <DHT.h>
#include "UbidotsEsp32Mqtt.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTPin 5

EnergyMonitor emon;
//int potValue = 0;
#define vCalibration 210
#define currCalibration 111.2
#define PUBLISH_FREQUENCY 1000
#define API_KEY "AIzaSyDEcPCgQuwvFKN88l4GLEG34RA2JUCOqiA"
#define DATABASE_URL "https://fir-iot-ec360-default-rtdb.asia-southeast1.firebasedatabase.app/" 
const char *UBIDOTS_TOKEN = "BBFF-w4V5RHXuY912iDpetacCwu0GCFAfuL";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "Guest-PTS";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "Potensi@guestii88";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "Temperature"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL_2 = "Voltage";
const char *VARIABLE_LABEL_3 = "Current";
const char *VARIABLE_LABEL_4 = "rpm";
const char *VARIABLE_LABEL_5 = "Humidity";

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = millis();
int count = 0;
bool signupOK = false;


//const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
unsigned long timer;
volatile int check_millis_changed = millis();
float probability = 0;
String prediction = "";
Ubidots ubidots(UBIDOTS_TOKEN);


DHT dht(DHTPin, DHTTYPE);  

float Temperature = 0;
float Humidity = 0;
 
float kWh = 0;
unsigned long lastmillis = millis();
float value = 0;
float rev = 0;
int rpm;
volatile bool flag = 0;


void isr(){
  if(millis() >= check_millis_changed+20){
    rev++;
    check_millis_changed = millis();
    //print_flag = 1;
  }else{
    
  }
  //check_millis_changed = millis();
  //Serial.print("hehe");
}

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
 
 
void myTimerEvent() {
    emon.calcVI(20, 2000);
    Serial.print("Vrms: ");
    Serial.print(emon.Vrms, 2);
    Serial.print("V");
//    Blynk.virtualWrite(V0, emon.Vrms);
    Serial.print("\tIrms: ");
    Serial.print(emon.Irms, 4);
    Serial.print("A");
//    Blynk.virtualWrite(V1, emon.Irms);
    Serial.print("\tPower: ");
    Serial.print(emon.apparentPower, 4);
    Serial.print("W");
//    Blynk.virtualWrite(V2, emon.apparentPower);
    Serial.print("\tkWh: ");
    kWh = kWh + emon.apparentPower*(millis()-lastmillis)/3600000000.0;
    Serial.print(kWh, 4);
    Serial.println("kWh");
    lastmillis = millis();
//    Blynk.virtualWrite(V3, kWh);
}

void readTemp(){
  Humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  Temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(Humidity) || isnan(Temperature) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, Humidity);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(Temperature, Humidity, false);

  Serial.print(F("Humidity: "));
  Serial.print(Humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(Temperature);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
}


void setup() {
  Serial.begin(115200);
  emon.voltage(35, vCalibration, 1.7); // Voltage: input pin, calibration, phase_shift
  emon.current(34, currCalibration); // Current: input pin, calibration.
  pinMode(25, INPUT_PULLUP);
  attachInterrupt(25, isr, RISING);
  //Blynk.begin(auth, ssid, pass);
  //timer.setInterval(5000L, myTimerEvent);
  dht.begin();
//  ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

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
  //Blynk.run();
  //timer.run();
  #ifdef SENDUBIDOTS
    if (!ubidots.connected())
    {
      ubidots.reconnect();
    }
    if (abs(millis() - lastmillis) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
    {
      myTimerEvent();
      rpm = (rev*60000/PUBLISH_FREQUENCY);
      readTemp();
      ubidots.add(VARIABLE_LABEL, Temperature); // Insert your variable Labels and the value to be sent
      ubidots.add(VARIABLE_LABEL_5, Humidity);
      ubidots.add(VARIABLE_LABEL_2, emon.Vrms);
      ubidots.add(VARIABLE_LABEL_3, emon.Irms);
      ubidots.add(VARIABLE_LABEL_4, rpm);
  
      ubidots.publish(DEVICE_LABEL);
  ////    lastmillis = millis();
  ////    rev = 0;
    }
  #endif
//  if(flag) {Serial.print("hehe");flag = !flag;}
  #ifdef SENDFIREBASE
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      // Write an Int number on the database path test/int
      if (Firebase.RTDB.setFloat(&fbdo, "test/Temperature", Temperature)){
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
      if (Firebase.RTDB.setFloat(&fbdo, "test/Humidity", Humidity)){
  //      Serial.println("PASSED");
  //      Serial.println("PATH: " + fbdo.dataPath());
  //      Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo.errorReason());
      }
  
      if (Firebase.RTDB.setFloat(&fbdo, "test/Current(rms)", emon.Irms)){
  //      Serial.println("PASSED");
  //      Serial.println("PATH: " + fbdo.dataPath());
  //      Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo.errorReason());
      }
    
      if (Firebase.RTDB.setFloat(&fbdo, "test/Voltage(rms)", emon.Vrms)){
  //      Serial.println("PASSED");
  //      Serial.println("PATH: " + fbdo.dataPath());
  //      Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.getString(&fbdo, "/Predictions/Predicted Machine Status")){
        if (fbdo.dataType() == "string") {
            prediction = fbdo.stringData();
            Serial.println(prediction);
          }
        }
        else {
          //Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
    }
    #endif
}
