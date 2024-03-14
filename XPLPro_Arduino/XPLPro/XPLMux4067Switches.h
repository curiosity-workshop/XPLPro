//   XPLMux4067Switches.h - Library for 4067 multiplexer 
//   Created by Curiosity Workshop, Michael Gerlicher,  2024
//   
//   To report problems, download updates and examples, suggest enhancements or get technical support, please visit:
//      discord:  https://discord.gg/gzXetjEST4
//      patreon:  www.patreon.com/curiosityworkshop

#ifndef XPLMux4067Switches_h
#define XPLMux4067Switches_h

// Parameters around the interface
#define XPLMUX4067_DEBOUNCETIME 50
#define XPLMUX4067_PRESSED      0
#define XPLMUX4067_RELEASED     1

#define XPLMUX4067_SENDTOHANDLER   0                   // Default is to send switch events to the supplied handler.  This always occurs regardless.
#define XPLMUX4067_DATAREFWRITE    1                   // Update dataref with switch status 
#define XPLMUX4067_COMMANDTRIGGER  2                   // Trigger command with pressed
#define XPLMUX4067_COMMANDSTARTEND 3                   // Start command when pressed, end command when released
#define XPLMUX4067_DATAREFWRITE_INVERT 4               // same as datarefwrite but invert the signal

/// @brief Core class for the XPLPro Arduino library
class XPLMux4067Switches
{
public:
    /// @brief Constructor
    /// @param inPinSig Pin connection for reading
    /// @param inPinS0, Pin connection for s0
    /// @param inPinS1, Pin connection for s1
    /// @param inPinS2, Pin connection for s2
    /// @param inPinS3, Pin connection for s3
    /// @param muxHandler, function called when pin activity is detected, or NULL
    XPLMux4067Switches(int inPinSig, int inPinS0, int inPinS1, int inPinS2, int inPinS3, void (*muxHandler)(int muxChannel, int muxValue));

    void begin(XPLPro* xplpro);

    int addPin(int inPin, byte inMode, int inHandle);

    int getHandle(int inPin);

  
    /// @brief Scan mux pins and call handler if any changes are detected.  Run regularly
    void check(void);  

    void clear(void);
    
private:

  XPLPro* _XP;          
  int _maxSwitches;
  int _switchCount;

  void (*_muxHandler)(int muxChannel, int muxValue) = NULL;  // this function will be called when activity is detected on the mux
   
  int _pinS0;
  int _pinS1;
  int _pinS2;
  int _pinS3;
  int _pinSig;

  int _muxChannel[16][4]=                   // this table reduces processing time and space
  {
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  struct XPLSwitch
  {
      int muxPin;                     // connected pin
      byte prevStatus;              //  last known status
      byte mode;
      int  handle;
      long int prevTime;            //  time of last change

  };

  struct XPLSwitch _switches[16];
 // struct XPLSwitch _switches[];
};


XPLMux4067Switches::XPLMux4067Switches(int inPinSig, int inPinS0, int inPinS1, int inPinS2, int inPinS3, void (*muxHandler)(int inChannel, int inValue))
{

  _pinSig = inPinSig;         pinMode(_pinSig, INPUT_PULLUP); 
  _pinS0 = inPinS0;           pinMode(_pinS0, OUTPUT);            digitalWrite(_pinS0, LOW);
  _pinS1 = inPinS1;           pinMode(_pinS1, OUTPUT);            digitalWrite(_pinS1, LOW);
  _pinS2 = inPinS2;           pinMode(_pinS2, OUTPUT);            digitalWrite(_pinS2, LOW);
  _pinS3 = inPinS3;           pinMode(_pinS3, OUTPUT);            digitalWrite(_pinS3, LOW);

  _muxHandler = muxHandler;
 // _maxSwitches = inMaxSwitches;
 // if (_maxSwitches > 16) _maxSwitches = 16;                 // max for 4067 mux
  _maxSwitches = 16;                // my intention is to make this dynamic but it is problematic
 //switches = new struct XPLSwitch[ _maxSwitches];
 
};

void XPLMux4067Switches::begin(XPLPro* xplpro)
{
    _XP = xplpro;
    clear();

}

void XPLMux4067Switches::clear(void)           // call this prior to adding pins if not the first run
{
    _switchCount = 0;

}

int XPLMux4067Switches::addPin(int inPin, byte inMode, int inHandle)
{
    if (_switchCount >= _maxSwitches) return -1;

    _switches[_switchCount].muxPin = inPin;
    _switches[_switchCount].mode = inMode;
    _switches[_switchCount].handle = inHandle;
    _switches[_switchCount].prevStatus = -1;                // this will force it to update to the plugin.
  
    return _switchCount++;

}

int XPLMux4067Switches::getHandle(int inPin)
{
    for (int i = 0; i < _maxSwitches; i++) if (_switches[i].muxPin == inPin) return _switches[i].handle;
    return -1;

}


void XPLMux4067Switches::check(void)
{
 
  long int timeNow = millis();
  for (int i = 0; i < _switchCount; i++)
  { 
    digitalWrite(_pinS0, _muxChannel[_switches[i].muxPin][0]);
    digitalWrite(_pinS1, _muxChannel[_switches[i].muxPin][1]);
    digitalWrite(_pinS2, _muxChannel[_switches[i].muxPin][2]);
    digitalWrite(_pinS3, _muxChannel[_switches[i].muxPin][3]);
    
    int pinValue = digitalRead(_pinSig);

    if (pinValue != _switches[i].prevStatus && timeNow - _switches[i].prevTime >= XPLMUX4067_DEBOUNCETIME)
    {
      _switches[i].prevStatus = pinValue;
      _switches[i].prevTime   = timeNow;

      switch (_switches[i].mode)
      {

      case XPLMUX4067_DATAREFWRITE:
          _XP->datarefWrite(_switches[i].handle, pinValue);
          break;

      case XPLMUX4067_DATAREFWRITE_INVERT:
          _XP->datarefWrite(_switches[i].handle, !pinValue);
          break;

      case XPLMUX4067_COMMANDTRIGGER:
          if (pinValue == XPLMUX4067_PRESSED) _XP->commandTrigger(_switches[i].handle);
          break;

      case XPLMUX4067_COMMANDSTARTEND:
          if (pinValue == XPLMUX4067_PRESSED)     _XP->commandStart(_switches[i].handle);
          if (pinValue == XPLMUX4067_RELEASED)    _XP->commandEnd(_switches[i].handle);
          break;


      }


      if (_muxHandler != NULL) _muxHandler(_switches[i].muxPin, pinValue);
    }

  }
 
}

#endif