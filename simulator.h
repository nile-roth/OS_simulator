#ifndef SIM_OPS_H
#define SIM_OPS_H

//header files
#include "datatypes.h"
#include "stringUtils.h"
#include "simtimer.h"
#include "metaDataOps.h"
#include "StandardConstants.h"
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>

//function prototypes

void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr );

void addStringNode( stringNode *stringHead, char appendStr[MAX_STR_LEN]);

Memory *addMemNode( Memory *memHeadPtr, int baseReq, int offsetReq, int pid);

Thread *addThreadNode( Thread *threadHeadPtr, double start, double end, int pid);

PCBDataType *createPCBs( OpCodeType *metaDataHeadPtr, 
                                                  ConfigDataType *configPtr);

void compressMem( Memory *memHeadPtr);

void display(PrintCodes printCode, char *timeStr, PCBDataType *PCBPtr, 
                       ConfigDataType *configPtr, stringNode *stringHeadPtr, 
                                        Memory *progMem, processState newState);

void displayAccordingly( char Str[MAX_STR_LEN], ConfigDataType *configPtr, 
                                                    stringNode *stringHead );

void displayMem( Memory *memHeadPtr, ConfigDataType *configPtr, 
                                                  stringNode *stringHeadPtr );

void freeStringList(stringNode *head);

void freeMemList(Memory *head);

void freePCBList(PCBDataType *head);

void freeThreadList(Thread *head);

PCBDataType *getNextProcess( ConfigDataType *configPtr, 
                                                    PCBDataType *PCBHeadPtr);                   

PCBDataType *getProcessFromThread( Thread *threadHead, PCBDataType *PCBHead );

Thread *interruptReady (Thread* threadHeadPtr, double currentTime);

bool processExistsInMem( int pid, Memory *memHeadPtr );

Memory *processMem( Memory *memHeadPtr, int pid, int baserequest, 
                                      int offsetRequest, MemRequests reqType, 
                                                  ConfigDataType *configPtr );

PCBDataType *resolveProcess( PCBDataType *PCBHeadPtr, int pid, double completedTime );

Thread *removeThread( Thread *head, int pid );

#endif
