//   XPLPotentiometers.h - XPLPro Add-on Library for simple potentiometer connections 
//   Created by Curiosity Workshop, Michael Gerlicher,  2024
//   
//   To report problems, download updates and examples, suggest enhancements or get technical support, please visit:
//      discord:  https://discord.gg/gzXetjEST4
//      patreon:  www.patreon.com/curiosityworkshop

#ifndef XPLSwitches_h
#define XPLSwitches_h

// Parameters around the interface
#define XPLPOTS_SENDTOHANDLER   0                   // Default is to send switch events to the supplied handler.  This always occurs regardless.
#define XPLPOTS_DATAREFWRITE    1                   // Update dataref with switch status 

#define XPLPOTS_UPDATERATE  50                       // default minimum time between updates in milliseconds


#ifndef XPLPOTS_MAXPOTS 
    #define XPLPOTS_MAXPOTS     10                  //Default to 10.  
#endif


/// @brief Core class for the XPLPro Potentiometers Addon
class XPLPotentiometers
{
public:
    /// @brief Constructor
    /// @param potHandler, Function called when pot activity detected, or NULL if not needed
    XPLPotentiometers(void (*potHandler)(int pin, float potValue));

    /// <summary>
    /// @brief begin
    /// </summary>
    /// <param name="xplpro"></param>
    void begin(XPLPro *xplpro);

    int addPin(int inPin, int inMode, int inHandle, int inPrecision, int inLow, int inHigh, int outLow, int outHigh);
    void setUpdateRate(int inRate);
    int getHandle(int inPin);

   
    /// @brief Scan pins and call handler if any changes are detected.  Run regularly
    void check(void);  

    void clear(void);
    
private:
 
    XPLPro* _XP;
  
  int _potCount;             // how many are registered
  int _updateRate;              // in milliseconds


  void (*_potHandler)(int inSwitchID, float inPotValue) = NULL;  // this function will be called when activity is detected on the pot, if not NULL
   
  
  struct XPLPot
  {
      int arduinoPin;                // connected pin
      int prevValue;              //  last known value
      int handle;                  // handle to dataref
      int mode;                     //  what to do with new data
      long int prevTime;            //  time of last change
      int precision;              // divide by this to reduce data flow

  };

  struct XPLPot _pots[XPLPOTS_MAXPOTS];

};


XPLPotentiometers::XPLPotentiometers(void (*potHandler)(int inPotID, float inValue))
{

   _potHandler = potHandler;
   _updateRate = XPLPOTS_UPDATERATE;


};

void XPLPotentiometers::begin(XPLPro* xplpro)
{
    _XP = xplpro;
    clear();

}

void XPLPotentiometers::clear(void)           // call this prior to adding pins if not the first run
{
    _potCount = 0;

}

void XPLPotentiometers::setUpdateRate(int inRate)
{
    _updateRate = inRate;
}

int XPLPotentiometers::addPin(int inPin, int inMode, int inHandle, int inPrecision, int inLow, int inHigh, int outLow, int outHigh)
{
    if (_potCount >= XPLPOTS_MAXPOTS) return -1;

    _pots[_potCount].arduinoPin = inPin;
    _pots[_potCount].precision = inPrecision;
    _pots[_potCount].mode = inMode;
    _pots[_potCount].handle = inHandle;
    _pots[_potCount].prevValue = -1;        // This will force update to the plugin
    
    _XP->setScaling(inHandle, inLow, inHigh, outLow, outHigh);

    return _potCount++;


}
int XPLPotentiometers::getHandle(int inPin)
{
    for (int i = 0; i < XPLPOTS_MAXPOTS; i++) if (_pots[i].arduinoPin == inPin) return _pots[i].handle;
    return -1;
   
}


void XPLPotentiometers::check(void)
{
 
  unsigned long timeNow = millis();

 
  for (int i = 0; i < _potCount; i++)
  {
      int pinValue = analogRead(_pots[i].arduinoPin);

      if (_pots[i].precision)  pinValue = ((int)(pinValue / _pots[i].precision) * _pots[i].precision);

      if (pinValue != _pots[i].prevValue && timeNow - _pots[i].prevTime >= XPLPOTS_UPDATERATE)
      {

         _pots[i].prevValue = pinValue;
         _pots[i].prevTime   = timeNow;

         switch (_pots[i].mode)
         {
         
         case XPLPOTS_DATAREFWRITE:
             _XP->datarefWrite(_pots[i].handle, pinValue);
             break;

                 
         }

         if (_potHandler != NULL) _potHandler(_pots[i].arduinoPin, pinValue);

       }
   }

}

#endif