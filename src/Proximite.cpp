#include "Proximite.h"
#include <Wire.h>
 #include "APDS9930.h"

int personne_assise(uint8_t data){
    uint8_t valeur=0;
    bool personne=false;
    if(data>=600){
      personne=true;
    }
    else if(data<600){
      personne = false;
    }
    if(personne==true){
      valeur+=1;
    }
    return valeur;
}