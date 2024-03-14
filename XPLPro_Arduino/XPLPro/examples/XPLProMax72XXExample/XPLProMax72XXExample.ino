
/*
 * 
 * XPLProMax72XXExample
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
#include <LedControl.h>             // for MAX72xx displays.  This is an external library that you need to install.
#include <XPLPro.h>

XPLPro XP(&Serial);     
LedControl myLedDisplays=LedControl(34,36,35,1);                  // data, clock, cs, number of devices, they can be chained.

int drefAltitude;         // handle to current altitude dataref

void setup() 
{
  Serial.begin(XPL_BAUDRATE);  
  XP.begin("XPLPro MAX72XX Display Example", &xplRegister, &xplShutdown, &xplInboundHandler);         

   myLedDisplays.shutdown(0, false);     myLedDisplays.setIntensity(0, 5);  // start the LED displays

}

void loop() 
{
  XP.xloop();  
  
 //if (millis() % 1000 > 900) digitalWrite(LED_BUILTIN, HIGH);    else digitalWrite(LED_BUILTIN, LOW);        // Heartbeat
}      

void xplInboundHandler(inStruct *inData)
{
  
  if (inData->handle == drefAltitude)  { printNumber(0, "% 5.3u", inData->inFloat, 1, &myLedDisplays);  return; }

}

void xplShutdown()
{
  
  myLedDisplays.shutdown(0, true);  
}


void xplRegister()          
{
  
  drefAltitude = XP.registerDataRef(F("sim/cockpit2/gauges/indicators/altitude_ft_pilot"));
   XP.requestUpdates(drefAltitude, 100, 10);
  //XP.registerDataRef(F("sim/cockpit2/switches/strobe_lights_on")) );
  

}



/* 
 *  prints a number on the MAX 72xx display
 */
void printNumber(int ledDisplay, char *printMask, long int v, int pos, LedControl *lc) 
{  
char tString[10];
int tpos = strlen(tString) + pos;

  sprintf(tString, printMask, v);
  for (int i = strlen(tString)-1; i>=0; i--)
  {
    lc->setChar(ledDisplay, tpos++, tString[i], false);
  }
  
    
} 
