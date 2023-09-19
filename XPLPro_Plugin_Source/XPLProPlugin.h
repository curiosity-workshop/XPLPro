
#pragma once

#define XPL_VERSION 2302091
#define XPLM200



float	MyFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

static void	MyMenuHandlerCallback(
    void* inMenuRef,
    void* inItemRef);

int    ResetCommandHandler(XPLMCommandRef        inCommand,
	XPLMCommandPhase      inPhase,
	void* inRefcon);
