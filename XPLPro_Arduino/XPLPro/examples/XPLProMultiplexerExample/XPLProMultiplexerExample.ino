
/*
 * 
 * XPLProMultiplexerExample
 * 
 * Created by Curiosity Workshop for XPL/Pro arduino->XPlane system.
 * 
 * This sketch was developed and tested on an Arduino Mega.
 * This sketch was originally accompanied with a sample wiring diagram within the folder.
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
#include <XPLMux4067Switches.h>


#define MUX1PIN_BEACON    0
#define MUX1PIN_NAV       1
#define MUX1PIN_STROBE    2
#define MUX1PIN_LEDSWITCH 3
#define MUX1PIN_STARTER   5

XPLPro XP(&Serial);     

void muxHandler(int muxChannel, int muxValue);          
XPLMux4067Switches mux1(7, 3, 4, 5, 6, &muxHandler);         // multiplexer signal is on 7 and select pins are 3-6.  muxHandler can be NULL if you don't intend to use it.
                                                               // eg: XPLMux4067Switches mux1(7, 3, 4, 5, 6, NULL);
                                 

void setup() 
{
  Serial.begin(XPL_BAUDRATE);  
  XP.begin("XPLPro Multiplexer Example", &xplRegister, &xplShutdown, &xplInboundHandler);         
  
  mux1.begin(&XP);
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() 
{
  XP.xloop();  
  mux1.check();
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
  mux1.clear();               // Reset switches

  mux1.addPin(MUX1PIN_BEACON,     XPLMUX4067_COMMANDTRIGGER,         XP.registerCommand(F("sim/lights/beacon_lights_toggle")) );
  mux1.addPin(MUX1PIN_STARTER,    XPLMUX4067_COMMANDSTARTEND,        XP.registerCommand(F("sim/engines/engage_starters")) );
  mux1.addPin(MUX1PIN_NAV,        XPLMUX4067_DATAREFWRITE,           XP.registerDataRef(F("sim/cockpit2/switches/navigation_lights_on")) );
  mux1.addPin(MUX1PIN_STROBE,     XPLMUX4067_DATAREFWRITE_INVERT,    XP.registerDataRef(F("sim/cockpit2/switches/strobe_lights_on")) );
  mux1.addPin(MUX1PIN_LEDSWITCH,  XPLMUX4067_SENDTOHANDLER,          0 );   // this only sends the mux event to the handler.
}

void muxHandler(int inPin, int inValue)
{


// Process inbound mux signal.  Use this if you have other uses for the incoming mux pins.  
// Every mux pin event will call this after it triggers the xplane commands/dataref changes.  
// I could also specify XPLMUX4067_SENDTOHANDLER when adding the pin, if I dont want anything done automatically and it will only call here.

  switch (inPin)
  {
      case MUX1PIN_LEDSWITCH :                      // all that will happen is the builtin LED will track the position of the switch
        digitalWrite(LED_BUILTIN, !inValue);                      
      break;

      case MUX1PIN_STROBE :                         // the dataref was already updated, but we can add functionality.
        if (inValue == 0) XP.sendSpeakMessage("Strobe ON");                      
        if (inValue == 1) XP.sendSpeakMessage("Strobe OFF");
      break;
  }

}



