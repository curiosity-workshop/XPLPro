
/*
 * 
 * XPLProPotentiometersExample
 * 
 * Created by Curiosity Workshop for XPL/Pro arduino->XPlane system.
 * 
 * This sketch was developed and tested on an Arduino Mega.
 * 
   To report problems, download updates and examples, suggest enhancements or get technical support:
  
      discord:  https://discord.gg/RacvaRFsMW
      patreon:  www.patreon.com/curiosityworkshop
      YouTube:  https://youtube.com/channel/UCISdHdJIundC-OSVAEPzQIQ
 * 
 * 
 */

#include <arduino.h>

#include <XPLPro.h>

#define XPLPOTS_MAXPOTS  3    //  adjust this as required for your needs.  Higher number of course costs memory.  Default is 10 if not specified
#include <XPLPotentiometers.h>
              

#define PIN_THROTTLE  A0
#define PIN_PROP      A1
#define PIN_MIXTURE   A2

XPLPro XP(&Serial);     

void potHandler(int pin, float potValue);
XPLPotentiometers pots(&potHandler);             // potHandler is a function that will be called if we want to provide additional functionality.  It can also be NULL if not needed.             

void setup() 
{
  Serial.begin(XPL_BAUDRATE);  
  XP.begin("XPLPro Potentiometer Example", &xplRegister, &xplShutdown, &xplInboundHandler);         

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pots.begin(&XP);

}

void loop() 
{
  XP.xloop();  
  pots.check();
 //if (millis() % 1000 > 900) digitalWrite(LED_BUILTIN, HIGH);    else digitalWrite(LED_BUILTIN, LOW);        // Heartbeat
}      

void xplInboundHandler(inStruct *inData)
{
 
}

void xplShutdown()
{
  
  
}


void xplRegister()          
{
  pots.clear();               // Reset pots

  pots.addPin(PIN_THROTTLE, XPLPOTS_DATAREFWRITE, XP.registerDataRef(F("sim/cockpit2/engine/actuators/throttle_ratio_all")), 10 ,0, 1024, 0, 1);
  pots.addPin(PIN_MIXTURE,  XPLPOTS_DATAREFWRITE, XP.registerDataRef(F("sim/cockpit2/engine/actuators/mixture_ratio_all")), 10, 0, 1024, 0, 1 );
  pots.addPin(PIN_PROP,     XPLPOTS_DATAREFWRITE, XP.registerDataRef(F("sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec_all")), 10, 0, 1024, 77, 283);
 
 
}

void potHandler(int inPin, float inValue)
{

// Process inbound potentiometer events.  Use this if you have other uses for the incoming data.  
// Every registered event will call this after it sends the value to xplane.  
// I could also specify XPLPOTS_SENDTOHANDLER when adding the pin, if I dont want anything done automatically and it will only call here.

  switch (inPin)
  {
      case PIN_THROTTLE :                   
        // do something cool                      
      break;
    
  }

}
