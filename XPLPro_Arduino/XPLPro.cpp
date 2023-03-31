/*
  XPLPro.cpp 
  Created by Curiosity Workshop, Michael Gerlicher, 2023.
 
*/


#include <arduino.h>
#include "XPLPro.h"


XPLPro::XPLPro(Stream* device)
{
	streamPtr = device;
	streamPtr->setTimeout(XPL_RX_TIMEOUT);
}

void XPLPro::begin(char * devicename, void *initFunction, void *stopFunction, void *inboundHandler)
{
		
	_deviceName = devicename;
	_connectionStatus = 0;
	_receiveBuffer[0] = 0;
	_registerFlag = 0;
		
	_xplInitFunction = initFunction;
	_xplStopFunction = stopFunction;
	_xplInboundHandler = inboundHandler;
}



int XPLPro::xloop(void)
{
	_processSerial();
	if (_registerFlag) _xplInitFunction();				// handle registrations
	_registerFlag = 0;

	return _connectionStatus;
}

int XPLPro::commandTrigger(int commandHandle)
{
	return commandTrigger(commandHandle, 1);

}

int XPLPro::commandTrigger(int commandHandle, int triggerCount)
{
	_sendPacketInt(XPLCMD_COMMANDTRIGGER, commandHandle, (long int)triggerCount);

	return 0;
}

int XPLPro::commandStart(int commandHandle)
{
	
	_sendPacketVoid(XPLCMD_COMMANDSTART, commandHandle);
	return 0;

}

int XPLPro::commandEnd(int commandHandle)
{
	
	_sendPacketVoid(XPLCMD_COMMANDEND, commandHandle);
	return 0;

}

int XPLPro::connectionStatus()
{
	return _connectionStatus;

}


int XPLPro::sendDebugMessage(const char* msg)
{
	
	_sendPacketString(XPLCMD_PRINTDEBUG, msg);

	return 1;
}

int XPLPro::sendSpeakMessage(const char* msg)
{

	_sendPacketString(XPLCMD_SPEAK, msg);

	return 1;
}

/*

int XPLPro::hasUpdated(int handle)
{
	
	return false;
}

int XPLPro::datarefsUpdated()
{
	
	return false;
}

void XPLPro::datarefRead(int handle, long int * value)
{
	//if (_dataRefs[handle]->dataRefHandle >= 0 && (_dataRefs[handle]->dataRefRWType == XPL_READ || _dataRefs[handle]->dataRefRWType == XPL_READWRITE))
	//    *value =  *(long int *)_dataRefs[handle]->latestValue;
	
}

void XPLPro::datarefRead(int handle, float * value)
{
	//if (_dataRefs[handle]->dataRefHandle >= 0 && (_dataRefs[handle]->dataRefRWType == XPL_READ || _dataRefs[handle]->dataRefRWType == XPL_READWRITE))
//		*value = *(float*)_dataRefs[handle]->latestValue;

	
}
*/

void XPLPro::datarefWrite(int handle, int value)
{
	_sendPacketInt(XPLCMD_DATAREFUPDATE, handle, value);
}

void XPLPro::datarefWrite(int handle, long int value)
{
	_sendPacketInt(XPLCMD_DATAREFUPDATE, handle, value);
}

void XPLPro::datarefWrite(int handle, float value)
{
	_sendPacketFloat(XPLCMD_DATAREFUPDATE, handle, value);

}



void XPLPro::_sendname()
{
	if (_deviceName != NULL)
	{
		_sendPacketString(XPLRESPONSE_NAME, _deviceName);
		
	}
		
	//_xplInitFunction();						// Call the init function since we know xplane and the plugin are up and running

}



void XPLPro::sendResetRequest()
{

	if (_deviceName != NULL)
	{
		_sendPacketVoid(XPLCMD_RESET, 0);
	}


}

void XPLPro::_processSerial()
{


	while (streamPtr->available() && _receiveBuffer[0] != XPL_PACKETHEADER)  _receiveBuffer[0] = (char)streamPtr->read();

	if (_receiveBuffer[0] != XPL_PACKETHEADER) return;


	_receiveBufferBytesReceived = streamPtr->readBytesUntil(XPL_PACKETTRAILER, (char*)&_receiveBuffer[1], XPLMAX_PACKETSIZE - 1);

	if (_receiveBufferBytesReceived == 0)
	{
		_receiveBuffer[0] = 0;
		return;
	}


	_receiveBuffer[++_receiveBufferBytesReceived] = XPL_PACKETTRAILER;
	_receiveBuffer[++_receiveBufferBytesReceived] = 0;							// old habits die hard.  

	// at this point we should have a valid frame
	_processPacket();
	
}

void XPLPro::_processPacket()
{
//	char tStr[80];				// remove for distrubution
	
//	lcd.setCursor(0, 1);
//	lcd.print(_receiveBuffer);
	
	if (_receiveBuffer[0] != XPL_PACKETHEADER) return;

	switch (_receiveBuffer[1])
	{
		

		case XPL_EXITING :					
		
			_connectionStatus = false;
			_xplStopFunction();
			break;
		

	    case XPLCMD_SENDNAME :
		
			_sendname();
			_connectionStatus = true;			//not considered active till you know my name
			_registerFlag = 0;	
			break;
		

		case XPLCMD_SENDREQUEST :
			_registerFlag = 1;				// plugin is ready for registrations.  Use a flag so recursion doesn't occur
			
			break;

		case XPLRESPONSE_DATAREF :
		   
			_parseInt(&_handleAssignment, _receiveBuffer, 2);
		//	sprintf(tStr, "dr response rx: %i", _handleAssignment);
	//		sendDebugMessage(tStr);
			break;
		

		case XPLRESPONSE_COMMAND:
		
			_parseInt(&_handleAssignment, _receiveBuffer, 2);
			break;
		

		
		case XPLCMD_DATAREFUPDATE :
		
			//int refhandle = _getHandleFromFrame();
						
			break;
		
		
		case XPLREQUEST_REFRESH:
			break;
		

		
		default:
		
			
			break;
		

	}

	_receiveBuffer[0] = 0;
}

void XPLPro::_sendPacketInt(int command, int handle, int value)			// for ints
{
	if (handle < 0) return;


	sprintf(_sendBuffer, "%c%c,%i,%i%c", XPL_PACKETHEADER, command, handle, value, XPL_PACKETTRAILER);

	_transmitPacket();

}

void XPLPro::_sendPacketInt(int command, int handle, long int value)			// for ints
{
	if (handle < 0) return;

	
	sprintf(_sendBuffer, "%c%c,%i,%ld%c", XPL_PACKETHEADER, command, handle, value, XPL_PACKETTRAILER);

	_transmitPacket();

}

void XPLPro::_sendPacketFloat(int command, int handle, float value)		// for floats
{
	if (handle < 0) return;
// some boards cant do sprintf with floats so this is a workaround.  What a pain.  Implementing 2 decimals of precision for now.
	//sprintf(_sendBuffer, "%c%c%3.3i%07i.%02i%c", 
	  sprintf(_sendBuffer, "%c%c,%i0%i.%i%c",					// change to reduce data flow 01/26/2021 MG
																	// fix problem with leading zeros 03/01/2022
		XPL_PACKETHEADER, 
		command, 
		handle, 
		abs((int)value), 
		abs((int)(value * 1000) % 1000), 
		XPL_PACKETTRAILER);

	if (value < 0) _sendBuffer[5] = '-';
/*	
Alternate method perhaps

char tBuf[20];
	dtostrf(value, 0, 2, tBuf);

	sprintf(_sendBuffer, "%c%c%3.3i%s%c",
		XPLDIRECT_PACKETHEADER,
		command,
		handle,
		buff,
		XPLDIRECT_PACKETTRAILER);

		
*/
	_transmitPacket();
	
}

void XPLPro::_sendPacketVoid(int command, int handle)			// just a command with a handle
{
	if (handle < 0) return;

	sprintf(_sendBuffer, "%c%c,%i%c", XPL_PACKETHEADER, command, handle, XPL_PACKETTRAILER);
  
  _transmitPacket();
}

void XPLPro::_sendPacketString(int command, const char *str)			// for a string
{
	
	
	sprintf(_sendBuffer, "%c%c,\"%s\"%c", XPL_PACKETHEADER, command, str, XPL_PACKETTRAILER);

	_transmitPacket();
}


void XPLPro::_transmitPacket(void)
{
	
	streamPtr->write(_sendBuffer);
	if (strlen(_sendBuffer) == 64) streamPtr->print(" ");			// apparantly a bug in arduino with some boards when we transmit exactly 64 bytes.  That took a while to track down...

}


int XPLPro::_parseString(char* outBuffer, char* inBuffer, int parameter, int maxSize)
{
	int cBeg;
	int pos = 0;
	int len;

	for (int i = 1; i < parameter; i++)
	{

		while (inBuffer[pos] != ',' && inBuffer[pos] != NULL) pos++;
		pos++;

	}

	while (inBuffer[pos] != '\"' && inBuffer[pos] != NULL) pos++;
	cBeg = ++pos;

	while (inBuffer[pos] != '\"' && inBuffer[pos] != NULL) pos++;
	len = pos - cBeg;
	if (len > maxSize) len = maxSize;


	strncpy(outBuffer, (char*)&inBuffer[cBeg], len);
	outBuffer[len] = 0;
	//fprintf(errlog, "_parseString, pos: %i, cBeg: %i, deviceName: %s\n", pos, cBeg, target);

	return 0;
}

int XPLPro::_parseInt(int* outTarget, char* inBuffer, int parameter)
{
	int cBeg;
	int pos = 0;

	for (int i = 1; i < parameter; i++)
	{

		while (inBuffer[pos] != ',' && inBuffer[pos] != NULL) pos++;
		pos++;

	}
	cBeg = pos;

	while (inBuffer[pos] != ',' && inBuffer[pos] != NULL && inBuffer[pos] != XPL_PACKETTRAILER) pos++;

	char holdChar = inBuffer[pos];
	inBuffer[pos] = 0;
	*outTarget = atoi((char*)&inBuffer[cBeg]);

	//	fprintf(errlog, "outTarget: %i cBeg: %i pos: %i string: %s\n", *outTarget, cBeg, pos, (char*)&inBuffer[cBeg]);

	inBuffer[pos] = holdChar;



	return 0;

}


/*
int XPLPro::_getHandleFromFrame()			// Assuming receive buffer is holding a good frame
{
	char holdChar;
	int handleRet;

	holdChar = _receiveBuffer[5];
	_receiveBuffer[5] = 0;
	handleRet = atoi((char*)&_receiveBuffer[2]);
	
	_receiveBuffer[5] = holdChar;
	return handleRet;

}

int XPLPro::_getPayloadFromFrame(long int *value)			// Assuming receive buffer is holding a good frame
{
	char holdChar;
	
	holdChar = _receiveBuffer[15];
	_receiveBuffer[15] = 0;
	*value = atol((char*)&_receiveBuffer[5]);
	
	_receiveBuffer[15] = holdChar;

	return 0;

}

int XPLPro::_getPayloadFromFrame(float *value)			// Assuming receive buffer is holding a good frame
{
	char holdChar;
	
	holdChar = _receiveBuffer[15];
	_receiveBuffer[15] = 0;
	*value = atof((char*)&_receiveBuffer[5]);

	_receiveBuffer[15] = holdChar;
	return 0;

}

int XPLPro::_getPayloadFromFrame(char* value)			// Assuming receive buffer is holding a good frame
{
	
	memcpy(value, (char*)&_receiveBuffer[5], _receiveBufferBytesReceived -6);
	value[_receiveBufferBytesReceived -6] = 0;							// erase the packet trailer
	for (int i = 0; i < _receiveBufferBytesReceived - 6; i++)
		if (value[i] == 7) value[i] = XPL_PACKETTRAILER;			//  How I deal with the possibility of the packet trailer being within a string
	//streamPtr->println(value);
	return 0;

}

*/

/*
#ifdef XPL_USE_PROGMEM
int XPLPro::registerDataRef(const __FlashStringHelper* datarefName)
{
	return -1;
}
#endif
*/

int XPLPro::registerDataRef(const char* datarefName)
{
	long int startTime;

	
	if (!_registerFlag) return -1;

	sprintf(_sendBuffer, "%c%c,\"%s\"%c", XPL_PACKETHEADER, XPLREQUEST_REGISTERDATAREF, datarefName, XPL_PACKETTRAILER);
	_transmitPacket();

	_handleAssignment = -1;
	startTime = millis();					// for timeout function
	
	while (millis() - startTime < XPL_RESPONSE_TIMEOUT && _handleAssignment<0 )
		_processSerial();
		
	//if (millis() - startTime > XPL_RESPONSE_TIMEOUT) sendDebugMessage("dr timed out...");

	return _handleAssignment;
	

	
}

/*

#ifdef XPL_USE_PROGMEM
int XPLPro::registerCommand(const __FlashStringHelper* commandName)		// user will trigger commands with commandTrigger
{

	return -1;
	
}
#endif
*/

int XPLPro::registerCommand(const char* commandName)		// user will trigger commands with commandTrigger
{

	long int startTime;

	startTime = millis();					// for timeout function

	sprintf(_sendBuffer, "%c%c,\"%s\"%c", XPL_PACKETHEADER, XPLREQUEST_REGISTERCOMMAND, commandName, XPL_PACKETTRAILER);
	_transmitPacket();

	_handleAssignment = -1;

	while (millis() - startTime < XPL_RESPONSE_TIMEOUT && _handleAssignment<0)
		_processSerial();

	return _handleAssignment;
	
	
	
}

