/*
  XPLPro.h - Library for serial interface to Xplane SDK.
  Created by Curiosity Workshop, Michael Gerlicher,  2020-2023

  See readme.txt file for information on updates.

  To report problems, download updates and examples, suggest enhancements or get technical support, please visit:
     
     discord:  https://discord.gg/gzXetjEST4
     patreon:  www.patreon.com/curiosityworkshop
 
*/
 
#ifndef XPLPro_h
#define XPLPro_h

#define XPL_RX_TIMEOUT          500               // after detecting a frame header, how long will we wait to receive the rest of the frame.  (default 500 ms)
#define XPL_RESPONSE_TIMEOUT    60000              // after sending a registration request, how long will we wait for the response.  (default 2000 ms)  

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)        // add to this for boards that need it
    #define XPL_USE_PROGMEM                                 // define this for boards with limited memory that can use PROGMEM to store strings.  
                                                            // You will need to wrap your dataref names with F() macro ie:
                                                            // Xinterface.registerDataref(F("laminar/B738/annunciator/drive2"), XPL_READ, 100, 0, &drive2);
                                                            // Disable for boards that have issues compiling: errors with strncmp_PF for instance.

#endif


#define XPLMAX_PACKETSIZE 200                           // Probably leave this alone.  If you need a few extra bytes of RAM it could be reduced, but it needs to
                                                        // be as long as the longest dataref name + 10.  If you are using datarefs 
                                                        // that transfer strings it needs to be big enough for those too.  (default 200) 


//////////////////////////////////////////////////////////////
// STOP! Dont change any other defines in this header!
//////////////////////////////////////////////////////////////

#define XPL_BAUDRATE 115200           // don't mess with this, it needs to match the plugin which won't change
#define XPL_PACKETHEADER  '['         // ...or this
#define XPL_PACKETTRAILER ']'         // ...or this                                                                           
#define XPL_VERSION 2303281           // The plugin will start to verify that a compatible version is being used
//#define XPL_ID 0                      // Used for relabled plugins to identify the company.  0 = normal distribution version


// Items in caps generally come from xplane.  Items in lower case are generally sent from the arduino.

#define XPLRESPONSE_NAME           'n'          // Arduino responds with device name as initialized in the "begin" function
#define XPLRESPONSE_DATAREF        'D'          // Plugin responds with handle to dataref or - value if not found.  dataref handle, dataref name 
#define XPLRESPONSE_COMMAND        'C'          // Plugin responds with handle to command or - value if not found.  command handle, command name
#define XPLRESPONSE_VERSION        'v'          // Arduino responds with build version number if requested
#define XPLCMD_PRINTDEBUG          'g'          // plugin logs string sent from arduino
#define XPLCMD_RESET               '2'
#define XPLCMD_SPEAK               's'          // plugin speaks string through xplane speech 
#define XPLCMD_SENDNAME            'N'          // plugin request name from arduino
#define XPLREQUEST_REGISTERDATAREF 'b'   // 
#define XPLREQUEST_REGISTERCOMMAND 'm'  // just the name of the command to register
#define XPLREQUEST_NOREQUESTS      'c'   // nothing to request
#define XPLREQUEST_REFRESH         'd'	//  the plugin will call this once xplane is loaded in order to get fresh updates from arduino handles that write

#define XPLCMD_DATAREFUPDATE       'u'
#define XPLCMD_SENDREQUEST         'f'          // plugin sends this when it is ready to register bindings
//#define XPLCMD_DEVICEREADY         'g'
//#define XPLCMD_DEVICENOTREADY      'h'
#define XPLCMD_COMMANDSTART         'i'
#define XPLCMD_COMMANDEND           'j'
#define XPLCMD_COMMANDTRIGGER       'k'   //  %3.3i%3.3i   command handle, number of triggers

#define XPL_EXITING                 'X'     // MG 03/14/2023: xplane sends this to the arduino device during normal shutdown of xplane.  It may not happen if xplane crashes.


#define XPL_DATATYPE_INT    1
#define XPL_DATATYPE_FLOAT  2
#define XPL_DATATYPE_STRING 3



class XPLPro
{
  public:

    XPLPro(Stream*); 
    void begin(char *devicename, void *initFunction, void *shutdownFunction, void *inboundHandler);     // parameter is name of your device for reference 
  
    int connectionStatus(void);
    int commandTrigger(int commandHandle);                                                      // triggers specified command 1 time;
    int commandTrigger(int commandHandle, int triggerCount);                                    // triggers specified command triggerCount times.  
    int commandStart(int commandHandle);                                                        // Avoid this unless you know what you are doing.  Command "begins" must be balanced with command "ends"
    int commandEnd(int commandHandle);
  
    void datarefWrite(int handle, int value);
    void datarefWrite(int handle, long int value);                                               // write directly to variable
    void datarefWrite(int handle, float value);
   // void datarefRead(int handle, long int *value);
   // void datarefRead(int handle, float *value);


    int registerDataRef(const char *datarefName);                    
  //  int registerDataRef(const __FlashStringHelper*);
    int registerCommand(const char* commandName);                                           // register a command    
  //  int registerCommand(const __FlashStringHelper* commandName);                                // use this overload to trigger the command manually (commandTrigger)

     
                                                                                                //int sendReadyCommand(void);
    //int getCurrentData(int, int*);
    int sendDebugMessage(const char*);
    int sendSpeakMessage(const char* msg);
 //   int allDataRefsRegistered(void);
   			                                       
    void sendResetRequest(void);
    int xloop(void);                                                                            // where the magic happens!
  
    
  private:
      void _processSerial();
      void _processPacket();

      void _sendPacketInt(int command, int handle, int value);
      void _sendPacketInt(int command, int handle, long int value);         // for ints
      void _sendPacketFloat(int command, int handle, float value);       // for floats
      void _sendPacketVoid(int command, int handle);                    // just a command with a handle
      void _sendPacketString(int command, const char* str);                     // send a string
   
      void _transmitPacket();

      void _sendname();
     
      
      int _parseInt(int* outTarget, char* inBuffer, int parameter);
      int _parseString(char* outBuffer, char* inBuffer, int parameter, int maxSize);
          
      Stream* streamPtr;

    
      char _receiveBuffer[XPLMAX_PACKETSIZE]; int _receiveBufferBytesReceived;
      char _sendBuffer[XPLMAX_PACKETSIZE];
  
     
      byte _registerFlag;
      byte _connectionStatus;
      
      void (*_xplInitFunction)(void);                    // this function will be called when the plugin is ready to receive binding requests
      void (*_xplStopFunction)(void);                       // this function will be called with the plugin receives message or detects xplane flight model inactive
      void (*_xplInboundHandler)(int);                  // this function will be called when the plugin sends dataref values

      const char * _deviceName;

      int _handleAssignment;
};

#endif
