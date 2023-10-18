

#pragma once

#define CFG_FILE				"Resources\\plugins\\XPLPro\\XPLPro.cfg"
#define CFG_ABBREVIATIONS_FILE  "Resources\\plugins\\XPLPro\\abbreviations.txt"

#define ARDUINO_WAIT_TIME 2000

#define XPL_BAUDRATE 115200
#define XPL_MILLIS_BETWEEN_FRAMES_DEFAULT 0			// for data sends
#define XPL_RETURN_TIME   .05							// request next visit every .05 seconds or - for cycles
#define XPL_PACKETHEADER  '['							// 
#define XPL_PACKETTRAILER ']'									

#define XPL_MAXDATAREFS_PC 1000
#define XPL_MAXCOMMANDS_PC 1000

#define XPLDEVICES_MAXDEVICES 30

#define XPLMAX_PACKETSIZE 200
#define XPLMAX_ELEMENTS 10
#define XPL_TIMEOUT_SECONDS 3

#define XPLRESPONSE_NAME           'n'       
#define XPLRESPONSE_DATAREF        'D'   // %3.3i%s    dataref handle, dataref name 
#define XPLRESPONSE_COMMAND        'C'   // %3.3i%s    command handle, command name
#define XPLRESPONSE_VERSION		   'v'	// %3.3i%u	   customer build ID, version
#define XPLCMD_PRINTDEBUG          'g'
#define XPLCMD_RESET               'z'
#define XPLCMD_SPEAK				's'
#define XPLCMD_SENDNAME            'N'
#define XPLCMD_FLIGHTLOOPPAUSE		'p'		// stop flight loop while we register
#define XPLCMD_FLIGHTLOOPRESUME		'q'		// 
#define XPLREQUEST_REGISTERDATAREF 'b'   //  dataref name
#define XPLREQUEST_REGISTERCOMMAND 'm'  // name of the command to register
#define XPLREQUEST_NOREQUESTS      'c'   // nothing to request
#define XPLREQUEST_REFRESH         'd'	//  the plugin will call this once xplane is loaded in order to get fresh updates from arduino handles that write
#define XPLREQUEST_UPDATES         'r'          // arduino is asking the plugin to update the specified dataref with rate and divider parameters
#define XPLREQUEST_UPDATESARRAY     't'
#define XPLREQUEST_SCALING          'u'          // arduino requests the plugin apply scaling to the dataref values
#define XPLREQUEST_DATAREFVALUE 'e'

#define XPLCMD_DATAREFUPDATEINT			'1'
#define XPLCMD_DATAREFUPDATEFLOAT		'2'
#define XPLCMD_DATAREFUPDATEINTARRAY	'3'
#define XPLCMD_DATAREFUPDATEFLOATARRAY	'4'
#define XPLCMD_DATAREFUPDATESTRING		'9'

#define XPLCMD_SENDREQUEST         'Q'

#define XPLCMD_COMMANDSTART         'i'
#define XPLCMD_COMMANDEND           'j'
#define XPLCMD_COMMANDTRIGGER       'k'    //  command handle, number of triggers
#define XPLCMD_SENDVERSION          'v'     // get current build version from arduino device


#define XPL_EXITING					'X'		// xplane is closing

#define XPLTYPE_XPLPRO 1


#define XPL_READ		1
#define XPL_WRITE       2
#define XPL_READWRITE	3

#define XPL_DATATYPE_INT    1
#define XPL_DATATYPE_FLOAT  2
#define XPL_DATATYPE_STRING 3

