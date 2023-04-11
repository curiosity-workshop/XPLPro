//   XPLPro.h - Library for serial interface to Xplane SDK.
//   Created by Curiosity Workshop, Michael Gerlicher,  2020-2023
//   See readme.txt file for information on updates.
//   To report problems, download updates and examples, suggest enhancements or get technical support, please visit:
//      discord:  https://discord.gg/gzXetjEST4
//      patreon:  www.patreon.com/curiosityworkshop

#ifndef XPLPro_h
#define XPLPro_h

#include <Arduino.h>


//////////////////////////////////////////////////////////////
// Parameters that can be overwritten by command line defines
//////////////////////////////////////////////////////////////

// Decimals of precision for floating point datarefs. More increases dataflow (default 4)
#ifndef XPL_FLOATPRECISION
#define XPL_FLOATPRECISION 4
#endif

// Timeout after sending a registration request, how long will we wait for the response.
// This is giant because sometimes xplane says the plane is loaded then does other stuff for a while. (default 90000 ms)
#ifndef XPL_RESPONSE_TIMEOUT
#define XPL_RESPONSE_TIMEOUT 90000
#endif

// For boards with limited memory that can use PROGMEM to store strings.
// You will need to wrap your dataref names with F() macro ie:
// Xinterface.registerDataref(F("laminar/B738/annunciator/drive2"), XPL_READ, 100, 0, &drive2);
// Disable for boards that have issues compiling: errors with strncmp_PF for instance.
#ifndef XPL_USE_PROGMEM
#ifdef __AVR_ARCH__
// flash strings are default on on AVR architecture
#define XPL_USE_PROGMEM 1
#else
// and off otherwise
#define XPL_USE_PROGMEM 0
#include <avr/dtostrf.h>                // this is needed for non-AVR boards to include the dtostrf function

#endif
#endif

// Package buffer size for send and receive buffer each.
// If you need a few extra bytes of RAM it could be reduced, but it needs to
// be as long as the longest dataref name + 10.  If you are using datarefs
// that transfer strings it needs to be big enough for those too.  (default 200)
#ifndef XPLMAX_PACKETSIZE_TRANSMIT
#define XPLMAX_PACKETSIZE_TRANSMIT 200
#endif

#ifndef XPLMAX_PACKETSIZE_RECEIVE
#define XPLMAX_PACKETSIZE_RECEIVE 200
#endif

//////////////////////////////////////////////////////////////
// All other defines in this header must not be modified
//////////////////////////////////////////////////////////////

// define whether flash strings will be used
#if XPL_USE_PROGMEM
// use Flash for strings, requires F() macro for strings in all registration calls
typedef const __FlashStringHelper XPString_t;

#else
typedef const char XPString_t;
#endif

// Parameters around the interface
#define XPL_BAUDRATE 115200   // Baudrate needed to match plugin
#define XPL_RX_TIMEOUT 500    // Timeout for reception of one frame
#define XPL_PACKETHEADER '['  // Frame start character
#define XPL_PACKETTRAILER ']' // Frame end character
#define XPL_HANDLE_INVALID -1 // invalid handle

// Items in caps generally come from XPlane. Items in lower case are generally sent from the arduino.
#define XPLCMD_SENDNAME 'N'                // plugin request name from arduino
#define XPLRESPONSE_NAME 'n'               // Arduino responds with device name as initialized in the "begin" function
#define XPLCMD_SENDREQUEST 'Q'             // plugin sends this when it is ready to register bindings
#define XPLREQUEST_REGISTERDATAREF 'b'     // Register a dataref
#define XPLREQUEST_REGISTERCOMMAND 'm'     // Register a command
#define XPLRESPONSE_DATAREF 'D'            // Plugin responds with handle to dataref or - value if not found.  dataref handle, dataref name
#define XPLRESPONSE_COMMAND 'C'            // Plugin responds with handle to command or - value if not found.  command handle, command name
#define XPLCMD_PRINTDEBUG 'g'              // Plugin logs string sent from arduino
#define XPLCMD_SPEAK 's'                   // plugin speaks string through xplane speech
//#define XPLREQUEST_REFRESH 'd'             // the plugin will call this once xplane is loaded in order to get fresh updates from arduino handles that write (reserve until we are sure it is unneeded)
#define XPLREQUEST_UPDATES 'r'             // arduino is asking the plugin to update the specified dataref with rate and divider parameters
#define XPLREQUEST_UPDATESARRAY 't'        // arduino is asking the plugin to update the specified array dataref with rate and divider parameters
#define XPLREQUEST_SCALING 'u'             // arduino requests the plugin apply scaling to the dataref values
#define XPLREQUEST_DATAREFVALUE 'e'        // one off request for a dataref value.  Avoid doing this every loop, better to use REQUEST_UPDATES.  Either way, value will be sent via the inbound callback
#define XPLCMD_RESET 'z'                   // Request a reset and reregistration from the plugin
#define XPLCMD_DATAREFUPDATEINT '1'        // Int DataRef update
#define XPLCMD_DATAREFUPDATEFLOAT '2'      // Float DataRef update
#define XPLCMD_DATAREFUPDATEINTARRAY '3'   // Int array DataRef update
#define XPLCMD_DATAREFUPDATEFLOATARRAY '4' // Float array DataRef Update
#define XPLCMD_DATAREFUPDATESTRING '9'     // String DataRef update
#define XPLCMD_COMMANDTRIGGER 'k'          // Trigger command n times
#define XPLCMD_COMMANDSTART 'i'            // Begin command (Button pressed)
#define XPLCMD_COMMANDEND 'j'              // End command (Button released)
#define XPL_EXITING 'X'                    // XPlane sends this to the arduino device during normal shutdown of XPlane. It may not happen if xplane crashes.

/// @brief Core class for the XPLPro Arduino library
class XPLPro
{
public:
    /// @brief Constructor
    /// @param device Device to use (should be &Serial)
    XPLPro(Stream *device);

    /// @brief Register device and set callback functions
    /// @param devicename Device name
    /// @param initFunction Callback for DataRef and Command registration
    /// @param stopFunction Callback for XPlane shutdown or plane change
    /// @param inboundHandler Callback for incoming DataRefs
    void begin(const char *devicename, void (*initFunction)(void), void (*stopFunction)(void), void (*inboundHandler)(int));

    /// @brief Return connection status
    /// @return True if connection to XPlane established
    int connectionStatus();

    /// @brief Trigger a command once
    /// @param commandHandle of the command to trigger
    /// @return 0: OK, -1: command was not registered
    int commandTrigger(int commandHandle) { return commandTrigger(commandHandle, 1); };

    /// @brief Trigger a command multiple times
    /// @param commandHandle Handle of the command to trigger
    /// @param triggerCount Number of times to trigger the command
    /// @return 0: OK, -1: command was not registered
    int commandTrigger(int commandHandle, int triggerCount);

    /// @brief Start a command. All commandStart must be balanced with a commandEnd
    /// @param commandHandle Handle of the command to start
    /// @return 0: OK, -1: command was not registered
    int commandStart(int commandHandle);

    /// @brief End a command. All commandStart must be balanced with a commandEnd
    /// @param commandHandle Handle of the command to start
    /// @return 0: OK, -1: command was not registered
    int commandEnd(int commandHandle);

    /// @brief Write an integer DataRef.
    /// @param handle Handle of the DataRef to write
    /// @param value Value to write to the DataRef
    void datarefWrite(int handle, long value);

    /// @brief Write an integer DataRef. Maps to long DataRefs.
    /// @param handle Handle of the DataRef to write
    /// @param value Value to write to the DataRef
    void datarefWrite(int handle, int value);

    /// @brief Write a Integer DataRef to an array element.
    /// @param handle Handle of the DataRef to write
    /// @param value Value to write to the DataRef
    /// @param arrayElement Array element to write to
    void datarefWrite(int handle, long value, int arrayElement);

    /// @brief Write a Integer DataRef to an array element. Maps to long DataRefs.
    /// @param handle Handle of the DataRef to write
    /// @param value Value to write to the DataRef
    /// @param arrayElement Array element to write to
    void datarefWrite(int handle, int value, int arrayElement);

    /// @brief Write a float DataRef.
    /// @param handle Handle of the DataRef to write
    /// @param value Value to write to the DataRef
    void datarefWrite(int handle, float value);

    /// @brief Write a float DataRef to an array element.
    /// @param handle Handle of the DataRef to write
    /// @param value Value to write to the DataRef
    /// @param arrayElement Array element to write to
    void datarefWrite(int handle, float value, int arrayElement);

    /// @brief Request DataRef updates from the plugin
    /// @param handle Handle of the DataRef to subscribe to
    /// @param rate Maximum rate for updates to reduce traffic
    /// @param precision Floating point precision
    void requestUpdates(int handle, int rate, float precision);

    /// @brief Request DataRef updates from the plugin for an array DataRef
    /// @param handle Handle of the DataRef to subscribe to
    /// @param rate Maximum rate for updates to reduce traffic
    /// @param precision Floating point precision
    /// @param arrayElement Array element to subscribe to
    void requestUpdates(int handle, int rate, float precision, int arrayElement);

    /// @brief set scaling factor for a DataRef (offload mapping to the plugin)
    void setScaling(int handle, int inLow, int inHigh, int outLow, int outHigh);

    /// @brief Register a DataRef and obtain a handle
    /// @param datarefName Name of the DataRef (or abbreviation)
    /// @return Assigned handle for the DataRef, -1 if DataRef was not found
    int registerDataRef(XPString_t *datarefName);

    /// @brief Register a Command and obtain a handle
    /// @param commandName Name of the Command (or abbreviation)
    /// @return Assigned handle for the Command, -1 if Command was not found
    int registerCommand(XPString_t *commandName);

    /// @brief Read the received float DataRef
    /// @return Received value
    float datarefReadFloat() { return _readValueFloat; }

    /// @brief Read the received integer DataRef
    /// @return Received value
    long datarefReadInt() { return _readValueLong; }

    /// @brief Read the received array element
    /// @return Received array element
    int datarefReadElement() { return _readValueElement; }

    /// @brief Send a debug message to the plugin
    /// @param msg Message to show as debug string
    /// @return
    int sendDebugMessage(const char *msg);

    /// @brief Send a speech message to the plugin
    /// @param msg Message to speak
    /// @return
    int sendSpeakMessage(const char *msg);

    /// @brief Request a reset from the plugin
    void sendResetRequest(void);

    /// @brief Cyclic loop handler, must be called in idle task
    /// @return Connection status
    int xloop();

private:
    void _processSerial();
    void _processPacket();
    void _transmitPacket();
    void _sendname();
    void _sendPacketVoid(int command, int handle);        // just a command with a handle
    void _sendPacketString(int command, const char *str); // send a string
    int _parseInt(int *outTarget, char *inBuffer, int parameter);
    int _parseInt(long *outTarget, char *inBuffer, int parameter);
    int _parseFloat(float *outTarget, char *inBuffer, int parameter);
    int _parseString(char *outBuffer, char *inBuffer, int parameter, int maxSize);

    Stream *_streamPtr;
    const char *_deviceName;
    byte _registerFlag;
    byte _connectionStatus;

    char _sendBuffer[XPLMAX_PACKETSIZE_TRANSMIT];
    char _receiveBuffer[XPLMAX_PACKETSIZE_RECEIVE];
    int _receiveBufferBytesReceived;

    void (*_xplInitFunction)(void);  // this function will be called when the plugin is ready to receive binding requests
    void (*_xplStopFunction)(void);  // this function will be called with the plugin receives message or detects xplane flight model inactive
    void (*_xplInboundHandler)(int); // this function will be called when the plugin sends dataref values

    int _handleAssignment;
    long _readValueLong;
    float _readValueFloat;
    int _readValueElement;
};

#endif