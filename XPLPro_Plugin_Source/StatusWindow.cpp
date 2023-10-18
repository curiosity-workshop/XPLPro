
#include <stdio.h>
#include <string>
#include <ctime>  



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


#include "serialclass.h"
#include "Config.h"

#include "XPLProPlugin.h"

#include "DataTransfer.h"
#include "StatusWindow.h"

XPLMWindowID	statusWindow = NULL;

extern CommandBinding myCommands[XPL_MAXCOMMANDS_PC];
extern DataRefBinding myBindings[XPL_MAXDATAREFS_PC];


extern long int packetsSent;
extern long int packetsReceived;
extern int validPorts;

extern long cycleCount;
extern float elapsedTime;

extern int cmdHandleCounter;
extern int refHandleCounter;

int lastCmdAction = -1;
int lastRefSent = -1;
int lastRefElementSent = 0;
int lastRefReceived = -1;
int lastRefElementReceived = 0;


void statusWindowCreate()
{
	statusWindow = XPLMCreateWindow(
		50, 600, 800, 200,			/* Area of the window. */
		1,							/* Start visible. */
		statusDrawWindowCallback,		/* Callbacks */
		statusHandleKeyCallback,
		statusHandleMouseClickCallback,
		NULL);						/* Refcon - not used. */
}

/**************************************************************************************/
/* MyDrawWindowCallback -- Called by xplane while window is active        */
/**************************************************************************************/
void statusDrawWindowCallback(
	XPLMWindowID         inWindowID,
	void* inRefcon)
{
	int		left, top, right, bottom;
	float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */
	char tstring[800];

	/* First we get the location of the window passed in to us. */
	XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

	/* We now use an XPLMGraphics routine to draw a translucent dark
	 * rectangle that is our window's shape. */
	XPLMDrawTranslucentDarkBox(left, top, right, bottom);

	/* Finally we draw the text into the window, also using XPLMGraphics
	 * routines.  The NULL indicates no word wrapping. */
	XPLMDrawString(color, left + 5, top - 20, "Curiosity Workshop XPLPro Arduino Interface.  Updates and Examples: www.patreon.com/curiosityworkshop", NULL, xplmFont_Basic);


	sprintf(tstring, "Distribution Build: %u.  Click this window when complete", XPL_VERSION);
	XPLMDrawString(color, left + 5, top - 35, tstring, NULL, xplmFont_Basic);


	sprintf(tstring, "Devices Detected: %i, Registered DataRefs: %i, Registered Commands: %i, tx: %i, rx: %i", validPorts, refHandleCounter, cmdHandleCounter, packetsSent, packetsReceived);
	XPLMDrawString(color, left + 5, top - 55, tstring, NULL, xplmFont_Basic);
	sprintf(tstring, "Elapsed time since start: %i, cycles: %i, average time between cycles: %3.2f", (int)elapsedTime, cycleCount, elapsedTime / cycleCount);
	XPLMDrawString(color, left + 5, top - 70, tstring, NULL, xplmFont_Basic);

	if (lastRefReceived >= 0)
	{
		//sprintf(tstring, "Last ref received: %i, %s\n", lastRefReceived, myBindings[lastRefReceived].xplaneDataRefName);
		
		if (myBindings[lastRefReceived].xplaneDataRefTypeID & xplmType_Int)			sprintf(tstring, "Last Dataref Received: %s, %i", myBindings[lastRefReceived].xplaneDataRefName, myBindings[lastRefReceived].currentReceivedl[lastRefElementReceived]);
		if (myBindings[lastRefReceived].xplaneDataRefTypeID & xplmType_Float)		sprintf(tstring, "Last Dataref Received: %s, %f", myBindings[lastRefReceived].xplaneDataRefName, myBindings[lastRefReceived].currentReceivedf[lastRefElementReceived]);
		if (myBindings[lastRefReceived].xplaneDataRefTypeID & xplmType_Double)		sprintf(tstring, "Last Dataref Received: %s, %i", myBindings[lastRefReceived].xplaneDataRefName, myBindings[lastRefReceived].currentReceivedl[lastRefElementReceived]);
		if (myBindings[lastRefReceived].xplaneDataRefTypeID & xplmType_IntArray)	sprintf(tstring, "Last Dataref Received: %s, Element: %i, %i", myBindings[lastRefReceived].xplaneDataRefName, lastRefElementReceived, myBindings[lastRefReceived].currentReceivedl[lastRefElementReceived]);
		if (myBindings[lastRefReceived].xplaneDataRefTypeID & xplmType_FloatArray)	sprintf(tstring, "Last Dataref Received: %s, Element: %i, %f", myBindings[lastRefReceived].xplaneDataRefName, lastRefElementReceived, myBindings[lastRefReceived].currentReceivedf[lastRefElementReceived]);	
	//	if (myBindings[lastRefReceived].xplaneDataRefTypeID & xplmType_Data)			sprintf(tstring, "Last Dataref Action: %s, %s", myBindings[lastRefReceived].xplaneDataRefName, myBindings[lastRefReceived].xplaneCurrentReceiveds);
		XPLMDrawString(color, left + 5, top - 90, tstring, NULL, xplmFont_Basic);
		
	}
	
	if (lastRefSent >= 0)
	{
		//sprintf(tstring, "Last ref sent: %i, %s\n", lastRefReceived, myBindings[lastRefReceived].xplaneDataRefName);

		if (myBindings[lastRefSent].xplaneDataRefTypeID & xplmType_Int)			sprintf(tstring, "Last Dataref Sent: %s, %i", myBindings[lastRefSent].xplaneDataRefName, myBindings[lastRefSent].currentSentl[lastRefElementSent]);
		if (myBindings[lastRefSent].xplaneDataRefTypeID & xplmType_Float)		sprintf(tstring, "Last Dataref Sent: %s, %f", myBindings[lastRefSent].xplaneDataRefName, myBindings[lastRefSent].currentSentf[lastRefElementSent]);
		if (myBindings[lastRefSent].xplaneDataRefTypeID & xplmType_Double)		sprintf(tstring, "Last Dataref Sent: %s, %i", myBindings[lastRefSent].xplaneDataRefName, myBindings[lastRefSent].currentSentl[lastRefElementSent]);
		if (myBindings[lastRefSent].xplaneDataRefTypeID & xplmType_IntArray)	sprintf(tstring, "Last Dataref Sent: %s, Element: %i, %i", myBindings[lastRefSent].xplaneDataRefName, lastRefElementSent, myBindings[lastRefSent].currentSentl[lastRefElementSent]);
		if (myBindings[lastRefSent].xplaneDataRefTypeID & xplmType_FloatArray)	sprintf(tstring, "Last Dataref Sent: %s, Element: %i, %f", myBindings[lastRefSent].xplaneDataRefName, lastRefElementSent, myBindings[lastRefSent].currentSentf[lastRefElementSent]);	
		if (myBindings[lastRefSent].xplaneDataRefTypeID & xplmType_Data)		sprintf(tstring, "Last Dataref Sent: %s, %s", myBindings[lastRefSent].xplaneDataRefName, myBindings[lastRefSent].currentSents[lastRefElementSent]);
		XPLMDrawString(color, left + 5, top - 105, tstring, NULL, xplmFont_Basic);
		
	}

	if (lastCmdAction >= 0)
	{
		sprintf(tstring, "Last Command Action: %s, accumulator: %i", myCommands[lastCmdAction].xplaneCommandName, myCommands[lastCmdAction].accumulator);
		XPLMDrawString(color, left + 5, top - 120, tstring, NULL, xplmFont_Basic);
	}

	

}

int statusWindowActive(void)
{
	if (!statusWindow) return 0;
	else return 1;
}


/**************************************************************************************/
/* MyHandleMouseClickCallback -- Called by xplane while status window is active  */
/**************************************************************************************/
int statusHandleMouseClickCallback(
	XPLMWindowID         inWindowID,
	int                  x,
	int                  y,
	XPLMMouseStatus      inMouse,
	void* inRefcon)
{

	XPLMDestroyWindow(statusWindow);
	statusWindow = NULL;

	return 1;
}

/**************************************************************************************/
/* MyHandleKeyCallback -- Called by xplane while status window is active        */
/**************************************************************************************/
void statusHandleKeyCallback(
	XPLMWindowID         inWindowID,
	char                 inKey,
	XPLMKeyFlags         inFlags,
	char                 inVirtualKey,
	void* inRefcon,
	int                  losingFocus)
{
}

