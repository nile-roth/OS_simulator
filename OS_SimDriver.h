//protection from multiple compiling
#ifndef OS_SIM_DRIVER_H
#define OS_SIM_DRIVER_H

//header files
#include <stdio.h>
#include "datatypes.h"
#include "stringUtils.h"
#include "configOps.h"
#include "metaDataOps.h"
#include "simulator.h"

//function prototypes
void clearCmdLineStruct( CmdLineData *clDataPtr );
bool processCmdLine( int numArgs, char **strVector, CmdLineData *clDataPtr );
void showCmdLineFormat();

#endif
