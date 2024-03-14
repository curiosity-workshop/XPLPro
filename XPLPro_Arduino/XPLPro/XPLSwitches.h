//   XPLSwitches.h - XPLPro Add-on Library for simple switch connections 
//   Created by Curiosity Workshop, Michael Gerlicher,  2024
//   
//   To report problems, download updates and examples, suggest enhancements or get technical support, please visit:
//      discord:  https://discord.gg/gzXetjEST4
//      patreon:  www.patreon.com/curiosityworkshop

#ifndef XPLSwitches_h
#define XPLSwitches_h

// Parameters around the interface
#define XPLSWITCHES_SENDTOHANDLER   0                   // Default is to send switch events to the supplied handler.  This always occurs regardless.
#define XPLSWITCHES_DATAREFWRITE    1                   // Update dataref with switch status 
#define XPLSWITCHES_COMMANDTRIGGER  2                   // Trigger command with pressed
#define XPLSWITCHES_COMMANDSTARTEND 3                   // Start command when pressed, end command when released
#define XPLSWITCHES_DATAREFWRITE_INVERT 4               // same as datarefwrite but invert the signal

#define XPLSWITCHES_DEBOUNCETIME 50
#define XPLSWITCHES_PRESSED      0
#define XPLSWITCHES_RELEASED     1

#ifndef XPLSWITCHES_MAXSWITCHES 
    #define XPLSWITCHES_MAXSWITCHES     40                  //Default to 40.  This costs ~400 bytes.
#endif


/// @brief Core class for the XPLPro Switches Addon
class XPLSwitches
{
public:
    /// @brief Constructor
    /// @param switchHandler, Function called when pin activity is detected, or NULL if not needed
    XPLSwitches(void (*switchHandler)(int pin, int switchValue));

    /// <summary>
    /// @brief begin
    /// </summary>
    /// <param name="xplpro"></param>
    void begin(XPLPro *xplpro);

    int addPin(int inPin, byte inMode, int inHandle);

    int getHandle(int inPin);

   
    /// @brief Scan pins and call handler if any changes are detected.  Run regularly
    void check(void);  

    void clear(void);
    
private:
 
    XPLPro* _XP;
  
  int _switchCount;             // how many are registered


  void (*_switchHandler)(int inSwitchID, int inSwitchValue) = NULL;  // this function will be called when activity is detected on the mux, if not NULL
   
  
  struct XPLSwitch
  {
      int arduinoPin;                // connected pin
      byte prevStatus;              //  last known status
      byte mode;
      int  handle;
      long int prevTime;            //  time of last change

  };

  struct XPLSwitch _switches[XPLSWITCHES_MAXSWITCHES];

};


XPLSwitches::XPLSwitches(void (*switchHandler)(int inSwitchID, int inValue))
{

 
  _switchHandler = switchHandler;


};

void XPLSwitches::begin(XPLPro* xplpro)
{
    _XP = xplpro;
    clear();

}

void XPLSwitches::clear(void)           // call this prior to adding pins if not the first run
{
    _switchCount = 0;

}

int XPLSwitches::addPin(int inPin, byte inMode, int inHandle)
{
    if (_switchCount >= XPLSWITCHES_MAXSWITCHES) return -1;

    _switches[_switchCount].arduinoPin = inPin;
    _switches[_switchCount].mode = inMode;
    _switches[_switchCount].handle = inHandle;
    _switches[_switchCount].prevStatus = -1;        // This will force update to the plugin
    pinMode(inPin, INPUT_PULLUP);

    return _switchCount++;


}
int XPLSwitches::getHandle(int inPin)
{
    for (int i = 0; i < XPLSWITCHES_MAXSWITCHES; i++) if (_switches[i].arduinoPin == inPin) return _switches[i].handle;
    return -1;
   
}


void XPLSwitches::check(void)
{
 
  unsigned long timeNow = millis();

 
  for (int i = 0; i < _switchCount; i++)
  {
      int pinValue = digitalRead(_switches[i].arduinoPin);

      if (pinValue != _switches[i].prevStatus && timeNow - _switches[i].prevTime >= XPLSWITCHES_DEBOUNCETIME)
      {

         _switches[i].prevStatus = pinValue;
         _switches[i].prevTime   = timeNow;

         switch (_switches[i].mode)
         {
         
         case XPLSWITCHES_DATAREFWRITE:
             _XP->datarefWrite(_switches[i].handle, pinValue);
             break;

         case XPLSWITCHES_DATAREFWRITE_INVERT:
             _XP->datarefWrite(_switches[i].handle, !pinValue); 
             break;
         
         case XPLSWITCHES_COMMANDTRIGGER:
             if (pinValue == XPLSWITCHES_PRESSED) _XP->commandTrigger(_switches[i].handle);
             break;
         
         case XPLSWITCHES_COMMANDSTARTEND:
             if (pinValue == XPLSWITCHES_PRESSED)     _XP->commandStart(_switches[i].handle);
             if (pinValue == XPLSWITCHES_RELEASED)    _XP->commandEnd(_switches[i].handle);
             break;
         
         
         }

         if (_switchHandler != NULL) _switchHandler(_switches[i].arduinoPin, pinValue);

       }
   }

}

#endif