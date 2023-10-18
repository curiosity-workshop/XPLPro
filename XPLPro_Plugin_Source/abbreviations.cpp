
#include <string.h>
#include <ctype.h>

#include "XPLProCommon.h"
#include "abbreviations.h"


extern FILE* errlog;

abbreviations::abbreviations()
{
	
	
	
}



abbreviations::~abbreviations()
{
		
	if (_abbFile)
	{
		fclose(_abbFile);
		fprintf(errlog, "Abbreviation file closed.\n");
	}

	

}

int abbreviations::begin()
{
	
	_abbFile = fopen(CFG_ABBREVIATIONS_FILE, "r");
	if (_abbFile)
	{
		fprintf(errlog, "Abbreviation file opened successfully. \n");
		return 1;
	}
	
	fprintf(errlog, "** I was unable to open abbreviation.txt!  This is not critical.  \n");
	return -1;
}

int abbreviations::convertString(char* inString)
{
	char inBuffer[200];
	char *retBuffer;
	size_t inLength;
	size_t startPos;
	size_t endPos;

	if (!_abbFile) return -1;

	inLength = strlen(inString);

//fprintf(errlog, "searching abbreviation file for: %s\n", inString);
	rewind(_abbFile);
	do 
	{
		retBuffer = fgets(inBuffer, 200, _abbFile);
		if (!strncmp(inBuffer, inString, inLength))
		{	
			if (inBuffer[inLength] != ' ' && inBuffer[inLength] != '=') continue;

			fprintf(errlog, "\n  Abbreviations: converting: %s to: ", inString);

			startPos = inLength+1;
			while ((!isgraph(inBuffer[startPos]) || inBuffer[startPos] == '=') && startPos < 100)  startPos++;
			endPos = startPos;
			while (isgraph(inBuffer[endPos]) && endPos < 200)  endPos++;

//fprintf(errlog, "startpos: %i endpos: %i \n", startPos, endPos);

			strncpy(inString, &inBuffer[startPos], endPos - startPos);						// dirty but it works

			fprintf(errlog, "%s... ", inString);
			return 1;

		}
//fprintf(errlog, inBuffer);


	} while (retBuffer != NULL);

	return -1;
}