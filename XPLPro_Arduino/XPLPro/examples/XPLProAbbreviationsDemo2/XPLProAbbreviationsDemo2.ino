
// ARDUINO MEGA


#include <arduino.h>
#include <XPLPro.h>               //  include file for the X-plane direct interface
#include <LiquidCrystal_I2C.h>

XPLPro XP(&Serial);               // create an instance of it
LiquidCrystal_I2C lcd(0x26,16,2);

unsigned long startTime;            // for timing purposes
bool baroMode;
bool alt1000Mode;

//  =============================== Rotary Encoder vars ======================================================================
bool RE1_lastStateCLK;  bool RE1_lastStateDT; bool RE1_lastStateSW; const int RE1_CLK = 22; const int RE1_DT =  23; const int RE1_SW = 30;
bool RE2_lastStateCLK;  bool RE2_lastStateDT; bool RE2_lastStateSW; const int RE2_CLK = 24; const int RE2_DT =  25; const int RE2_SW = 31;
bool RE3_lastStateCLK;  bool RE3_lastStateDT; bool RE3_lastStateSW; const int RE3_CLK = 26; const int RE3_DT =  27; const int RE3_SW = 32;

//  ================================== LIGHTS STATUS =========================================================================
long int beacon;      long int landing;     long int taxi;    long int navLights;   long int strobes;

//  ================================== C O M M A N D S =======================================================================
int cmdBcnToggle;     int cmdLandToggle;  int cmdTaxiToggle;    int cmdNavToggle;   int cmdStrobeToggle;
int cmdBattToggle;    int cmdAvioToggle;  int cmdGenToggle; 

int cmdHeadUp;        int cmdHeadDown;    int cmdHeadSync;                              // on Rotaly Encoder 1
int cmdAltUp;         int cmdAltDown;     int cmdBaroUp;      int cmdBaroDown;          // on Rotary Encoder 2
int cmdAlt100_Up;     int cmdAlt100_Down; int cmdAlt1000_Up;  int cmdAlt1000_Down;      // on Rotary Encoder 2 v.2
int cmdCrsUp;         int cmdCrsDown;                                                   // on Rotary Encoder 3

// =====================================   pin LEDs  and SWITCHES ============================================================
bool beaconStatus;      bool beaconLastStatus = true;       const int beaconLed = 12;       const int beaconSwitch = 6;
bool landingStatus;     bool landingLastStatus = true;      const int landingLed = 11;      const int landingSwitch = 5;
bool taxiStatus;        bool taxiLastStatus = true;         const int taxiLed = 10;         const int taxiSwitch = 4;
bool navLightsStatus;   bool navLightsLastStatus = true;    const int navLightsLed = 9;     const int navLightsSwitch = 3;
bool strobesStatus;     bool strobesLastStatus = true;      const int strobesLed = 8;       const int strobesSwitch = 2;

bool battStatus;        bool battLastStatus = true;         const int battSwitch = 48;
bool avioStatus;        bool avioLastStatus = true;         const int avioSwitch = 49;
bool geneStatus;        bool geneLastStatus = true;         const int geneSwitch = 50;

// =======================================   END of VARs DECLARIATIONs and DEFINITIONs =======================================


void setup() 
{

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("initializing..");
    
  pinMode(RE1_CLK,INPUT);   pinMode(RE1_DT,INPUT);    pinMode(RE1_SW,INPUT_PULLUP);
  pinMode(RE2_CLK,INPUT);   pinMode(RE2_DT,INPUT);    pinMode(RE2_SW,INPUT_PULLUP);

// -------------  SWITCHes  --------------------
  pinMode(beaconSwitch, INPUT_PULLUP);
  pinMode(landingSwitch, INPUT_PULLUP);
  pinMode(navLightsSwitch, INPUT_PULLUP);
  pinMode(strobesSwitch, INPUT_PULLUP);
  pinMode(taxiSwitch, INPUT_PULLUP);
  pinMode(battSwitch, INPUT_PULLUP);
  pinMode(avioSwitch, INPUT_PULLUP);
  pinMode(geneSwitch, INPUT_PULLUP);

// ---------------  LEDs  ----------------------
  pinMode(beaconLed, OUTPUT);
  pinMode(landingLed, OUTPUT);
  pinMode(navLightsLed, OUTPUT);
  pinMode(strobesLed, OUTPUT);
  pinMode(taxiLed, OUTPUT);

  Serial.begin(XPL_BAUDRATE); // start serial interface.  Baudrate is specified in the header, dont change   

  XP.begin("XPLPro Giorgio Lights & Switches", &xplRegister, &xplShutdown, &xplInboundHandler);               
  ErrorBlink(LED_BUILTIN, 5);       // let everyone know we are awake
 
  while (!Serial)
  {
    ErrorBlink(LED_BUILTIN, 2);
    delay(300);
  }

  digitalWrite(beaconLed, LOW);
  digitalWrite(landingLed, LOW);
  digitalWrite(navLightsLed, LOW);
  digitalWrite(strobesLed, LOW);
  digitalWrite(taxiLed, LOW);

  RE1_lastStateCLK = digitalRead(RE1_CLK);   RE1_lastStateDT = digitalRead(RE1_DT); RE1_lastStateSW = digitalRead(RE1_SW);
  RE2_lastStateCLK = digitalRead(RE2_CLK);   RE2_lastStateDT = digitalRead(RE2_DT); RE2_lastStateSW = digitalRead(RE2_SW);
  RE3_lastStateCLK = digitalRead(RE3_CLK);   RE3_lastStateDT = digitalRead(RE3_DT); RE3_lastStateSW = digitalRead(RE3_SW);
  baroMode = false;
  alt1000Mode = false;
}


void loop() 
{
  XP.xloop();

// Rotary Encoders: 1 =  Heading + Head Sync      2 = Altitude ( + Altitude x 1000 on G1000)      3 = Cursor + Barometer
// =======================================================================================================================
                    checkRotaryEnc(1, RE1_CLK, RE1_DT, RE1_lastStateCLK, RE1_lastStateDT, cmdHeadDown, cmdHeadUp, RE1_SW, RE1_lastStateSW, cmdHeadSync);   
  if (!alt1000Mode) checkRotaryEnc(2, RE2_CLK, RE2_DT, RE2_lastStateCLK, RE2_lastStateDT, cmdAltDown, cmdAltUp, RE2_SW, RE2_lastStateSW, 3);
  else              checkRotaryEnc(2, RE2_CLK, RE2_DT, RE2_lastStateCLK, RE2_lastStateDT, cmdAlt1000_Down, cmdAlt1000_Up, RE2_SW, RE2_lastStateSW, 3);
  if (!baroMode)    checkRotaryEnc(3, RE3_CLK, RE3_DT, RE3_lastStateCLK, RE3_lastStateDT, cmdCrsDown, cmdCrsUp, RE3_SW, RE3_lastStateSW, 3);
  else              checkRotaryEnc(3, RE3_CLK, RE3_DT, RE3_lastStateCLK, RE3_lastStateDT, cmdBaroDown, cmdBaroUp, RE3_SW, RE3_lastStateSW, 3);

  if (millis() - startTime > 100) startTime = millis();   else return;         

  checkStatus(beaconSwitch, beaconLastStatus, cmdBcnToggle);
  checkStatus(landingSwitch, landingLastStatus, cmdLandToggle);
  checkStatus(taxiSwitch, taxiLastStatus, cmdTaxiToggle);
  checkStatus(navLightsSwitch, navLightsLastStatus, cmdNavToggle);
  checkStatus(strobesSwitch, strobesLastStatus, cmdStrobeToggle);
  checkStatus(battSwitch, battLastStatus, cmdBattToggle);
  checkStatus(avioSwitch, avioLastStatus, cmdAvioToggle);
  checkStatus(geneSwitch, geneLastStatus, cmdGenToggle);
}

void checkRotaryEnc(int devNum, int pinA, int pinB, bool& lastA, bool& lastB, int cmdCode1, int cmdCode2, int pinSw, bool& lastSw, int cmdCode3) {
  bool flagA = !digitalRead(pinA);
  bool flagB = !digitalRead(pinB);
  if (flagA != lastA) {
    if(flagB == flagA)  XP.commandTrigger(cmdCode1);
    else                XP.commandTrigger(cmdCode2);
  }
  lastA = flagA;
  lastB = flagB;
  bool flagSw = !digitalRead(pinSw);
  if (flagSw == lastSw)  return;
  if (flagSw) {
    switch (devNum) {
      case 1:
        XP.commandTrigger(cmdCode3);
        break;
      case 2:
        alt1000Mode = !alt1000Mode;
        break;
      case 3:
        baroMode =    !baroMode;
        break;
    }
  }
  lastSw = flagSw;
}

void checkStatus(int pin, bool& last, int cmdCode) {
  bool flag = !digitalRead(pin);
  if(flag == last) return;
  if(flag) XP.commandTrigger(cmdCode);
  last = flag;
}

void xplInboundHandler(inStruct *inData) {

lcd.setCursor(0,0);

  if (inData->handle == beacon) {   
    lcd.print("Beacon:  ");
    if (inData->inLong) lcd.print("ON "); //digitalWrite(beaconLed,HIGH);
    else                lcd.print("OFF"); //digitalWrite(beaconLed,LOW);
  } else if (inData->handle == landing) {   
    lcd.print("Landing: ");
   
      if (inData->inLong) lcd.print("ON "); // digitalWrite(landingLed,HIGH);
      else                lcd.print("OFF"); // digitalWrite(landingLed,LOW);
  } else if (inData->handle == taxi) {   
    lcd.print("Taxi:    ");
      if (inData->inLong) lcd.print("ON "); //digitalWrite(taxiLed,HIGH);
      else                lcd.print("OFF"); //digitalWrite(taxiLed,LOW);
  } else if (inData->handle == navLights) {  
    lcd.print("Nav:     "); 
      if (inData->inLong) lcd.print("ON ");  //digitalWrite(navLightsLed,HIGH);
      else                lcd.print("OFF"); //digitalWrite(navLightsLed,LOW);
  } else if (inData->handle == strobes) {   
    lcd.print("Strobes: ");
      if (inData->inLong) lcd.print("ON "); //digitalWrite(strobesLed,HIGH);
      else                lcd.print("OFF"); //digitalWrite(strobesLed,LOW);
  }
}

void xplRegister()         
{
beacon = XP.registerDataRef(F("LTbcn") );    
  XP.requestUpdates(beacon, 100, 0);
  taxi = XP.registerDataRef(F("LTtax") );    
  XP.requestUpdates(taxi, 100, 0);
  landing = XP.registerDataRef(F("LTlnd") );    
  XP.requestUpdates(landing, 100, 0);
  navLights = XP.registerDataRef(F("LTnav") );    
  XP.requestUpdates(navLights, 100, 0);
  strobes = XP.registerDataRef(F("LTstr") );    
  XP.requestUpdates(strobes, 100, 0);

  cmdBcnToggle =    XP.registerCommand(F("sim/lights/beacon_lights_toggle"));
  cmdLandToggle =   XP.registerCommand(F("sim/lights/landing_lights_toggle"));
  cmdNavToggle =    XP.registerCommand(F("sim/lights/nav_lights_toggle"));
  cmdTaxiToggle =   XP.registerCommand(F("sim/lights/taxi_lights_toggle"));
  cmdStrobeToggle = XP.registerCommand(F("sim/lights/strobe_lights_toggle"));
  cmdBattToggle =   XP.registerCommand(F("sim/electrical/batteries_toggle"));
  cmdAvioToggle =   XP.registerCommand(F("sim/systems/avionics_toggle"));
  cmdAltUp =        XP.registerCommand(F("sim/autopilot/altitude_up"));
  cmdAltDown =      XP.registerCommand(F("sim/autopilot/altitude_down"));
  cmdHeadUp =       XP.registerCommand(F("sim/autopilot/heading_up"));
  cmdHeadDown =     XP.registerCommand(F("sim/autopilot/heading_down"));
  cmdBaroUp =       XP.registerCommand(F("sim/instruments/barometer_up"));
  cmdBaroDown =     XP.registerCommand(F("sim/instruments/barometer_down"));
  cmdHeadSync =     XP.registerCommand(F("sim/GPS/g1000n1_hdg_sync"));
//  cmdHeadSync =     XP.registerCommand(F("sim/autopilot/heading_sync"));
  cmdAlt100_Up =    XP.registerCommand(F("sim/GPS/g1000n1_alt_inner_up"));
  cmdAlt100_Down =  XP.registerCommand(F("sim/GPS/g1000n1_alt_inner_down"));
  cmdAlt1000_Up =   XP.registerCommand(F("sim/GPS/g1000n1_alt_outer_up"));
  cmdAlt1000_Down = XP.registerCommand(F("sim/GPS/g1000n1_alt_outer_down"));
  cmdGenToggle =    XP.registerCommand(F("sim/electrical/generators_toggle"));
  cmdCrsUp =        XP.registerCommand(F("sim/GPS/g1000n1_crs_up"));
  cmdCrsDown =      XP.registerCommand(F("sim/GPS/g1000n1_crs_down"));
}

void xplShutdown()
{
  // if you need to do things when xplane shuts down or unloads an aircraft, do it here.
  
}

void ErrorBlink(int pin, int count)
{
  for (int i = 0; i< count; i++)
  { 
   digitalWrite(pin, HIGH);
   delay(200);
   digitalWrite(pin, LOW);
   delay(100);
  }
}
