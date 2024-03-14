

/*
 * 
 * XPLProSequenceExample
 * 
 * This sketch runs a series of commands or dataref writes over time when triggered  
 *
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


long int startTime;

#define PIN_TRIGGER      24        // momentary switch to trigger the sequence

int drefBeacon;         // this stores a handle to the beacon light dataref
int drefNavLight;       // this stores a handle to the nav light dataref
int drefThrottle;       // this stores a handle to the throttle position dataref

int cmdLdgLightToggle;  // this stores a handle to the landing light toggle command


void setup() 
{
 
  pinMode(LED_BUILTIN, OUTPUT);     // built in LED on arduino board will turn on and off with the status of the beacon light
  pinMode(PIN_TRIGGER, INPUT_PULLUP);       
  
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

 /*
    needed for initialization.  Parameters are:
        a texual identifier of your device 
        your function that will be called when xplane and the plugin are ready for dataref and command registrations
        your function that will be called when xplane either shuts down or unloads an aircraft model
        your function that will be called when we have requested sends of datarefs that have changed
 */
  XP.begin("XPLPro Sequence Example!", &xplRegister, &xplShutdown, &xplInboundHandler);               
  digitalWrite(LED_BUILTIN, LOW);


}


void loop() 
{
 
  XP.xloop();  //  needs to run every cycle.  

  if (!digitalRead(PIN_TRIGGER)) sequenceTrigger();

  sequenceProcess();


  

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
 

   
}

/*
 *  this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
 *  It could be called multiple times if for instance xplane is restarted or loads a different aircraft
 */
void xplRegister()         
{

  drefBeacon = XP.registerDataRef(F("sim/cockpit2/switches/beacon_on") );    
  drefNavLight = XP.registerDataRef(F("sim/cockpit/electrical/nav_lights_on") );    
  drefThrottle = XP.registerDataRef(F("sim/cockpit2/engine/actuators/throttle_ratio") );
  cmdLdgLightToggle = XP.registerCommand(F("sim/lights/landing_lights_toggle") );

 
}

void xplShutdown()
{
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}


/*
   The following code handles the sequencing of the datarefs and commands.  This would be better to encapsulate into its own
   class or at least its own file, but I put it here for simplicity in the example
*/
long int sequenceTimer;
int sequenceActive;
int sequenceCount;

void sequenceTrigger()
{
  sequenceTimer = millis();
  sequenceActive = true;
  sequenceCount = 0;

  digitalWrite(LED_BUILTIN, HIGH);          // for information purposes, turn LED on while running the sequence

}

void sequenceProcess()
{
  if (!sequenceActive) return;

  switch (sequenceCount) 
  {

    case 0 :
     if (millis() - sequenceTimer > 100)   
     { XP.datarefWrite(drefBeacon, 1);        // turn on the beacon light
       sequenceCount++;
     }
     break;

    case 1 :
      if (millis() - sequenceTimer > 2000)    
      { XP.datarefWrite(drefNavLight, 1);   // 2 seconds later turn on the nav light
        sequenceCount++;
      }
      break;
    
    case 2 :
      if (millis() - sequenceTimer > 4000)     
      { XP.commandTrigger(cmdLdgLightToggle);   // another 2 seconds later toggle the landing light on/off
        sequenceCount++;
      }
      break;
        
    case 3 :
      if (millis() - sequenceTimer > 6000)    
      { XP.datarefWrite(drefThrottle, .2F);  // had to specify the type of .2 with "F" so it knows it is a float.  You could also do (float).2
        sequenceCount++;
      }
      break;

    default :
      sequenceActive = false;
      digitalWrite(LED_BUILTIN, LOW);       // sequence complete, turn off light


  }

}