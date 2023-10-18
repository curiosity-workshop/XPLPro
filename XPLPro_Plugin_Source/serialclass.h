#pragma once

#define XPL_MAX_SERIALPORTS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

class serialClass
{
private:
    //Serial comm handler
    HANDLE hSerial = NULL;
    //Connection status
    bool connected = false;
    //Get various information about the connection
    COMSTAT status;
    //Keep track of last error
    DWORD errors;

   
   

public:
    //Initialize Serial communication with the given COM port
    serialClass();
    //Close the connection
    ~serialClass();

    int begin(int portNumber);
    int shutDown(void);
    int findAvailablePort(void);


    //Read data in a buffer, if nbChar is greater than the
    //maximum number of bytes available, it will return only the
    //bytes available. The function return -1 when nothing could
    //be read, the number of bytes actually read.
    int readData(char* buffer, size_t nbChar);
    //Writes data from a buffer through the Serial connection
    //return true on success.
    bool writeData(const char* buffer, size_t nbChar);
    //Check if we are actually connected
    bool IsConnected(void);

    char   portName[20];					// port name
    int valid;
   
};

