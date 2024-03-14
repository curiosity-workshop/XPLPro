

/*
 * 
 * XPLProShutdownExample
 *
 * This examples turns on the built in LED while Xplane is active with a plane loaded and off when the plane is unloaded
 *  or if Xplane shuts down
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

#include <XPLPro.h>              //  include file for the X-plane direct interface 
XPLPro XP(&Serial);      // create an instance of it


void setup() 
{

  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

 /*
    needed for initialization.  Parameters are:
        a texual identifier of your device 
        your function that will be called when xplane and the plugin are ready for dataref and command registrations
        your function that will be called when xplane either shuts down or unloads an aircraft model
        your function that will be called when we have requested sends of datarefs that have changed
 */
  XP.begin("XPLPro Shutdown Demo!", &xplRegister, &xplShutdown, &xplInboundHandler);               
  digitalWrite(LED_BUILTIN, LOW);

 }


void loop() 
{

  XP.xloop();  //  needs to run every cycle.  

}

/*
 * This function is the callback function we specified that will be called any time our requested data is sent to us.
 * handle is the handle to the dataref.  The following values are transfered to the callback through inStruct:
 * 
 *  int inData->handle          The handle of the incoming dataref
 *  int inData->element         If the dataref is an array style dataref, this is the element of the array
 *  long inData->inLong         long value for datarefs that are long values
 *  float inData->inFloat       float value for datarefs that are float values
 */
void xplInboundHandler(inStruct *inData)
{
 
 
  
}

void xplRegister()          // this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
{

 digitalWrite(LED_BUILTIN, HIGH);     //turn on the built in LED while xplane is actively running a plane

 }

void xplShutdown()
{
  
  digitalWrite(LED_BUILTIN, LOW);      // turn off the built in LED when xplane exits or unloads a plane.
  
}
