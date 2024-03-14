

/*
 * 
 * XPLProParkingBrakeDemo
 * This example will light up a properly installed LED on a specified arduino pin, or you could use the built-in LED 
 * usually on pin 13.
 * 
 * Created by Curiosity Workshop for XPL/Pro arduino->XPlane system.
 * 
 * This sketch was developed and tested on an Arduino Mega in response to a user request.
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


long int startTime;

#define PINLED_PARKINGBRAKE 13

int drefParkingBrake;   // this stores a handle to the parking brake dataref 

void setup() 
{
  
  pinMode(PINLED_PARKINGBRAKE, OUTPUT);     // built in LED on arduino board will turn on and off with the status of the beacon light
 
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

 /*
    needed for initialization.  Parameters are:
        a texual identifier of your device 
        your function that will be called when xplane and the plugin are ready for dataref and command registrations
        your function that will be called when xplane either shuts down or unloads an aircraft model
        your function that will be called when we have requested sends of datarefs that have changed
 */
  XP.begin("XPLPro Parking Brake Demo!", &xplRegister, &xplShutdown, &xplInboundHandler);               
  digitalWrite(PINLED_PARKINGBRAKE, LOW);


}


void loop() 
{
 
  XP.xloop();  //  needs to run every cycle.  

/************************************************************************************************************************************
 * everything after the next line will only occur every 100ms.  You can also utilize other time values.
 * This helps maintain serial data flow and also helps with switch debounce.
 * do NOT add delays anywhere in the loop cycle, it will interfere with the reception of serial data from xplane and the plugin.
 ************************************************************************************************************************************
*/

  if (millis() - startTime > 100) startTime = millis();   else return;          

}

/*
 * This function is the callback function we specified that will be called any time our requested data is sent to us.
 * handle is the handle to the dataref.  The following variables within the plugin are how we receive the data:
 * 
 * inStruct *inData is a pointer to a structure that contains information about the incoming dataref. 
 * 
 *  int inData->handle          The handle of the incoming dataref
 *  int inData->element         If the dataref is an array style dataref, this is the element of the array
 *  long inData->inLong         long value for datarefs that are long values
 *  float inData->inFloat       float value for datarefs that are float values
 */
 void xplInboundHandler(inStruct *inData)
{
  if (inData->handle == drefParkingBrake)
  {   if (inData->inFloat > .2)   digitalWrite(PINLED_PARKINGBRAKE, HIGH);        // if parking brake is set turn the LED on
      else                        digitalWrite(PINLED_PARKINGBRAKE, LOW);
      
      
  }

   
}

/*
 *  this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
 *  It could be called multiple times if for instance xplane is restarted or loads a different aircraft
 */
void xplRegister()         
{
/*
 * This example registers a dataref for the beacon light.  
 * In the inbound handler section of the code we will turn on/off the LED on the arduino board to represent the status of the beacon light within xplane.
 * On non-AVR boards remove the F() macro.
 * 
 */
  drefParkingBrake = XP.registerDataRef(F("sim/cockpit2/controls/parking_brake_ratio") );    
  XP.requestUpdates(drefParkingBrake, 100, .1);          // Tell xplane to send us updates when the status of the parking brake changes.  
                                                  // 100 means don't update more often than every 100ms and .1 is a precision specifier for float variables which is explained in another example, use .1 for this example
  

  
}

void xplShutdown()
{
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}
