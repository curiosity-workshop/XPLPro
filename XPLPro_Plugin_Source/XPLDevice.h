#pragma once
#include "SerialClass.h"
#include "XPLProCommon.h"

class XPLDevice
{
public:

	XPLDevice(int inReference);
	~XPLDevice();
	int _writePacket(char cmd, char* packet);
	int _writePacketN(char cmd, char* packet, int packetSize);
//	char* getDeviceName(void);
//	int   getDeviceType(void);
//	char* getLastDebugMessageReceived(void);
	int isActive(void);						// returns true if device has communicated its name
	void setActive(int activeFlag);
	void processSerial(void);

	char   readBuffer[XPLMAX_PACKETSIZE];
	
	char   lastDebugMessageReceived[80];	// what was last sent by the device as a debug string
	float  lastSendTime;					// last time data update occurred

//	int    deviceType;						// XPLDirect = 1    XPLWizard = 2
	int    boardType;						// Type of arduino device, if XPLWizard
	int    RefsLoaded;						// true if device has sent all dataref bindings it wants
	char   deviceName[80];					// name of device as returned from device
	//char   boardName[80];					// name of board type, implemented for XPLWizard Devices
	
	float  minTimeBetweenFrames;			// only implemented for dataref updates.
	int    bufferPosition;

	serialClass* port;							// handle to open com port



private: 
	
	void _processPacket(void);
		
	int _parseString(char* outBuffer, char* inBuffer, int parameter, int maxSize);
	int _parseInt(int* outTarget, char* inBuffer, int parameter);
	int _parseInt(long int* outTarget, char* inBuffer, int parameter);
	int _parseFloat(float* outTarget, char* inBuffer, int parameter);

	int    _active;							// true if device responds
	int    _referenceID;					// possibly temporary to id ourselves exterally
	
	int _flightLoopPause;							// while initializing datarefs and commands this can be true to stop flight loop cycle.  Downside is, if it never becomes false...
	
	
	
		


};
