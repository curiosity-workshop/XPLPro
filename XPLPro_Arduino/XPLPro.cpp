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
    dtostrf(value, 0, XPL_FLOATPRECISION, tBuf);
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
    dtostrf(value, 0, XPL_FLOATPRECISION, tBuf);
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

void XPLPro::_processPacket()
{
   
    // check whether we have a valid frame
    if (_receiveBuffer[0] != XPL_PACKETHEADER)
    {
        return;
    }
    // branch on receiverd command
    switch (_receiveBuffer[1])
    {
    // plane unloaded or XP exiting
    case XPL_EXITING:
        _connectionStatus = false;
        _xplStopFunction();
        break;

    // register device
    case XPLCMD_SENDNAME:
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
   
        // Todo:  Still need to deal with inbound strings
        // Todo:  implement type within struct
        

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

void XPLPro::_seekParm(const char* buf, int paramIdx, int& start, int &end)
{
    int pos = 0;
    // search for the selected parameter
    for (int i = 1; i < paramIdx; i++) {
        while (buf[pos] != ',' && buf[pos] != 0) pos++;
        if(buf[pos] != 0)pos++; // skip separator
    }
    // parameter starts here
    start = pos;
    // search for end of parameter
    while (buf[pos] != ',' && buf[pos] != 0 && buf[pos] != XPL_PACKETTRAILER) pos++;
    end = pos;
}


int XPLPro::_parseString(char *outBuffer, char *inBuffer, int parameter, int maxSize)
{
    // todo:  Confirm 0 length strings ("") dont cause issues
    int cBeg;
    int pos = 0;
    int len;

    for (int i = 1; i < parameter; i++) {
        while (inBuffer[pos] != ',' && inBuffer[pos] != 0) pos++;
        if(inBuffer[pos] != 0) pos++;
    }

    while (inBuffer[pos] != '\"' && inBuffer[pos] != 0) pos++;
    if(inBuffer[pos] != 0) ++pos;
    cBeg = pos;

    while (inBuffer[pos] != '\"' && inBuffer[pos] != 0) pos++;

    len = pos - cBeg;
    if (len > maxSize) len = maxSize;

    strncpy(outBuffer, (char *)&inBuffer[cBeg], len);
    outBuffer[len] = 0;
    // fprintf(errlog, "_parseString, pos: %i, cBeg: %i, deviceName: %s\n", pos, cBeg, target);
    return 0;
}

int XPLPro::_parseInt(int *outTarget, char *inBuffer, int parameter)
{
    int cSt, cEnd;
    _seekParm(inBuffer, parameter, cSt, cEnd);
    // temporarily make parameter null terminated
    char holdChar = inBuffer[cEnd];
    inBuffer[cEnd] = 0;
    // get integer value from string
    *outTarget = atoi((char *)&inBuffer[cSt]);
    // restore buffer
    inBuffer[cEnd] = holdChar;
    return 0;
}

int XPLPro::_parseInt(long *outTarget, char *inBuffer, int parameter)
{
    int cSt, cEnd;
    _seekParm(inBuffer, parameter, cSt, cEnd);
    char holdChar = inBuffer[cEnd];
    inBuffer[cEnd] = 0;
    *outTarget = atoi((char *)&inBuffer[cSt]);
    inBuffer[cEnd] = holdChar;
    return 0;
}

int XPLPro::_parseFloat(float *outTarget, char *inBuffer, int parameter)
{
    int cSt, cEnd;
    _seekParm(inBuffer, parameter, cSt, cEnd);
    char holdChar = inBuffer[cEnd];
    inBuffer[cEnd] = 0;
    *outTarget = atof((char *)&inBuffer[cSt]);
    inBuffer[cEnd] = holdChar;
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
