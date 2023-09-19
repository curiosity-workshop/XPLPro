#pragma once



void statusWindowCreate(void);
int statusWindowActive(void);

void statusDrawWindowCallback(	XPLMWindowID,  void*);
int statusHandleMouseClickCallback(XPLMWindowID, int , int , XPLMMouseStatus , void*);
void statusHandleKeyCallback(
	XPLMWindowID         inWindowID,
	char                 inKey,
	XPLMKeyFlags         inFlags,
	char                 inVirtualKey,
	void* inRefcon,
	int                  losingFocus);