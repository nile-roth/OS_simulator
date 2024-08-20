//preprocessor directive
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

//header files
#include "StandardConstants.h"
#include <stdio.h>
#include <stdlib.h>

//prototypes
//function prototypes
int compareStr( const char *leftStr, const char *rightStr );
void concatStr( char *destStr, const char *sourceStr );
void copyStr( char *destStr, const char *sourceStr );
int findSubStr( const char *testStr, const char *searchSubStr );
bool getStrConstrained( FILE *inStream, bool clearLeadingNonPrintable, bool clearLeadingSpace, bool stopAtNonPrintable, char delimeter, char *capturedString);
int getStringLength( const char *testStr );
bool getStrToDelimiter( FILE *inStream, char delimeter, char *capturedStr );
bool getStrToLineEnd( FILE *inStream, char *capturedStr );
void getSubStr( char *destStr, const char *sourceStr, 
                int startIndex, int endIndex );
void setStrToLowerCase( char *destStr, const char *sourceStr );
char toLowerCase( char testChar );

#endif   //STRING_UTILS_H
