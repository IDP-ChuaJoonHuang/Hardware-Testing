/******************************************
 *
 * This example works for both Industrial and STEM users.
 *
 * Developed by Jose Garcia, https://github.com/jotathebest/
 *
 * ****************************************/

/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include <DHT.h>
#include <Arduino.h>
#include <Wire.h>
#include "RTClib.h"
//Provide the token generation process info.
//#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
//#include "addons/RTDBHelper.h"

/****************************************
 * defines
 ****************************************/
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTPin 5

/****************************************
 * Define Constants
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-z1BQlo0p0ya6TuRfe7znNAoEqfdWqw";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "你回来了";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "okvx4408";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "temperature"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL_2 = "Date"; // Put here your Variable label to which data  will be published

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds

DHT dht(DHTPin, DHTTYPE);  
float Temperature = 0;
float Humidity = 0;

RTC_DS1307 RTC;     // Setup an instance of DS1307 naming it RTC
char Time[50];
int ss = 0;
int mm = 0;
int hh = 0;
int DD = 0;
int dd = 0;
int MM = 0;
int yyyy = 0;

unsigned long timer;
//uint8_t analogPin = 34; // Pin used to read data from GPIO34 ADC_CH6.

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/

 void RTCSetup(){
  RTC.begin();  // Init RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date is expanded to date and time on your computer at compiletime
  #ifdef DEBUG_RTC
    Serial.print("Time and date set");
  #endif
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

//  Serial.print(F("Humidity: "));
//  Serial.print(Humidity);
//  Serial.print(F("%  Temperature: "));
//  Serial.print(Temperature);
//  Serial.print(F("°C "));
//  Serial.print(f);
//  Serial.print(F("°F  Heat index: "));
//  Serial.print(hic);
//  Serial.print(F("°C "));
//  Serial.print(hif);
//  Serial.println(F("°F"));
}

void readTime(){
  DateTime now = RTC.now();
  ss = now.second();
  mm = now.minute();
  hh = now.hour();
  //DD = now.dayOfWeek();
  dd = now.day();
  MM = now.month();
  yyyy = now.year();
  sprintf(Time,"%d/%d/%d %d:%d:%d \n", yyyy,MM,dd,hh,mm,ss);
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  dht.begin();
  Wire.begin(); // Start the I2C
  RTCSetup();

  timer = millis();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    
    readTemp();
    //float value = analogRead(analogPin);
    ubidots.add(VARIABLE_LABEL, Temperature); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL_2,1,Time); // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
}
