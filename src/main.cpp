#include <Arduino.h>
#include "affichage.h"
#include "Proximite.h"
#include <Wire.h>
#include "APDS9930.h"

// Global Variables
APDS9930 apds = APDS9930();
uint16_t proximity_data = 0;
int proximity_max = 0;
float ambient_light = 0; // can also be an unsigned long
uint16_t ch0 = 0;
uint16_t ch1 = 1;
bool proxi = true;
bool light = false;

#define DUMP_REGS
#define PWM_LED_PIN       10



// Global Variables
//APDS9930 apds = APDS9930();
//uint16_t proximity_data = 0;
//int proximity_max = 0;

void setup() {
  //analogReference(EXTERNAL);
  pinMode(PWM_LED_PIN, OUTPUT);

  // Initialize Serial port
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("------------------------"));
  Serial.println(F("APDS-9930 - ProximityLED"));
  Serial.println(F("------------------------"));
  
  // Initialize APDS-9930 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9930 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9930 init!"));
  }
  
  // Adjust the Proximity sensor gain
  if ( !apds.setProximityGain(PGAIN_1X) ) {
    Serial.println(F("Something went wrong trying to set PGAIN"));
  }
  
  // Start running the APDS-9930 proximity sensor (no interrupts)
  if ( apds.enableProximitySensor(true) ) {
    Serial.println(F("Proximity sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during sensor init!"));
  }

  if ( apds.enableLightSensor(true) ) {
    Serial.println(F("Light sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during light sensor init!"));
  }

#ifdef DUMP_REGS
  /* Register dump */
  uint8_t reg;
  uint8_t val;

  for(reg = 0x00; reg <= 0x19; reg++) {
    if( (reg != 0x10) && \
        (reg != 0x11) )
    {
      apds.wireReadDataByte(reg, val);
      Serial.print(reg, HEX);
      Serial.print(": 0x");
      Serial.println(val, HEX);
    }
  }
  apds.wireReadDataByte(0x1E, val);
  Serial.print(0x1E, HEX);
  Serial.print(": 0x");
  Serial.println(val, HEX);
#endif
}

void loop() {
  
  // Read the proximity value
  
    if ( !apds.readProximity(proximity_data) ) {
      Serial.println("Error reading proximity value");
    }
     else 
     {
      Serial.print("Proximity: ");
      Serial.print(proximity_data);
     }
     
  
  
   
  // Read the light levels (ambient, red, green, blue)
  
    if (  !apds.readAmbientLightLux(ambient_light) ||
        !apds.readCh0Light(ch0) || 
        !apds.readCh1Light(ch1) ) {
    Serial.println(F("Error reading light values"));
  } 
  else {
    Serial.print(F("Ambient: "));
    Serial.print(ambient_light);
    Serial.print(F("  Ch0: "));
    Serial.print(ch0);
    Serial.print(F("  Ch1: "));
    Serial.println(ch1);
  }
    
  
  if (  !apds.readAmbientLightLux(ambient_light) ||
        !apds.readCh0Light(ch0) || 
        !apds.readCh1Light(ch1) ) {
    Serial.println(F("Error reading light values"));
  } 
  else {
    Serial.print(F("Ambient: "));
    Serial.print(ambient_light);
    Serial.print(F("  Ch0: "));
    Serial.print(ch0);
    Serial.print(F("  Ch1: "));
    Serial.println(ch1);
  }
    // This is an ugly hack to reduce sensor noise.
    // You may want to adjust POFFSET instead.
    /*
    proximity_data -= 200;
    if (proximity_data > 50000) {
      proximity_data = 0;
    }
    if (proximity_data > proximity_max) {
      proximity_max = proximity_data;
    }
    proximity_data = map(proximity_data, 0, proximity_max, 0, 1023);
    */
    uint8_t assise = personne_assise(proximity_data);

    
    Serial.println(proximity_data);
    Serial.println(assise);
    analogWrite(PWM_LED_PIN, proximity_data);
    delay(250);
  }
  
  // Wait 250 ms before next reading
 

