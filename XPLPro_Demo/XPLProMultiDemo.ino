

/*
 * 
 * XPLProMultiDemo
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
#include <LiquidCrystal_I2C.h>

#include <XPLPro.h>              //  include file for the X-plane direct interface 
XPLPro XP(&Serial);      // create an instance of it

LiquidCrystal_I2C lcd(0x26,16,2);
long int startTime;


#define PIN_GOAROUND    22          // go around button
#define PIN_GEARTOGGLE  23           // pin for gear toggle

#define PIN_PAUSE         9         // pin to pause xplane
#define PIN_NAVLIGHT      24        // pin for nav light switch
#define PIN_LANDINGLIGHT  7         // pin for landing light switch
#define PIN_SAYSOMETHING  10        // for demonstration of speech function
#define PIN_THROTTLE1      A0            // throttle 1
#define PIN_THROTTLE2      A1            // throttle 2

#define PIN_LEFTGEARLED   40        // for nose gear LED
#define PIN_NOSEGEARLED   41        //     left gear LED
#define PIN_RIGHTGEARLED  42        //     right gear LED


int drefBeacon;
int drefNavLight, navlightPrevious = -1;
int drefLdgLight;
int drefGearDeployed;
int drefThrottle;  float throttle1Previous;  float throttle2Previous;
int drefEngineRPM;


int cmdGearToggle;             // a handle for toggling landing gear up and down
int cmdPause;                   // pause the sim
int cmdToga;                    // take off/go around


void setup() 
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("XPLPro Demo!");
  
  pinMode(PIN_GEARTOGGLE, INPUT_PULLUP);     // if you are doing pin handling yourself you will want a pullup resistor or use the built-in one
  pinMode(PIN_NAVLIGHT, INPUT_PULLUP);        // otherwise the value will be erratic when the pin is open and nobody likes that.
  pinMode(PIN_GOAROUND, INPUT_PULLUP);
  pinMode(PIN_LANDINGLIGHT, INPUT_PULLUP);
  pinMode(PIN_SAYSOMETHING, INPUT_PULLUP);
  pinMode(PIN_PAUSE, INPUT_PULLUP);
   
  pinMode(LED_BUILTIN, OUTPUT);     // built in LED on arduino board for debug and demonstration purposes
  pinMode(PIN_LEFTGEARLED, OUTPUT);
  pinMode(PIN_NOSEGEARLED, OUTPUT);
  pinMode(PIN_RIGHTGEARLED, OUTPUT);
  
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

 /*
    needed for initialization.  Parameters are:
        a texual identifier of your device 
        your function that will be called when xplane and the plugin are ready for dataref and command registrations
        your function that will be called when xplane either shuts down or unloads an aircraft model
        your function that will be called when we have requested sends of datarefs that have changed
 */
  XP.begin("XPLCore Multi Demo!", &registerXplaneStuff, &shutdownXplaneStuff, &inboundHandler);               
  digitalWrite(LED_BUILTIN, LOW);


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


/*
 * This reads the status of the assigned pins and triggers commands accordingly.  It would be best to use a button library for this for debounce and one-shot purposes
 * I am not using one for simplicity in the demo.
 */
  if (!digitalRead(PIN_GEARTOGGLE)) XP.commandTrigger(cmdGearToggle);   
  if (!digitalRead(PIN_GOAROUND))   XP.commandTrigger(cmdToga);
  if (!digitalRead(PIN_PAUSE))      XP.commandTrigger(cmdPause);
  

  if (digitalRead(PIN_NAVLIGHT) != navlightPrevious)      // to conserve the flow of data we should keep track of the previously sent value so we don't send a new one every cycle
  {
    navlightPrevious = digitalRead(PIN_NAVLIGHT);
    XP.datarefWrite(drefNavLight, navlightPrevious);
  }
  
  //landingLights = digitalRead(PIN_LANDINGLIGHT);


  
  
/*
 * This reads the status of the assigned pin and sends a message to initiation speech.  This should probably use a switch library for one-shot so it 
 * doesnt repeat send/stutter while it is being held down.  For now it works for demo purposes. 
 */
  if (!digitalRead(PIN_SAYSOMETHING)) XP.sendSpeakMessage("speech demonstration");

/*
 * Read analog position of potentiometers representing throttles.  Use the mapfloat function (below) to change the read value 0-1024 to a float 0.0 - 1.0
 * In this case the throttle dataref is an array, so we specify which element of the array as the third parameter of datarefWrite.
 */
  int throttle1Current = analogRead(PIN_THROTTLE1) /10;          // I am dividing by 10 here because I don't need 1024 units of resolution for the throttle position and this will reduce dataflow.
  if (throttle1Current != throttle1Previous)     
  {
    throttle1Previous = throttle1Current;
    XP.datarefWrite(drefThrottle, mapfloat(throttle1Previous, 0, 102, 0, 1 ), 0);
  }

  int throttle2Current = analogRead(PIN_THROTTLE2) /10;
  if (throttle2Current != throttle2Previous)     
  {
    throttle2Previous = throttle2Current;
    XP.datarefWrite(drefThrottle, mapfloat(throttle2Previous, 0, 102, 0, 1 ), 1);
  }

 
  
}

/*
 * This function is the callback function we specified that will be called any time our requested data is sent to us.
 * handle is the handle to the dataref.  The following variables within the plugin are how we receive the data:
 * 
 * readValueLong        If the dataref is of type INT
 * readValueFloat       If the dataref is of type FLOAT
 * readValueElement     If the dataref is an array, this is the element that changed
 */
void inboundHandler(int handle)
{
  if (handle == drefBeacon)
  {   if (XP.readValueLong)   digitalWrite(LED_BUILTIN, HIGH);        // if beacon is on set the builtin led on
      else                    digitalWrite(LED_BUILTIN, LOW);
  }

  if (handle == drefEngineRPM)                      // display RPM on the LCD
  {   lcd.setCursor(0,1);
      lcd.print("Alt: ");
      lcd.print((int)XP.readValueFloat);            // casted to INT to remove decimals
  }

/*
 * This reads the status of the landing gear.  You could turn on lights as in this example.  The position is represented as a float between 0 and 1, 1 representing down and locked.
 */
  if (handle == drefGearDeployed)
  { switch (XP.readValueElement)
    {
      case 0:     // nose gear
        if (XP.readValueFloat == 1)    digitalWrite(PIN_NOSEGEARLED, HIGH);   else digitalWrite(PIN_NOSEGEARLED, LOW);
        break;

      case 1:     // Left Main 
        if (XP.readValueFloat == 1)    digitalWrite(PIN_LEFTGEARLED, HIGH);   else digitalWrite(PIN_LEFTGEARLED, LOW);   
        break;

      case 2:     // Right Main
        if (XP.readValueFloat == 1)    digitalWrite(PIN_RIGHTGEARLED, HIGH);  else digitalWrite(PIN_RIGHTGEARLED, LOW);   
        break;
  
    }
  }     
     
    
  
}

void registerXplaneStuff()          // this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
{

 
 // register whatever datarefs we want to use.
  // Parameters are:  registerDataRef(  char *        dataref name, a c style string that identifies the dataref
  //                                    int           read mode, can be XPL_READ or XPL_WRITE or XPL_READWRITE
  //                                    
     

/*
 * This example registers a dataref for the beacon light.  
 * In the loop section of the code we will turn on/off the LED on the arduino board to represent the status of the beacon light within xplane.
 */
  drefBeacon = XP.registerDataRef("sim/cockpit2/switches/beacon_on");    
  XP.requestUpdates(drefBeacon, 100, 0);          // Tell xplane to send us updates when the status of the beacon light changes.  
                                                  // 100 means don't update more often than every 100ms and 0 is a resolution divider which is explained in another dataref, use 0 if no divider required
  

/*
 * This example registers a dataref for the nav light.  This time we will control the status of the light with a switch.
 * In the loop section of the code we will check the status of the switch send it to the navLight dataref within Xplane.
 * To test this, connect one side of a normal toggle switch to ground and the other to the pin used in the #define for PIN_NAVLIGHT (9)
 */
  drefNavLight = XP.registerDataRef("sim/cockpit/electrical/nav_lights_on");    
 
  
/*
 * The text for the names of datarefs and commands can be a large memory hit for many arduino boards.  Here are some ways to reduce the impact:
 * 
 * This first one utilizes abbreviations.  In the folder where the XPLDirect plugin is installed there is a file called abbreviations.txt.  You can add abbreviations to this as you like.
 * The plugin will search for the abbreviation string you use for dataref names and command names, then convert them to the full length specified.  Example:
 * 
 * LTland = sim/cockpit/electrical/landing_lights_on
 * 
 * This comes preset in the distrubution version of the abbreviations.txt file so we will try it in the example:
 */
  drefLdgLight = XP.registerDataRef("LTland");    // "LTland" will be converted to "sim/cockpit/electrical/landing_lights_on"


/* Another way to save RAM on AVR boards such as the NANO and the UNO and the MEGA is to use the F() macro to store the string in flash memory rather than RAM.  
 * It would look like this:
 * 
 *   Xc.registerDataRef(F("sim/cockpit/electrical/nav_lights_on"), XPL_READ);
 *   // NOT IMPLEMENTED YET!
 */
  
  
/*
 * more examples for the testing box
 */
  drefGearDeployed = XP.registerDataRef("sim/flightmodel2/gear/deploy_ratio");            // this will be updated from xplane to tell us what position the landing gear is in
  XP.requestUpdates(drefGearDeployed, 100, 1, 0);          // Tell xplane to send us updates when the status of the gear position changes.  
  XP.requestUpdates(drefGearDeployed, 100, 1, 1);          // 100 means don't update more often than every 100ms and .1 is a resolution divider to reduce data flow.  eg we probably arent interested in .001 of gear position.
  XP.requestUpdates(drefGearDeployed, 100, 1, 2);          // The additional parameter is the array element to reference, since this dataref is an array of values.  0=nose, 1=left, 2=right
    
  drefThrottle = XP.registerDataRef("sim/cockpit2/engine/actuators/throttle_ratio");      // This is an array dataref.  We will be sending this data from a potentiometer

  drefEngineRPM = XP.registerDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot");   // indicated altitude for display on the LCD screen.  This is a float 
  XP.requestUpdates(drefEngineRPM, 100, 10);                                                // divide by 10 to show increments of 10 feet
  
   
/*
 * Now register commands.  
 * 
 */
  cmdPause       = XP.registerCommand("sim/operation/pause_toggle");
  cmdToga        = XP.registerCommand("sim/autopilot/take_off_go_around");
  cmdGearToggle  = XP.registerCommand("sim/flight_controls/landing_gear_toggle");

  
}

void shutdownXplaneStuff()
{
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}

/*
 * Floating point version of the map function
 */
float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}
