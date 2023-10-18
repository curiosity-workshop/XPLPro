#pragma once
#ifndef CONFIGCLASS_H_INCLUDED
#define CONFIGCLASS_H_INCLUDED

#include "libconfig.h"



extern FILE* errlog;

class Config
{
private:
    
    char _cfgFileName[512];

    config_t _cfg;
       
   // void createNewConfigFile(void);
   


public:
   
    Config(char *inFileName);
    ~Config();
    void saveFile(void);

    int getSerialLogFlag(void);
    void setSerialLogFlag(int);

    // stuff for components
    int getComponentCount(void);
    int Config::getComponentInfo(int element, int* type, const char** name, const char** board, int* pinCount, int* linkCount);
    void setComponentInfo(int element, int* type, const char* name, const char* board, const char* dataref, int arrayElement);
    int getComponentPinInfo(int componentElement, int pinElement, const char** pinName);
    
    int Config::getComponentLinkInfo(int componentIndex, int linkIndex, char* inName, const char** outData);
    int Config::getComponentLinkInfo(int componentIndex, int linkIndex, char* inName, int* outData);

    // local globals
    int _validConfig;
};

#endif 

