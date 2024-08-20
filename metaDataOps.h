//protection from multiple compiling
#ifndef META_DATA_OPS_H
#define META_DATA_OPS_H

//header files
#include "datatypes.h"
#include "StandardConstants.h"
#include "stringUtils.h"
#include <stdlib.h>

//function prototypes
OpCodeType *addNode( OpCodeType *localPtr, OpCodeType *newNode );
OpCodeType *clearMetaDataList( OpCodeType *localPtr );
void displayMetaData( const OpCodeType *localPtr );
int getCommand( char *cmd, const char *inputStr, int index );
bool getMetaData( const char *fileName, OpCodeType **opCodeDataHead, 
                                                       char *endStateMsg );
int getNumberArg( int *number, const char *inputStr, int index);
OpCodeMessages getOpCommand( FILE *filePtr, OpCodeType *inData );
int getStringArg(char *strArg, const char *inputStr, int index);
bool isDigit( char testChar );
int updateStartCount( int count, const char *opString );
int updateEndCount( int count, const char *opString );
bool verifyFirstStringArg( const char *strArg );
bool verifyValidCommand( char *testCmd );


#endif