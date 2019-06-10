


#include "Tlc5940.h"
#define LIGHT_LED 0
#define CLOSE_ALL 1

unsigned char action[3];

void setup()
{
  Tlc.init();

  /* Call Tlc.init() to setup the tlc.*/
  Serial.begin(9600);

  //Send connection message
  Serial.print("PRIMA");
}

void loop()
{
  if (Serial.available()){
    
    Serial.readBytes(action, 3);
    
    switch(action[0]){
      
      case LIGHT_LED:
        Tlc.set(action[1], map(action[2], 0, 255, 0 ,4095));
        Tlc.update();
        Serial.print("Ok.");
      
      break;

      case CLOSE_ALL:      
        Tlc.clear();
        Tlc.update();
      break;

          
    }  
  }
}



