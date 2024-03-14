// XPLPro.cpp
// Created by Curiosity Workshop, Michael Gerlicher, 2023.
#include "XPLPro.h"

XPLPro::XPLPro(Stream *device)
{
    _streamPtr = device;
    _streamPtr->setTimeout(XPL_RX_TIMEOUT);
}

void XPLPro::begin(const char *devicename, void (*initFunction)(void), void (*stopFunction)(void), void (*inboundHandler)(inStruct *))
{
    _deviceName = (char *)devicename;
    _connectionStatus = 0;
    _receiveBuffer[0] = 0;
    _registerFlag = 0;
    _xplInitFunction = initFunction;
    _xplStopFunction = stopFunction;
    _xplInboundHandler = inboundHandler;
}

int XPLPro::xloop(void)
{
    // handle incoming serial data
    _processSerial();
    // when device is registered, perform handle registrations
    if (_registerFlag)
    {
        _xplInitFunction();
        _registerFlag = 0;
    }
    // return status of connection
    return _connectionStatus;
}

// TODO: is a return value necessary? These could also be void like for the datarefs
int XPLPro::commandTrigger(int commandHandle, int triggerCount)
{
    if (commandHandle < 0)
    {
        return XPL_HANDLE_INVALID;
    }
    sprintf(_sendBuffer, "%c%c,%i,%i%c", XPL_PACKETHEADER, XPLCMD_COMMANDTRIGGER, commandHandle, triggerCount, XPL_PACKETTRAILER);
    _transmitPacket();
    return 0;
}

int XPLPro::commandStart(int commandHandle)
{
    if (commandHandle < 0)
    {
        return XPL_HANDLE_INVALID;
    }
    _sendPacketVoid(XPLCMD_COMMANDSTART, commandHandle);
    return 0;
}

int XPLPro::commandEnd(int commandHandle)
{
    if (commandHandle < 0)
    {
        return XPL_HANDLE_INVALID;
    }
    _sendPacketVoid(XPLCMD_COMMANDEND, commandHandle);
    return 0;
}

int XPLPro::connectionStatus()
{
    return _connectionStatus;
}

int XPLPro::sendDebugMessage(const char *msg)
{
    _sendPacketString(XPLCMD_PRINTDEBUG, msg);
    return 1;
}

int XPLPro::sendSpeakMessage(const char *msg)
{
    _sendPacketString(XPLCMD_SPEAK, msg);
    return 1;
}

void XPLPro::flightLoopPause(void)              // experimental!  Do not use!
{
    _sendPacketVoid(XPLCMD_FLIGHTLOOPPAUSE, 0);

}

void XPLPro::flightLoopResume(void)
{
    _sendPacketVoid(XPLCMD_FLIGHTLOOPRESUME, 0);
}

// these could be done better:

void XPLPro::datarefWrite(int handle, int value)
{
    if (handle < 0)
    {
        return;
    }
    sprintf(_sendBuffer, "%c%c,%i,%i%c", XPL_PACKETHEADER, XPLCMD_DATAREFUPDATEINT, handle, value, XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::datarefWrite(int handle, int value, int arrayElement)
{
    if (handle < 0)
    {
        return;
    }
    sprintf(_sendBuffer, "%c%c,%i,%i,%i%c", XPL_PACKETHEADER, XPLCMD_DATAREFUPDATEINTARRAY, handle, value, arrayElement, XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::datarefWrite(int handle, long value)
{
    if (handle < 0)
    {
        return;
    }
    sprintf(_sendBuffer, "%c%c,%i,%ld%c", XPL_PACKETHEADER, XPLCMD_DATAREFUPDATEINT, handle, value, XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::datarefWrite(int handle, long value, int arrayElement)
{
    if (handle < 0)
    {
        return;
    }
    sprintf(_sendBuffer, "%c%c,%i,%ld,%i%c", XPL_PACKETHEADER, XPLCMD_DATAREFUPDATEINTARRAY, handle, value, arrayElement, XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::datarefWrite(int handle, float value)
{
    if (handle < 0)
    {
        return;
    }
    char tBuf[20]; // todo:  rewrite to eliminate this buffer.  Write directly to _sendBuffer
   
    Xdtostrf(value, 0, XPL_FLOATPRECISION, tBuf);
    sprintf(_sendBuffer, "%c%c,%i,%s%c",
            XPL_PACKETHEADER,
            XPLCMD_DATAREFUPDATEFLOAT,
            handle,
            tBuf,
            XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::datarefWrite(int handle, float value, int arrayElement)
{
    if (handle < 0)
    {
        return;
    }
    char tBuf[20]; // todo:  rewrite to eliminate this buffer.  Write directly to _sendBuffer
    Xdtostrf(value, 0, XPL_FLOATPRECISION, tBuf);
    sprintf(_sendBuffer, "%c%c,%i,%s,%i%c",
            XPL_PACKETHEADER,
            XPLCMD_DATAREFUPDATEFLOATARRAY,
            handle,
            tBuf,
            arrayElement,
            XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::_sendname()
{
    // register device on request only when we have a valid name
    if (_deviceName != NULL)
    {
        _sendPacketString(XPLRESPONSE_NAME, _deviceName);
    }
}

void XPLPro::_sendVersion()
{
    // register device on request only when we have a valid name
    if (_deviceName != NULL)
    {
        char version[25];
        sprintf(version, "%s %s", __DATE__, __TIME__);
        _sendPacketString(XPLRESPONSE_VERSION, version);
    }
}

void XPLPro::sendResetRequest()
{
    // request a reset only when we have a valid name
    if (_deviceName != NULL)
    {
        _sendPacketVoid(XPLCMD_RESET, 0);
    }
}

void XPLPro::_processSerial()
{
    // read until package header found or buffer empty
    while (_streamPtr->available() && _receiveBuffer[0] != XPL_PACKETHEADER)
    {
        _receiveBuffer[0] = (char)_streamPtr->read();
    }
    // return when buffer empty and header not found
    if (_receiveBuffer[0] != XPL_PACKETHEADER)
    {
        return;
    }
    // read rest of package until trailer
    _receiveBufferBytesReceived = _streamPtr->readBytesUntil(XPL_PACKETTRAILER, (char *)&_receiveBuffer[1], XPLMAX_PACKETSIZE_RECEIVE - 1);
    // if no further chars available, delete package
    if (_receiveBufferBytesReceived == 0)
    {
        _receiveBuffer[0] = 0;
        return;
    }
    // add package trailer and zero byte to frame
    _receiveBuffer[++_receiveBufferBytesReceived] = XPL_PACKETTRAILER;
    _receiveBuffer[++_receiveBufferBytesReceived] = 0; // old habits die hard.
    // at this point we should have a valid frame
    _processPacket();
}

int XPLPro::_receiveNSerial(int inSize)
{
    if (inSize > XPLMAX_PACKETSIZE_RECEIVE) inSize = XPLMAX_PACKETSIZE_RECEIVE;

    return Serial.readBytes(_receiveBuffer, inSize);
}

void XPLPro::_processPacket()
{
   
    // check whether we have a valid frame
    if (_receiveBuffer[0] != XPL_PACKETHEADER)
    {
        return;
    }
    // branch on received command
    switch (_receiveBuffer[1])
    {
    // plane unloaded or XP exiting
    case XPL_EXITING:
        _connectionStatus = false;
        _xplStopFunction();
        break;

    // register device
    case XPLCMD_SENDNAME:
        _sendVersion();
        _sendname();
        _connectionStatus = true; // not considered active till you know my name
        _registerFlag = 0;
        
        break;

    // plugin is ready for registrations.
    case XPLCMD_SENDREQUEST:
        _registerFlag = 1; // use a flag to signal registration so recursion doesn't occur
        break;

    // get handle from response to registered dataref
    case XPLRESPONSE_DATAREF:
        _parseInt(&_handleAssignment, _receiveBuffer, 2);
        break;

    // get handle from response to registered command
    case XPLRESPONSE_COMMAND:
        _parseInt(&_handleAssignment, _receiveBuffer, 2);
        break;

    // int dataref received
    case XPLCMD_DATAREFUPDATEINT:
        _parseInt(&_inData.handle, _receiveBuffer, 2);
        _parseInt(&_inData.inLong, _receiveBuffer, 3);
        _inData.inFloat = 0;
        _inData.element = 0;
        _xplInboundHandler(&_inData);
        break;

    // int array dataref received
    case XPLCMD_DATAREFUPDATEINTARRAY:
        _parseInt(&_inData.handle, _receiveBuffer, 2);
        _parseInt(&_inData.inLong, _receiveBuffer, 3);
        _parseInt(&_inData.element, _receiveBuffer, 4);
        _inData.inFloat = 0;
        _xplInboundHandler(&_inData);
        break;

    // float dataref received
    case XPLCMD_DATAREFUPDATEFLOAT:
        _parseInt(&_inData.handle, _receiveBuffer, 2);
        _parseFloat(&_inData.inFloat, _receiveBuffer, 3);
        _inData.inLong = 0;
        _inData.element = 0;
        _xplInboundHandler(&_inData);
        break;

    // float array dataref received
    case XPLCMD_DATAREFUPDATEFLOATARRAY:
        _parseInt(&_inData.handle, _receiveBuffer, 2);
        _parseFloat(&_inData.inFloat, _receiveBuffer, 3);
        _parseInt(&_inData.element, _receiveBuffer, 4);
        _inData.inLong = 0;
        _xplInboundHandler(&_inData);
        break;
   
    case XPLCMD_DATAREFUPDATESTRING:
        _parseInt(&_inData.handle, _receiveBuffer, 2);
        _parseInt(&_inData.strLength, _receiveBuffer, 3);
        _receiveNSerial(_inData.strLength);
        _inData.inStr = _receiveBuffer;
        
        _xplInboundHandler(&_inData);
        break;
       

    // obsolete?            reserve for the time being...
  //  case XPLREQUEST_REFRESH:
  //      break;

    default:
        break;
    }
    // empty receive buffer
    _receiveBuffer[0] = 0;
}

void XPLPro::_sendPacketVoid(int command, int handle) // just a command with a handle
{
    // check for valid handle
    if (handle < 0)
    {
        return;
    }
    sprintf(_sendBuffer, "%c%c,%i%c", XPL_PACKETHEADER, command, handle, XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::_sendPacketString(int command, const char *str) // for a string
{
    sprintf(_sendBuffer, "%c%c,\"%s\"%c", XPL_PACKETHEADER, command, str, XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::_transmitPacket(void)
{
    _streamPtr->write(_sendBuffer);
    if (strlen(_sendBuffer) == 64)
    {
        // apparently a bug in arduino with some boards when we transmit exactly 64 bytes. That took a while to track down...
        _streamPtr->print(" ");
    }
}

int XPLPro::_parseString(char *outBuffer, char *inBuffer, int parameter, int maxSize)// todo:  Confirm 0 length strings ("") dont cause issues
{
    int cBeg;
    int pos = 0;
    int len;

    for (int i = 1; i < parameter; i++)
    {
        while (inBuffer[pos] != ',' && inBuffer[pos] != 0)
        {
            pos++;
        }
        pos++;
    }

    while (inBuffer[pos] != '\"' && inBuffer[pos] != 0)
    {
        pos++;
    }
    cBeg = ++pos;

    while (inBuffer[pos] != '\"' && inBuffer[pos] != 0)
    {
        pos++;
    }
    len = pos - cBeg;
    if (len > maxSize)
    {
        len = maxSize;
    }
    strncpy(outBuffer, (char *)&inBuffer[cBeg], len);
    outBuffer[len] = 0;
    // fprintf(errlog, "_parseString, pos: %i, cBeg: %i, deviceName: %s\n", pos, cBeg, target);
    return 0;
}

int XPLPro::_parseInt(int *outTarget, char *inBuffer, int parameter)
{
    int cBeg;
    int pos = 0;
    // search for the selected parameter
    for (int i = 1; i < parameter; i++)
    {
        while (inBuffer[pos] != ',' && inBuffer[pos] != 0)
        {
            pos++;
        }
        pos++;
    }
    // parameter starts here
    cBeg = pos;
    // search for end of parameter
    while (inBuffer[pos] != ',' && inBuffer[pos] != 0 && inBuffer[pos] != XPL_PACKETTRAILER)
    {
        pos++;
    }
    // temporarily make parameter null terminated
    char holdChar = inBuffer[pos];
    inBuffer[pos] = 0;
    // get integer value from string
    *outTarget = atoi((char *)&inBuffer[cBeg]);
    // restore buffer
    inBuffer[pos] = holdChar;
    return 0;
}

int XPLPro::_parseInt(long *outTarget, char *inBuffer, int parameter)
{
    int cBeg;
    int pos = 0;
    for (int i = 1; i < parameter; i++)
    {
        while (inBuffer[pos] != ',' && inBuffer[pos] != 0)
        {
            pos++;
        }
        pos++;
    }
    cBeg = pos;
    while (inBuffer[pos] != ',' && inBuffer[pos] != 0 && inBuffer[pos] != XPL_PACKETTRAILER)
    {
        pos++;
    }
    char holdChar = inBuffer[pos];
    inBuffer[pos] = 0;
    *outTarget = atol((char *)&inBuffer[cBeg]);
    inBuffer[pos] = holdChar;
    return 0;
}

int XPLPro::_parseFloat(float *outTarget, char *inBuffer, int parameter)
{
    int cBeg;
    int pos = 0;
    for (int i = 1; i < parameter; i++)
    {
        while (inBuffer[pos] != ',' && inBuffer[pos] != 0)
        {
            pos++;
        }
        pos++;
    }
    cBeg = pos;
    while (inBuffer[pos] != ',' && inBuffer[pos] != 0 && inBuffer[pos] != XPL_PACKETTRAILER)
    {
        pos++;
    }
    char holdChar = inBuffer[pos];
    inBuffer[pos] = 0;
    *outTarget = atof((char *)&inBuffer[cBeg]);
    inBuffer[pos] = holdChar;
    return 0;
}

int XPLPro::registerDataRef(XPString_t *datarefName)
{
    long int startTime;

    // registration only allowed in callback (TODO: is this limitation really necessary?)
    if (!_registerFlag)
    {
        return XPL_HANDLE_INVALID;
    }
#if XPL_USE_PROGMEM
    sprintf(_sendBuffer, "%c%c,\"%S\"%c", XPL_PACKETHEADER, XPLREQUEST_REGISTERDATAREF, (wchar_t *)datarefName, XPL_PACKETTRAILER);
#else
    sprintf(_sendBuffer, "%c%c,\"%s\"%c", XPL_PACKETHEADER, XPLREQUEST_REGISTERDATAREF, (char *)datarefName, XPL_PACKETTRAILER);
#endif
    _transmitPacket();

    _handleAssignment = XPL_HANDLE_INVALID;
    startTime = millis(); // for timeout function

    while (millis() - startTime < XPL_RESPONSE_TIMEOUT && _handleAssignment < 0)
        _processSerial();

    return _handleAssignment;
}

int XPLPro::registerCommand(XPString_t *commandName)
{
    long int startTime = millis(); // for timeout function
#if XPL_USE_PROGMEM
    sprintf(_sendBuffer, "%c%c,\"%S\"%c", XPL_PACKETHEADER, XPLREQUEST_REGISTERCOMMAND, (wchar_t *)commandName, XPL_PACKETTRAILER);
#else
    sprintf(_sendBuffer, "%c%c,\"%s\"%c", XPL_PACKETHEADER, XPLREQUEST_REGISTERCOMMAND, (char *)commandName, XPL_PACKETTRAILER);
#endif
    _transmitPacket();
    _handleAssignment = XPL_HANDLE_INVALID;
    while (millis() - startTime < XPL_RESPONSE_TIMEOUT && _handleAssignment < 0)
    {
        _processSerial();
    }
    return _handleAssignment;
}

void XPLPro::requestUpdates(int handle, int rate, float precision)
{
    char tBuf[20]; // todo:  rewrite to eliminate this buffer.  Write directly to _sendBuffer?
    dtostrf(precision, 0, XPL_FLOATPRECISION, tBuf);
    sprintf(_sendBuffer, "%c%c,%i,%i,%s%c",
            XPL_PACKETHEADER,
            XPLREQUEST_UPDATES,
            handle,
            rate,
            tBuf,
            XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::requestUpdates(int handle, int rate, float precision, int element)
{
    char tBuf[20]; // todo:  rewrite to eliminate this buffer.  Write directly to _sendBuffer?
    dtostrf(precision, 0, XPL_FLOATPRECISION, tBuf);
    sprintf(_sendBuffer, "%c%c,%i,%i,%s,%i%c",
            XPL_PACKETHEADER,
            XPLREQUEST_UPDATESARRAY,
            handle,
            rate,
            tBuf,
            element,
            XPL_PACKETTRAILER);
    _transmitPacket();
}

void XPLPro::setScaling(int handle, int inLow, int inHigh, int outLow, int outHigh)
{
    sprintf(_sendBuffer, "%c%c,%i,%i,%i,%i,%i%c",
            XPL_PACKETHEADER,
            XPLREQUEST_SCALING,
            handle,
            inLow,
            inHigh,
            outLow,
            outHigh,
            XPL_PACKETTRAILER);
    _transmitPacket();
}
// Re-creation of dtostrf for non-AVR boards
char *XPLPro::Xdtostrf(double val, signed char width, unsigned char prec, char* sout) 
{
#ifdef __AVR_ARCH__
    return dtostrf(val, width, prec, sout);
#else
    char fmt[20];
    sprintf(fmt, "%%%d.%df", width, prec);
    sprintf(sout, fmt, val);
    return sout;
#endif

}

