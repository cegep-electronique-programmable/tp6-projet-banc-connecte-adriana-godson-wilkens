#include "Proximite.h"
#include <Wire.h>
#include "APDS9930.h"
bool personne;
APDS9930 test = APDS9930();

int personne_assise(uint16_t data,int valeur)
{
  if(data>=600)
  {

    while (data>=600)
    {
      test.readProximity(data);
    }
    valeur++;
  }
  return valeur;
}