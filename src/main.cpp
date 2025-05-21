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

//includes 
#include <Arduino.h>
#include "Proximite.h"
#include "APDS9930.h"
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>
// define
#define LED_PIN     10
#define LED_COUNT  10
#define chargeur_gpio 16
//enum
enum mode_du_banc {
  etat_de_charge,
  etat_aucun_appareil,
};enum mode_du_banc mode_du_banc;

// variable global
int assise=0;
uint16_t proximity_data = 0;
float ambient_light = 0; // can also be an unsigned long
uint16_t ch0 = 0;
uint16_t ch1 = 0;
uint8_t BRIGHTNESS = 255; // NeoPixel brightness, 0 (min) to 255 (max)

// construteur
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // écran
APDS9930 apds = APDS9930(); // capteur luminosité et proximité
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);// led
//fonction
int calcul_luminosité_del(int valeur_capteur_luminosité);
void led(uint8_t rouge,uint8_t vert,uint8_t bleu,uint8_t valeur_de_luminosité);

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
  strip.clear();

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

void loop() 
{
  while(1)
  {
    
    int valeur_de_luminosité=200;
    //BRIGHTNESS=calcul_luminosité_del(valeur_de_luminosité);
    //if (digitalRead(chargeur_gpio)==1){mode_du_banc=etat_de_charge;}
    //else{mode_du_banc=etat_aucun_appareil;}
    mode_du_banc=etat_aucun_appareil;
    switch(mode_du_banc)
    {
      case etat_de_charge:
        
      led(255,0,0,255);//rouge
      break;
      case etat_aucun_appareil:
        led(255,255,0,255);//jaune
      break;

    }
      
    
   
    // Read the proximity value 
    if ( !apds.readProximity(proximity_data) ) {
      Serial.println("Error reading proximity value");
    }
    else 
    {
    Serial.print("Proximity: ");
    Serial.println(proximity_data);
    }
    // Read the light levels (ambient, red, green, blue) 
    apds.readAmbientLightLux(ambient_light);
    apds.readCh0Light(ch0); 
    apds.readCh1Light(ch1);

    Serial.print(F("Ambient: "));
    Serial.print(ambient_light);
    Serial.print(F("  Ch0: "));
    Serial.print(ch0);
    Serial.print(F("  Ch1: "));
    Serial.println(ch1);
    
      
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    assise = personne_assise(proximity_data,assise);
    u8g2.setCursor(0,10);
    u8g2.printf("Nbr de personnes : %d",assise);
    u8g2.sendBuffer();					// transfer internal memory to the display
    delay(10); 
  }
}


void led(uint8_t rouge,uint8_t vert,uint8_t bleu,uint8_t valeur_de_luminosité) 
{
  strip.clear();
  strip.setBrightness(valeur_de_luminosité);
  for (size_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i,strip.Color(rouge,vert,bleu));// jaune
  }
  strip.show();
  delay(500);
}

int calcul_luminosité_del(int valeur_capteur_luminosité)
{
  valeur_capteur_luminosité= (-1*valeur_capteur_luminosité)+256;
  return valeur_capteur_luminosité;
}

