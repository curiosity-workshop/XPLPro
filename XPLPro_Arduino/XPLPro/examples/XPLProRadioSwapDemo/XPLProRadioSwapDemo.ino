

/*
 * 
 * XPLProRadioSwapDemo
 * 
 * Created by Curiosity Workshop for XPL/Pro arduino->XPlane system.
 * 
 * This registers a command to swap the standby frequency with the active frequency on com1 and displays it on an i2c 2x16 LCD display
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


#define PIN_COMSWAP     22          // momentary switch for nav swap


int drefComActive;              // handle to dataref containing active com frequency
int drefComStandby;             // handle to dataref containing standby com frequency
int cmdComSwap;                // a command handle for toggling standby/active com frequencyn


void setup() 
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("XPLDemo");
    
  pinMode(PIN_COMSWAP, INPUT_PULLUP);     // if you are doing pin handling yourself you will want a pullup resistor or use the built-in one
   
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

 /*
    needed for initialization.  Parameters are:
        a texual identifier of your device 
        your function that will be called when xplane and the plugin are ready for dataref and command registrations
        your function that will be called when xplane either shuts down or unloads an aircraft model
        your function that will be called when we have requested sends of datarefs that have changed
 */
  XP.begin("XPLPro Radio Swap Demo!", &xplRegister, &xplShutdown, &xplInboundHandler);               
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
  if (!digitalRead(PIN_COMSWAP) ) XP.commandTrigger(cmdComSwap);   
 
  
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
  if (inData->handle == drefComActive)
  {  
     lcd.setCursor(0,0);
     lcd.print("COM1: ");       
     lcd.print(inData->inLong);
     
  }

  if (inData->handle == drefComStandby)                      
  {   lcd.setCursor(0,1);
      lcd.print("STBY: ");
      lcd.print(inData->inLong);            // casted to INT to remove decimals
  }

    
  
}

void xplRegister()          // this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
{

 
 
/*
 * 
 * On non-AVR boards remove the F() macro.
 * 
 */
  drefComActive = XP.registerDataRef(F("sim/cockpit/radios/com1_freq_hz") );    
  XP.requestUpdates(drefComActive, 100, 0);          // Tell xplane to send us updates when the value changes.  
                                                  // 100 means don't update more often than every 100ms and 0 is a resolution divider which is explained in another dataref, use 0 if no divider required
  drefComStandby = XP.registerDataRef(F("sim/cockpit/radios/com1_stdby_freq_hz") );    
  XP.requestUpdates(drefComStandby, 100, 0);

  cmdComSwap = XP.registerCommand(F("sim/radios/com1_standy_flip") );
  
}

void xplShutdown()
{
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}
