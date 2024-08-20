//header files
#include "OS_SimDriver.h"

int main( int argc, char **argv )
    {
    //initialize program
    
    //initialize variables
    ConfigDataType *configDataPtr = NULL;
    CmdLineData cmdLineData;
    char errorMessage[ MAX_STR_LEN ];
    bool configUploadSuccess = false;
    OpCodeType *metaDataPtr = NULL;

    //show title
      //function: printf
    printf("\nSimulator Program\n");
    printf("=================\n\n");

    //process command line, check for program run
    //atlease one correct switch and config file name verified at end          
      //function: processCmdLine
    if ( processCmdLine( argc, argv, &cmdLineData) )
        {
        //upload config file, check for success
          //function: getConfigData
        if (getConfigData( cmdLineData.fileName, &configDataPtr, 
                                                               errorMessage ))
            {
             
            //check for config display flag
            if (cmdLineData.configDisplayFlag)
                {
        
                //display config data
                  //function: displayConfigData
                displayConfigData( configDataPtr );

                }

            //set config upload success flag
            configUploadSuccess = true; 
            }
   
        //otherwise assume fail (config file upload)
        else 
            {

            //show error message, end program
              //function: printf
            printf("\nConfig Upload Error: %s, \nprogram aborted\n\n", 
                                                              errorMessage);
            }

        //check for config success and need for metadata
        if ( configUploadSuccess && 
                       (cmdLineData.mdDisplayFlag || cmdLineData.runSimFlag))
            {

            //upload metadata file, check for success
              //function: getMetaData
            if (getMetaData( configDataPtr->metaDataFileName, 
                                                 &metaDataPtr, errorMessage))
                {

                //check meta data display flag
                if (cmdLineData.mdDisplayFlag)
                    {

                    //display it
                      //function: displayMetaData
                    displayMetaData( metaDataPtr );
                    }
  
                //check run simulator flag
                if (cmdLineData.runSimFlag)
                    {

                    //run simulator
                      //function: runSim
                    runSim( configDataPtr, metaDataPtr );
                    }
                }
            //otherwise assume meta data filure
            else
                {
                
                 //show error message, end program
                   //function: printf
                 printf("\nMetaData Upload Error: %s, \nprogram aborted\n\n", 
                                                                errorMessage);
                 }

             }
             //end check for metadata upload
         
        //clean up config data
          //function: clearConfigData
        configDataPtr = clearConfigData( configDataPtr );

        //clean up metadata
           //function: clearConfigData
        metaDataPtr = clearMetaDataList( metaDataPtr );

        //end check for good cmd line
        }

    //otherwise assume cmd line failure
    else
        {
        //show command argument requirments
          //function: showCmdLineFormat
        showCmdLineFormat();
        }

    //show program end
      //function: printf
    printf("\nSimulator Program End.\n\n");

    //return success
    return 0;
    }

/*
Name: clearCmdLineStruct
Process: sets command line struccture data to deafaults
         booleans to false, filename to empty string
Function Input/params: pointer to command line structure (CmdLineData *)
Function Output/params: pointer to command line structure (CmdLineData *)
         with updated members,
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: none
*/
void clearCmdLineStruct( CmdLineData *clDataPtr )
    {
    //set all struct members to default
    clDataPtr->programRunFlag = false;
    clDataPtr->configDisplayFlag = false;
    clDataPtr->mdDisplayFlag = false;
    clDataPtr->runSimFlag = false;
    clDataPtr->fileName[0] = NULL_CHAR;
    }


/*
Name: processCmdLine
Process: checks for atleast two arguments,
         then sets booleans depending on command line switches
         which can be in any order,
         also captures config file name which must be the last argument
Function Input/params: number of arguments (int)
                       vector of arguments (char **)
Function Output/params: pointer to command line structure (cmdLineData *)
         with updated members,
         set to default values if failure
         no capture arguments
Function Output/returned: Boolean result of argument capture,
                          true if atleast one switch and config file name,
                          false otherwise
Device Input: none
Device Output: none
Dependencies: tbd
*/
bool processCmdLine( int numArgs, char **strVector, CmdLineData *clDataPtr )
    {
    //intialize function variables
    //initialize structure to defaults
      //function: clearCmdLineStruct
    clearCmdLineStruct( clDataPtr );

    //initialize success flags to false
    bool atLeastOneSwitchFlag = false;
    bool correctConfigFileFlag = false;

    //initialize first arg index to one
    int argIndex = 1;

    //declare other variables
    int fileStrLen, fileStrSubloc;

    //make sure has prog name, atleast one switch, and config file name
    if (numArgs >= MIN_NUM_ARGS)
        {

        //loop across args (starting from 1) and program run flag
        while (argIndex < numArgs)
            {

            //check for -dc (display config flag)
              //function: compareStr
            if (compareStr(strVector[argIndex], "-dc") == STR_EQ)
                {
                //set display config flag
                clDataPtr->configDisplayFlag = true;

                //set atleast one switch flag
                atLeastOneSwitchFlag = true;
                }

            //otherwise check for -dm (display metadata) flag
              //function: compareStr
            else if(compareStr(strVector[argIndex], "-dm") == STR_EQ)
                {
                //set metadata flag
                clDataPtr->mdDisplayFlag = true;

                //set atleast one switch flag
                atLeastOneSwitchFlag = true;
                }

            //otherwise check for -rs (run simulator)
              //function: compareStr
            else if(compareStr(strVector[argIndex], "-rs") == STR_EQ)
                {
                //set run sim flag
                clDataPtr->runSimFlag = true;

                //set atleast one switch flag
                atLeastOneSwitchFlag = true;
                }

            //check for file name ending in .cfg
            //must be last four 
            else
                {
                //find lengths to verify filename
                //filename must be last arg and have .cnf
                  //functions: getStringLength, findSubString
                fileStrLen = getStringLength( strVector[numArgs-1] );
                fileStrSubloc = findSubStr( strVector[numArgs-1], ".cnf" );

                //check for file existence and correct format
                if (fileStrSubloc == fileStrLen - LAST_FOUR_LETTERS
                                      && fileStrSubloc != SUBSTRING_NOT_FOUND)
                    {
                    //set file name to variable
                      //function: copyStr
                    copyStr( clDataPtr->fileName, strVector[ numArgs-1] );
 
                    //set success flag to true
                    correctConfigFileFlag = true;
                    }

                //otherwise assume bad config file name
                else
                    {
                    //reset struct, correct config file flag stays false
                      //function: clearStruct
                    clearCmdLineStruct( clDataPtr );
                    }

                //end check for cfg filename
                }
            //update arg index
            argIndex++;
            
            //end arg loop
            }

        //end test for min number of command line args
        }

    //return verification of atleast one switch and correct file name
    return atLeastOneSwitchFlag && correctConfigFileFlag;  //temp
    }


/*
Name: showCmdLineFormat
Process: displays command line format as assistance to user
Function Input/params: none
Function Output/params: none
Function Output/returned: none
Device Input: none
Device Output: cmd line data displayed 
Dependencies: printf
*/
void showCmdLineFormat()
    {
    //display command line format
      //function: printf
    printf("Command Line Format:\n");
    printf("    sim_01 [-dc] [-dm] [-rs] <config filename>\n");
    printf("    -dc [optional] displays configuration data\n");
    printf("    -dm [optional] displays meta data\n");
    printf("    -rs [optional] runs simulator\n");
    printf("    required config filename\n");
    }

         
                        

