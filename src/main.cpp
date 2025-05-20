/*

  GraphicsTest.ino

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include <Arduino.h>


#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
# include <Wire.h>
//define 
#define LED_PIN     25
#define LED_COUNT  9
#define chargeur_gpio 16
#include "affichage.h"
#include "Proximite.h"
#include "APDS9930.h"
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C

#endif

// Global Variables
bool assis= false;
int variable=0;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
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

// enum

//enum
enum mode_du_banc {
  etat_de_charge,
  etat_aucun_appareil,
  etat_sombre,
};enum mode_du_banc mode_du_banc;
// variable global


uint8_t BRIGHTNESS = 255; // NeoPixel brightness, 0 (min) to 255 (max)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
void colorWipe(uint32_t color, int wait);
void pulseWhite(uint8_t wait);
int calcul_luminosité_del(int valeur_capteur_température);
 
void setup() {
  pinMode(chargeur_gpio, INPUT);           // set pin to input
  u8g2.begin();
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
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
  
  while (1)
  {
    int valeur_de_luminosité=200;
    BRIGHTNESS=calcul_luminosité_del(valeur_de_luminosité);
    if (digitalRead(chargeur_gpio)==1){mode_du_banc=etat_de_charge;}
    else{mode_du_banc=etat_aucun_appareil;}
    
    switch(mode_du_banc)
    {
      case etat_de_charge:
        strip.setBrightness(BRIGHTNESS);
        strip.setPixelColor(0,255,0,0);// rouge
      break;
      case etat_aucun_appareil:
        strip.setBrightness(BRIGHTNESS);
        strip.setPixelColor(0,255,255,0);// jaune
      break;
      case etat_sombre:
      
      break;
    }
  }
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
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    uint8_t assise = personne_assise(proximity_data);

    u8g2.drawStr(0,10,"Nbr de personnes : %d",proximity_data);
    u8g2.drawStr(0,10,"Nbr de personnes : %d",assise);
    analogWrite(PWM_LED_PIN, proximity_data);
    u8g2.sendBuffer();					// transfer internal memory to the display
    delay(1000); 
  }
  
  // Wait 250 ms before next reading
 

}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void pulseWhite(uint8_t wait) {
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }

  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }
}

int calcul_luminosité_del(int valeur_capteur_température)
{
  return valeur_capteur_température;
}


