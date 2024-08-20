
//protection from multiple compiling
#ifndef DATATYPES_H
#define DATATYPES_H

//header files
#include <stdio.h>
#include <stdbool.h>
#include "StandardConstants.h"

////////////////////////////////////////////enumerations
typedef enum { CMD_STR_LEN = 5, 
               IO_ARG_STR_LEN = 5, 
               STR_ARG_LEN = 15 } OpCodeArrayCapacity;

typedef enum { BAD_ARG_VAL = -1,
               NO_ACCESS_ERR,
               MD_FILE_ACCESS_ERR,
               MD_CORRUPT_DESCRIPTOR_ERR,
               OPCMD_ACCESS_ERR,
               CORRUPT_OPCMD_ERR,
               CORRUPT_OPCMD_ARG_ERR,
               UNBALANCED_START_END_ERR,
               COMPLETE_OPCMD_FOUND_MSG,
               LAST_OPCMD_FOUND_MSG } OpCodeMessages;

typedef enum { NEW,    
               READY,  
               RUNNING,
               BLOCKED,
               EXIT } processState;

typedef enum { CFG_FILE_ACCESS_ERR,
               CFG_CORRUPT_DESCRIPTION_ERR,
               CFG_DATA_OUT_OF_RANGE_ERR,
               CFG_CORRUPT_PROMPT_ERR,
               CFG_VERSION_CODE,
               CFG_MD_FILE_NAME_CODE,
               CFG_CPU_SCHED_CODE,
               CFG_QUANT_CYCLES_CODE,
               CFG_MEM_DISPLAY_CODE,
               CFG_MEM_AVAILABLE_CODE,
               CFG_PROC_CYCLES_CODE,
               CFG_IO_CYCLES_CODE,
               CFG_LOG_TO_CODE,
               CFG_LOG_FILE_NAME_CODE } ConfigCodeMessages;

typedef enum { CPU_SCHED_SJF_N_CODE,
               CPU_SCHED_SRTF_P_CODE,
               CPU_SCHED_FCFS_P_CODE,
               CPU_SCHED_RR_P_CODE,
               CPU_SCHED_FCFS_N_CODE,
               LOGTO_MONITOR_CODE,
               LOGTO_FILE_CODE,
               LOGTO_BOTH_CODE,
               NON_PREEMPTIVE_CODE,
               PREEMPTIVE_CODE } ConfigDataCodes;

typedef enum { SIM_START_CODE,
               CHANGE_STATE_CODE,
               PROCESS_SELECT_CODE,
               OP_START_CODE,
               OP_END_CODE,
               PROCESS_COMPLETE_CODE,
               MEM_REQ_CODE,
               MEM_INIT_CODE, 
               MEM_SUCCESS_CODE,
               MEM_FAIL_CODE,
               MEM_CLEAR_CODE,
               PROCESS_BLOCKED_CODE,
               PROCESS_INTERRUPT_CODE,
               CPU_IDLE_CODE,
               END_CPU_IDLE_CODE,
               QUANTUM_OUT_CODE,
               CPU_INTERRUPTED_CODE} PrintCodes;

typedef enum { OPEN_MEM,
               USED_MEM} MemUO;

typedef enum { INIT_REQ_TYPE,
              ACCESS_REQ_TYPE,
              ALLOCATE_REQ_TYPE,
              CLEAR_REQ_TYPE} MemRequests;
                             
typedef enum { MIN_NUM_ARGS = 3, 
               LAST_FOUR_LETTERS = 4 } PROGRAM_CONSTANTS;

enum TIMER_CTRL_CODES { ZERO_TIMER, LAP_TIMER, STOP_TIMER };

///////////////////////////////////////////datatype structs
typedef struct ConfigDataType 
    { 
    double version;
    char metaDataFileName[ MAX_STR_LEN ];
    ConfigDataCodes cpuSchedCode;   
    int quantumCycles;
    bool memDisplay;
    int memAvailable;
    int procCycleRate;
    int ioCycleRate;
    ConfigDataCodes logToCode;
    char logToFileName[ MAX_STR_LEN ];
    } ConfigDataType;

typedef struct OpCodeType 
    { 
    int pid;                        
    char command[MAX_STR_LEN];
    char inOutArg[MAX_STR_LEN];
    char strArg1[MAX_STR_LEN];
    int intArg2;
    int intArg3;
    double opEndTime;                 
    struct OpCodeType *nextNode;
    } OpCodeType;

typedef struct PCBDataType 
    { 
    processState state;
    int processID;
    OpCodeType *programCounter;
    int timeRemaining;
    struct PCBDataType *nextNode;
    } PCBDataType;


typedef struct CmdLineDataStruct
    {
    bool programRunFlag;
    bool configDisplayFlag;
    bool mdDisplayFlag;
    bool runSimFlag;
    char fileName[ STD_STR_LEN ];
    } CmdLineData;

typedef struct stringNodeStruct
    {
    char str[MAX_STR_LEN];
    struct stringNodeStruct *nextNode;
    } stringNode;

typedef struct MemoryStruct
    {
    int logicalLow;
    int logicalHigh;
    int physicalLow;
    int physicalHigh;
    MemUO UO;
    int pNum;
    struct MemoryStruct *nextNode;
    } Memory;

typedef struct ThreadStruct
    {
    double startTime;
    double endTime; 
    int pid;
    struct ThreadStruct *nextNode;
    } Thread;

#endif
