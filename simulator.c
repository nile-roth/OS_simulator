#include "simulator.h"

/*
ToDo:
make overlapping logicals of same process seg fault
fix negative time issue
handle interrupt in cpu op
set max cols to 80
Remove all extra text/tests

debug tool:

printf("\nafter remove thread %d\n", readyThread->pid);
threadWkg = threadHead;
while (threadWkg != NULL)
{
printf("%d\n", threadWkg->pid);
threadWkg = threadWkg->nextNode;
}

*/

//create segfault bool and set to 
bool segFault = false;

/*
Name: runSim
Process: primary simulation driver
Function Input/params: config data (ConfigDataType *),
                       metadata (OpCOdeType *)
Function Output/params: none
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: printf, createPCBs, accessTimer, display, changeState, 
                                                             executeProgCtr 
*/
void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr )
{
    /////////////////////////INITIALIZATION////////////////////////////////    

    //FILE VARS
    FILE *outputFile = NULL;
    
    //TIME VARIABLES
    char timeStr[MAX_STR_LEN];
    double startTime, endTime, currentTime;
    int freezeTime, totalCycles; 
    int cycleIndex = 0;
   
    //PCB POINTERS
    PCBDataType *PCBHeadPtr = NULL;
    PCBDataType *PCBWkgPtr = NULL; 
    PCBDataType *currentProcess, *testProcess;   

    //MEM PTRS
    Memory *memHeadPtr = (Memory *)malloc(sizeof(Memory));
    memHeadPtr->nextNode = NULL;

    //STRING POINTERS
    stringNode *stringListHead = (stringNode *)malloc(sizeof(stringNode));
    stringListHead->nextNode = NULL;
    stringNode *stringListWkg;

    //THREAD POINTERS
    Thread *threadHead = (Thread *)malloc(sizeof(Thread));
    threadHead->startTime = NO_PROCESS;      //identifier for first node
    threadHead->nextNode = NULL;
    Thread *readyThread = NULL;

    //BOOLEANS
    bool preemptive = false;
    bool RR = false;
    bool broke = false;
    bool cpuEndedIdle = false;

    //SET BOOLEANS
    if (configPtr->cpuSchedCode == CPU_SCHED_FCFS_P_CODE ||
        configPtr->cpuSchedCode == CPU_SCHED_SRTF_P_CODE ||
        configPtr->cpuSchedCode == CPU_SCHED_RR_P_CODE)
    {
        preemptive = true;
    }
    if (configPtr->cpuSchedCode == CPU_SCHED_RR_P_CODE)
    {
        RR = true;
    }

    /////////////////////////END INITIALIZATION////////////////////////////

    copyStr( stringListHead->str, "\nSimulator running...\n\n" );

    //state that sim is running
    printf("\nSimulator running...\n\n");

    //check if log to file only
    if (configPtr->logToCode == LOGTO_FILE_CODE)
    {
        //let user know outputting to file
        printf("Writing output to file only...\nfilename: %s\n", 
                                                   configPtr->logToFileName);
    }

    //create process control blocks with metadata ptr
      //function: createPCBs
    PCBHeadPtr = createPCBs( metaDataMstrPtr, configPtr );
    
    //set wkg pointer to head
    PCBWkgPtr = PCBHeadPtr;

    //start simulation timer
      //function: accessTimer
    accessTimer( ZERO_TIMER, timeStr );

    //display simulator start
    display( SIM_START_CODE, timeStr, PCBWkgPtr, configPtr, stringListHead, 
                                                            memHeadPtr, NEW );

    //loop through all processes and set to ready
    while( PCBWkgPtr != NULL)
    {
        //display operation of state change
        display( CHANGE_STATE_CODE, timeStr, PCBWkgPtr, configPtr, 
                                           stringListHead, memHeadPtr, READY);

        //set state to ready from new
        PCBWkgPtr->state = READY;

        //increment wkgPtr
        PCBWkgPtr = PCBWkgPtr->nextNode;
    }

    //set wkg pointer back to head
    PCBWkgPtr = PCBHeadPtr;

    //initialize memory
    memHeadPtr = processMem(memHeadPtr, 0, 0, 0, INIT_REQ_TYPE, configPtr);

    //display memory initialized
    display( MEM_INIT_CODE, timeStr, PCBWkgPtr, configPtr, stringListHead, 
                                                            memHeadPtr, NEW );

    //get the next process
    currentProcess = getNextProcess( configPtr, PCBHeadPtr );

    //display current process selected with time remaining
    display(PROCESS_SELECT_CODE, timeStr, currentProcess, configPtr, 
                                              stringListHead, memHeadPtr, NEW);

    //display operation of state change
    display(CHANGE_STATE_CODE, timeStr, currentProcess, configPtr, 
                                          stringListHead, memHeadPtr, RUNNING);

    //set process state to running
    currentProcess->state = RUNNING;

    /////////////////////////START OF MASTER LOOP///////////////////////////

    while(currentProcess != NULL)
    {   
        //a check to ensure not operating off cpu idle
        if (currentProcess->timeRemaining != NO_PROCESS)
        {
            ////////////////////////CHECK FOR CPU OPERATION///////////////////////

            //check for cpu command  
            if (compareStr( currentProcess->programCounter->command, "cpu" ) 
                                                                    == STR_EQ)
            {
                //get freeze time
                freezeTime = 
                currentProcess->programCounter->intArg2 * configPtr->procCycleRate;

                //print start operation 
                  //function: printf
                display( OP_START_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );
                if (preemptive)
                {
                    totalCycles = currentProcess->programCounter->intArg2;

                    //loop until all cycles complete
                    for(cycleIndex = 0; cycleIndex < totalCycles && !broke;
                                                               cycleIndex++)
                    {
                        //freeze time for one cycle
                        runTimer( configPtr->procCycleRate );

                        //subtract time from time remaining
                        currentProcess->timeRemaining -= configPtr->procCycleRate;

                        //update cycles to complete
                        currentProcess->programCounter->intArg2--;

                        //get current time
                        currentTime = accessTimer( LAP_TIMER, timeStr );

                        //check for thread ready
                        readyThread = interruptReady( threadHead, currentTime );

                        if ((RR && cycleIndex >= configPtr->quantumCycles - 1) || 
                            readyThread != NULL)
                        { 
                            broke = true; 

                            //get start time
                            startTime = accessTimer( LAP_TIMER, timeStr );

                            //set end time = startTime + freezeTime 
                            endTime = startTime + ((double)freezeTime / 1000 );       

                            if (readyThread != NULL)
                            {
                                testProcess = getProcessFromThread( readyThread, PCBHeadPtr );
                                //check if interrupt is cpu
                                if (compareStr( testProcess->programCounter->command, "cpu" ) 
                                                                          == STR_EQ)
                                {
                                    //we should not increment program counter
                                    cpuEndedIdle = true;
                                }

                                display( CPU_INTERRUPTED_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );
                            }
                            else //quantum time out
                            {
                                display( QUANTUM_OUT_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );
                            }

                            display( CHANGE_STATE_CODE, timeStr, currentProcess, 
                             configPtr, stringListHead, memHeadPtr, BLOCKED );

                            //set process to blocked
                            currentProcess->state = BLOCKED;

                            //add node 
                            threadHead = addThreadNode( threadHead, startTime, endTime, 
                                                   currentProcess->processID );
                        }
                    }
                }
                else
                {
                    //freeze program 
                      //function: runTimer
                    runTimer( freezeTime );
                }

                //will need a check for if broke 
                if (!broke)
                {
                    //print end operation
                      //function: printf
                    display( OP_END_CODE, timeStr, currentProcess, 
                                     configPtr, stringListHead, memHeadPtr, NEW );
                }   
                //resolve boolean
                broke = false;
            }

            /////////////////END CHECK FOR CPU OPERATION/////////////////////////

            ////////////////////CHECK FOR I/O OPERATION//////////////////////////

            //check for IO command
            if (compareStr( currentProcess->programCounter->command, "dev" ) 
                                                                   == STR_EQ )
            {
                //get freeze time
                freezeTime = 
                  currentProcess->programCounter->intArg2 * configPtr->ioCycleRate;

                //print start operation 
                  //function: printf
                display( OP_START_CODE, timeStr, currentProcess, 
                                    configPtr, stringListHead, memHeadPtr, NEW );
               
                //check for preemptive
                if (preemptive)
                {
                    //get start time
                    startTime = accessTimer( LAP_TIMER, timeStr );

                    //set end time = startTime + freezeTime 
                    endTime = startTime + ((double)freezeTime / 1000 );       

                    display( PROCESS_BLOCKED_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );

                    display( CHANGE_STATE_CODE, timeStr, currentProcess, 
                             configPtr, stringListHead, memHeadPtr, BLOCKED );

                    //set process to blocked
                    currentProcess->state = BLOCKED;

                    //add node 
                    threadHead = addThreadNode( threadHead, startTime, endTime, 
                                                   currentProcess->processID );
                }
            
                else //nonpreemptive
                {
                    //freeze program 
                      //function: runTimer
                    runTimer( freezeTime );

                    //print end operation
                      //function: printf
                    display( OP_END_CODE, timeStr, currentProcess, 
                                     configPtr, stringListHead, memHeadPtr, NEW ); 
                }
      
            }

            //////////////////END CHECK FOR I/O OPERATION////////////////////////

            ///////////////////////MEMORY MANAGEMENT UNIT////////////////////////

            //check for mem command
            if(compareStr( currentProcess->programCounter->command, "mem" ) 
                                                                    == STR_EQ)
            {
                //check for allocate 
                if (compareStr( currentProcess->programCounter->strArg1, 
                                                        "allocate" ) == STR_EQ)
                {
                    //display allocate request
                    display( MEM_REQ_CODE, timeStr, currentProcess, 
                                     configPtr, stringListHead, memHeadPtr, NEW ); 

                    //run processMem function with ALLOCATE_REQ_TYPE 
                    memHeadPtr = processMem( memHeadPtr, currentProcess->processID,
                                  currentProcess->programCounter->intArg2,
                                     currentProcess->programCounter->intArg3,
                                                ALLOCATE_REQ_TYPE, configPtr );
                }

                //else if check for access
                else if (compareStr( currentProcess->programCounter->strArg1, 
                                                          "access" ) == STR_EQ)
                {
                    //display access request
                    display( MEM_REQ_CODE, timeStr, currentProcess, 
                                     configPtr, stringListHead, memHeadPtr, NEW ); 

                    //run processMem function with ALLOCATE_REQ_TYPE 
                    memHeadPtr = processMem( memHeadPtr, currentProcess->processID,
                                  currentProcess->programCounter->intArg2,
                                     currentProcess->programCounter->intArg3,
                                                ACCESS_REQ_TYPE, configPtr );
                }

                //check for segfault
                if (segFault)
                {
                    //display mem fail
                    display( MEM_FAIL_CODE, timeStr, currentProcess, configPtr,
                                              stringListHead, memHeadPtr, NEW);   

                    //set program counter to app end
                      //this terminates the process    
                    copyStr(currentProcess->programCounter->command, "app");    
                    copyStr(currentProcess->programCounter->strArg1, "end");  
                }
                //else no segfault
                else
                { 
                    //display mem success
                    display( MEM_SUCCESS_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );
                }

            }
     
        ///////////////////END MEMORY MANAGEMENT UNIT///////////////////////
        }
        /////////////////////////INTERRUPT HANDLER//////////////////////////

        if ((currentProcess->state == BLOCKED && !segFault) || 
             currentProcess->timeRemaining == NO_PROCESS) //cpu idle check
        {
            //get any interrupts available
            readyThread = interruptReady( threadHead, currentTime );

            //check if thread is actually ready
            if (readyThread != NULL)
            {
                //set current process to ready thread process
                currentProcess = 
                               getProcessFromThread( readyThread, PCBHeadPtr );

                //check if interrupt is cpu
                if (compareStr( currentProcess->programCounter->command, "cpu" ) 
                                                                          == STR_EQ)
                {
                    //we should not increment program counter
                    cpuEndedIdle = true;
                }

                //display operation of state change
                display( CHANGE_STATE_CODE, timeStr, currentProcess, configPtr, 
                                                stringListHead, memHeadPtr, READY);

                currentProcess->state = READY;

                //resolve thread
                PCBHeadPtr = resolveProcess( PCBHeadPtr, readyThread->pid, 
                                                        readyThread->endTime - readyThread->startTime);
                   
                //display current process selected with time remaining
                display(PROCESS_SELECT_CODE, timeStr, currentProcess, configPtr, 
                                              stringListHead, memHeadPtr, NEW);

                //display operation of state change
                display(CHANGE_STATE_CODE, timeStr, currentProcess, configPtr, 
                                           stringListHead, memHeadPtr, RUNNING);

                currentProcess->state = RUNNING;  

                //remove thread node
                threadHead = removeThread( threadHead, readyThread->pid );

                //resolve readyThread
                readyThread = NULL;
            }
            else  //no interrupt - get next ready process
            {
                //get next process
                currentProcess = getNextProcess( configPtr, PCBHeadPtr );

                //check for easy grab of next process
                if (currentProcess != NULL && 
                        currentProcess->timeRemaining != NO_PROCESS)
                {
                    //display current process selected with time remaining
                    display(PROCESS_SELECT_CODE, timeStr, currentProcess, 
                                  configPtr, stringListHead, memHeadPtr, NEW);

                    //display operation of state change
                    display(CHANGE_STATE_CODE, timeStr, currentProcess, configPtr, 
                                           stringListHead, memHeadPtr, RUNNING);

                    currentProcess->state = RUNNING;  //should we display this?
                }
                //check for cpu idle
                else if (currentProcess->timeRemaining == NO_PROCESS)
                {
                    //display cpu idle
                    display( CPU_IDLE_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );

                    //loop while threadReady is NULL
                    while (readyThread == NULL)
                    {
                        //get current time
                        currentTime = accessTimer( LAP_TIMER, timeStr );

                        //check for thread ready
                        readyThread = interruptReady( threadHead, currentTime );
                    }

                    //set current process to ready thread process
                    currentProcess = 
                               getProcessFromThread( readyThread, PCBHeadPtr );

                    //check if interrupt is cpu
                    if (compareStr( currentProcess->programCounter->command, "cpu" ) 
                                                                          == STR_EQ)
                    {
                        //we should not increment program counter
                        cpuEndedIdle = true;
                    }

                    //display operation of state change
                    display( CHANGE_STATE_CODE, timeStr, currentProcess, configPtr, 
                                                stringListHead, memHeadPtr, READY);

                    currentProcess->state = READY;

                    //display cpu interrupt
                    display( END_CPU_IDLE_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );

                    display( PROCESS_INTERRUPT_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );

                    //resolve thread
                    PCBHeadPtr = resolveProcess( PCBHeadPtr, readyThread->pid, 
                               readyThread->endTime - readyThread->startTime);
                   
                    //display current process selected with time remaining
                    display(PROCESS_SELECT_CODE, timeStr, currentProcess, 
                                  configPtr, stringListHead, memHeadPtr, NEW);

                    //display operation of state change
                    display(CHANGE_STATE_CODE, timeStr, currentProcess, configPtr, 
                                           stringListHead, memHeadPtr, RUNNING);

                    currentProcess->state = RUNNING;  

                    //remove thread node
                    threadHead = removeThread( threadHead, readyThread->pid );

                    //resolve readyThread
                    readyThread = NULL;
                }
            }
        }

        /////////////////////END INTERRUPT HANDLER//////////////////////////


        ///////////////////CHECK FOR END OF PROCESS////////////////////////
 
        //check for app end (start of new process)
        if ((compareStr( currentProcess->programCounter->command, "app" ) 
                                                                    == STR_EQ
              && compareStr( currentProcess->programCounter->strArg1, "end" )
                                                                    == STR_EQ))
        {
            //make sure process is terminating due to success
            if (!segFault)
            {
                display( PROCESS_COMPLETE_CODE, timeStr, currentProcess, configPtr, 
                                                stringListHead, memHeadPtr, NEW );
            }

            //check if process is using memory
            if( processExistsInMem( currentProcess->processID, memHeadPtr ))
            {
                //if so clear process memory 
                memHeadPtr = processMem(memHeadPtr, currentProcess->processID,
                                  currentProcess->programCounter->intArg2,
                                     currentProcess->programCounter->intArg3,
                                                   CLEAR_REQ_TYPE, configPtr);
                
                //display clear mem
                display( MEM_CLEAR_CODE, timeStr, currentProcess, 
                                 configPtr, stringListHead, memHeadPtr, NEW );
                
            }  

            //display operation of state change
            display( CHANGE_STATE_CODE, timeStr, currentProcess, configPtr, 
                                                stringListHead, memHeadPtr, EXIT);
   
            //set process to exit and display
            currentProcess->state = EXIT;

            //get next process
            currentProcess = getNextProcess( configPtr, PCBHeadPtr );

            //check if all process complete (getNext returns NULL)
            if (currentProcess == NULL) 
            {
                printf("All processes complete\n");
            }
            //otherwise, processes still need execution
            else if (currentProcess->timeRemaining != NO_PROCESS)  
            {
                //display current process selected with time remaining
                display( PROCESS_SELECT_CODE, timeStr, currentProcess, 
                                     configPtr, stringListHead, memHeadPtr, NEW );

                //display operation of state change
                display( CHANGE_STATE_CODE, timeStr, currentProcess, 
                                  configPtr, stringListHead, memHeadPtr, RUNNING);

                //set process state to running
                currentProcess->state = RUNNING;
            }
        }
        //if not end of process, simply increment program counter
        //if all processes complete do not enter
        else if(currentProcess != NULL && currentProcess->state != BLOCKED && !cpuEndedIdle)
        {
            //increment program counter to next node
            currentProcess->programCounter = 
                                    currentProcess->programCounter->nextNode;
        }

        //////////////////END OF CHECK FOR END OF PROCESS////////////////////

        //resolve booleans
        if (segFault)
        {
            segFault = false;
        }
        if (cpuEndedIdle)
        {
            cpuEndedIdle = false;
        }

    //end of master loop
    }

    ////////////////////////////END OF MASTER LOOP///////////////////////////

    /////////////////////////OUTPUT TO FILE//////////////////////////////////

    //set string list wkg to head for traversing
    stringListWkg = stringListHead;

    //check if we need to upload to file
    if (configPtr->logToCode == LOGTO_FILE_CODE || 
                                     configPtr->logToCode == LOGTO_BOTH_CODE)
    {
        //open file
        outputFile = fopen( configPtr->logToFileName, WRITE_ONLY_FLAG );

        //loop through string linked list
        while( stringListWkg != NULL )
        {
             //write string to file
               //function: fprintf
             fprintf( outputFile, stringListWkg->str );

             //increment to next string node
             stringListWkg = stringListWkg->nextNode;
        }
        fprintf(outputFile, 
                      "All processes complete\n\nSimulator Program End.\n\n");

        //close file
        fclose( outputFile );
    }

    /////////////////////////END OUTPUT TO FILE///////////////////////////////

    /////////////////////////FREE/RELEASE MEMORY//////////////////////////////

    //free string LLs
    freeStringList(stringListHead);
    freeMemList(memHeadPtr);
    freePCBList(PCBHeadPtr);

    //////////////////////END FREE/RELEASE MEMORY/////////////////////////////
}

/*
Name: addStringNode
Process: Loops until last node and appends new string node
Function Input/params: pointer to head of string LL (stringNode *)
                       pointer to string node to append (stringNode *)
Function Output/params: none
Function Output/returned: head of string LL (stringNode *)
Device Input: none
Device Output: none
Dependencies: tbd
*/
void addStringNode( stringNode *stringHead, char appendStr[] )
{
    //create string wkg pointer
    stringNode *stringWkg = stringHead;

    //loop until at end of string LL
    while (stringWkg->nextNode != NULL)
    {
        stringWkg = stringWkg->nextNode;
    }

    //give next node memory
    stringWkg->nextNode = (stringNode *)malloc(sizeof(stringNode));

    stringWkg->nextNode->nextNode = NULL;

    //set next node to appendNode
    copyStr( stringWkg->nextNode->str, appendStr );
}

/*
Name: addMemNode
Process: creates new memory node and fills in data
         loops through LL and inserts mem node where it fits
         (based on logical limits)
Function Input/params: pointer to head of mem LL (Memory *)
                       base request (int)
                       offset request (int)
                       process ID (int)
Function Output/params: head of mem LL (Memory *)
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: tbd
Note: this function is only called if mem should be added
      therefor no need to check if it fits
*/
Memory *addMemNode( Memory *memHeadPtr, int baseReq, int offsetReq, int pid)
{
    //boolean for inserting
    bool notInserted = true;

    //create working mem node for traversing
    Memory *memWkgPtr = memHeadPtr;

    //create a temp mem node for adding
    Memory *tempMemPtr;

    //create mem node and give memory
    Memory *addNode = (Memory *)malloc(sizeof(Memory));

    //set next node to NULL
    addNode->nextNode = NULL;

    //set values from parameters
    addNode->logicalLow = baseReq;
    addNode->logicalHigh = (baseReq + offsetReq) - 1;
    addNode->pNum = pid;
    addNode->UO = USED_MEM;

    //check if head is open mem
    if (memHeadPtr->UO == OPEN_MEM)
    {
        //check if offsetReq is less than nodes physical difference
        if (offsetReq < (memHeadPtr->physicalHigh - memHeadPtr->physicalLow))
        {
            //update head pointer (change physical address) 
            memHeadPtr->physicalLow = offsetReq;

            //set addNode's nextNode = tempNode
            addNode->nextNode = memHeadPtr;

            //set physical adresses of new node
            addNode->physicalLow = 0;
            addNode->physicalHigh = offsetReq - 1;

            //return new head pointer (addNode)
            return addNode;
        }

    }

    //loop through mem LL until insert is made 
    while (notInserted)
    {
        //check for next node is open mem
        if( memWkgPtr->nextNode->UO == OPEN_MEM )
        {
            //check if offsetReq is less than next nodes physical difference
            if (offsetReq < (memWkgPtr->nextNode->physicalHigh -  
                                           memWkgPtr->nextNode->physicalLow))
            {
                //set temp mem = wkg's next node
                tempMemPtr = memWkgPtr->nextNode;

                //set wkg's next node = newly created node
                memWkgPtr->nextNode = addNode;

                //set newly created node's next node to temp node
                addNode->nextNode = tempMemPtr;

                //set physical address of new node
                addNode->physicalLow = memWkgPtr->physicalHigh + 1;
                addNode->physicalHigh = addNode->physicalLow + offsetReq - 1;

                //update temp mem (change physical address)
                tempMemPtr->physicalLow = addNode->physicalHigh + 1;

                //set inserted to true
                notInserted = false;
            }
        }
        //increment to next node    
        memWkgPtr = memWkgPtr->nextNode;
    }

    //return head of mem LL
    return memHeadPtr;
}

/*
Name: addThreadNode
Process: creates new thread node, fills in data, appends node to list
Function Input/params: pointer to head of thread LL (Thread *)
                       base request (int)
                       offset request (int)
                       process ID (int)
Function Output/params: head of thread LL (Memory *)
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: tbd
*/
Thread *addThreadNode( Thread *threadHeadPtr, double start, double end, int pid)
{
    //create threadWkg and set to head
    Thread *threadWkgPtr = threadHeadPtr;

    //create new thread node
    Thread *addThread = (Thread *)malloc(sizeof(Thread));
    addThread->nextNode = NULL;

    //set parameter values
    addThread->startTime = start;
    addThread->endTime = end;
    addThread->pid = pid;

    //check if head is empty
    if (threadHeadPtr == NULL || threadHeadPtr->startTime == NO_PROCESS)
    {
        //return new thread
        return addThread;
    }

    //loop until wkg next node is null
    while (threadWkgPtr->nextNode != NULL)
    {
        //increment wkg to next pointer
        threadWkgPtr = threadWkgPtr->nextNode;
    }

    //set next node of wkg to new thread node 
    threadWkgPtr->nextNode = addThread;

    //return head pointer   
    return threadHeadPtr;
}

/*
Name: createPCBs
Process: iterates through opCode creating processes from app start to app end 
         sets pcb members and calculates time remaining for each process.
Function Input/params: config data (ConfigDataType *),
                       metadata (OpCOdeType *)
Function Output/params: head ptr to pcb linked list (PCBDataType *)
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: compareStr
*/
PCBDataType *createPCBs(OpCodeType *metaDataHeadPtr, ConfigDataType *configPtr)                         
{
    //initialize vars 
    int pid = 0;

    OpCodeType *localOpPtr = metaDataHeadPtr;
    
    //create pcb head pointer
    PCBDataType *PCBHeadPtr = NULL;

    //create localPtr for PCB linked list
    PCBDataType *localPCBNode = NULL;

    //loop across Op Code until temp meta data is null
    while ( !(compareStr( localOpPtr->command, "sys" ) == STR_EQ
              && compareStr( localOpPtr->strArg1, "end" ) == STR_EQ) )
    {

        //check for app start
        if ( compareStr( localOpPtr->command, "app" ) == STR_EQ
              && compareStr( localOpPtr->strArg1, "start" ) == STR_EQ )
        {
            //if first process, set head pointer and give memory 
            if (pid == 0)
            {
                //give pcb memory
                localPCBNode = (PCBDataType *)malloc(sizeof(PCBDataType));

                localPCBNode->timeRemaining = 0;

                //set next node to null
                localPCBNode->nextNode = NULL;

                //set a head pointer
                PCBHeadPtr = localPCBNode;
            }
            else
            {
                //give next node memory
                localPCBNode->nextNode = 
                                   (PCBDataType *)malloc(sizeof(PCBDataType));

                localPCBNode->nextNode->nextNode = NULL;

                localPCBNode->nextNode->timeRemaining = 0;

                //increment to next node
                localPCBNode = localPCBNode->nextNode;
            }

            //set process id to pid
            localPCBNode->processID = pid;

            //increment pid
            pid++;

            //set process state to new
            localPCBNode->state = NEW;

            localPCBNode->programCounter = localOpPtr;
        }

        //check for cpu 
        else if ( compareStr( localOpPtr->command, "cpu" ) == STR_EQ )
        {
            //set time remaining of new process
            localPCBNode->timeRemaining += 
                              localOpPtr->intArg2 * configPtr->procCycleRate;
        }

        //check for io
        else if ( compareStr( localOpPtr->command, "dev" ) == STR_EQ )
        {
            //set time remaining of new process
            localPCBNode->timeRemaining += 
                               localOpPtr->intArg2 * configPtr->ioCycleRate;
        }

        //check for mem

        //go to next line with localOpPtr
        localOpPtr = localOpPtr->nextNode;  
    }
    return PCBHeadPtr;
}

//combines sequential open mem nodes
void compressMem( Memory *memHeadPtr)
{
    //create working ptr for traversing
    Memory *memWkgPtr = memHeadPtr;
 
    //create a bool to signify action made
    bool compressed = false;

    //loop through memory LL
    while( memWkgPtr->nextNode != NULL )
    {
        //check if node is open
        if (memWkgPtr->UO == OPEN_MEM)
        {
            //check if next node is open
            if (memWkgPtr->nextNode->UO == OPEN_MEM)
            {
                //update current node physical address
                memWkgPtr->physicalHigh = memWkgPtr->nextNode->physicalHigh;

                if (memWkgPtr->nextNode->nextNode != NULL)
                {
                    //unlink next node
                    memWkgPtr->nextNode = memWkgPtr->nextNode->nextNode;
                }
                else
                {
                    memWkgPtr->nextNode = NULL;
                }
                compressed = true;
            }
        }

        if (memWkgPtr->nextNode != NULL) 
        {
            //increment to next pointer
            memWkgPtr = memWkgPtr->nextNode;
        }

    }

    //check if action was made
    if (compressed)
    {
        //if so recurse 
        compressMem( memHeadPtr );
    }

}

/*
Name: display
Process: displays print statements either to file or to monitor 
         Uses control code to determine what to print
 
Function Input/params: Control code for which statement to print (PrintCodes)
                       time string (char *)
                       pointer to PCB (PDBDataType *)
                       new process state for changing state (processState)
                       old process state for changing state (processState)

Function Output/params: Pointer to PCB (PDBDataType *)
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: tbd
*/
void display(PrintCodes printCode, char *timeStr, PCBDataType *PCBPtr, 
                     ConfigDataType *configPtr, stringNode *stringHeadPtr,
                                     Memory *memHeadPtr, processState newState)
{
    //create string for sprintf
    char currentString[MAX_STR_LEN];

    char stateList[5][STD_STR_LEN] = { "NEW", "READY", "RUNNING", "BLOCKED",
                                                                      "EXIT" };

    //get current time for change state
    accessTimer( LAP_TIMER, timeStr );

    switch( printCode )
    {
        case SIM_START_CODE :

            sprintf(currentString, "%s, OS: Simulator start\n", timeStr);

            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

        case CHANGE_STATE_CODE :

            sprintf(currentString, 
                          "%s, OS: Process %d set to %s state from %s state\n", 
                           timeStr, PCBPtr->processID, stateList[newState], 
                                                   stateList[PCBPtr->state] );
         
            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

        case PROCESS_SELECT_CODE :

            sprintf(currentString, 
                     "%s, OS: Process %d selected with %d ms remaining \n\n", 
                            timeStr, PCBPtr->processID, PCBPtr->timeRemaining);

            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

        case OP_START_CODE :
            if (compareStr( PCBPtr->programCounter->command, "dev" ) == STR_EQ)
            {
                sprintf(currentString, 
                               "%s, Process %d, %s %sput operation start\n",
                                           timeStr, PCBPtr->processID, 
                                           PCBPtr->programCounter->strArg1,
                                           PCBPtr->programCounter->inOutArg);

                displayAccordingly( currentString, configPtr, stringHeadPtr );
            }

            else if (compareStr( PCBPtr->programCounter->command, "cpu" ) 
                                                                     == STR_EQ)
            {
                sprintf(currentString, 
                             "%s, Process %d, cpu process operation start\n", 
                                                   timeStr, PCBPtr->processID);

                displayAccordingly( currentString, configPtr, stringHeadPtr );
            }
            break;

        case OP_END_CODE :
            if (compareStr( PCBPtr->programCounter->command, "dev" ) == STR_EQ)
            {
                sprintf(currentString, 
                            "%s, Process %d, %s %sput operation end\n",
                                           timeStr, PCBPtr->processID, 
                                           PCBPtr->programCounter->strArg1,
                                           PCBPtr->programCounter->inOutArg);

                displayAccordingly( currentString, configPtr, stringHeadPtr );
            }
            else if (compareStr( PCBPtr->programCounter->command, "cpu" ) 
                                                                     == STR_EQ)
            {
                sprintf(currentString, 
                               "%s, Process %d, cpu process operation end\n", 
                                                   timeStr, PCBPtr->processID);

                displayAccordingly( currentString, configPtr, stringHeadPtr );
            }

            break;

        case PROCESS_COMPLETE_CODE :

            sprintf(currentString, "\n%s, OS: Process %d complete\n",
                                                timeStr, PCBPtr->processID);

            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

        case MEM_REQ_CODE :
            sprintf(currentString, 
                            "%s, Process %d, mem %s request (%d, %d)\n",
                                           timeStr, PCBPtr->processID, 
                                           PCBPtr->programCounter->strArg1,
                                           PCBPtr->programCounter->intArg2,
                                           PCBPtr->programCounter->intArg3);

            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;
           
        case MEM_INIT_CODE :

            if (configPtr->memDisplay)
            {
                sprintf(currentString, 
                "\nAfter memory initialization\n%d [ Open, P#: X, %d-%d ] %d\n\n", 
                                memHeadPtr->physicalLow, memHeadPtr->logicalLow, 
                                memHeadPtr->logicalHigh, memHeadPtr->physicalHigh);
          
                displayAccordingly( currentString, configPtr, stringHeadPtr );
            }
                
            break;

        case MEM_SUCCESS_CODE :
            if (configPtr->memDisplay)
            {
                //sprintf (after allocate/access/clear success)
                sprintf(currentString, "\nAfter %s success\n", 
                                              PCBPtr->programCounter->strArg1);

                //display accordingly 
                displayAccordingly( currentString, configPtr, stringHeadPtr );

                //display current memory 
                displayMem( memHeadPtr, configPtr, stringHeadPtr );

                sprintf(currentString, 
                      "%s, Process %d, succesful mem %s request\n", 
                            timeStr, PCBPtr->processID, 
                                             PCBPtr->programCounter->strArg1);
            }
            break;

        case MEM_FAIL_CODE :
            if (configPtr->memDisplay)
            {
                //sprintf (After allocate/access faliure)
                sprintf(currentString, "\nAfter %s failure\n", 
                                              PCBPtr->programCounter->strArg1);

                //display accordingly 
                displayAccordingly( currentString, configPtr, stringHeadPtr );

                //display current memory
                displayMem( memHeadPtr, configPtr, stringHeadPtr );

                //display failed mem allocate/access request
                sprintf(currentString, 
                      "%s, Process %d, failed mem %s request\n", 
                            timeStr, PCBPtr->processID, 
                                             PCBPtr->programCounter->strArg1);

                displayAccordingly( currentString, configPtr, stringHeadPtr );

  
                //display segault message
                sprintf(currentString, 
                      "\n%s, OS: Segmentation fault, Process %d ended\n", 
                                                  timeStr, PCBPtr->processID);

                displayAccordingly( currentString, configPtr, stringHeadPtr );

            }

            break;

        case MEM_CLEAR_CODE :
            if (configPtr->memDisplay)
            {
                //sptrinf (after clear process # success)
                //can use currentprocess for P#
                sprintf(currentString, "\nAfter clear process %d success\n", 
                                                           PCBPtr->processID);

                //display accordingly 
                displayAccordingly( currentString, configPtr, stringHeadPtr );

                //display current memory
                displayMem( memHeadPtr, configPtr, stringHeadPtr );

            }
            break;

        case PROCESS_BLOCKED_CODE :
            sprintf(currentString, 
                          "%s, OS: Process %d blocked for %sput operation\n", 
                                                  timeStr, PCBPtr->processID, 
                                             PCBPtr->programCounter->inOutArg);
         
            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

        case PROCESS_INTERRUPT_CODE :
            if (compareStr( PCBPtr->programCounter->command, "dev" ) == STR_EQ)
            {
                sprintf(currentString, 
                    "%s, OS: Interrupted by process %d, completed %s %sput operation\n", 
                                                   timeStr, PCBPtr->processID,
                                              PCBPtr->programCounter->strArg1, 
                                              PCBPtr->programCounter->inOutArg);
         
                displayAccordingly( currentString, configPtr, stringHeadPtr );
            }
            else if (compareStr( PCBPtr->programCounter->command, "cpu" ) 
                                                                     == STR_EQ)
            {
                sprintf(currentString, 
                    "%s, OS: Interrupted by process %d, unfinished cpu operation\n", 
                                                   timeStr, PCBPtr->processID);
         
                displayAccordingly( currentString, configPtr, stringHeadPtr );
            }

            break;

        case CPU_INTERRUPTED_CODE :
            sprintf(currentString, "%s, OS: CPU operation interrupted\n", 
                                                                  timeStr);
         
            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;


        case CPU_IDLE_CODE :
            sprintf(currentString, 
                          "%s, OS: CPU idle, all active processes blocked\n\n", 
                                                                     timeStr);
         
            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

        case END_CPU_IDLE_CODE :
            sprintf(currentString, "%s, OS: CPU interrupt, end idle\n", 
                                                                     timeStr);
         
            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

        case QUANTUM_OUT_CODE :
            sprintf(currentString, 
                          "%s, OS: Quantum time out, Process %d blocked\n", 
                                                  timeStr, PCBPtr->processID);
         
            displayAccordingly( currentString, configPtr, stringHeadPtr );

            break;

    }
}

/*
Name: displayAccordingly
Process: either displays print statement to monitor OR
         adds string to LL
Function Input/params: string to display (char[])
                       pointer to config data (ConfigDataType *)
                       pointer to head of string LL (stringNode *)
Function Output/params: Pointer to PCB (PDBDataType *)
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: copyStr
*/
void displayAccordingly( char Str[], ConfigDataType *configPtr, 
                                                    stringNode *stringHead )
{

    if (configPtr->logToCode == LOGTO_MONITOR_CODE || 
                                       configPtr->logToCode == LOGTO_BOTH_CODE)
    {
        printf("%s", Str);
    }
    if (configPtr->logToCode == LOGTO_FILE_CODE || 
                                     configPtr->logToCode == LOGTO_BOTH_CODE)
    {
        addStringNode(stringHead, Str);
    }
    
}

/*
Name: displayMem
Process: Loops through memory linked list and displays each node
 
Function Input/params: pointer to head of mem LL (Memory *)
                       pointer to configuration data (ConfigDataType *)
                       pointer to head of string LL (stringNode *)

Function Output/params: none
Function Output/returned: none
Device Input: none
Device Output: none
Dependencies: displayAccordingly 
*/
void displayMem( Memory *memHeadPtr, ConfigDataType *configPtr, 
                                                  stringNode *stringHeadPtr )
{
    //create working mem to traverse ll
    Memory *progMemWkg = memHeadPtr;

    //create string for sprintf
    char currentString[MAX_STR_LEN];

    //loop through memHeadPtr LL
    while (progMemWkg != NULL)
    {
        //check for used mem
        if (progMemWkg->UO == USED_MEM)
        {
            sprintf( currentString, "%d [ Used, P#: %d, %d-%d ] %d\n", 
            progMemWkg->physicalLow,  progMemWkg->pNum, progMemWkg->logicalLow, 
                            progMemWkg->logicalHigh, progMemWkg->physicalHigh);
        }
        //else is open
        else
        {
            sprintf( currentString, "%d [ Open, P#: X, 0-0 ] %d\n", 
                            progMemWkg->physicalLow, progMemWkg->physicalHigh);

        }

        //display accordingly
        displayAccordingly( currentString, configPtr, stringHeadPtr );

        //increment to next pointer
        progMemWkg = progMemWkg->nextNode; 
    }

    sprintf(currentString, "\n");
    displayAccordingly( currentString, configPtr, stringHeadPtr );

}

/*
Name: freeStringList
Process: iterates through string LL freeing each node 
*/
void freeStringList(stringNode *head)
{
    stringNode *tmp;

    while (head != NULL)
    {
       tmp = head;
       head = head->nextNode;
       free(tmp);
    }
}

/*
Name: freeMemList
Process: iterates through Mem LL freeing each node 
*/
void freeMemList(Memory *head)
{
    Memory *tmp;

    while (head != NULL)
    {
       tmp = head;
       head = head->nextNode;
       free(tmp);
    }
}

/*
Name: freePCBList
Process: iterates through PCB LL freeing each node 
*/
void freePCBList(PCBDataType *head)
{
    PCBDataType *tmp;

    while (head != NULL)
    {
       tmp = head;
       head = head->nextNode;
       free(tmp);
    }
}

/*
Name: freeThreadList
Process: iterates through PCB LL freeing each node 
*/
void freeThreadList(Thread *head)
{
    Thread *tmp;

    while (head != NULL)
    {
       tmp = head;
       head = head->nextNode;
       free(tmp);
    }
}

/*
Name: getNextProcess
Process: gets the next process in queue depending on scheduling algorithm
 
Function Input/params: pointer to config data (ConfigDataType *)
                       pointer to Head of PCB LL (PDBDataType *)
Function Output/params: none
Function Output/returned: Pointer to PCB (PDBDataType *)
Device Input: none
Device Output: none
Dependencies: tbd
*/
PCBDataType *getNextProcess( ConfigDataType *configPtr, PCBDataType *PCBHeadPtr)
{
    //create working pointer and set to head
    PCBDataType *PCBWkgPtr = PCBHeadPtr;

    //create a current min PCB variable and set to PCB Head
    PCBDataType *currentSelection = NULL;

    //boolean and PCB for cpu idle
    bool cpuIdle = false;
    PCBDataType *cpuIdlePCB = NULL;
    
    //check for SJF-N
    if (configPtr->cpuSchedCode == CPU_SCHED_SJF_N_CODE ||
        configPtr->cpuSchedCode == CPU_SCHED_SRTF_P_CODE)
    {
        //loop through PCB LL
        while( PCBWkgPtr != NULL )
        {
            //check if is NOT in BLOCKED, EXIT, or WAITING state
            if (PCBWkgPtr->state != BLOCKED && PCBWkgPtr->state != EXIT)
            {
                //check if process not yet selected
                if (currentSelection == NULL)
                {
                    //set current min to PCB
                    currentSelection = PCBWkgPtr;
                }

                //check if PCB's time remaining less than current min
                if( PCBWkgPtr->timeRemaining < currentSelection->timeRemaining )
                {
                    //set current min to PCB
                    currentSelection = PCBWkgPtr;
                }
            }

            //increment to next node in LL
            PCBWkgPtr = PCBWkgPtr->nextNode;
        }
    }

    //check for FCFS or RR-P 
    if (configPtr->cpuSchedCode == CPU_SCHED_FCFS_N_CODE ||
        configPtr->cpuSchedCode == CPU_SCHED_FCFS_P_CODE ||        
        configPtr->cpuSchedCode == CPU_SCHED_RR_P_CODE)
    {

        //loop through PCB LL
        while(PCBWkgPtr != NULL)
        {
            //check if is NOT in BLOCKED, EXIT, or WAITING state
            if (PCBWkgPtr->state != BLOCKED && 
                PCBWkgPtr->state != EXIT )
            {
                //return PCB
                return PCBWkgPtr;
            }

            //increment to next node in LL
            PCBWkgPtr = PCBWkgPtr->nextNode;
        }
    }
   
    //if nothing selected, all processes are either blocked or exited
    if (currentSelection == NULL)
    {
        //we must check to make sure all are exit or else cpu idle

        PCBWkgPtr = PCBHeadPtr;
        
        //loop through PCB LL
        while(PCBWkgPtr != NULL)
        {
            //check if blocked
            if (PCBWkgPtr->state == BLOCKED)
            {
                //return PCB
                cpuIdle = true;
            }

            //increment to next node in LL
            PCBWkgPtr = PCBWkgPtr->nextNode;
        }
    }

    //if cpu idle return PCB with cpu idle identifyer
    if (cpuIdle)
    {
        cpuIdlePCB = (PCBDataType *)malloc(sizeof(PCBDataType));
        cpuIdlePCB->state = NEW;
        cpuIdlePCB->processID = NO_PROCESS; 
        cpuIdlePCB->programCounter = NULL;         
        cpuIdlePCB->timeRemaining = NO_PROCESS;    //cpu Idle identity
        cpuIdlePCB->nextNode = NULL;
        
        return cpuIdlePCB;
    }

    //return currentSelection 
    return currentSelection;
}

//returns process associated with thread
PCBDataType *getProcessFromThread( Thread *threadHead, PCBDataType *PCBHead )
{
    //create working PCB and set to head
    PCBDataType *PCBWkg = PCBHead;

    //loop through pcbs until pid matches thread's
    while (PCBWkg->processID != threadHead->pid)
    {
        //increment to next pointer
        PCBWkg = PCBWkg->nextNode;
    }

    //return pcb 
    return PCBWkg;
}

/*
loops through threads and returns first completed one
returns NULL if no threads ready
*/
Thread *interruptReady (Thread* threadHeadPtr, double currentTime)
{
    //create working and set to head
    Thread *threadWkgPtr = threadHeadPtr;

    //loop through thread LL
    while (threadWkgPtr != NULL)
    {
        //check if endtime is less than or equal to current time
        if (threadWkgPtr->endTime <= currentTime)
        {  
            //return Thread
            return threadWkgPtr;
        }

        //else increment to next node
        threadWkgPtr = threadWkgPtr->nextNode;
    }
    //return NULL
    return NULL;
}


//Returns true if process is using memory, false if not
bool processExistsInMem( int pid, Memory *memHeadPtr )
{
    Memory *memWkgPtr = memHeadPtr;

    while (memWkgPtr != NULL)
    {
        if (memWkgPtr->pNum == pid)
        {
            return true;
        }
        memWkgPtr = memWkgPtr->nextNode;
    }

    return false;
}


Memory *processMem( Memory *memHeadPtr, int pid, int baseReq, 
                                 int offsetReq, MemRequests reqType, 
                                                    ConfigDataType *configPtr )
{    
    bool actionMade = false;

    //create a mem pointer and set to head
    Memory *memWkgPtr = memHeadPtr;

    switch (reqType )
    {
        //check for initialization
        case INIT_REQ_TYPE :
            //set deafult values to head
            memHeadPtr->logicalLow = 0;
            memHeadPtr->logicalHigh = 0;
            memHeadPtr->physicalLow = 0;
            memHeadPtr->physicalHigh = configPtr->memAvailable - 1;
            memHeadPtr->UO = OPEN_MEM;
            memHeadPtr->pNum = NO_PROCESS;    

            return memHeadPtr;

        //check for access 
        case ACCESS_REQ_TYPE :
            segFault = true;

            //loop from head of LL until NULL
            while( memWkgPtr != NULL )
            {
                //check for correct process
                if( memWkgPtr->pNum == pid)
                {
                    //check if base request between logical low and high 
                    //AND check if base+offset between logical low and high
                    if ((baseReq >= memWkgPtr->logicalLow 
                              && (baseReq+offsetReq-1) <= memWkgPtr->logicalHigh))
                    {
                        segFault = false;
                    }
                }

                //increment to next node in LL
                memWkgPtr = memWkgPtr->nextNode;
            }

            //return head pointer 
            return memHeadPtr;

        //check for allocate
        case ALLOCATE_REQ_TYPE :
            segFault = true;

            //loop from head of LL until NULL
            while( memWkgPtr != NULL )
            {
                //check for open memory
                if (memWkgPtr->UO == OPEN_MEM)
                {
                    //check if base request between physical low and high 
                    //and check if base+offset between logical low and high
                    if (offsetReq <= 
                           (memWkgPtr->physicalHigh - memWkgPtr->physicalLow))
                    {
                        //set segfault to false
                        segFault = false;
                    }
                }

                //increment to next node in LL
                memWkgPtr = memWkgPtr->nextNode;
            }

            //check if segfault is true
            if( segFault )
            {
                //return false
                return memHeadPtr;
            }
            //else allocation is possible
            else
            {
                //insert node into correct position of mem LL
                memHeadPtr = addMemNode( memHeadPtr, baseReq, offsetReq, pid);

                //return success
                return memHeadPtr;
            }

        //check for clear process
        case CLEAR_REQ_TYPE :
            //loop until process matches parameter 
            while( memWkgPtr != NULL )
            {
                //check if at correct process to clear
                if ( memWkgPtr->pNum == pid)
                {
                    //set logical vals to 0 and set open
                    memWkgPtr->logicalLow = 0;
                    memWkgPtr->logicalHigh = 0;
                    memWkgPtr->UO = OPEN_MEM;
                    memWkgPtr->pNum = NO_PROCESS;

                    actionMade = true;
                }
                memWkgPtr = memWkgPtr->nextNode;
            }
            
            //if action was made recurse 
            //this beats case of mulitple mem blocks of same process
            if (actionMade)
            {
                memHeadPtr = processMem( memHeadPtr, pid, baseReq, offsetReq, 
                                                          reqType, configPtr );
            }

            //call compress memory to remove sequential open nodes 
            compressMem( memHeadPtr );

            //return success;
            return memHeadPtr;
    }

    //default return
    return memHeadPtr; 
}

//sets blocked process to ready and updates timeRemaining 
PCBDataType *resolveProcess( PCBDataType *PCBHeadPtr, int pid, double completedTime )
{
    //create working pointer and set to head
    PCBDataType *PCBWkgPtr = PCBHeadPtr;

    //loop while pcb's id is not equal to pid
    while (PCBWkgPtr->processID != pid)
    {
        //increment to next pointer
        PCBWkgPtr = PCBWkgPtr->nextNode;
    }

    //set process to ready
    PCBWkgPtr->state = READY; 

    if (compareStr( PCBWkgPtr->programCounter->command, "dev" ) == STR_EQ )
    {
        //subtract completedTime from timeRemaining 
        PCBWkgPtr->timeRemaining -= (int)(completedTime * 1000);
    }

    return PCBHeadPtr;
}


//removes a thread from LL and reconnects
Thread *removeThread( Thread *head, int pid )
{
    Thread *wkg = head;

    //check if head is remove node
    if (head->pid == pid)
    {
        wkg = wkg->nextNode;
        head->nextNode = NULL;
        return wkg;
    }

    //get to the node before the node to remove
    while (wkg->nextNode->pid != pid)
    {
        wkg = wkg->nextNode;
    }

    //check if node after remove node exists
    if (wkg->nextNode->nextNode != NULL)
    {
        //unlink next node
        wkg->nextNode = wkg->nextNode->nextNode;
    }
    else
    {
        wkg->nextNode = NULL;
    }
    return head;
}


