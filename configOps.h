//protection from multiple compiling
#ifndef CONFIG_OPS_H
#define CONFIG_OPS_H

//header files
#include "StandardConstants.h"
#include "stringUtils.h"
#include "datatypes.h"

//function prototypes
bool getConfigData( const char *fileName, ConfigDataType **configData, 
                                                         char *endStateMsg );
ConfigDataType *clearConfigData( ConfigDataType *configData );
void displayConfigData( ConfigDataType *configData );
ConfigDataType *clearConfigData( ConfigDataType *configData );
void configCodeToString( int code, char *outString);
ConfigDataCodes getCpuSchedCode( const char *lowerCaseCodeStr );
ConfigCodeMessages getDataLineCode(const char *dataBuffer);
ConfigDataCodes getLogToCode( const char *lowerCaseLogToStr );
void stripTrailingSpaces( char *str );
bool valueInRange( int lineCode, int intVal, double doubleVal, 
                                              const char *lowerCaseStrVal);

#endif