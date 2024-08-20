//header files
#include "configOps.h"

/*
Name: getConfigData
Process: driver function for capturing configuration data from a config file
Function Input/params: file name (const char *)
Function Output/params: pointer to config data pointer (configDataType **),
                        end/result state message pointer (char *)
Function Output/returned: Boolean result of data access operation (bool)
Device Input: config data uploaded
Device Output: none
Dependencies: tbd
*/
bool getConfigData( const char *fileName, ConfigDataType **configData, 
                                                         char *endStateMsg )
{
    //initialize variables
    //set constant number of data lines
    const int NUM_DATA_LINES = 10;

    //create pointer for data input
    ConfigDataType *tempData;

    //declare other variables
    FILE *fileAccessPtr;
    char dataBuffer[ MAX_STR_LEN ], lowerCaseDataBuffer[ MAX_STR_LEN ];
    int intData, dataLineCode, lineCtr = 0;
    double doubleData;

    //set endstatemsg to success
      //function: copyStr
    char testMsg[] = "Configuration file upload success";
    copyStr( endStateMsg, testMsg );

    //initialize config data pointer in case of return error
    *configData = NULL;

    //open file
      //function: fopen
    fileAccessPtr = fopen( fileName, READ_ONLY_FLAG );

    //check for file open failure
    if (fileAccessPtr == NULL)
    {
        //set end state message to config file access error
          //function: copyStr
        char testMsg[] = "Configuration file access error";
        copyStr( endStateMsg, testMsg );

        //return file access error
        return false;
    }

    //get first line, check for failure
      //function:getStrToDelimiter, compareStr
    if (!getStrToDelimiter( fileAccessPtr, COLON, dataBuffer )
        || compareStr( dataBuffer,
                         "Start Simulator Configuration File") != STR_EQ )
    {
        //close file access
          //function: fclose
        fclose( fileAccessPtr );

        //set end state message to corrupt leader line error
          //function: copyStr
        copyStr( endStateMsg, "Corrupt configuration leader line error" );

        //return corrupt file data
        return false;
    }

    //create temp pointer to configuration data structure
      //function:malloc
    tempData = (ConfigDataType *) malloc( sizeof( ConfigDataType ));
  
    //loop to end of config data items
    while (lineCtr < NUM_DATA_LINES)
    {
        //get line leader, check for failure
          //function: getStrToDelimiter
        if (!getStrToDelimiter( fileAccessPtr, COLON, dataBuffer ))
        {
            //free temp struct memory
              //function: free
            free(tempData);

            //close file access
              //function: fclose
            fclose( fileAccessPtr);

            //set end state message to line capture failure
              //function:copyStr
            copyStr( endStateMsg, 
                            "Configuration data leader line capture error");

            //return incomplete file error
            return false;
        }

        //strip trailing spaces of config leader lines
          //function: stripTrailingSpaces
        stripTrailingSpaces( dataBuffer );

        //find correct data line code number from string
          //function: getDataLineCode
        dataLineCode = getDataLineCode( dataBuffer );

        //check for data line found
        if ( dataLineCode != CFG_CORRUPT_PROMPT_ERR )
        {

            //check for version number
            if ( dataLineCode == CFG_VERSION_CODE )
            {
                //get version number
                  //function: fscanf
                fscanf( fileAccessPtr, "%lf", &doubleData );
            }

            //otherwise check for metadata, filenames, CPU scheduling,
            //or member display setting
            else if (dataLineCode == CFG_MD_FILE_NAME_CODE
                     || dataLineCode == CFG_LOG_FILE_NAME_CODE
                     || dataLineCode == CFG_CPU_SCHED_CODE
                     || dataLineCode == CFG_LOG_TO_CODE
                     || dataLineCode == CFG_MEM_DISPLAY_CODE)
            {

                //get string input
                  //function:fscanf
                fscanf( fileAccessPtr, "%s", dataBuffer);

                //set string to lower case for testing in ValueinRange
                  //function: setStrToLowerCase
                setStrToLowerCase( lowerCaseDataBuffer, dataBuffer);
            }

            //otherwise assume integer data
            else
            {
                //get integer input
                  //function: fscanf
                fscanf( fileAccessPtr, "%d", &intData );
            }

            //check for data value in range
              //function: valueInRange
            if (valueInRange( dataLineCode, intData, doubleData,
                                                       lowerCaseDataBuffer))
            {
                //assign to data pointer depending on config item 
                //(all possibilities)
                  //functions: copyStr, getCpuSchedCode, compareStr, 
                  //getLogToCode
                switch( dataLineCode )
                {
                    case CFG_VERSION_CODE:
                        tempData->version = doubleData;
                        break;

                    case CFG_MD_FILE_NAME_CODE:
                        copyStr( tempData->metaDataFileName, dataBuffer );
                        break;

                    case CFG_CPU_SCHED_CODE:
                        tempData->cpuSchedCode = 
                                        getCpuSchedCode( lowerCaseDataBuffer );
                        break;

                    case CFG_QUANT_CYCLES_CODE:
                        tempData->quantumCycles = intData;
                        break;

                    case CFG_MEM_DISPLAY_CODE:
                        tempData->memDisplay =
                                 compareStr( lowerCaseDataBuffer, "on" ) == 0;
                        break;

                    case CFG_MEM_AVAILABLE_CODE:
                        tempData->memAvailable = intData;
                        break;

                    case CFG_PROC_CYCLES_CODE:
                        tempData->procCycleRate = intData;
                        break;

                    case CFG_IO_CYCLES_CODE:
                        tempData->ioCycleRate = intData;
                        break;

                    case CFG_LOG_TO_CODE:
                        tempData->logToCode = 
                                            getLogToCode( lowerCaseDataBuffer );
                        break;

                    case CFG_LOG_FILE_NAME_CODE:
                        copyStr( tempData->logToFileName, dataBuffer );
                        break;
                }
            }

            //otherwise assume data value not in range
            else
            {
                //free temp struct memory
                  //function: free
                free( tempData );

                //close file access
                  //function: fclose
                fclose( fileAccessPtr );

                //set end state message to config data out of range 
                  //function: copyStr
                copyStr( endStateMsg, "configuration item out of range" );

                //return data out of range
                return false;
            }
        }

        //otherwise assume data line not found
        else
        {
            //free temp struct memory
              //function: free
            free( tempData );

            //close file access
              //function: fclose
            fclose( fileAccessPtr );

            //set end state message to config corrupt prompt error
              //function: copyStr
            copyStr (endStateMsg, "Corrupted configuration prompt");

            //return corrupt config file code
            return false;
        }

        //increment line counter
        lineCtr++;

        //end master loop
    }

    //acquire end of sim config string
      //function: getStrToDelimiter, compareStr
    if ( !getStrToDelimiter( fileAccessPtr, PERIOD, dataBuffer )
       || compareStr( dataBuffer, "End Simulator Configuration File" ) 
                                                                != STR_EQ)
    {
        //free temp struct memory
          //function: free
        free( tempData );

        //close file access
          //function: fclose
        fclose( fileAccessPtr );

        //set end state message to config corrupt config end line
          //function: copyStr
        copyStr( endStateMsg, "Configuration end line capture error" );

        //return corrupt file data
        return false;
    }

    //test for file only output so memory diagnosted not displayed
    tempData->memDisplay = tempData->memDisplay 
                                  && tempData->logToCode != LOGTO_FILE_CODE;

    //assign temp ptr to parameter configdata (return pointer)
    *configData = tempData;

    //close file access
      //function: fclose
    fclose( fileAccessPtr );

    //return no error
    return true;  
}

/*
Name: displayConfigData
Process: screen display of all config data
Function Input/params: pointer to config data strcuture (ConfigDataType *)
Function Output/params: none
Function Output/returned: none
Device Input: none
Device Output: display of config data
Dependencies: printf
*/
void displayConfigData( ConfigDataType *configData )
{
    //initialize function variables
    char displayString[ STD_STR_LEN ];

    //print lines of display for all member values
      //function: printf, codeToString
    printf("Config File Display\n");
    printf("-------------------\n");
    printf("Version                : %3.2f\n", configData->version);
    printf("Program file name      : %s\n", configData->metaDataFileName);
    configCodeToString( configData->cpuSchedCode, displayString);
    printf("CPU schedule selection : %s\n", displayString);
    printf("Quantum time           : %d\n", configData->quantumCycles);
    printf("Memory Display         : ");
    if ( configData->memDisplay )
    {
        printf("On\n");
    }
    else
    {
        printf("Off\n");
    }
    printf("Memory Available       : %d\n", configData->memAvailable);
    printf("Process cycle rate     : %d\n", configData->procCycleRate);
    printf("I/O cycle rate         : %d\n", configData->ioCycleRate);
    configCodeToString( configData->logToCode, displayString);
    printf("Log to selection       : %s\n", displayString);
    printf("Log file name          : %s\n", configData->logToFileName);
}



/*
Name: clearConfigData
Process: frees dynamically allocated config data structure
         if it has not already been freed
Function Input/params: pointer to config data strcuture (ConfigDataType *)
Function Output/params: none
Function Output/returned: NULL (ConfigDataType *)
Device Input: none
Device Output: none
Dependencies: tbd
*/
ConfigDataType *clearConfigData( ConfigDataType *configData )
{
    //check config data not null
    if (configData != NULL)
    {
        //free config data ptr
          //function: free
        free(configData);

        //set config ptr to null
        configData = NULL;
    }

    return NULL;
}

/*
Name: configCodeToString
Process: utility function converts config code numbers to string 
Function Input/params: config code (int)
Function Output/params: resulting output string (char *)
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: copyStr
*/
void configCodeToString( int code, char *outString)
{
    //define array with eight items, and short (10) lengths 
    char displayStrings[8][10]= { "SJF-N", "SRTF-P", "FCFS-P",
                                  "RR-P", "FCFS-N", "Monitor",
                                  "File", "Both" };

    //copy string to return param
      //function: copyStr
    copyStr( outString, displayStrings[code] );
}

/*
Name: getCpuSchedCode
Process: converts cpu schedule string to code (all scheduling possibiliets)
Function Input/params: lower case code string (const char *)
Function Output/params: none
Function Output/returned: cpu schedule code (ConfigDataCodes)
Device Input: none
Device Output: none
Dependencies: compareStr
*/
ConfigDataCodes getCpuSchedCode( const char *lowerCaseCodeStr )
{
    //initialize function variables

    //set default return to FCFS-N
    ConfigDataCodes returnVal = CPU_SCHED_FCFS_N_CODE;

    //check for SJF-N
      //function: compareStr
    if ( compareStr( lowerCaseCodeStr, "sjf-n" ) == STR_EQ)
    {
        //set SJF-N code
        returnVal = CPU_SCHED_SJF_N_CODE;
    }

    //check for SRTF-P
      //function: compareStr
    if ( compareStr( lowerCaseCodeStr, "srtf-p" ) == STR_EQ)
    {
        //set SRTF-P code
        returnVal = CPU_SCHED_SRTF_P_CODE;
    }

    //check for FCFS-P
      //function: compareStr
    if ( compareStr( lowerCaseCodeStr, "fcfs-p" ) == STR_EQ)
    {
        //set FCFS-P code
        returnVal = CPU_SCHED_FCFS_P_CODE;
    }

    //check for RR-P
      //function: compareStr
    if ( compareStr( lowerCaseCodeStr, "rr-p" ) == STR_EQ)
    {
        //set RR-P code
        returnVal = CPU_SCHED_RR_P_CODE;
    }

    return returnVal;
}

/*
Name: getDataLineCode
Process: converts leader line string to configuration code value
         (all config file leader lines) 
Function Input/params: config leader line string (const char *)
Function Output/params: none
Function Output/returned: config code value (ConfigCodeMessages)
Device Input: none
Device Output: none
Dependencies: copyStr
*/
ConfigCodeMessages getDataLineCode(const char *dataBuffer)
    {
    //return appropriate code depending on prompt string provided
      //function: compareStr
    if (compareStr( dataBuffer, "Version/Phase" ) == STR_EQ)
        {
        return CFG_VERSION_CODE;
        }

    if (compareStr( dataBuffer, "File Path" ) == STR_EQ)
        {
        return CFG_MD_FILE_NAME_CODE;
        }

    if (compareStr( dataBuffer, "CPU Scheduling Code" ) == STR_EQ)
        {
        return CFG_CPU_SCHED_CODE;
        }

    if (compareStr( dataBuffer, "Quantum Time (cycles)" ) == STR_EQ)
        {
        return CFG_QUANT_CYCLES_CODE;
        }

    if (compareStr( dataBuffer, "Memory Display (On/Off)" ) == STR_EQ)
        {
        return CFG_MEM_DISPLAY_CODE;
        }

    if (compareStr( dataBuffer, "Memory Available (KB)" ) == STR_EQ)
        {
        return CFG_MEM_AVAILABLE_CODE;
        }

    if (compareStr( dataBuffer, "Processor Cycle Time (msec)" ) == STR_EQ)
        {
        return CFG_PROC_CYCLES_CODE;
        }

    if (compareStr( dataBuffer, "I/O Cycle Time (msec)" ) == STR_EQ)
        {
        return CFG_IO_CYCLES_CODE;
        }

    if (compareStr( dataBuffer, "Log To" ) == STR_EQ)
        {
        return CFG_LOG_TO_CODE;
        }

    if (compareStr( dataBuffer, "Log File Path" ) == STR_EQ)
        {
        return CFG_LOG_FILE_NAME_CODE;
        }

    //return corrupt leader line error code (if falls through all conditionals)
    return CFG_CORRUPT_PROMPT_ERR;
    }

/*
Name: getLogToCode
Process: converts "log to" test to config data code
         (three log to strings) 
Function Input/params: lower case log to string (const char *)
Function Output/params: none
Function Output/returned: config data code value (ConfigDataCodes)
Device Input: none
Device Output: none
Dependencies: copyStr
*/
ConfigDataCodes getLogToCode( const char *lowerCaseLogToStr )
    {
    //initialize function variables
    
    //set default to log to monitor
    ConfigDataCodes returnVal = LOGTO_MONITOR_CODE;

    //check for both
      //function: compareStr
    if (compareStr( lowerCaseLogToStr, "both" ) == STR_EQ)
        {
        //set return value to both code
        returnVal = LOGTO_BOTH_CODE;
        }

    //check for file
      //function: compareStr
    if (compareStr( lowerCaseLogToStr, "file" ) == STR_EQ)
        {
        //set return value to file code
        returnVal = LOGTO_FILE_CODE;
        }

    //return selected code
    return returnVal; //temp
    }

/*
Name: stripTrailingSpaces
Process: removes trailing spaces from input config leader lines
Function Input/params: config leader line string (char *)
Function Output/params: updated config leader line string (char *)
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: getStringLength
*/
void stripTrailingSpaces( char *str )
    {
    //initialize index to length of string - 1
      //function: getStringLength
    int index = getStringLength(str) - 1;

    //loop while space is found at end of string
    while (str[index] == SPACE)
        {
        //set element to NULL_CHAR
        str[index] = NULL_CHAR;

        //decrement index
        index--;
        }
    //end loop from end of string
    }

/*
Name: valueInRange
Process: checks for config data values in range, including string values
         (all config data values)
Function Input/params: line code number for specific config value (int)
                       integer value as needed (int)
                       double value as needed (double)
                       string value as needed (const char *)
Function Output/params: none
Function Output/returned: Boolean result of range test (bool)
Device Input: none
Device Output: none
Dependencies: compareStr
*/
bool valueInRange( int lineCode, int intVal, double doubleVal, 
                                                const char *lowerCaseStrVal)
    {
    //initialize function variables
    bool result; 

    //set result to true
    result = true;

    //use line code to identify prompt line
    switch( lineCode )
        {
        //for version code
        case CFG_VERSION_CODE:

            //check if limits of version code are exceeded
            if ( doubleVal < 0.00 || doubleVal > 10.00 )
                {
                //set boolean result to false
                result = false;
                }
            //break
            break;


        //for cpu scheduling code
        case CFG_CPU_SCHED_CODE:
        
            //check for not finding one of the scheduling strings
              //function: compareStr
            if ( compareStr( lowerCaseStrVal, "fcfs-n" ) != STR_EQ
                 && compareStr( lowerCaseStrVal, "sjf-n" ) != STR_EQ
                 && compareStr( lowerCaseStrVal, "srtf-p" ) != STR_EQ
                 && compareStr( lowerCaseStrVal, "fcfs-p" ) != STR_EQ
                 && compareStr( lowerCaseStrVal, "rr-p" ) != STR_EQ  )
                {
                //set boolean result to false
                result = false;
                }
            //break
            break;


        //for quantum cycles
        case CFG_QUANT_CYCLES_CODE:

            //check for quantum cycles limits exceeded
            if (intVal < 0 || intVal > 100)
                {
                //set boolean result to false
                result = false;
                }

            //break
            break;


        //for memory display
        case CFG_MEM_DISPLAY_CODE:

            //check for finding neither "on" or "off"
              //function: compareStr
            if ( compareStr( lowerCaseStrVal, "on" ) != STR_EQ
                 && compareStr( lowerCaseStrVal, "off" ) != STR_EQ )
                {
                //set boolean to false
                result = false;
                }

            //break
            break;


        //for memory available
        case CFG_MEM_AVAILABLE_CODE:

            //check for available memory limits exceeded
            if (intVal < 1024 || intVal > 102400)
                {
                //set boolean to false
                result = false;
                }

            //break
            break;


        //check for process cycles
        case CFG_PROC_CYCLES_CODE:

            //check for process cycles limits exceeded
            if (intVal < 1 || intVal > 100)
                {
                //set boolean to false;
                result = false;
                }

            //break
            break;


        //check for IO cycles
        case CFG_IO_CYCLES_CODE:

            //check for IO cycles limit exceeded
            if (intVal < 1 || intVal > 1000)
                {
                //set boolean to false
                result = false;
                }

            //break 
            break;


        //check for log to operation
        case CFG_LOG_TO_CODE:

            //check for not finding one of the log to strings
              //function: compareStr
            if ( compareStr( lowerCaseStrVal, "both") != STR_EQ
                 && compareStr( lowerCaseStrVal, "monitor") != STR_EQ
                 && compareStr( lowerCaseStrVal, "file") != STR_EQ )
                {
                //set boolean to false
                result = false;
                }

            //break
            break;

        }

    //return boolean
    return result; 
    }


