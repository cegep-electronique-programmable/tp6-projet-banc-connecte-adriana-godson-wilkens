#include "Proximite.h"
#include <Wire.h>
#include "APDS9930.h"

bool personne;
APDS9930 test = APDS9930();
uint16_t valeur = 0;

int personne_assise(uint16_t data)
{
  test.readProximity(data);
  if(data>=600)
  {
    if(personne == true){
      valeur++;
      personne = false;
    }
  }

  else{
    personne = true;
  }
  return valeur;

}