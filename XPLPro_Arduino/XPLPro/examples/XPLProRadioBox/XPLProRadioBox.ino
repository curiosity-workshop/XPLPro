/*
 * 
 * XPLProRadioBox
 * 
 * Created by Curiosity Workshop for XPL/Pro arduino->XPlane system.
 * 
 * This sketch was developed and tested on an Arduino Mega.
 * 
 *   Video of this sketch in action:  https://youtu.be/T1LJyjB_sV8
 *
 *  To report problems, download updates and examples, suggest enhancements or get technical support:
 * 
 *    discord:  https://discord.gg/RacvaRFsMW
 *    patreon:  www.patreon.com/curiosityworkshop
 *    YouTube:  https://youtube.com/channel/UCISdHdJIundC-OSVAEPzQIQ
 * 
 * 
 */

#include <LiquidCrystal_I2C.h>
#include <Encoder.h>
#include <AceButton.h>
#include <XPLPro.h>              //  include file for the X-plane direct interface 

XPLPro XP(&Serial);      // create an instance of it

using namespace ace_button;
void handleButton(AceButton* button, uint8_t eventType,  uint8_t /*buttonState*/) ;     // header for button callback

/**
 * Get the current radio mode according to the rotary switch
 */
 
 #define MODE_COM1 0
 #define MODE_COM2 1
 #define MODE_NAV1 2
 #define MODE_NAV2 3M
 #define MODE_XPDR 4
 #define MODE_ADF1 5
 #define MODE_MAX  5
 
#define PIN_ENCODER1_A      18  
#define PIN_ENCODER1_B      19
#define PIN_BUTTON_ENCODER1 16

#define PIN_BUTTON_MODE      7
#define PIN_BUTTON_FLIP      9  

LiquidCrystal_I2C lcd(0x26,16,2);           // Adjust this to match your LCD
Encoder encoder1(PIN_ENCODER1_A, PIN_ENCODER1_B);

AceButton encoder1Button(PIN_BUTTON_ENCODER1);
AceButton modeButton(PIN_BUTTON_MODE);
AceButton flopButton(PIN_BUTTON_FLIP);

int cmdCom1CourseDown;
int cmdCom1CourseUp;
int cmdCom1FineDown;
int cmdCom1FineUp;

int cmdCom2CourseDown;
int cmdCom2CourseUp;
int cmdCom2FineDown;
int cmdCom2FineUp;

int cmdNav1CourseDown;
int cmdNav1CourseUp;
int cmdNav1FineDown;
int cmdNav1FineUp;

int cmdNav2CourseDown;
int cmdNav2CourseUp;
int cmdNav2FineDown;
int cmdNav2FineUp;

int cmdXpdr12Down;
int cmdXpdr12Up;
int cmdXpdr34Down;
int cmdXpdr34Up;  

int cmdAdf110Down;
int cmdAdf110Up;
int cmdAdfOTDown;
int cmdAdfOTUp;
int cmdCom1Flip;
int cmdCom2Flip;
int cmdNav1Flip;
int cmdNav2Flip;
int cmdAdf1Flip;

int drefCom1Freq;
int drefCom1StbyFreq;
int drefCom2Freq;
int drefCom2StbyFreq;

int drefNav1Freq;
int drefNav1StbyFreq;
int drefNav2Freq;
int drefNav2StbyFreq;

int drefXpdrCode;
int drefXpdrMode;

int drefADFFreq;
int drefADFstbyFreq;




long int Com1 = 0;
long int Com1st = 0;
long int Com2 = 0;
long int Com2st = 0;
long int Nav1 = 0;
long int Nav1st = 0;
long int Nav2 = 0;
long int Nav2st = 0;
long int Adf1 = 0;
long int Adf1st = 0;

long int Xpdr = 0;
long int xpdrMode = 0;

int mode = 0;
int encMode = 0;

int encoderPos;


//------------------------------------------
void setup()  
{ 
 
  lcd.init();
  lcd.backlight();
  
  mode = MODE_COM1;

  pinMode(PIN_BUTTON_MODE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_FLIP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_ENCODER1, INPUT_PULLUP);


  // Configure the ButtonConfig with the event handler.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleButton);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

 
  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   
  XP.begin("XPLPro Radiobox Demo!", &xplRegister, &xplShutdown, &xplInboundHandler);     
  displayUpdate();
} 

//------------------------------------------
void loop()   
{ 

  XP.xloop();  //  needs to run every cycle.    
  
  encoder1Button.check();
  modeButton.check();
  flopButton.check();
  

  // Get encoder direction (-1, 0, +1)
  int encVal = encoder1.read() / 4;
  if (encVal < 0)
  {   switch(mode)
      {
        case MODE_COM1:
          if (encoderPos == 0) XP.commandTrigger(cmdCom1FineDown);
          if (encoderPos == 1) XP.commandTrigger(cmdCom1CourseDown);
          break;

         case MODE_COM2:
          if (encoderPos == 0) XP.commandTrigger(cmdCom2FineDown);
          if (encoderPos == 1) XP.commandTrigger(cmdCom2CourseDown);
          break;  

         case MODE_NAV1:
          if (encoderPos == 0) XP.commandTrigger(cmdNav1FineDown);
          if (encoderPos == 1) XP.commandTrigger(cmdNav1CourseDown);
          break;

         case MODE_NAV2:
          if (encoderPos == 0) XP.commandTrigger(cmdNav2FineDown);
          if (encoderPos == 1) XP.commandTrigger(cmdNav2CourseDown);
          break;  
        
        case MODE_XPDR:
          if (encoderPos == 0) XP.commandTrigger(cmdXpdr34Down);
          if (encoderPos == 1) XP.commandTrigger(cmdXpdr12Down); 
          break;

        case MODE_ADF1:
          if (encoderPos == 0) XP.commandTrigger(cmdAdf110Down);
          if (encoderPos == 1) XP.commandTrigger(cmdAdfOTDown);
          break;
      }
      encoder1.write(0);


 //   #define MODE_COM1 0
 //#define MODE_COM2 1
 //#define MODE_NAV1 2
 //#define MODE_NAV2 3
 //#define MODE_XPDR 4
 //#define MODE_ADF1 5
  }
  if (encVal > 0)
  {   switch(mode)
      {
        case MODE_COM1:
          if (encoderPos == 0) XP.commandTrigger(cmdCom1FineUp);
          if (encoderPos == 1) XP.commandTrigger(cmdCom1CourseUp);
          break;
        case MODE_COM2:
          if (encoderPos == 0) XP.commandTrigger(cmdCom2FineUp);
          if (encoderPos == 1) XP.commandTrigger(cmdCom2CourseUp);
          break;  

         case MODE_NAV1:
          if (encoderPos == 0) XP.commandTrigger(cmdNav1FineUp);
          if (encoderPos == 1) XP.commandTrigger(cmdNav1CourseUp);
          break;

         case MODE_NAV2:
          if (encoderPos == 0) XP.commandTrigger(cmdNav2FineUp);
          if (encoderPos == 1) XP.commandTrigger(cmdNav2CourseUp);
          break;  
        
         case MODE_XPDR:
          if (encoderPos == 0) XP.commandTrigger(cmdXpdr34Up);
          if (encoderPos == 1) XP.commandTrigger(cmdXpdr12Up); 
          break;
 
         case MODE_ADF1:
          if (encoderPos == 0) XP.commandTrigger(cmdAdf110Up);
          if (encoderPos == 1) XP.commandTrigger(cmdAdfOTUp);
          break;


      }
      encoder1.write(0);


 
 //#define MODE_ADF1 5
  }
}      

// The event handler for the buttons.
void handleButton(AceButton* button, uint8_t eventType,
    uint8_t /*buttonState*/) 
{
  uint8_t pin = button->getPin();

  if (pin == PIN_BUTTON_ENCODER1) 
  {
    switch (eventType) 
    {
      // Interpret a Released event as a Pressed event, to distiguish it
      // from a LongPressed event.
      case AceButton::kEventReleased:
 
        encoderPos ++;
        if (encoderPos >1) encoderPos = 0;   
        displayUpdate();
     
        break;
     
    }
  
  }

  if (pin == PIN_BUTTON_MODE)
  {
    
    switch (eventType) 
    {
      // Interpret a Released event as a Pressed event, to distiguish it
      // from a LongPressed event.
      case AceButton::kEventReleased:
        mode++;
        if (mode > MODE_MAX) mode = 0;
        encoderPos = 1;
        displayUpdate();
        break;
  
    } 
  }

  if (pin == PIN_BUTTON_FLIP)
  {
    
    switch (eventType) 
    {
      // Interpret a Released event as a Pressed event, to distiguish it
      // from a LongPressed event.
      case AceButton::kEventReleased:
        switch(mode)
        {
          case MODE_COM1:   XP.commandTrigger(cmdCom1Flip);       break;
          case MODE_COM2:   XP.commandTrigger(cmdCom2Flip);       break;
          case MODE_NAV1:   XP.commandTrigger(cmdNav1Flip);       break;
          case MODE_NAV2:   XP.commandTrigger(cmdNav2Flip);       break;
          case MODE_ADF1:   XP.commandTrigger(cmdAdf1Flip);       break;
          case MODE_XPDR:   
            xpdrMode ++;
            if (xpdrMode > 3) xpdrMode = 0;               // 3 is the highest standard transponder mode.  Increase if you want other available modes.
            XP.datarefWrite(drefXpdrMode, xpdrMode);
            displayUpdate();
            break;
 
        } 
        break;
  
    } 
  }
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

  if (inData->handle == drefCom1Freq)     { Com1 = inData->inLong;      displayUpdate();  return; }
  if (inData->handle == drefCom1StbyFreq) { Com1st = inData->inLong;    displayUpdate();  return; }
  if (inData->handle == drefCom2Freq)     { Com2 = inData->inLong;      displayUpdate();  return; }
  if (inData->handle == drefCom2StbyFreq) { Com2st = inData->inLong;    displayUpdate();  return; }
  if (inData->handle == drefNav1Freq)     { Nav1 = inData->inLong;      displayUpdate();  return; }
  if (inData->handle == drefNav1StbyFreq) { Nav1st = inData->inLong;    displayUpdate();  return; }
  if (inData->handle == drefNav2Freq)     { Nav2 = inData->inLong;      displayUpdate();  return; }
  if (inData->handle == drefNav2StbyFreq) { Nav2st = inData->inLong;    displayUpdate();  return; }
  if (inData->handle == drefXpdrCode)     { Xpdr = inData->inLong;      displayUpdate();  return; }
  if (inData->handle == drefXpdrMode)     { xpdrMode = inData->inLong;  displayUpdate();  return; }
  if (inData->handle == drefADFFreq)      { Adf1 = inData->inLong;      displayUpdate();  return; }
  if (inData->handle == drefADFstbyFreq)  { Adf1st = inData->inLong;    displayUpdate();  return; }

}

void xplRegister()          // this is the function we set as a callback for when the plugin is ready to receive dataref and command bindings requests
{

  drefCom1Freq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/com1_frequency_hz"));
    XP.requestUpdates(drefCom1Freq, 100, 0);   
  drefCom1StbyFreq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/com1_standby_frequency_hz"));
    XP.requestUpdates(drefCom1StbyFreq, 100, 0);
  drefCom2Freq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/com2_frequency_hz"));
    XP.requestUpdates(drefCom2Freq, 100, 0);   
  drefCom2StbyFreq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/com2_standby_frequency_hz"));
    XP.requestUpdates(drefCom2StbyFreq, 100, 0);

  drefNav1Freq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/nav1_frequency_hz"));
    XP.requestUpdates(drefNav1Freq, 100, 0);   
  drefNav1StbyFreq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/nav1_standby_frequency_hz"));
    XP.requestUpdates(drefNav1StbyFreq, 100, 0);
  drefNav2Freq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/nav2_frequency_hz"));
    XP.requestUpdates(drefNav2Freq, 100, 0);   
  drefNav2StbyFreq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/nav2_standby_frequency_hz"));
    XP.requestUpdates(drefNav2StbyFreq, 100, 0);

  drefXpdrCode = XP.registerDataRef(F("sim/cockpit/radios/transponder_code"));
    XP.requestUpdates(drefXpdrCode, 100, 0);
  drefXpdrMode = XP.registerDataRef(F("sim/cockpit2/radios/actuators/transponder_mode"));
    XP.requestUpdates(drefXpdrMode, 100, 0);

  drefADFFreq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/adf1_frequency_hz"));
    XP.requestUpdates(drefADFFreq, 100, 0);
  drefADFstbyFreq = XP.registerDataRef(F("sim/cockpit2/radios/actuators/adf1_standby_frequency_hz"));
    XP.requestUpdates(drefADFstbyFreq, 10, 0);

/*
 * Now register commands.  
 * 
 */
    cmdCom1CourseDown = XP.registerCommand(F("sim/radios/stby_com1_coarse_down"));
    cmdCom1CourseUp   = XP.registerCommand(F("sim/radios/stby_com1_coarse_up"));
    cmdCom1FineDown   = XP.registerCommand(F("sim/radios/stby_com1_fine_down"));
    cmdCom1FineUp     = XP.registerCommand(F("sim/radios/stby_com1_fine_up"));
    cmdCom2CourseDown = XP.registerCommand(F("sim/radios/stby_com2_coarse_down"));
    cmdCom2CourseUp   = XP.registerCommand(F("sim/radios/stby_com2_coarse_up"));
    cmdCom2FineDown   = XP.registerCommand(F("sim/radios/stby_com2_fine_down"));
    cmdCom2FineUp     = XP.registerCommand(F("sim/radios/stby_com2_fine_up"));

    cmdNav1CourseDown = XP.registerCommand(F("sim/radios/stby_nav1_coarse_down"));
    cmdNav1CourseUp   = XP.registerCommand(F("sim/radios/stby_nav1_coarse_up"));
    cmdNav1FineDown   = XP.registerCommand(F("sim/radios/stby_nav1_fine_down"));
    cmdNav1FineUp     = XP.registerCommand(F("sim/radios/stby_nav1_fine_up"));
    cmdNav2CourseDown = XP.registerCommand(F("sim/radios/stby_nav2_coarse_down"));
    cmdNav2CourseUp   = XP.registerCommand(F("sim/radios/stby_nav2_coarse_up"));
    cmdNav2FineDown   = XP.registerCommand(F("sim/radios/stby_nav2_fine_down"));
    cmdNav2FineUp     = XP.registerCommand(F("sim/radios/stby_nav2_fine_up"));

    cmdXpdr12Down     = XP.registerCommand(F("sim/transponder/transponder_12_down"));
    cmdXpdr12Up       = XP.registerCommand(F("sim/transponder/transponder_12_up"));
    cmdXpdr34Down     = XP.registerCommand(F("sim/transponder/transponder_34_down"));
    cmdXpdr34Up       = XP.registerCommand(F("sim/transponder/transponder_34_up"));
    
    cmdAdf110Down     = XP.registerCommand(F("sim/radios/stby_adf1_ones_tens_down"));
    cmdAdf110Up       = XP.registerCommand(F("sim/radios/stby_adf1_ones_tens_up"));
    cmdAdfOTDown      = XP.registerCommand(F("sim/radios/stby_adf1_hundreds_thous_down"));
    cmdAdfOTUp        = XP.registerCommand(F("sim/radios/stby_adf1_hundreds_thous_up"));

    cmdCom1Flip       = XP.registerCommand(F("sim/radios/com1_standy_flip"));
    cmdCom2Flip       = XP.registerCommand(F("sim/radios/com2_standy_flip"));
    cmdNav1Flip       = XP.registerCommand(F("sim/radios/nav1_standy_flip"));
    cmdNav2Flip       = XP.registerCommand(F("sim/radios/nav2_standy_flip"));   

    cmdAdf1Flip       = XP.registerCommand(F("sim/radios/adf1_standy_flip"));
    
}

void xplShutdown()
{
  lcd.clear();
  displayUpdate();
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}


void printFixedWidth(int x, int y, float in, byte width, byte decimals);

/**
 * Show selected mode on screen
 */
void displayUpdate() 
{
  //lcd.clear();
  lcd.setCursor(0, 0);  

  if (!XP.connectionStatus())
  { lcd.clear();
    lcd.print("INOP..");
    return;
  }
  
    switch(mode) 
    {
      

      case MODE_COM1: 
        lcd.print("COM 1 "); 
        
        printFixedWidth(0,1, (float)Com1/100, 5, 2);
        printFixedWidth(10,1, (float)Com1st/100, 5, 2);
       
        lcd.setCursor(10,0);
        if (encoderPos == 0)  lcd.print("    __");
        if (encoderPos == 1)  lcd.print("___   ");

        break;

      case MODE_COM2: 
        lcd.print("COM 2"); 
        printFixedWidth(0,1, (float)Com2/100, 5, 2);
        printFixedWidth(10,1, (float)Com2st/100, 5, 2);
        
        lcd.setCursor(10,0);
        if (encoderPos == 0)  lcd.print("    __");
        if (encoderPos == 1)  lcd.print("___   ");
        break;

      case MODE_NAV1: 
        lcd.print("NAV 1"); 

        printFixedWidth(0,1, (float)Nav1/100, 5, 2);
        printFixedWidth(10,1, (float)Nav1st/100, 5, 2);

        lcd.setCursor(10,0);
        if (encoderPos == 0)  lcd.print("    __");
        if (encoderPos == 1)  lcd.print("___   ");
        break;

      case MODE_NAV2: 
        lcd.print("NAV 2"); 
       
        printFixedWidth(0,1, (float)Nav2/100, 5, 2);
        printFixedWidth(10,1, (float)Nav2st/100, 5, 2);

        lcd.setCursor(10,0);
        if (encoderPos == 0)  lcd.print("    __");
        if (encoderPos == 1)  lcd.print("___   ");
        break;
      
      case MODE_XPDR: 
        lcd.print("XPDR "); 
        printFixedWidth(10,1, (float)Xpdr, 4, 0);
      
        lcd.print("  ");
        lcd.setCursor(0,1);
        switch(xpdrMode) 
        {
          case 0:  lcd.print("OFF   "); break;
          case 1:  lcd.print("STBY  "); break;
          case 2:  lcd.print("ON    "); break;
          case 3:  lcd.print("ALT   "); break;  
          case 4:  lcd.print("TEST  "); break; 
          case 5:  lcd.print("GND   "); break; 
          case 6:  lcd.print("TA    "); break;
          case 7:  lcd.print("TA/RA "); break;
          default: lcd.print("???   ");
        }
        lcd.setCursor(10,0);
        if (encoderPos == 0)  lcd.print("  __  ");
        if (encoderPos == 1)  lcd.print("__    ");
        break;

      case MODE_ADF1: 
        lcd.print("ADF1 "); 
        printFixedWidth(0,1, (float)Adf1, 4, 0);
        printFixedWidth(10,1, (float)Adf1st, 4, 0);
              
        lcd.setCursor(10,0);

        if (encoderPos == 0)  lcd.print("  __  ");
        if (encoderPos == 1)  lcd.print("__    ");
      
        break;
      
      default: 
        lcd.print("-----=INOP=-----");
    }
}

/**
 * Print numbers on fixed width
 * @param out reference to print interface
 * @param in value to format
 * @param width number of total digits
 * @param decimal number of decimal digits
 */
void printFixedWidth(int inX, int inY, float in, byte width, byte decimals)
{
  
  lcd.setCursor(inX, inY);
  float temp = in;
  
  if (decimals == 0){
    temp += 0.5;
  }
 
  if (in < 0){
    width--;
  }
 
  width -= decimals + 1;
 
  if (width < 0){
    width = 0;
  }
 
  while (temp > 10 && width){
    temp /= 10;
    width--;
  }
 
  if (in < 0){
    lcd.print('-');
  }
 
  while (width){
    lcd.print('0');
    width--;
  }
 
  lcd.print(abs(in), decimals);
}
