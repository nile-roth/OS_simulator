//header files
#include "stringUtils.h"
/*
Name: compareStr
Process: compares two strings with the following results:
        if left string less than right, returns less than 0
        if left string greater than right, returns greater than 0
        if left string equals right string, returns 0
Function input/params: c-style left and right strings (char *)
Function output/params: none
Function output/returned: result as specified (int)
Device input/keyboard: none
Device output/monitor: none
Dependencies: getStringLength
*/
int compareStr( const char *leftStr, const char *rightStr )
    {
    //initalize function variables
    int diff, index = 0;

    //loop to end of shortest string
    //with overrun protection
    while (leftStr[index] != NULL_CHAR && rightStr[index] != NULL_CHAR
                                                 && index < MAX_STR_LEN)
        {
        //get difference in characters
        diff = leftStr[index] - rightStr[index];

        //check for difference between characters
        if (diff != 0)
            {
            //return difference
            return diff;
            }

        //increment index
        index++;
    
        //end loop
        }

    //return difference in lengths
      //function: getStringLength
    return getStringLength( leftStr ) - getStringLength( rightStr);
    }

/*
Name: concatStr
Process: appends one string to another
Function input/params: c-style source string (char *)
Function output/params: c-style destination string (char *)
Function output/returned: none
Device input/keyboard: none
Device output/monitor: none
Dependencies: getStringLength, copyStr
*/
void concatStr( char *destStr, const char *sourceStr)
    {
    //initialize function variables
    int destIndex, srcStrLen;
    int srcIndex = 0;
    char *tempSrc;

    //set dest Index
      //function: getStringLength
    destIndex = getStringLength(destStr);

    //get source string length set to dest index
      //function: getStringLength
    srcStrLen = getStringLength( sourceStr );

    //copy source str set to temp
      //function: copyStr, malloc
    tempSrc = (char *)malloc( sizeof( srcStrLen + 1 ));
    copyStr(tempSrc, sourceStr);
    

    //loop to end of source str
    while (destIndex < MAX_STR_LEN && tempSrc[destIndex] != NULL_CHAR)
        {
        //assign charcaters to end of deststr
        destStr[destIndex] = sourceStr[srcIndex];
        
        //update indecies
        srcIndex++;
        destIndex++;

        //set temporary end of dest str
        destStr[destIndex] = NULL_CHAR;
        }
        //end loop 

    //release temp str memory
      //function: free
    free(tempSrc);   
    }

/*
Name: copyStr
Process: copies one string into another,
         overwriting data in destStr
Function input/params: c-style source string (char *)
Function output/params: c-style destination string (char *)
Function output/returned: none
Device input/keyboard: none
Device output/monitor: none
Dependencies: getStringLength, compareStr
*/
void copyStr( char *destStr, const char *sourceStr )
    {
    //intialize function varialbes
    int index = 0;

    // check for source & dest not the same
    if (destStr != sourceStr)
        {
        //loop to end of source str
        while (index < MAX_STR_LEN && sourceStr[index] != NULL_CHAR)
            {
            //assign characters to end of destination str
            destStr[index] = sourceStr[index];

            //update index
            index++;

            //set temp end of dest str
            destStr[index] = NULL_CHAR;
       }
       //end loop
    }
}

/*
Name: findSubStr
Process: linear search for given substring within another string
Function input/params: c-style source test string (char *)
                       c-style source search string (char *)
Function output/params: none
Function output/returned: index of found substring, or
                          SUBSTRING_NOT_FOUND constant if not found
Device input/keyboard: none
Device output/monitor: none
Dependencies: getStringLength
*/
int findSubStr( const char *testStr, const char *searchSubStr )
    {
    //initialize function variables
    //initialize test string length
      //function: getStringLength
    int testStrLen;
    testStrLen = getStringLength(testStr);

    //initialize master index - location of substring start point
    int masterIndex = 0;

    //other vars
    int searchIndex, internalIndex;

    //loop across testStr
    while (masterIndex < testStrLen)
        {
        //set internal loop index to current test string index
        internalIndex = masterIndex;

        //set internal search index to zero
        searchIndex = 0;

        //loop to end of test string while test and sub strings are the same
        while( internalIndex < testStrLen && testStr[internalIndex] == 
                                                  searchSubStr[searchIndex])
            {
            //increment test & sub string indecies
            internalIndex++;
            searchIndex++;

            //check for end of substring
            if (searchSubStr[ searchIndex ] == NULL_CHAR)
                {
                //return beginning location of substring 
                return masterIndex; 
                }

            //end internal comparison loop
            }
        //increment current beginning location index
        masterIndex++;

        //end loop across test string
        }

    //assume test has failed and return SUBSTRING_NOT_FOUND
    return SUBSTRING_NOT_FOUND;
    }

/*
Name: getStrConstrained
process: captures a string from the input stream with various constraints
function input/params: input stream (FILE *)
                       clears leading non printable (bool)
                       clears leading space (bool)
                       stops at non printable (bool)
                       stops at specified delimeter (char)
                       Note: consumes delimeter
function output/params: string returned (char *)
function output/returned: success of operation (bool)
device input/keyboard: none
device output/monitor: none
dependencies: fgetc
*/
bool getStrConstrained( FILE *inStream, bool clearLeadingNonPrintable, 
                              bool clearLeadingSpace, bool stopAtNonPrintable, 
                                        char delimeter, char *capturedString )
    {
    //initialize function variables
    int intChar = EOF;
    int index = 0;

    //initialize output string
    capturedString[index] = NULL_CHAR;

    //capture first value in stream
       //function: fgetc
    intChar = fgetc( inStream );

    //loop to clear whitespace

    while( (intChar != EOF) && 
           ((clearLeadingNonPrintable && intChar < (int)SPACE) ||
           (clearLeadingSpace && intChar == (int)SPACE))
                                                         )
        {
        //get next char
          //function: fgetc
        intChar = fgetc( inStream );

        //end clear non printables loop
        }

    //check for end of file found
    if (intChar == EOF)
        {
        //return failed operation
        return false;
        }

    //loop to capture input
    while (
        //continue if not at end of file and max string len not reached 
        (intChar != EOF && index < MAX_STR_LEN - 1)  
         
        //ALSO continues if not printable flad set and characters are printable
        &&((stopAtNonPrintable && intChar >= (int)SPACE)

        //OR continues if nor printable flag not set
        ||(!stopAtNonPrintable))

        //AND continues if specified delimeter is not found
        && (intChar != (char)delimeter))
        {
        //place char in array element
        capturedString[index] = (char)intChar;

        //increment array index
        index++;

        //set next element to null
        capturedString[index] = NULL_CHAR;

        //get next char as int
          //function: fgetc
        intChar = fgetc(inStream);

        }
        //end loop

    //return success
    return true;
    }

/*
Name: getStringLength
process: finds the length of a string by counting characters up to NULL
function input/parans: c-style string (char *)
function output/params: none
function output/returned: length of string
device input/keyboard: none
device output/monitor: none
dependencies: none
*/
int getStringLength( const char *testStr )
    {
    //initialie variables 
    int index = 0;
    
    //loop until end of string, protect from overflow
    while (index < STD_STR_LEN && testStr[index] != NULL_CHAR)
        {
        //update index
        index++;   
        }
        //end loop

    //return index
    return index;     }

/*
Name: getStrToDelimeter
process: captures a string from the input stream to a specified delimeter
         Note: consumes delimeter
function input/params: input stream (FILE *)
                       stops at specified delimeter (char)
function output/params: string returned (char *)
function output/returned: success of operation (bool)
device input/keyboard: none
device output/monitor: none
dependencies: getStringConstrained
*/
bool getStrToDelimiter( FILE *inStream, char delimeter, char *capturedStr )
    {
    //call engine function with specified constraints
      //function: getStrConstrained
    return getStrConstrained(inStream, true, true, true, delimeter, capturedStr);
    }

/*
Name: getStrToLineEnd
process: captures a string from the input stream to the end of a line
function input/params: input stream (FILE *)
function output/params: string returned (char *)
function output/returned: success of operation (bool)
device input/keyboard: none
device output/monitor: none
dependencies: getStrConstrained
*/
bool getStrToLineEnd( FILE *inStream, char *capturedStr )
    {
    //call engine function with specified constraints
      //function: getStrConstrained
    getStrConstrained(inStream, true, true, true, NON_PRINTABLE_CHAR, capturedStr);

    //return success
    return true;
    }

/*
Name: getSubStr
process: compares sub string within larger string
         between two inclusive indices.
         returns empty string if either index is out of range,
         assumes enough memory in destStr
function input/params: c-style source string (char *)
                       start and end indices (int)
function output/params: c-style destination string (char *)
function output/returned: none
device input/keyboard: none
device output/monitor: none
dependencies: getStringLength, malloc, copyStr, free
*/
void getSubStr( char *destStr, const char *sourceStr, int startIndex, 
                                                           int endIndex )
    {
    //initialize variables
    int sourceStrLen;

    //set length of source str
      //function:getStringLength
    sourceStrLen = getStringLength( sourceStr );

    //initialzie the destination index to 0
    int destIndex = 0;

    //initialize source index to start index
    int sourceIndex = startIndex;

    //create pointer for temp str
    char *tempStr;

    //check for indecies in the limits
    if (endIndex < sourceStrLen && startIndex <= endIndex && startIndex >= 0)
        {
        //create temp str
          //function: malloc, copyStr
        tempStr = (char *)malloc( sourceStrLen + 1 );
        copyStr( tempStr, sourceStr );

        //loop across requested substring starting at index 
        while ( sourceIndex <= endIndex )
            {
            //assign current char to dest str
            destStr[destIndex] = tempStr[sourceIndex];

            //increment index
            destIndex++;
            sourceIndex++;

            //set temp end of dest str
            destStr[destIndex] = NULL_CHAR;

            //end loop
            }

        //end conditional
        }

    //return memory for temp str
      //function: free
    free(tempStr);
    }
/*
Name: setStrToLowerCase
process: iterates through string, sets any upper case letter
         to lower case; no effect on other letters
function input/parans: c-style source string (char *)
function output/params: c-style destination string (char *)
function output/returned: none
device input/keyboard: none
device output/monitor: none
dependencies: toLowerCase, getStringLength
*/
void setStrToLowerCase( char *destStr, const char *sourceStr )
    {
    //initialize vars
    int index = 0;
    int sourceStrLen;

    //get source str length
      //function: getStringLength
    sourceStrLen = getStringLength( sourceStr );

    //create temp str pointer
    char *tempStr;

    //copy source str to avoid aliasing
      //function: malloc, copyStr
    tempStr = (char *)malloc( sourceStrLen + 1 );
    copyStr( tempStr, sourceStr );

    //loop across source str
    while ( tempStr[index] != NULL_CHAR && index < MAX_STR_LEN )
        {
        //make lowercase using func
           //function: toLowerCase
        destStr[index] = toLowerCase(tempStr[index]);
        
        //update index
        index++;

        //set temp end of dest str
        destStr[index] = NULL_CHAR;

        //end loop
        }

    //release memory of temp str
      //function: free
    free(tempStr);
    }

/*
Name: toLowerCase
process: if character is upper case, sets it to lower case
         otherwise returns character unchagned
function input/parans: test character (char)
function output/params: none
function output/returned: character set to lower case
device input/keyboard: none
device output/monitor: none
dependencies: none
*/
char toLowerCase( char testChar )
    {
    //check for uppercase
    if (testChar >= 'A' && testChar <= 'Z')
        {
        //if so return lower case letter
        return testChar - 'A' + 'a';
        }
    //if not return unchanged
    return testChar;
    }
