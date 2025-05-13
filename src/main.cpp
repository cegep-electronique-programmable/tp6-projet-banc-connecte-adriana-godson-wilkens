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

  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
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

