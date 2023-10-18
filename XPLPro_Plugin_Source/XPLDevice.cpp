
#define XPLM200

#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"

#include "XPWidgets.h"
#include "XPStandardWidgets.h"

#include "XPLMUtilities.h"
#include "XPLMProcessing.h"

#include "XPLMCamera.h"
#include "XPUIGraphics.h"
#include "abbreviations.h"
#include "XPWidgetUtils.h"

#include "DataTransfer.h"
#include "XPLDevice.h"

extern long int packetsSent;
extern long int packetsReceived;
extern FILE* serialLogFile;			// for serial data log
extern FILE* errlog;				// Used for logging problems
extern abbreviations gAbbreviations;
extern float elapsedTime;

extern int refHandleCounter;
extern int cmdHandleCounter;

extern CommandBinding myCommands[XPL_MAXCOMMANDS_PC];
extern DataRefBinding myBindings[XPL_MAXDATAREFS_PC];

extern int lastRefReceived;
extern int lastRefSent;
extern int lastCmdAction;
extern int lastRefElementSent;
extern int lastRefElementReceived;

XPLDevice::XPLDevice(int inReference)
{
	bufferPosition = 0;
	readBuffer[0] = '\0';
	lastDebugMessageReceived[0] = '\0';
	RefsLoaded = 0;
	lastSendTime = 0;
	_referenceID = inReference;

	_active = 0;
	_flightLoopPause = 0;

	minTimeBetweenFrames = XPL_MILLIS_BETWEEN_FRAMES_DEFAULT;

}

XPLDevice::~XPLDevice()
{
	//if (Port)  delete Port;
		
}

int XPLDevice::isActive(void)
{

	return _active;

}

void XPLDevice::setActive(int flag)
{
	_active = flag;

}


void XPLDevice::processSerial(void)
{
	do
	{
		while (port->readData(&readBuffer[bufferPosition], 1))
		{
			readBuffer[bufferPosition + 1] = '\0';
			//fprintf(errlog, "Buffer currently: %s\r\n", readBuffer);


			if (readBuffer[0] != XPL_PACKETHEADER)
			{
				readBuffer[0] = '\0';
				bufferPosition = -1;
			}


			if (readBuffer[0] == XPL_PACKETHEADER
				&& readBuffer[bufferPosition] == XPL_PACKETTRAILER)

			{

				_processPacket();
				readBuffer[0] = '\0';
				bufferPosition = -1;
			}


			if (strlen(readBuffer) >= XPLMAX_PACKETSIZE)
			{
				readBuffer[0] = '\0';    // packet size exceeded / bad packet
				bufferPosition = -1;
			}


			bufferPosition++;
			readBuffer[bufferPosition] = '\0';
		}
	
	} while (_flightLoopPause);

}



void XPLDevice::_processPacket(void)
{

	char   writeBuffer[XPLMAX_PACKETSIZE];
	char   speechBuf[XPLMAX_PACKETSIZE];

	int bindingNumber;
	long int rate;
	float precision;
	int element;

	packetsReceived++;


	if (!port) return;

//		fprintf(errlog, "_processPacket:  %s \r\n", readBuffer);

	if (serialLogFile) fprintf(serialLogFile, "et: %5.0f rx port: %s length: %3.3i buffer:         %s\n", elapsedTime, port->portName, (int)strlen(readBuffer), readBuffer);


	readBuffer[bufferPosition+1] = 0;			// terminate the string just in case


	switch (readBuffer[1])
	{

	case XPLREQUEST_REGISTERDATAREF:
	{
		
		_parseString(myBindings[refHandleCounter].xplaneDataRefName, readBuffer,2, 80 );
		//_parseInt(&myBindings[refHandleCounter].RWMode, readBuffer, 3);
		
		fprintf(errlog, "\n   Device %s is requesting handle for dataref: \"%s\"...", deviceName, myBindings[refHandleCounter].xplaneDataRefName);

		myBindings[refHandleCounter].xplaneDataRefHandle = XPLMFindDataRef(myBindings[refHandleCounter].xplaneDataRefName);
		if (myBindings[refHandleCounter].xplaneDataRefHandle == NULL)	// if not found, try searching the abbreviations file before giving up
		{
			gAbbreviations.convertString(myBindings[refHandleCounter].xplaneDataRefName);
			myBindings[refHandleCounter].xplaneDataRefHandle = XPLMFindDataRef(myBindings[refHandleCounter].xplaneDataRefName);
		}

		if (myBindings[refHandleCounter].xplaneDataRefHandle != NULL)
		{
			fprintf(errlog, "I found that DataRef!\n");
			
			myBindings[refHandleCounter].bindingActive = 1;
			myBindings[refHandleCounter].deviceIndex = _referenceID;
			//myBindings[refHandleCounter].xplaneDataRefArrayOffset = atoi(arrayReference);
			//myBindings[refHandleCounter].divider = atof(dividerString);
			//myBindings[refHandleCounter].RWMode = readBuffer[2] - '0';
			//fprintf(errlog, "RWMode %c was saved as int %i\r\n", strippedPacket[1], myBindings[refHandleCounter].RWMode);
			myBindings[refHandleCounter].xplaneDataRefTypeID = XPLMGetDataRefTypes(myBindings[refHandleCounter].xplaneDataRefHandle);

			sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",%i", refHandleCounter);

			_writePacket(XPLRESPONSE_DATAREF, writeBuffer);
			
			fprintf(errlog, "      I responded with %3.3i as a handle using this packet:  %s\n", refHandleCounter, writeBuffer);
			if (myBindings[refHandleCounter].xplaneDataRefTypeID & xplmType_Int)        fprintf(errlog, "      This dataref returns that it is of type: int\n");
			if (myBindings[refHandleCounter].xplaneDataRefTypeID & xplmType_Float)      fprintf(errlog, "      This dataref returns that it is of type: float\n");
			if (myBindings[refHandleCounter].xplaneDataRefTypeID & xplmType_Double)     fprintf(errlog, "      This dataref returns that it is of type: double\n");
			if (myBindings[refHandleCounter].xplaneDataRefTypeID & xplmType_FloatArray) fprintf(errlog, "      This dataref returns that it is of type: floatArray\n");
			if (myBindings[refHandleCounter].xplaneDataRefTypeID & xplmType_IntArray)   fprintf(errlog, "      This dataref returns that it is of type: intArray\n");
			if (myBindings[refHandleCounter].xplaneDataRefTypeID & xplmType_Data)
				{
					fprintf(errlog, "      This dataref returns that it is of type: data ***Currently supported only for data sent from xplane (read only)***\n");
					myBindings[refHandleCounter].currentSents[0] = (char*)malloc(XPLMAX_PACKETSIZE - 5);
				}
			

			refHandleCounter++;
		}
		else
		{
			sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",-02,\"%s\"", myBindings[refHandleCounter].xplaneDataRefName);
			_writePacket(XPLRESPONSE_DATAREF, writeBuffer);
			myBindings[refHandleCounter].bindingActive = 0;
			fprintf(errlog, "   requested DataRef not found, sorry. \n   I sent back data frame: %s\n", writeBuffer);
			refHandleCounter++;			// to avoid timeout
		}


		break;
	}

	case XPLREQUEST_UPDATES:
		

		_parseInt(&bindingNumber, readBuffer, 2);
		_parseInt(&rate, readBuffer, 3);
		_parseFloat(&precision, readBuffer, 4);

		myBindings[bindingNumber].readFlag[0] = 1;
		myBindings[bindingNumber].updateRate = rate;
		myBindings[bindingNumber].precision = precision;
		fprintf(errlog, "   Device requested that %s dataref be updated at rate: %i and precision %f\n", myBindings[bindingNumber].xplaneDataRefName, rate, precision);

		break;

	case XPLREQUEST_UPDATESARRAY:
		

		_parseInt(&bindingNumber, readBuffer, 2);
		_parseInt(&rate, readBuffer, 3);
		_parseFloat(&precision, readBuffer, 4);
		_parseInt(&element, readBuffer, 5);

		myBindings[bindingNumber].readFlag[element] = 1;
		myBindings[bindingNumber].updateRate = rate;
		myBindings[bindingNumber].precision = precision;
		fprintf(errlog, "   Device requested that %s dataref element %i be updated at rate: %i and precision %f\n", myBindings[bindingNumber].xplaneDataRefName, element, rate, precision);

		break;

	case XPLREQUEST_SCALING:
		_parseInt(&bindingNumber, readBuffer, 2);
		_parseInt(&myBindings[bindingNumber].scaleFromLow, readBuffer, 3);
		_parseInt(&myBindings[bindingNumber].scaleFromHigh, readBuffer, 4);
		_parseInt(&myBindings[bindingNumber].scaleToLow, readBuffer, 5);
		_parseInt(&myBindings[bindingNumber].scaleToHigh, readBuffer, 6);
		myBindings[bindingNumber].scaleFlag = 1;

		break;

	case XPLREQUEST_REGISTERCOMMAND:
	{

		_parseString(myCommands[cmdHandleCounter].xplaneCommandName, readBuffer, 2, 80);
		
		fprintf(errlog, "   Device %s is requesting command: %s...", deviceName, myCommands[cmdHandleCounter].xplaneCommandName);

		myCommands[cmdHandleCounter].xplaneCommandHandle = XPLMFindCommand(myCommands[cmdHandleCounter].xplaneCommandName);
		if (myCommands[cmdHandleCounter].xplaneCommandHandle == NULL)   // if not found, try searching the abbreviations file before giving up
		{
				gAbbreviations.convertString(myCommands[cmdHandleCounter].xplaneCommandName);
				myCommands[cmdHandleCounter].xplaneCommandHandle = XPLMFindCommand(myCommands[cmdHandleCounter].xplaneCommandName);
		}

		

		if (myCommands[cmdHandleCounter].xplaneCommandHandle != NULL)
		{
			fprintf(errlog, "I found that Command!\n");
			
			myCommands[cmdHandleCounter].bindingActive = 1;
			myCommands[cmdHandleCounter].deviceIndex = _referenceID;

			sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",%i", cmdHandleCounter);

			_writePacket(XPLRESPONSE_COMMAND, writeBuffer);

			//if (!myXPLDevices[port].Port->WriteData(writeBuffer, strlen(writeBuffer)))
			//	fprintf(errlog, "      Problem occurred sending handle to the device, sorry.\n");
			//else
			{
				fprintf(errlog, "      I responded with %3.3i as a handle using this packet:  %s\n", cmdHandleCounter, writeBuffer);
			}

			cmdHandleCounter++;
		}
		else

		{
			sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",-02,\"%s\"", &readBuffer[2]);
			_writePacket(XPLRESPONSE_COMMAND, writeBuffer);
			myCommands[cmdHandleCounter].bindingActive = 0;
			fprintf(errlog, "   requested Command not found, sorry. \n   I sent back data frame: %s\n", writeBuffer);
			cmdHandleCounter++;
		}


		break;
	}

	case XPLCMD_DATAREFUPDATEINT:
	case XPLCMD_DATAREFUPDATEFLOAT:
	case XPLCMD_DATAREFUPDATEFLOATARRAY:
	case XPLCMD_DATAREFUPDATEINTARRAY:
	{
		
		float tempFloat;
		int tempInt;
		int tempElement;

		//	fprintf(errlog, "Device is sending dataRef update with packet: %s \r\n", strippedPacket);

		_parseInt(&bindingNumber, readBuffer, 2);

		if (bindingNumber < 0 && bindingNumber > refHandleCounter) break;

		lastRefReceived = bindingNumber;			// for the status window

		if (myBindings[bindingNumber].xplaneDataRefTypeID & xplmType_Int)
		{
			_parseInt(&tempInt, readBuffer, 3);
			if (myBindings[bindingNumber].scaleFlag) tempInt = mapInt(tempInt,	myBindings[bindingNumber].scaleFromLow,
																				myBindings[bindingNumber].scaleFromHigh,
																				myBindings[bindingNumber].scaleToLow,
																				myBindings[bindingNumber].scaleToHigh);
			
			XPLMSetDatai(myBindings[bindingNumber].xplaneDataRefHandle, tempInt);
			myBindings[bindingNumber].currentReceivedl[0] = tempInt;
			myBindings[bindingNumber].currentSentl[0] = tempInt;
		}
		
		if (myBindings[bindingNumber].xplaneDataRefTypeID & xplmType_Float)
		{
			
			_parseFloat(&tempFloat, readBuffer, 3);
			if (myBindings[bindingNumber].scaleFlag) tempFloat = mapFloat(tempFloat,	myBindings[bindingNumber].scaleFromLow,
																						myBindings[bindingNumber].scaleFromHigh,
																						myBindings[bindingNumber].scaleToLow,
																						myBindings[bindingNumber].scaleToHigh);


			XPLMSetDataf(myBindings[bindingNumber].xplaneDataRefHandle, tempFloat);
			myBindings[bindingNumber].currentReceivedf[0] = tempFloat;
			myBindings[bindingNumber].currentSentf[0] = tempFloat;
		}
		
		if (myBindings[bindingNumber].xplaneDataRefTypeID & xplmType_Double)
		{
			_parseFloat(&tempFloat, readBuffer, 3);
			if (myBindings[bindingNumber].scaleFlag) tempFloat = mapFloat(tempFloat,	myBindings[bindingNumber].scaleFromLow,
																						myBindings[bindingNumber].scaleFromHigh,
																						myBindings[bindingNumber].scaleToLow,
																						myBindings[bindingNumber].scaleToHigh);
			XPLMSetDatad(myBindings[bindingNumber].xplaneDataRefHandle, tempFloat);
			myBindings[bindingNumber].currentReceivedf[0] = tempFloat;
			myBindings[bindingNumber].currentSentf[0] = tempFloat;
			
		}

		
		if (myBindings[bindingNumber].xplaneDataRefTypeID & xplmType_IntArray)
		{
			_parseInt(&tempInt, readBuffer, 3);
			_parseInt(&tempElement, readBuffer, 4);// need error checking here

			if (myBindings[bindingNumber].scaleFlag) tempInt = mapInt(tempInt,	myBindings[bindingNumber].scaleFromLow,
																				myBindings[bindingNumber].scaleFromHigh,
																				myBindings[bindingNumber].scaleToLow,
																				myBindings[bindingNumber].scaleToHigh);

			XPLMSetDatavi(myBindings[bindingNumber].xplaneDataRefHandle, &tempInt, tempElement, 1);
			myBindings[bindingNumber].currentReceivedl[tempElement] = tempInt;
			myBindings[bindingNumber].currentSentl[tempElement] = tempInt;
			myBindings[bindingNumber].currentElementSent[tempElement] = tempElement;
			
		}

		if (myBindings[bindingNumber].xplaneDataRefTypeID & xplmType_FloatArray)						// process for datarefs of type float (array)
		{
			_parseFloat(&tempFloat, readBuffer, 3);
			_parseInt(&tempElement, readBuffer, 4);		// need error checking here

			if (myBindings[bindingNumber].scaleFlag) tempFloat = mapFloat(tempFloat,	myBindings[bindingNumber].scaleFromLow,
																						myBindings[bindingNumber].scaleFromHigh,
																						myBindings[bindingNumber].scaleToLow,
																						myBindings[bindingNumber].scaleToHigh);

			XPLMSetDatavf(myBindings[bindingNumber].xplaneDataRefHandle, &tempFloat, tempElement, 1);
			myBindings[bindingNumber].currentReceivedf[tempElement] = tempFloat;
			myBindings[bindingNumber].currentSentf[tempElement] = tempFloat;
			lastRefElementSent = tempElement;
		}
		

		break;
		
	}

	case XPLCMD_COMMANDSTART:
	case XPLCMD_COMMANDEND:
	case XPLCMD_COMMANDTRIGGER:
	{

		
		int  commandNumber;  
		int  triggerCount;
		
		_parseInt(&commandNumber, readBuffer, 2);
		_parseInt(&triggerCount, readBuffer, 3);

		fprintf(errlog, "Command received for myCommands[%i].  cmdHandleCounter = %i. triggerCount: %i.  readBuffer: %s\n", commandNumber, cmdHandleCounter, triggerCount, readBuffer);


		if (commandNumber < 0 || commandNumber >= cmdHandleCounter) break;

		lastCmdAction = commandNumber;

		if (readBuffer[1] == XPLCMD_COMMANDTRIGGER)
		{

			
		//	fprintf(errlog, "Device is sending Command %i %i times \n", commandNumber, triggerCount);
			myCommands[commandNumber].accumulator += triggerCount-1;

			XPLMCommandOnce(myCommands[commandNumber].xplaneCommandHandle);

				


			break;
		}

		if (readBuffer[1] == XPLCMD_COMMANDSTART)
		{
		//	fprintf(errlog, "Device is sending Command Start to derived commandNumber: %i\n", commandNumber);
			XPLMCommandBegin(myCommands[commandNumber].xplaneCommandHandle);
		}

		if (readBuffer[1] == XPLCMD_COMMANDEND)
		{
		//	fprintf(errlog, "Device is sending Command End to derived commandNumber: %i\n", commandNumber);
			//fprintf(errlog, "Device is sending Command End with packet: %s \n", strippedPacket);
			XPLMCommandEnd(myCommands[commandNumber].xplaneCommandHandle);
		}

		break;
	}


	case XPLCMD_PRINTDEBUG:
	{
		_parseString(lastDebugMessageReceived, readBuffer, 2, 80);
		fprintf(errlog, "Device \"%s\" sent debug message: \"%s\"\n", deviceName, lastDebugMessageReceived);
		
		break;
	}

	case XPLCMD_SPEAK:
	{
		_parseString(speechBuf, readBuffer, 2, 80);
		XPLMSpeakString(&readBuffer[2]);
		break;
	}

	case XPLRESPONSE_NAME:
	{
		
		_parseString(deviceName, readBuffer, 2, 80);
		setActive(1);
		
		break;
	}

		
	case XPLCMD_FLIGHTLOOPPAUSE:
	{
		_flightLoopPause = 1;
		break;
	}

	case XPLCMD_FLIGHTLOOPRESUME:
	{	
		_flightLoopPause = 0;
		break;
	}

	case XPLREQUEST_NOREQUESTS:
	{
		//RefsLoaded = 1;
		//fprintf(errlog, "   Device \"%s\" says it has no more dataRefs or commands to register!\n\n", deviceName);
		break;
	}

	case XPLCMD_RESET:
	{
		reloadDevices();
		break;
	}

	default:
	{
		fprintf(errlog, "invalid command received\n");
		break;
	}

	}
}

int XPLDevice::_parseString(char *outBuffer, char *inBuffer, int parameter, int maxSize)		// todo:  Confirm 0 length strings ("") dont cause issues
{
	int cBeg;
	int pos = 0;
	int len;
	
	for (int i = 1; i < parameter; i++)
	{

		while (inBuffer[pos] != ',' && inBuffer[pos] != NULL ) pos++;
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

int XPLDevice::_parseInt(int *outTarget, char *inBuffer, int parameter)
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
int XPLDevice::_parseInt(long int* outTarget, char* inBuffer, int parameter)
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
	*outTarget = atol((char*)&inBuffer[cBeg]);

	//	fprintf(errlog, "outTarget: %i cBeg: %i pos: %i string: %s\n", *outTarget, cBeg, pos, (char*)&inBuffer[cBeg]);

	inBuffer[pos] = holdChar;



	return 0;

}

int XPLDevice::_parseFloat(float* outTarget, char* inBuffer, int parameter)
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
	*outTarget = atof((char*)&inBuffer[cBeg]);

	//	fprintf(errlog, "outTarget: %i cBeg: %i pos: %i string: %s\n", *outTarget, cBeg, pos, (char*)&inBuffer[cBeg]);

	inBuffer[pos] = holdChar;



	return 0;

}

int XPLDevice::_writePacket(char cmd, char* packet)
{
	//return 0;
	char writeBuffer[XPLMAX_PACKETSIZE];
	snprintf(writeBuffer, XPLMAX_PACKETSIZE, "%c%c%s%c", XPL_PACKETHEADER, cmd, packet, XPL_PACKETTRAILER);


	if (serialLogFile) fprintf(serialLogFile, "et: %5.0f tx port: %s length: %3.3zi packet: %s\n", elapsedTime, port->portName, strlen(writeBuffer), writeBuffer);



	if (!port->writeData(writeBuffer, strlen(writeBuffer)))
	{
		fprintf(errlog, "Problem occurred during write: %s.\n", writeBuffer);
		return 0;
	}

	packetsSent++;

	return 1;
}


int XPLDevice::_writePacketN(char cmd, char* packet, int packetSize)
{
	//return 0;
	char writeBuffer[XPLMAX_PACKETSIZE];
	snprintf(writeBuffer, XPLMAX_PACKETSIZE, "%c%c,", XPL_PACKETHEADER, cmd);
	memcpy((void*)&writeBuffer[3], packet, packetSize);
	writeBuffer[packetSize + 2] = XPL_PACKETTRAILER;
	writeBuffer[packetSize + 3] = 0;							// terminate with null

	if (serialLogFile)
	{
		fprintf(serialLogFile, "et: %5.0f tx port: %s length: %3.3i packet: ",elapsedTime, port->portName, packetSize + 3);
		for (int i = 0; i < packetSize + 3; i++)
		{
			if (isprint(writeBuffer[i]))	fprintf(serialLogFile, "%c", writeBuffer[i]);
			else fprintf(serialLogFile, "~");
		}
		fprintf(serialLogFile, "\n");

	}

	if (!port->writeData(writeBuffer, packetSize + 6))
	{
		fprintf(errlog, "Problem occurred during write: %s.\n", writeBuffer);
		return 0;
	}

	packetsSent++;

	return 1;
}
