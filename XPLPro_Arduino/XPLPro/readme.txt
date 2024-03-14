Curiosity Workshop XPLPro Arduino Library - Library for serial interface to Xplane SDK.
  
  Created by Michael Gerlicher, Curiosity Workshop,  September 2020 - 2023.

  To report problems, download updates and examples, suggest enhancements or get technical support, please visit one of the following:

     discord:  	https://discord.gg/gzXetjEST4
     patreon: 	www.patreon.com/curiosityworkshop
     youtube:	https://www.youtube.com/channel/UCISdHdJIundC-OSVAEPzQIQ
     facebook:  https://www.facebook.com/curiosity.workshop42

  Please support this project in one of the following ways:

        Free:  like and subscribe to our youtube channel!
        Support us at patreon.com/curiosityworkshop
        
Known issues:
    
    Arduino Uno with I2c display seems to interfere with serial dialog between the arduino and Xplane.
    Incompatibility with some bluetooth serial port installations, try disabling them if you have issues.

    Remember to close your serial monitor if it is open, it will tie up the serial port and your device won't be found.

Updates:
  
    06 Mar 2024:

        -- Examples are now contained within the xplpro library folder to ease distribution and so they will show up in the arduino IDE examples under xplpro.

        -- Added XPLProMax72XX example sketch for connecting 72xx series LED displays.  

        -- Arduino library now sends build version, this is derived automatically from the date and time of compilation.  It will be utilized in future updates.
        
        -- Added XPLSwitches.h to the library, as well as an example.  This takes care of common uses of switches used to control xplane and extensively simplifies coding.

        -- Added XPLMuxIn4067.h to the library, as well as an example.  This handles connection to a 4067 multiplexer, use 5 arduino pins to receive 16 inputs.  
           Use another arduino pin to add another 16 inputs.
           

    03 Mar 2024:
        -- String functionality implemented
        -- Several functionality bugs squashed

    16 May 2023:  
        --  When sending data to the plugin long ints were converted to ints in some cases.  This has been resolved.
        
        -- Additional examples added.

 



        ToDo:

        -- Add scaling to outbound (from the plugin) data.  Inbound data currently supported.
        -- When reengaging devices datarefs are not updated
