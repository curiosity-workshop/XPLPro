#pragma once
//#include "Serial.h"
#include "XPLProCommon.h"

void BindingsSetup(void);
void BindingsLoad(void);

int findDevices(void);
void activateDevices(void);
void sendExitMessage(void);
void sendRefreshRequest(void);
void disengageDevices(void);
void engageDevices(void);
void _processPacket(int);
void _processSerial(void);
void _updateDataRefs(int forceUpdate);
void _updateCommands(void);
int _writePacket(int port, char, char*);
int _writePacketN(int port, char, char*, int);
void reloadDevices(void);

float mapFloat(long x, long inMin, long inMax, long outMin, long outMax);
long mapInt(long x, long inMin, long inMax, long outMin, long outMax);

struct DataRefBinding
{
	int            deviceIndex;				// which XPLDirectDevice is this attached to
	int            bindingActive;			// Is this binding being used
	int            Handle;			        // Handle is arbitrary and incremental and assigned by this plugin to send to arduino board
//	int            RWMode;					// XPL_READ 1   XPL_WRITE   2   XPL_READWRITE	3
	int				readFlag[XPLMAX_ELEMENTS];				// true if device requests updates for this dataref value/element
	float		   precision;					// reduce resolution by dividing then remultiplying with this number, or 0 for no processing
	int            updateRate;				// minimum time in ms between updates sent 
	time_t		   lastUpdate;				// time of last update
	XPLMDataRef    xplaneDataRefHandle;		// Dataref handle of xplane element associated with binding
	XPLMDataTypeID xplaneDataRefTypeID;		// dataRef type
	char           xplaneDataRefName[80];		// character name of xplane dataref
	int				scaleFlag;
	int				scaleFromLow;
	int				scaleFromHigh;
	int				scaleToLow;
	int				scaleToHigh;
	int			   currentElementSent[XPLMAX_ELEMENTS];
	long           currentSentl[XPLMAX_ELEMENTS];		// Current  long value sent to device
	long           currentReceivedl[XPLMAX_ELEMENTS];   // Current long value sent to Xplane
	float          currentSentf[XPLMAX_ELEMENTS];      // Current float value sent to device
		
	float          currentReceivedf[XPLMAX_ELEMENTS];  // Current float value sent to Xplane
	double			currentSentD[XPLMAX_ELEMENTS];		// current double value sent to device
	double			currentReceivedD[XPLMAX_ELEMENTS];	// current double value sent to Xplane
	
	char*			currentSents[XPLMAX_ELEMENTS];	// dynamically allocated string buffer for string types.


};

struct CommandBinding
{
	int            deviceIndex;				// which XPL Device is this attached to
	int            bindingActive;			// Is this binding being used
	int            Handle;			        // Handle is incremental and assigned by this plugin to send to arduino board

	XPLMCommandRef xplaneCommandHandle;		// Dataref handle of xplane element associated with binding

	char           xplaneCommandName[80];		// character name of xplane dataref
	int			   accumulator;
	//int            xplaneCurrentReceived;   // Current value sent to Xplane

};
