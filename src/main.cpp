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
#define chargeur_gpio 14

//enum
enum mode_du_banc {
  etat_de_charge,
  etat_aucun_appareil,
};enum mode_du_banc mode_du_banc;

// variable global
int assise=0;
int telephone=0;
uint16_t proximity_data = 0;

// construteur
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // écran
APDS9930 apds = APDS9930(); // capteur luminosité et proximité
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);// led

//fonction
void led(uint8_t rouge, uint8_t vert, uint8_t bleu, uint8_t valeur_de_luminosité);
ICACHE_RAM_ATTR  void interrupt_chargeur();


void setup() 
{ 
  // Initialise l'ecran
  u8g2.begin();
  // Initialise les LEDs 
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255);
  strip.clear();
  
  // Initialize APDS-9930 (configure I2C and initial values)
  apds.init();
  // Ajuste le Proximity sensor gain
  apds.setProximityGain(PGAIN_1X); 
  // Active APDS-9930 proximity sensor et ambient sensor (non interrupts)
  apds.enableProximitySensor(true);
  apds.enableLightSensor(true);
  // Initialise le IO chargeur
  pinMode(chargeur_gpio,INPUT);
  // Interrupt
  attachInterrupt(digitalPinToInterrupt(chargeur_gpio),interrupt_chargeur,FALLING);
}

void loop(){

  while(1){
    float valeur_de_luminosité;
    // lecture de la valeur de luminosité (ambient, red, green, blue) 
    apds.readAmbientLightLux(valeur_de_luminosité);

    //Serial.print(F("Ambient: "));
    //Serial.println(valeur_de_luminosité);
  
    if (digitalRead(chargeur_gpio)==0){mode_du_banc=etat_de_charge;}
    else{mode_du_banc=etat_aucun_appareil;}
    
    switch(mode_du_banc)// Change les DELs de couleur si il y a une personne qui charge ou non
    {
      case etat_de_charge:
        led(255,0,0,valeur_de_luminosité);//met les leds à rouge
      break;
      case etat_aucun_appareil:
        led(255,255,0,valeur_de_luminosité);//met les leds à jaune
      break;
    }

    //Serial.print("Proximity: ");
    //Serial.println(proximity_data);
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    assise = personne_assise(proximity_data);// fonction qui permet de détecter lorqu'il y a une personne qui passe
    u8g2.setCursor(0,10);
    u8g2.printf("Nbr de personnes : %d",assise);// affiche le nombre de personne assise
    u8g2.setCursor(0,20);
    u8g2.printf("Nbr de telephones : %d",telephone);// affiche le nombre de téléphones chargé
    u8g2.sendBuffer();					// transfer internal memory to the display
  }
}

void led(uint8_t rouge,uint8_t vert,uint8_t bleu,uint8_t valeur_de_luminosité){// Permet de configurer la luminosité et la couleur des LEDs
  strip.clear();
  strip.setBrightness(valeur_de_luminosité);// selctionne la luminosité

  for (size_t i = 0; i < strip.numPixels(); i++){
    strip.setPixelColor(i,strip.Color(rouge,vert,bleu));// selctionne la couleur
  }
  //Serial.print("luminosite des led:");
  //Serial.println(valeur_de_luminosité);
  strip.show();
  delay(10);
}

ICACHE_RAM_ATTR  void interrupt_chargeur(){// Permet de detecter lorsqu'il y a un téléphone qui se charge
  telephone++;
  mode_du_banc=etat_de_charge;
}
