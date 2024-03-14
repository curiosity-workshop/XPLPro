
/*
 * 
 * XPLProSwitchesExample
 * 
 * Created by Curiosity Workshop for XPL/Pro arduino->XPlane system.
 * 
 * This sketch was developed and tested on an Arduino Mega.
 * This example accompanies the video on YouTube:  https://www.youtube.com/watch?v=SMJiGwNc7rY
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

#define XPLSWITCHES_MAXSWITCHES 10    //  adjust this as required for your needs.  Higher number of course costs memory ~10 bytes each.  Default is 40 if not specified
#include <XPLSwitches.h>
              

#define PIN_BEACON    45        // Connect to a momentary switch.  It will toggle the beacon lights on and off with each press      
#define PIN_STROBE    47        // Connect to a regular toggle switch.  We will control the dataref directly
#define PIN_STARTER   49        // Connect to a momentary switch.  It will activate the starter while the button is pressed.
#define PIN_LEDSWITCH 51        // Connect to a any type of switch.  The only function will be to turn the builtin LED on and off as a demonstration.
#define PIN_NAV       53        // Connect to a regular toggle switch.  We will control the dataref directly.

XPLPro XP(&Serial);     

void switchHandler(int pin, int switchValue);
XPLSwitches switches(&switchHandler);             // switchHandler is a function that will be called if we want to provide additional functionality.  It can also be NULL if not needed.             



void setup() 
{
  Serial.begin(XPL_BAUDRATE);  
  XP.begin("XPLPro Switches Example", &xplRegister, &xplShutdown, &xplInboundHandler);         

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  switches.begin(&XP);

}

void loop() 
{
  XP.xloop();  
  switches.check();
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
  switches.clear();               // Reset switches

  switches.addPin(PIN_BEACON,   XPLSWITCHES_COMMANDTRIGGER,         XP.registerCommand(F("sim/lights/beacon_lights_toggle")) );
  switches.addPin(PIN_STARTER,  XPLSWITCHES_COMMANDSTARTEND,        XP.registerCommand(F("sim/engines/engage_starters")) );
  switches.addPin(PIN_STROBE,   XPLSWITCHES_DATAREFWRITE_INVERT,    XP.registerDataRef(F("sim/cockpit2/switches/strobe_lights_on")) );
  switches.addPin(PIN_LEDSWITCH,XPLSWITCHES_SENDTOHANDLER, 0);    // this only sends the mux event to the handler.  Parameter "0" will be ignored in this case.
  switches.addPin(PIN_NAV,      XPLSWITCHES_DATAREFWRITE,           XP.registerDataRef(F("sim/cockpit2/switches/navigation_lights_on")) );
}

void switchHandler(int inPin, int inValue)
{

// Process inbound switch events.  Use this if you have other uses for the incoming mux pins.  
// Every registered switch event will call this after it triggers the xplane commands/dataref changes.  
// I could also specify XPLSWITCHES_SENDTOHANDLER when adding the pin, if I dont want anything done automatically and it will only call here.

  switch (inPin)
  {
      case PIN_LEDSWITCH :                   
        digitalWrite(LED_BUILTIN, !inValue);                      
      break;

      case PIN_STROBE :                   
        if (inValue == 0) XP.sendSpeakMessage("Strobe ON");                      
        if (inValue == 1) XP.sendSpeakMessage("Strobe OFF");                     
      break;
  }

}
