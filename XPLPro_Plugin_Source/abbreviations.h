#pragma once
#include <stdio.h>

class abbreviations
{

public:
	abbreviations();
	~abbreviations();
	int begin(void);
	int convertString(char* inString);
	
private:

	FILE* _abbFile;

};

