//header files
#include "metaDataOps.h"

/*
Name: addNode
Process: adds metadata node to linked list recursively,
         handles empty list condition
Function Input/params: pointer to head or next linked node (OpCodeType *)
                       pointer to new node (OpCodeType *)
Function Output/params: none
Function Output/returned: pointer to prev node or head node (OpCodeType *)
Device Input: none
Device Output: none
Dependencies: copyStr
*/
OpCodeType *addNode( OpCodeType *localPtr, OpCodeType *newNode )
    {
    //check for local ptr assigned to null
    if (localPtr == NULL)
        {
        //access memory for new link
          //function: malloc
        localPtr = (OpCodeType *) malloc(sizeof(OpCodeType));
   
        //assign all three values to newly created node
        //assign nextPtr to null
          //function: copyStr
        localPtr->pid = newNode->pid;
        copyStr(localPtr->command, newNode->command);
        copyStr(localPtr->inOutArg, newNode->inOutArg);
        copyStr(localPtr->strArg1, newNode->strArg1);
        localPtr->intArg2 = newNode->intArg2;
        localPtr->intArg3 = newNode->intArg3;
        localPtr->opEndTime = newNode->opEndTime;
        localPtr->nextNode = NULL;

        //return current local pointer
        return localPtr;
        }

    //assume end of list not found yet
    //assign recursive function to current's next link
      //function: addNode
    localPtr->nextNode = addNode( localPtr->nextNode, newNode);

    //return current local ptr
    return localPtr;
    }

/*
Name: clearMetaDataList
Process: recursively traverses list, frees dynamically allocated nodes
Function Input/params: node op code (const OpCodeType *)
Function Output/params: none
Function Output/returned: NULL (OpCodeType *)
Device Input: none
Device Output: none
Dependencies: free
*/
OpCodeType *clearMetaDataList( OpCodeType *localPtr )
    {
    //check for local pointer not null (list not empty)
    if (localPtr != NULL)
        {
        //call recursive function with next ptr
          //function: clearMetaDataList
        clearMetaDataList( localPtr->nextNode );

        //release memory to OS
          //function: free
        free( localPtr );

        //set given ptr to NULL
        localPtr = NULL;
        }

    //return null 
    return NULL;
    }

/*
Name: displayMetaData
Process: data display of all op code items
Function Input/params: pointer to head of opcode list (OpCodeType *)
Function Output/params: none
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: printf
*/
void displayMetaData( const OpCodeType *localPtr )
    {
    //display title 
      //function: printf
    printf("Meta-Data File Display\n");
    printf("----------------------\n");

    //loop to end of linked list
    while (localPtr != NULL)
        {
        //print leader
          //function: printf
        printf("Op Code: ");

        //print op code pid
          //function: printf
        printf("/pid: %d", localPtr->pid);

        //print op code command
          //function: printf
        printf("/cmd: %s", localPtr->command);

        //check for dev op
        if ( compareStr( localPtr->command, "dev" ) == STR_EQ)
            {
            //print I/O parameter
              //function: printf
            printf("/io: %s", localPtr->inOutArg);
            }

        //otherwise assume other than dev
        else
            {
            //print NA
              //function: printf
            printf("/io: NA");
            }

        //print first string arg
          //function: printf
        printf("\n\t /arg1: %s", localPtr->strArg1);

        //print first int arg
          //function: printf
        printf("/arg 2: %d", localPtr->intArg2);

        //print second int arg
          //function: printf
        printf("/arg 3: %d", localPtr->intArg3);

        //print op end time
          //function: printf
        printf("/op end time: %8.6f", localPtr->opEndTime);

        //end line
          //function: printf
        printf("\n\n");

        //assign local ptr to next node
        localPtr = localPtr->nextNode;
        }
    }

/*
Name: getCommand
Process: parses three letter command part of op code string
Function Input/params: input op code string (const char *)
                       starting index (int)
Function Output/params: parsed command (char *)
Function Output/returned: updated starting index for use
                          by calling function (int)
Device Input: none
Device Output: none
Dependencies: none
*/
int getCommand( char *cmd, const char *inputStr, int index )
    {
    //initialize variables
    int lengthOfCommand = 3;

    //loop across command length
    while (index < lengthOfCommand)
        {
        //assign char from input string to buffer string
        cmd[index] = inputStr[index];

        //incerment index
        index++;

        //set next char to null
        cmd[index] = NULL_CHAR;
        }

    //return current index
    return index;
    }


/*
Name: getMetaData
Process: screen display of all config data
Function Input/params: pointer to config data strcuture (ConfigDataType *)
Function Output/params: none
Function Output/returned: none
Device Input: none
Device Output: display of config data
Dependencies: printf
*/
bool getMetaData( const char *fileName, OpCodeType **opCodeDataHead, char *endStateMsg )
    {
    //initialize function variables
    int accessResult, startCount = 0, endCount = 0;
    char dataBuffer[MAX_STR_LEN];
    bool returnState = true;
    OpCodeType *newNodePtr;
    OpCodeType *localHeadPtr = NULL;
    FILE *fileAccessPtr;

    //initialize op code data pointer in case of return error
    *opCodeDataHead = NULL;

    //initialize end state message
      //function:copyStr
    copyStr( endStateMsg, "Metadata file upload successful");

    //open file for reading
      //function: fopen
    fileAccessPtr = fopen( fileName, READ_ONLY_FLAG );

    //check for file open failure
    if (fileAccessPtr == NULL)
        {
        //set end state message
          //function: copyStr
        copyStr( endStateMsg, "Metadata file access error");

        //return file access error
        return false;
        }

    //check first line for correct leader
      //functions: getStrToDelimiter, compareStr
    if (!getStrToDelimiter( fileAccessPtr, COLON, dataBuffer )
        || compareStr( dataBuffer, "Start Program Meta-Data Code") != STR_EQ)
        {
        //close file
          //function: fclose
        fclose( fileAccessPtr );

        //set end state message
          //function: copyStr
        copyStr( endStateMsg, "Corrupt metadata leader line error");

        //return corrupt descriptor error
        return false;
        }

    //allocate memory for temp data struct
      //function: malloc
    newNodePtr = (OpCodeType *) malloc(sizeof(OpCodeType));

    //get first op command
      //function: getOpCommand
    accessResult = getOpCommand( fileAccessPtr, newNodePtr );

    //get start and end counts 
      //function: updateStartCount, updateEndCount
    startCount = updateStartCount( startCount, newNodePtr->strArg1 );
    endCount = updateEndCount( endCount, newNodePtr->strArg1 );

    //check for failure of first complete op command
    if (accessResult != COMPLETE_OPCMD_FOUND_MSG)
        {
        //close file
          //function: fclose
        fclose( fileAccessPtr );

        //clear data from struct list
          //function: clearMetaDataList
        *opCodeDataHead = clearMetaDataList( localHeadPtr );

        //free temp struct memory
          //function: free
        free ( newNodePtr );

        //set end state message
          //function: copyStr
        copyStr( endStateMsg, "Metadata incomplete first op command found");

        //return result of operation
        return false;
        }

    //loop across all remaining commands
    //(while complete op commands are found)
    while ( accessResult == COMPLETE_OPCMD_FOUND_MSG )
        {
        //add new op comman to linked list
          //function: addNode
        localHeadPtr = addNode( localHeadPtr, newNodePtr );

        //get new op command
          //function: getOpCommand
        accessResult = getOpCommand( fileAccessPtr, newNodePtr );

        //update start and end counts
          //functions: updateStartCount, updateEndCount
        startCount = updateStartCount( startCount, newNodePtr->strArg1 );
        endCount = updateEndCount( endCount, newNodePtr->strArg1 );
        }

    //after loop completion check for last op command found
    if (accessResult == LAST_OPCMD_FOUND_MSG)
        {

        //check for start and end counts equal
        if (startCount == endCount)
            {
            //add last node to linked list
              //function: addNode
            localHeadPtr = addNode( localHeadPtr, newNodePtr );

            //set access result to no error for later
            accessResult = NO_ACCESS_ERR;

            //check last line for incorrect end descriptor
              //function: getStrToDelimiter, compareStr
            if (!getStrToDelimiter( fileAccessPtr, PERIOD, dataBuffer )
                || compareStr( dataBuffer, "End Program Meta-Data Code" )
                                                                  != STR_EQ)
                {
                //set access result to correupted descriptor error 
                accessResult = MD_CORRUPT_DESCRIPTOR_ERR;

                //set end state message
                  //function: copyStr
                copyStr( endStateMsg, "Metadata corrupted descriptor error");
                }
            }

        //otherwise assume start end counts not equal
        else 
            {
            //close file
              //function:fclose
            fclose( fileAccessPtr );

            //clear data from struct list
              //function: clearMetaDataList
            *opCodeDataHead = clearMetaDataList( localHeadPtr );

            //free temp struct memory
              //function: free
            free( newNodePtr );

            //set access result to error
            accessResult = UNBALANCED_START_END_ERR;

            //set end state message
              //function: copyStr
            copyStr( endStateMsg, 
                          "Unbalanced start and end arguments in metadata");

            //return result of operation
            return false;
            }
        } 

    //otherwise assume didnt find end
    else
        {
        //set end state message
          //function: copyStr
        copyStr( endStateMsg, "Corrupted metadata op code" );

        //unset return state
        returnState = false;
        }

    //check for any errors found
    if (accessResult != NO_ACCESS_ERR)
        {
        //clear op command list
          //function:clearMetaDataList
        localHeadPtr = clearMetaDataList( localHeadPtr );
        }

    //close file
      //function:fclose
    fclose( fileAccessPtr );

    //releast temp struct memory
      //function: free
    free( newNodePtr );

    //assign temp local head ptr to parameter return pointer
    *opCodeDataHead = localHeadPtr;

    //return access result
    return returnState; 
    }

/*
Name: getOpCommand
Process: acquires one op command line from a prev opened file
         parses it, and sets various struct members according
         to the three letter command
Function Input/params: pointer to open file handle (FILE *)
Function Output/params: pointer to one op code struct (OpCodeType *)
Function Output/returned: coded reult of operation (OpCodeMessages)
Device Input: op code line uploaded
Device Output: none
Dependencies: getStrToDelimeter, getCommand, copyStr, verifyValidCommand, compareStr,
              getStringArg, verifyFirstStringArg, getNumberArg
*/
OpCodeMessages getOpCommand( FILE *filePtr, OpCodeType *inData )
    {
    //initialize variables
    //initialize local constants
    const int MAX_ARG_STR_LENGTH = 15;

    //initialize other variables
    int numBuffer = 0;
    char strBuffer[ STD_STR_LEN ];
    char cmdBuffer[ STD_STR_LEN ];
    char argStrBuffer[ MAX_ARG_STR_LENGTH ];
    int runningStringIndex = 0;
    bool arg2FailureFlag = false;
    bool arg3FailureFlag = false;

    //get whole op command as string, check for success
      //function: getStrToDelimiter
    if ( getStrToDelimiter( filePtr, SEMICOLON, strBuffer ))
        {
        //get three letter command
          //function:getCommand
        runningStringIndex = getCommand( cmdBuffer, strBuffer, 
                                                       runningStringIndex);

        //assign op command to node
          //function: copyStr
        copyStr( inData->command, cmdBuffer );
        }

    //otherwise assume unsuccesful access
    else
        {
        //set pointer to data structure null
        inData = NULL;

        //return op command access failure
        return OPCMD_ACCESS_ERR;
        }

    //verify op command
    if ( !verifyValidCommand( cmdBuffer ) )
        {
        //return op command error
        return CORRUPT_OPCMD_ERR;
        }

    //set all struct values that may not be initialized to defaults
    inData->pid = 0;
    inData->inOutArg[0] = NULL_CHAR;
    inData->intArg2 = 0;
    inData->intArg3 = 0;
    inData->opEndTime = 0.0;
    inData->nextNode = NULL;

    //checks for device command
    if (compareStr( cmdBuffer, "dev" ) == STR_EQ)
        {
        //get I/O argument
        runningStringIndex = getStringArg( argStrBuffer, strBuffer, 
                                                       runningStringIndex );

        //set device in/out argument
        copyStr( inData->inOutArg, argStrBuffer );

        //check correct arg
        if ( compareStr( argStrBuffer, "in" ) != STR_EQ
             && compareStr( argStrBuffer, "out" ) != STR_EQ )
            {
            //return arg error
            return CORRUPT_OPCMD_ARG_ERR;
            }
        }

    //get first string arg
    runningStringIndex = getStringArg( argStrBuffer, strBuffer, 
                                                        runningStringIndex );

    //set device in/out arg
      //function: copyStr
    copyStr( inData->strArg1, argStrBuffer );

    //check for legit first string arg
    if( !verifyFirstStringArg( argStrBuffer ) )
        {
        //return arg error
        return CORRUPT_OPCMD_ARG_ERR;
        }

    //check for last op command found
    if ( compareStr( inData->command, "sys" ) == STR_EQ
                && compareStr( inData->strArg1, "end" ) == STR_EQ )
        {
        //return last op command found message
        return LAST_OPCMD_FOUND_MSG;
        }

    //check for app start seconds arg
    if ( compareStr( inData->command, "app" ) == STR_EQ
          && compareStr( inData->strArg1, "start" ) == STR_EQ )
        {
        //get number arg
          //function: getNumberArg
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
                                                   runningStringIndex );
        //check for failed number access
        if (numBuffer <= BAD_ARG_VAL)
            {
            //set failure flag
            arg2FailureFlag = true;
            }

        //set first int arg to number
        inData->intArg2 = numBuffer;
        }

    //check for cpu cycle time
    else if (compareStr( inData->command, "cpu" ) == STR_EQ )
        {
        //get number arg
          //function: getNumberArg
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,     
                                                       runningStringIndex);

        //check for failed number access
        if (numBuffer <= BAD_ARG_VAL)
            {
            //set failure flag
            arg2FailureFlag = true;
            }

        //set first int arg to number
        inData->intArg2 = numBuffer;
        }

    //check for device cycle time
    else if (compareStr(inData->command, "dev") == STR_EQ)
        {
        //get number arg
          //function: getNumberArg
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
                                                         runningStringIndex);

        //check for failed number access
        if (numBuffer <= BAD_ARG_VAL)
            {
            //set failure flag
            arg2FailureFlag = true;
            }

        //set first int arg to number
        inData->intArg2 = numBuffer;
        }

    //check for memory base and offset
    else if (compareStr(inData->command, "mem") == STR_EQ)
        {
        //get number arg
          //function: getNumberArg
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
                                                         runningStringIndex);

        //check for failed number access
        if (numBuffer <= BAD_ARG_VAL)
            {
            //set failure flag
            arg2FailureFlag = true;
            }

        //set first int arg to number
        inData->intArg2 = numBuffer;

        //get number arg for offset
          //function: getNumberArg
        runningStringIndex = getNumberArg( &numBuffer, strBuffer,
                                                         runningStringIndex);

        if (numBuffer <= BAD_ARG_VAL)
            {
            //set failure flag
            arg3FailureFlag = true;
            }

        //set first int arg to number
        inData->intArg3 = numBuffer;
        }

    //check int args for upload failure
    if (arg2FailureFlag || arg3FailureFlag)
        {
        //return corrupt op command error 
        return CORRUPT_OPCMD_ARG_ERR;
        }

    //return complete op command found message
    return COMPLETE_OPCMD_FOUND_MSG; 
    }

/*
Name: getNumberArg
Process: starts at given index, captures and assembles integer argument,
         and returns as parameter
Function Input/params: input string (const char *), starting index (int)
Function Output/params: pointer to captured integer value
Function Output/returned: updated index for next function start
Device Input: none
Device Output: none
Dependencies: isDigit
*/
int getNumberArg( int *number, const char *inputStr, int index)
    {
    //initialize function variables
    bool foundDigit = false;
    *number = 0;
    int multiplier = 1;

    //loop to skip white space
    while (inputStr[index] <= SPACE || inputStr[index] == COMMA)
        {
        //increment index
        index++;
        }

    // loop across string length
    while (isDigit( inputStr[index] ) == true && inputStr[index] != NULL_CHAR)
        {
        //set digit found flag
        foundDigit = true;

        //assign digit to output
        (*number) = (*number) * multiplier + inputStr[index] - '0';

        //increment index and multiplier
        index++;
        multiplier = 10;
        }

    //check for digit not found
    if (!foundDigit)
        {
        //set number to BAD_ARG_VAL, use dereference for pointer
        *number = BAD_ARG_VAL;
        }

    //return current index
    return index;
    }

/*
Name: getStringArg
Process: starts at given index, captures and assembles string argument,
         and returns as parameter
Function Input/params: input string (const char *), starting index (int)
Function Output/params: pointer to captured string arg (char *)
Function Output/returned: updated index for next function start
Device Input: none
Device Output: none
Dependencies: none
*/
int getStringArg(char *strArg, const char *inputStr, int index)
    {
    //initialize function variables
    int localIndex = 0;

    //loop to skip white space
    while (inputStr[index] <= SPACE || inputStr[index] == COMMA)
        {
        //increment index
        index++;
        }

    // loop across string length
    while (inputStr[index] != COMMA && inputStr[index] != NULL_CHAR)
        {
        //assign character from input string to buffer string
        strArg[ localIndex ] = inputStr[ index ];

        //increment index
        index++; localIndex++;

        //set next char to null char
        strArg[localIndex] = NULL_CHAR;
        }

    //return current index
    return index;
  }

/*
Name: isDigit
Process: test character parameter for digit, returns true if is digit,
         false otherwise
Function Input/params: test character (char)
Function Output/params: none
Function Output/returned: boolean result of test
Device Input: none
Device Output: none
Dependencies: none
*/
bool isDigit( char testChar )
    {
    //check if testchar is between '0' and '9'
    if (testChar >= '0' && testChar <= '9')
        {
        //return true
        return true;
        }

    //return false
    return false; 
    }

/*
Name: updateEndCount
Process: manages count of "end arguments to be compared at end
         of process
Function Input/params: initial count (int)
                       test string for "end" (const char *)
Function Output/params: none
Function Output/returned: updated count, if "end" string found,
                          otherwise no change
Device Input: none
Device Output: none
Dependencies: compareStr
*/
int updateEndCount( int count, const char *opString )
    {
    //check for end in op string
      //function: compareStr
    if (compareStr( opString, "end" ) == STR_EQ )
        {
        //return incremented end count
        return count + 1;
        }

    //return unchanged end count
    return count;
    }

/*
Name: updateStartCount
Process: manages count of "start" arguments to be compared at end
         of process input
Function Input/params: initial count (int)
                       test string for "start" (const char *)
Function Output/params: none
Function Output/returned: updated count, if "start" string found,
                          otherwise no change
Device Input: none
Device Output: none
Dependencies: compareStr
*/
int updateStartCount( int count, const char *opString )
    {
    //check for end in op string
      //function: compareStr
    if (compareStr( opString, "start" ) == STR_EQ )
        {
        //return incremented end count
        return count + 1;
        }

    //return unchanged end count
    return count;
    }

/*
Name: verifyFirstStringArg
Process: checks for all possibilities of first arg string of op command
Function Input/params: test string (const char *)
Function Output/params: none
Function Output/returned: boolean result of test
Device Input: none
Device Output: none
Dependencies: compareStr
*/
bool verifyFirstStringArg( const char *strArg )
    {
    //check for string holding correct first arg
      //function: compareStr
     return (compareStr( strArg, "access" ) == STR_EQ
          || compareStr( strArg, "allocate" ) == STR_EQ
          || compareStr( strArg, "end" ) == STR_EQ
          || compareStr( strArg, "ethernet" ) == STR_EQ
          || compareStr( strArg, "hard drive" ) == STR_EQ
          || compareStr( strArg, "keyboard" ) == STR_EQ
          || compareStr( strArg, "monitor" ) == STR_EQ
          || compareStr( strArg, "printer" ) == STR_EQ
          || compareStr( strArg, "process" ) == STR_EQ
          || compareStr( strArg, "serial" ) == STR_EQ
          || compareStr( strArg, "sound signal" ) == STR_EQ
          || compareStr( strArg, "start" ) == STR_EQ
          || compareStr( strArg, "usb" ) == STR_EQ
          || compareStr( strArg, "video signal" ) == STR_EQ );
    }

/*
Name: verifyValidCommand
Process: checks for all possibilities of three letter op code command
Function Input/params: test string (const char *)
Function Output/params: none
Function Output/returned: boolean result of test
Device Input: none
Device Output: none
Dependencies: compareStr
*/
bool verifyValidCommand( char *testCmd )
    {
    //check for string holding three letter op code command
      //function: compareStr
    return ( compareStr( testCmd, "sys" ) == STR_EQ 
          || compareStr( testCmd, "app" ) == STR_EQ 
          || compareStr( testCmd, "cpu" ) == STR_EQ 
          || compareStr( testCmd, "mem" ) == STR_EQ 
          || compareStr( testCmd, "dev" ) == STR_EQ );
    }

