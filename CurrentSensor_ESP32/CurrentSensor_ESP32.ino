// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3
// The GPIO pin were the CT sensor is connected to (should be an ADC input)
#define ADC_INPUT 34

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance

void setup()
{  
  Serial.begin(9600);
  emon1.current(ADC_INPUT, 111.1);             // Current: input pin, calibration.
}

void loop()
{
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  
  Serial.print(Irms*230.0);         // Apparent power
  Serial.print(" ");
  Serial.println(Irms);          // Irms
}
