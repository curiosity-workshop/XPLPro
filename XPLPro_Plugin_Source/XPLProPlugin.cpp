/*


  XPLPro - plugin for serial interface to Arduino
  Created by Curiosity Workshop -- Michael Gerlicher,  September 2020.

  	
*/




#include <stdio.h>



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


#include "StatusWindow.h"

#include "abbreviations.h"

//#include "serialclass.h"

#include "DataTransfer.h"
#include "Config.h"

#include "XPLProPlugin.h"

FILE* serialLogFile;			// for serial data log
FILE* errlog;				// Used for logging problems

Config *XPLConfig;

abbreviations gAbbreviations;


extern long int packetsSent;
extern long int packetsReceived;
extern int validPorts;

long cycleCount = 0l;
float elapsedTime = 0;
int logSerial = false;

int				gClicked = 0;
XPLMMenuID      myMenu;
int             disengageMenuItemIndex;
int				logSerialMenuItemIndex;



XPLMCommandRef ResetCommand = NULL;





PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
		
		int			mySubMenuItem;
		
		char outFilePath[256];
	
	
    fopen_s(&errlog, "XPLProError.log", "w");
	if (!errlog)
	{
		XPLMDebugString("XPLPro:  Unable to open error log file XPLCoreError.log\n");
		return 0;
	}
	setbuf(errlog, NULL);

// Provide our plugin's profile to the plugin system. 
	strcpy(outName, "XPLPro");
	strcpy(outSig, "xplpro.arduino.cw");
	strcpy(outDesc, "Direct communications with Arduino infrastructure");


	fprintf(errlog, "Curiosity Workshop XPLPro interface version %u copyright 2007-2023.  \n", XPL_VERSION );

	fprintf(errlog, "To report problems, download updates and examples, suggest enhancements or get technical support:\r\n");
	fprintf(errlog, "    discord:  https://discord.gg/gzXetjEST4\n");
	fprintf(errlog, "    patreon:  www.patreon.com/curiosityworkshop\n");
	fprintf(errlog, "    YouTube:  youtube.com/channel/UCISdHdJIundC-OSVAEPzQIQ \n\n");

	fprintf(errlog, "XPLPro interface Error log file begins now.\r\n");
	
	XPLMGetPluginInfo( XPLMGetMyID(), NULL, outFilePath, NULL, NULL);   
	fprintf(errlog, "Plugin Path: %s\r\n", outFilePath);

	// first load configuration stuff
	XPLConfig = new Config(CFG_FILE);
	logSerial = XPLConfig->getSerialLogFlag();

	if (logSerial) fprintf(errlog, "Serial logging enabled.\r\n");  else fprintf(errlog, "Serial logging disabled.\r\n");
	
	
	gAbbreviations.begin();

	
	
	XPLMDebugString("XPLPro:  Initializing Plug-in\n");

	
	
	/* First we put a new menu item into the plugin menu.
	 * This menu item will contain a submenu for us. */
	mySubMenuItem = XPLMAppendMenuItem(
						XPLMFindPluginsMenu(),	/* Put in plugins menu */
						"XPLPro",				/* Item Title */
						0,						/* Item Ref */
						1);						/* Force English */
	
	/* Now create a submenu attached to our menu item. */
	myMenu = XPLMCreateMenu(
						"XPLDirectB", 
						XPLMFindPluginsMenu(), 
						mySubMenuItem, 			/* Menu Item to attach to. */
						MyMenuHandlerCallback,	/* The handler */
						0);						/* Handler Ref */
						
	/* Append a few menu items to our submenu.   */
	
	XPLMAppendMenuItem(myMenu, "Status",(void *) "Status", 1);
	disengageMenuItemIndex = XPLMAppendMenuItem(myMenu, "Engage Devices", (void *) "Engage Devices", 1);
	logSerialMenuItemIndex = XPLMAppendMenuItem(myMenu, "Log Serial Data", (void*) "Log Serial Data", 1);
	XPLMAppendMenuSeparator(myMenu);


	if (logSerial) XPLMCheckMenuItem(myMenu, logSerialMenuItemIndex, xplm_Menu_Checked);
	else           XPLMCheckMenuItem(myMenu, logSerialMenuItemIndex, xplm_Menu_Unchecked);

	
	XPLMRegisterFlightLoopCallback(							// Setup timed processing		
		MyFlightLoopCallback,	// Callback 
		-2.0,					// Interval 
		NULL);					// refcon not used. 
	

	ResetCommand = XPLMCreateCommand("XPLPro/ResetDevices", "Disengage / re-engage XPLPro Devices");
	XPLMRegisterCommandHandler(ResetCommand,              // in Command name
		ResetCommandHandler,       // in Handler
		1,                      // Receive input before plugin windows.
		(void*)0);            // inRefcon.
	

	if (logSerial)
	{
		fopen_s(&serialLogFile, "XPLProSerial.log", "w");
		if (serialLogFile) fprintf(serialLogFile, "Serial logger.  Unprintable characters are represented by '~'\n");
	}

	XPLMDebugString("XPLPro:  Plugin initialization complete.\n");


	
	return 1;
}


// XPluginStop

PLUGIN_API void	XPluginStop(void)
{
//	cmpSelect.saveConfiguration();
//	cmpLEDEdit.saveConfiguration();

	
	disengageDevices();
	if (errlog) fprintf(errlog, "Ending plugin, cycle count: %u Packets transmitted: %u, Packets Received: %u\n", cycleCount, packetsSent, packetsReceived);
	if (errlog) fclose(errlog);


	if (serialLogFile) fclose(serialLogFile);
	
	
}


// XPluginDisable Handler.  We do nothing in this plugin
PLUGIN_API void XPluginDisable(void)
{
}

//XpluginEnable handler.  We do nothing in this plugin.
PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

// XPluginRecieveMessage Handler
PLUGIN_API void XPluginReceiveMessage(
	XPLMPluginID	inFromWho,
	int				inMessage,
	void* inParam)
{
	char pluginName[256];

	XPLMGetPluginInfo(inFromWho, pluginName, NULL, NULL, NULL);

	if (inMessage == XPLM_MSG_PLANE_UNLOADED)
	{
		fprintf(errlog, "%s says that the current plane was unloaded.  I will disengage devices.  \n", pluginName);
		disengageDevices();
		XPLMSetMenuItemName(myMenu, disengageMenuItemIndex, "Engage Devices", 0);

	}
	if (inMessage == 108) // 108 is supposed to = XPLM_MSG_LIVERY_LOADED
	//if (inMessage == XPLM_MSG_PLANE_LOADED)
	{
		fprintf(errlog, "%s says that a plane was loaded.  I will attempt to engage XPL/Direct devices.  \n", pluginName);
		engageDevices();
		if (validPorts) XPLMSetMenuItemName(myMenu, disengageMenuItemIndex, "Disengage Devices", 0);
	}

//	fprintf(errlog, "Xplane sent me a message from: %s, message: %i\n", pluginName, inMessage);
}


/**************************************************************************************/
/* MyMenuHandlerCallback-- Handle users request for calibration                       */
/**************************************************************************************/
void	MyMenuHandlerCallback(
	void* inMenuRef,
	void* inItemRef)
{

//	fprintf(errlog, "User selected inMenuRef: %i, inItemRef: %i\n", (int)(int *)inMenuRef, (int)(int *)inItemRef);
	

	// Handle request for status window
	if (!strcmp((const char*)inItemRef, "Status"))   // menu 0, item 1, "Status"
	{
		if (!statusWindowActive() ) statusWindowCreate();
	}
	
	// Handle request for arduino device engagement/disengagement
	if (!strcmp((const char*)inItemRef, "Engage Devices"))   // menu 0, item 2,   "disengage / re-engage"
	{
		if (validPorts)
		{
			fprintf(errlog, "User requested to disengage devices. \n");
			disengageDevices();
			
			XPLMSetMenuItemName(myMenu, disengageMenuItemIndex, "Engage Devices", 0);
			
		}
		else
		{
			fprintf(errlog, "User requested to re-engage devices. \n");

			disengageDevices();				// just to make sure we are cleared
			engageDevices();
			if (validPorts) XPLMSetMenuItemName(myMenu, disengageMenuItemIndex, "Disengage Devices", 0);
		}

	}
	

	// Handle request toggle for serial logging
	if (!strcmp((const char*)inItemRef, "Log Serial Data"))   // menu 0, item 1, "Log Serial Data"
	{
		if (logSerial)
		{
			fclose(serialLogFile);
			logSerial = false;
		    XPLMCheckMenuItem(myMenu, logSerialMenuItemIndex, xplm_Menu_Unchecked);
			XPLConfig->setSerialLogFlag(0);
		}
		else
		{
			fopen_s(&serialLogFile, "XPLProSerial.log", "w");
			
			if (serialLogFile)
			{
				fprintf(serialLogFile, "Serial logger.  Unprintable characters are represented by '~'\n");
				logSerial = true;
				XPLMCheckMenuItem(myMenu, logSerialMenuItemIndex, xplm_Menu_Checked);
				XPLConfig->setSerialLogFlag(1);
			}

		}
	}

}




/**************************************************************************************/
/* MyFlightLoopCallback -- Called by xplane every few flight loops                    */
/**************************************************************************************/
float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void* inRefcon)
{
	//return XPLDIRECT_RETURN_TIME;
	//fprintf(errlog, "Time:  %f \n", inElapsedSinceLastCall);
	elapsedTime += inElapsedSinceLastCall;
		
	_processSerial();
	_updateDataRefs(0);
	_updateCommands();
	
    cycleCount++;
	return (float)XPL_RETURN_TIME;
}




int widgetMessageDispatcher(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t	inParam1, intptr_t inParam2)
{
	// need to add for each dialog for now

	return 0;	// always return 0 if we don't know what to do with the message

}

int    ResetCommandHandler(XPLMCommandRef       inCommand,
	XPLMCommandPhase     inPhase,
	void* inRefcon)
{
	//  Use this structure to have the command executed on button up only.
	if (inPhase == xplm_CommandEnd)
	{

		fprintf(errlog, "XPLPro/ResetDevices command received, I am complying. \n");

		disengageDevices();				// just to make sure we are cleared
		engageDevices();
		if (validPorts) XPLMSetMenuItemName(myMenu, disengageMenuItemIndex, "Disengage Devices", 0);
	}

	// Return 1 to pass the command to plugin windows and X-Plane.
	// Returning 0 disables further processing by X-Plane.
	// In this case we might return 0 or 1 because X-Plane does not duplicate our command.
	return 0;
}