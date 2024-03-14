

/*
 * 
 * XPLProAbbreviationsDemo
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



int drefBeaconLight;
int drefNavLight, navlightPrevious = -1;
int drefLdgLight;
int drefTaxiLight;
int drefStrobeLight;

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
 
  pinMode(PIN_PAUSE, INPUT_PULLUP);
   
  pinMode(LED_BUILTIN, OUTPUT);     // built in LED on arduino board for debug and demonstration purposes
 
  
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

 /*
    needed for initialization.  Parameters are:
        a texual identifier of your device 
        your function that will be called when xplane and the plugin are ready for dataref and command registrations
        your function that will be called when xplane either shuts down or unloads an aircraft model
        your function that will be called when we have requested sends of datarefs that have changed
 */
  XP.begin("XPLPro Abbreviation Demo!", &xplRegister, &xplShutdown, &xplInboundHandler);               
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
  if (inData->handle == drefBeaconLight)
  {   if (inData->inLong)   digitalWrite(LED_BUILTIN, HIGH);        // if beacon is on set the builtin led on
      else                    digitalWrite(LED_BUILTIN, LOW);
  }


    
  
}

void xplRegister()          // this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
{

 
  drefLdgLight = XP.registerDataRef(F("LTland") );    // "LTland" will be converted to "sim/cockpit/electrical/landing_lights_on"
  drefTaxiLight = XP.registerDataRef(F("LTtaxi"));    // other abbreviations are listed in the abbreviations.txt file in the plugin folder.
  drefBeaconLight = XP.registerDataRef(F("LTbcn"));
  drefStrobeLight = XP.registerDataRef(F("LTstrobe"));
  drefNavLight = XP.registerDataRef(F("LTnav"));
  
   
/*
 * Now register commands.  
 * 
 */
  cmdPause       = XP.registerCommand(F("CMDpause") );
  cmdToga        = XP.registerCommand(F("CMDtoga") );
  cmdGearToggle  = XP.registerCommand(F("CMDgearToggle") );

  
}

void xplShutdown()
{
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}
