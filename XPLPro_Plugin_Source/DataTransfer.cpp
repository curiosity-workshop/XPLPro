#define XPLM200




#include "XPLProCommon.h"

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
#include "XPWidgetUtils.h"


#include "XPLProPlugin.h"
#include "XPLDevice.h"

#include "DataTransfer.h"

#include <ctime>



int refHandleCounter = 0;
int cmdHandleCounter = 0;


long int packetsSent;
long int packetsReceived;

int validPorts = 0;

extern FILE* errlog;
extern FILE* serialLogFile;
extern float elapsedTime;
extern int lastRefSent;
extern int lastRefElementSent;


CommandBinding myCommands[XPL_MAXCOMMANDS_PC];
DataRefBinding myBindings[XPL_MAXDATAREFS_PC];
XPLDevice* myXPLDevices[XPLDEVICES_MAXDEVICES];

/**************************************************************************************/
/* disengage -- unregister all datarefs and close all com ports                       */
/**************************************************************************************/
void disengageDevices(void)
{
	sendExitMessage();

	for (int i = 0; i < XPLDEVICES_MAXDEVICES; i++)
	{

		if (myXPLDevices[i])
		{
			myXPLDevices[i]->port->shutDown();
			delete myXPLDevices[i]->port;
			delete myXPLDevices[i];
			myXPLDevices[i] = NULL;
		}
	}

	validPorts = 0;

	for (int i = 0; i < refHandleCounter; i++)
	{
		myBindings[i].deviceIndex = -1;
		myBindings[i].bindingActive = 0;
		myBindings[i].Handle = -1;
		myBindings[i].scaleFlag = 0;

		for (int j = 0; j < XPLMAX_ELEMENTS; j++)
		{
			myBindings[i].readFlag[j] = 0;
		}
		
		XPLMUnregisterDataAccessor(myBindings[i].xplaneDataRefHandle);  // deregister with xplane
		myBindings[i].xplaneDataRefTypeID = 0;
		myBindings[i].xplaneDataRefName[0] = NULL;
		if (myBindings[i].currentSents[0] != NULL)
		{
			free(myBindings[i].currentSents[0]);
			myBindings[i].currentSents[0] = NULL;
		}

	}

	refHandleCounter = 0;

	for (int i = 0; i < cmdHandleCounter; i++)
	{
		myCommands[i].deviceIndex = -1;
		myCommands[i].bindingActive = 0;
		myCommands[i].Handle = -1;
		myCommands[i].xplaneCommandHandle = NULL;
		myCommands[i].xplaneCommandName[0] = NULL;

	}

	cmdHandleCounter = 0;


}

/*
*   Searches com ports for devices, then queries for datarefs and commands.
*/
void engageDevices(void)
{
	fprintf(errlog, "engageDevices: started...\n");
	findDevices();
	activateDevices();
	//_updateDataRefs(1);				// 1 represents to force updates to the devices
	//sendRefreshRequest();

}

/**************************************************************************************/
/* _updateDataRefs -- get current dataref values for all registered datarefs          */
/**************************************************************************************/
void _updateDataRefs(int forceUpdate)
{

	int newVall;
	float newValf;
	double newValD;

	char   writeBuffer[XPLMAX_PACKETSIZE];
	char   stringBuffer[XPLMAX_PACKETSIZE - 5];



	for (int i = 0; i < refHandleCounter; i++)
	{
		if (myBindings[i].bindingActive && myBindings[i].readFlag[0])					// todo:  this needs to check all possible readFlags
		{
			
		//	if (elapsedTime - myXPLDevices[myBindings[i].deviceIndex].lastSendTime < myXPLDevices[myBindings[i].deviceIndex].minTimeBetweenFrames/1000 && !forceUpdate)
	//			break;
			if (myBindings[i].xplaneDataRefTypeID & xplmType_Int)						// process for datarefs of type int
			{
				newVall = (long int)XPLMGetDatai(myBindings[i].xplaneDataRefHandle);
				
				if (newVall != myBindings[i].currentSentl[0] || forceUpdate)
				{
					lastRefSent = i;
					myBindings[i].currentSentl[0] = newVall;
					sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",%i,%ld", i, newVall);
					myXPLDevices[myBindings[i].deviceIndex]->_writePacket(XPLCMD_DATAREFUPDATEINT, writeBuffer);
					myXPLDevices[myBindings[i].deviceIndex]->lastSendTime = elapsedTime;

						//   fprintf(errlog, "using packet: %s\r\n", writeBuffer);

				}

			}
			
			if (myBindings[i].xplaneDataRefTypeID & xplmType_IntArray)						// process for datarefs of type int Array
			{
				for (int j = 0; j < XPLMAX_ELEMENTS; j++)				// todo, this method should be something better
				{
					XPLMGetDatavi(myBindings[i].xplaneDataRefHandle, &newVall, j, 1);
					if (myBindings[i].precision)  newVall = ((int)(newVall / myBindings[i].precision) * myBindings[i].precision);
					if (newVall != myBindings[i].currentSentl[j] || forceUpdate)
					{
						lastRefSent = i;
						lastRefElementSent = j;
						myBindings[i].currentSentl[j] = newVall;
						sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",%i,%ld,%i", i, newVall,j);
						myXPLDevices[myBindings[i].deviceIndex]->_writePacket(XPLCMD_DATAREFUPDATEINTARRAY, writeBuffer);
						myXPLDevices[myBindings[i].deviceIndex]->lastSendTime = elapsedTime;

						//   fprintf(errlog, "using packet: %s\r\n", writeBuffer);
					}
				}
			}
				
				
			if (myBindings[i].xplaneDataRefTypeID & xplmType_Float)						// process for datarefs of type float
			{

				newValf = (float)XPLMGetDataf(myBindings[i].xplaneDataRefHandle);

					// fprintf(errlog, "updating dataRef %s with value %f...\r\n  ", myBindings[i].xplaneDataRefName, newValf);
				if (myBindings[i].precision)  newValf = ((int)(newValf / myBindings[i].precision) * myBindings[i].precision);

				if (newValf != myBindings[i].currentSentf[0] || forceUpdate)
				{
					lastRefSent = i;
					myBindings[i].currentSentf[0] = newValf;
					sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",%i,%f", i, newValf);
					myXPLDevices[myBindings[i].deviceIndex]->_writePacket(XPLCMD_DATAREFUPDATEFLOAT, writeBuffer);
					myXPLDevices[myBindings[i].deviceIndex]->lastSendTime = elapsedTime;

						//   	   fprintf(errlog, "using packet: %s\r\n", writeBuffer);

				}
				
			}


			if (myBindings[i].xplaneDataRefTypeID & xplmType_FloatArray)						// process for datarefs of type float (array)
			{
				

				for (int j = 0; j < XPLMAX_ELEMENTS; j++)
				{
					XPLMGetDatavf(myBindings[i].xplaneDataRefHandle, &newValf, j, 1);
					if (myBindings[i].precision)  newValf = ((int)(newValf / myBindings[i].precision) * myBindings[i].precision);
					
					if (newValf != myBindings[i].currentSentf[j] || forceUpdate)
					{
						lastRefSent = i;
						lastRefElementSent = j;
						myBindings[i].currentSentf[j] = newValf;
						sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",%i,%f,%i", i, newValf,j);
						myXPLDevices[myBindings[i].deviceIndex]->_writePacket(XPLCMD_DATAREFUPDATEFLOATARRAY, writeBuffer);
						myXPLDevices[myBindings[i].deviceIndex]->lastSendTime = elapsedTime;

						//  fprintf(errlog, "using packet: %s\r\n", writeBuffer);

					}
				}

			}
			
			if (myBindings[i].xplaneDataRefTypeID & xplmType_Double)						// process for datarefs of type double
			{

				newValD = (double)XPLMGetDatad(myBindings[i].xplaneDataRefHandle);

					// fprintf(errlog, "updating dataRef %s with value %f...\r\n  ", myBindings[i].xplaneDataRefName, newValf);
				if (myBindings[i].precision)  newValD = ((int)(newValD / myBindings[i].precision) * myBindings[i].precision);

				if (newValD != myBindings[i].currentSentD[0] || forceUpdate)
				{
					lastRefSent = i;
					myBindings[i].currentSentD[0] = newValD;
					sprintf_s(writeBuffer, XPLMAX_PACKETSIZE, ",%i,%f", i, newValD);
					myXPLDevices[myBindings[i].deviceIndex]->_writePacket(XPLCMD_DATAREFUPDATEFLOAT, writeBuffer);
					myXPLDevices[myBindings[i].deviceIndex]->lastSendTime = elapsedTime;

						//   	   fprintf(errlog, "using packet: %s\r\n", writeBuffer);

				}
					
			}

			if (myBindings[i].xplaneDataRefTypeID & xplmType_Data)						// process for datarefs of type Data (strings)
			{

				newVall = (long int)XPLMGetDatab(myBindings[i].xplaneDataRefHandle, stringBuffer, 0, XPLMAX_PACKETSIZE - 5);
					//stringBuffer[newVall] = 0;		// null terminate
					//   fprintf(errlog, "updating dataRef %s with value %i...\r\n  ", myBindings[i].xplaneDataRefName, newVall);

				if (memcmp(myBindings[i].currentSents[0], stringBuffer, newVall) || forceUpdate)
				{
					lastRefSent = i;
					memcpy(myBindings[i].currentSents[0], stringBuffer, newVall);
					sprintf(writeBuffer, ",%i,", i);
					
					memcpy(&writeBuffer[3], stringBuffer, newVall);
					myXPLDevices[myBindings[i].deviceIndex]->_writePacketN(XPLCMD_DATAREFUPDATESTRING, writeBuffer, newVall + 3);
					myXPLDevices[myBindings[i].deviceIndex]->lastSendTime = elapsedTime;

					//	fprintf(errlog, "Updating dataref %s with packet: %s length: %i\r\n", myBindings[i].xplaneDataRefName, writeBuffer, newVall);
					
					
				}

			}
		}

	}


}

/*
	_updateCommands -- make sure commands are all updated.
 */
void _updateCommands(void)
{
	
	
	for (int i = 0; i < cmdHandleCounter; i++)
	{
		if (myCommands[i].bindingActive && myCommands[i].accumulator > 0)
		{
			
			XPLMCommandOnce(myCommands[i].xplaneCommandHandle);
			myCommands[i].accumulator--;
			if (myCommands[i].accumulator < 0) myCommands[i].accumulator = 0;
			
			
		}

	}


}

/**************************************************************************************/
/* activateDevices -- request dataref bindings from all active devices                */
/**************************************************************************************/
void activateDevices(void)
{
	
	fprintf(errlog, "XPLPro:  Activating Devices... \n");

	for (int i = 0; i < XPLDEVICES_MAXDEVICES; i++)
	{
		if (!myXPLDevices[i]) break;

		fprintf(errlog, "Requesting dataRef or Command registrations from port %s on device [%i]: %s\n", myXPLDevices[i]->port->portName, i, myXPLDevices[i]->deviceName);
		myXPLDevices[i]->_writePacket(XPLCMD_SENDREQUEST, "");
				
	}
	
}

/*
   findDevices -- Scan for XPLPro devices and fills array with active devices
*/

int findDevices(void)
{
	time_t startTime;
	serialClass* port;
	validPorts = 0;

	fprintf(errlog, "Searching Com Ports... ");

	for (UINT i = 1; i < 256; i++)
	{
		port = new serialClass;
		
		if (port->begin(i) == i)
		{

			fprintf(errlog, "\nFound valid port %s.  Attemping poll for XPLPro device... ", port->portName);
			myXPLDevices[validPorts] = new XPLDevice(validPorts);
			myXPLDevices[validPorts]->port = port;

			if (myXPLDevices[validPorts]->_writePacket(XPLCMD_SENDNAME, ""))
				fprintf(errlog, "Valid write operation, seems OK\n");


			startTime = time(NULL);

			while (difftime(time(NULL), startTime) < XPL_TIMEOUT_SECONDS && !myXPLDevices[validPorts]->isActive())	_processSerial();

			if (!myXPLDevices[validPorts]->isActive())
			{
				fprintf(errlog, "No response after %i seconds\n", XPL_TIMEOUT_SECONDS);
				XPLMDebugString(".");
				port->shutDown();
				delete myXPLDevices[validPorts];
				delete port;
				//myXPLDevices[validPorts]->comPortName[0] = 0;
			}

			else
			{
				myXPLDevices[validPorts]->readBuffer[0] = '\0';
				fprintf(errlog, "   Device [%i] on %s identifies as an XPLPro device named: %s\n", validPorts, port->portName, myXPLDevices[validPorts]->deviceName);

				validPorts++;
			}

		}
		else
		{
			port->shutDown();
			delete port;
		}

	}
	

	//delete myXPLDevices[validPorts];
	XPLMDebugString("  Done Searching Com Ports\n");
	fprintf(errlog, "Total of %i compatible devices were found.  \n\n", validPorts);
	return 0;
}


void _processSerial()
{
	int port = 0;

	while (myXPLDevices[port] && port < XPLDEVICES_MAXDEVICES)
	{
		//fprintf(errlog, "working on xpldevice %i ...", port);

		myXPLDevices[port]->processSerial();

		port++;
	}
}

/*
	 sendRefreshRequest -- request writable datarefs be refreshed
*/

void sendRefreshRequest(void)
{
	for (int i = 0; i < XPLDEVICES_MAXDEVICES; i++)
	{
		if (myXPLDevices[i])
			if (myXPLDevices[i] && myXPLDevices[i]->RefsLoaded)  myXPLDevices[i]->_writePacket(XPLREQUEST_REFRESH, "");
	}

}

void sendExitMessage(void)
{
	fprintf(errlog, "\n*Xplane indicates that it is closing or unloading the current aircraft.  I am letting all the devices know.\n");

	for (int i = 0; i < XPLDEVICES_MAXDEVICES; i++)
	{
		if (myXPLDevices[i])
			if (myXPLDevices[i] && myXPLDevices[i]->RefsLoaded)  myXPLDevices[i]->_writePacket(XPL_EXITING, "");
	}

}



/*
*   reloadDevices
*/
void reloadDevices(void)
{
	fprintf(errlog, "XPLPro device requested to reset and reload devices.  \n");


	disengageDevices();				// just to make sure we are cleared
	engageDevices();



}


/*
 * map functions
 */
float mapFloat(long x, long inMin, long inMax, long outMin, long outMax)
{
	return (float)(x - inMin) * (outMax - outMin) / (float)(inMax - inMin) + outMin;
}

long mapInt(long x, long inMin, long inMax, long outMin, long outMax)
{
	return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}