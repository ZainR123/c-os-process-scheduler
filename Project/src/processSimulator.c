#include "coursework.h"
#include "linkedlist.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

//Create semaphores so threads are locked from running until they have been posted after a thread has completed a certain task
sem_t newPID, freePID;

//Counters to keep track of the state of each thread so loops break once all processes have gone through all their states
int processesGen, processesAdmit, processesTerm, processesRan;
int processPreempted = 0;

//Break off priority level for where RR is used instead of FCFS
int lowerPriorityRR = 16;

//Time value from when program is started
struct timeval oBaseTime;

//Creates free PID list in which PIDs will be removed and used to generate new processes
struct element *freePidHead, *freePidTail;

//Creates a new queue list for newly generated processes to be added to
struct element *newQueueHead, *newQueueTail;

//Creates a ready queue array of 32 lists which holds multiple processes in different lists depending on their priority level
struct element *readyQueueHead[MAX_PRIORITY], *readyQueueTail[MAX_PRIORITY];

//Creates a terminated queue list for processes which have reached the end of their runtime cycle
struct element *terminatedQueueHead, *terminatedQueueTail;

//Creates PCB array which is populated by processes containing all their data
//It's of size specified in coursework.h
struct process *processTable[SIZE_OF_PROCESS_TABLE];

struct process *currentThread[NUMBER_OF_CPUS];

//Prototyping to declare my functions without providing the body right away
//Allows main to be situated at the top of my file
int milliSleep(long ms);

void *processGenerator();

void *longTerm();

void *shortTerm(void *currentCPU);

void *boosterDaemon();

void *terminationDaemon();

void printHeadersSVG();

void printProcessSVG(int iCPUId, struct process *pProcess, struct timeval oStartTime, struct timeval oEndTime);

void printPrioritiesSVG();

void printRasterSVG();

void printFootersSVG();

//Main runs the scheduler initializing threads, the free PID list and semaphores then doing memory management clearing all threads and populated lists
int main() {

    //Declaration of threads for each component of the scheduler
    //Short term scheduler can have more than one thread as there can be multiple CPUs so need a separate thread for each
    pthread_t processGeneratorThread, shortTermThread[NUMBER_OF_CPUS], longTermThread, boosterDaemonThread, terminationDaemonThread;

    //Print SVG formatting for HTML
    printHeadersSVG();
    printPrioritiesSVG();
    printRasterSVG();

    //Populate free PID list with values up to the total size of the PCB specified in coursework.h
    for (int i = 0; i < SIZE_OF_PROCESS_TABLE; i++) {

        addLast((void *) (size_t) i, &freePidHead, &freePidTail);
    }

    //Initialize semaphores
    sem_init(&freePID, 0, 1);
    sem_init(&newPID, 0, 0);

    //Create thread for the process generator assigning a function to it
    pthread_create(&processGeneratorThread, NULL, processGenerator, NULL);

    //Semaphore waits till all processes have generated in the process generator where it is then posted
    //Allowing processes to be admitted in the long term scheduler
    sem_wait(&newPID);

    //Create thread for the long term scheduler assigning a function to it
    pthread_create(&longTermThread, NULL, longTerm, NULL);

    //Semaphore waits till all processes have been admitted in the long term scheduler where it is then posted
    //Allowing processes to be ran in the short term scheduler
    sem_wait(&newPID);

    //Sets the base time of the program to right before the short term scheduler is run so HTML prints without empty space
    gettimeofday(&oBaseTime, NULL);

    //For the number of specified CPUs, create the same number of threads assigning the same function to all created threads
    for (int i = 0; i < NUMBER_OF_CPUS; i++) {

        //Short term scheduler thread passes the current CPU value as an argument
        //So it can be accessed by the thread function to print the consumer value
        pthread_create(&shortTermThread[i], NULL, shortTerm, (void *) (size_t) (i + 1));
    }

    //Create thread for the booster daemon assigning a function to it
    pthread_create(&boosterDaemonThread, NULL, boosterDaemon, NULL);

    //Create thread for the termination daemon assigning a function to it
    pthread_create(&terminationDaemonThread, NULL, terminationDaemon, NULL);

    //After the process generator thread has finished generating all it's processes, the thread is joined, terminating it and freeing it's memory
    pthread_join(processGeneratorThread, NULL);

    //After the long term scheduler thread has finished admitting all it's processes, the thread is joined, terminating it and freeing it's memory
    pthread_join(longTermThread, NULL);

    //For the number of specified CPUs, join the corresponding short term scheduler threads
    for (int i = 0; i < NUMBER_OF_CPUS; i++) {

        //After the short term scheduler threads have finished running all their processes, the threads are joined, terminating them and freeing their memory
        pthread_join(shortTermThread[i], NULL);
    }

    //After the booster daemon thread has finished boosting processes, the thread is joined, terminating it and freeing it's memory
    pthread_join(boosterDaemonThread, NULL);

    //After the termination daemon thread has finished terminating all it's processes, the thread is joined, terminating it and freeing it's memory
    pthread_join(terminationDaemonThread, NULL);

    //For the size of the process table remove elements from the free PID list
    for (int i = 0; i < SIZE_OF_PROCESS_TABLE; i++) {

        //Frees all data within the free PID list to make sure all "still reachable" memory is cleared before returning
        removeFirst(&freePidHead, &freePidTail);
    }

    //Prints SVG footer for HTML
    printFootersSVG();
}

//Function which allows for sleep inputs as milliseconds using nanoSleep
//nanoSleep most accurate form of sleep
//Used as uSleep is a deprecated function
int milliSleep(long ms) {

    //Creates timespec which is used to calculate output
    struct timespec timeSpec;

    //Converts millisecond input into seconds
    timeSpec.tv_sec = ms / 1000;

    //Converts millisecond input into nanoseconds
    timeSpec.tv_nsec = (ms % 1000) * 1000000;

    //Sleep thread for calculated time
    return nanosleep(&timeSpec, &timeSpec);
}

//Function which generates all processes
void *processGenerator() {

    //While the number of processes generated so far is less than the total number of processes specified to be ran...
    while (processesGen < NUMBER_OF_PROCESSES) {

        //Semaphore will halt loop till it's been posted by the termination daemon
        //After all processes in the termination queue have been moved to the free PID list
        sem_wait(&freePID);

        //While the free PID list is not empty and not all processes have been generated...
        while (freePidHead != NULL && freePidTail != NULL && (processesGen < NUMBER_OF_PROCESSES)) {

            //Create a struct process which generates a new process from the first element in the free PID list
            struct process *generate = generateProcess(removeFirst(&freePidHead, &freePidTail));

            //set the PCB value at the generated processes PID to the newly created process
            processTable[(size_t) (generate->pPID)] = generate;

            //Print details about the process that has been generated
            printf("TXT: Generated: Process Id = %zu, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",
                   (size_t) generate->pPID, generate->iPriority, generate->iPreviousBurstTime, generate->iRemainingBurstTime);

            //Add the generated process' PID to the end of the new queue list
            addLast(generate->pPID, &newQueueHead, &newQueueTail);

            //Add to the total number of processes generated so far in the process generator
            processesGen++;
        }

        //Post semaphore to allow long term scheduler thread to be created in Main so it can admit these newly generated processes
        //Only ran once
        sem_post(&newPID);
    }

    //Return NULL once thread is complete as function isn't void
    return NULL;
}

void *longTerm() {

    //While the number of processes admitted so far is less than the total number of processes
    while (processesAdmit < NUMBER_OF_PROCESSES) {

        //Sleep for a predetermined time to allow for all processes to be generated
        milliSleep(LONG_TERM_SCHEDULER_INTERVAL);

        //While the new queue is not empty...
        while (newQueueHead != NULL && newQueueTail != NULL) {

            //Create struct process which is set to the PCB data at the PID pointed by the first element of the new queue
            struct process *admit = processTable[(size_t) removeFirst(&newQueueHead, &newQueueTail)];

            //Print details about the process that has been admitted
            printf("TXT: Admitted: Process Id = %zu, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",
                   (size_t) admit->pPID, admit->iPriority, admit->iPreviousBurstTime, admit->iRemainingBurstTime);

            //Add the admitted process' PID to the end of the corresponding ready queue array list depending on it's priority level
            addLast(admit->pPID, &readyQueueHead[admit->iPriority], &readyQueueTail[admit->iPriority]);

            //Add to the total number of processes admitted so far in the long term scheduler

            for(int i = 0; i < NUMBER_OF_CPUS; i++)
            {
                if((currentThread[i] != NULL) && (currentThread[i]->iPriority > admit->iPriority)) {
                    preemptJob(currentThread[i]); //preempt for FCFS
                    printf("Hello, Preempted Job with PID: %zu\n", (size_t) admit->pPID);
                    processPreempted = 1;
                }
            }
            processesAdmit++;
        }

        //Post semaphore to allow short term scheduler thread to be created in Main so it can run these admitted processes
        //Only ran once
        sem_post(&newPID);
    }

    //Return NULL once thread is complete as function isn't void
    return NULL;
}

//Function runs processes after being moved from the new queue to the ready queue
void *shortTerm(void* currentCPU) {

    //Declare and initialize mutex as multiple threads have access to this one function
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    //Create struct process which will store PCB process data
    struct process *ready;

    //Create time intervals for calculating time difference for response and turnaround times
    struct timeval oStartTime, oEndTime;

    //While the number of processes ran so far is less than the total number of processes...
    while (processesRan < NUMBER_OF_PROCESSES) {

        //For values between 0 and the max priority level...
        for (int i = 0; i < MAX_PRIORITY; i++) {

            //If the ready queue array list at point i isn't empty...
            if (readyQueueHead[i] != NULL && readyQueueTail[i] != NULL) {

                //Lock mutex so only one thread can access it
                pthread_mutex_lock(&mutex);

                //Assign the struct process to the PCB data at the PID pointed by the first element of the specified ready queue array list
                ready = processTable[(size_t) removeFirst(&readyQueueHead[i], &readyQueueTail[i])];

                //Unlock mutex as needed value has been acquired
                pthread_mutex_unlock(&mutex);

                //Once the first element has been removed from the list break from the for loop
                break;
            }
        }

        //If the value received by the PCB isn't empty...
        if (ready != NULL) {

            currentThread[(size_t) currentCPU] = ready;

            if (processPreempted == 1 && ready->iRemainingBurstTime != 0) {

                int priorityLevel = 0;

                addLast(ready->pPID, &readyQueueHead[priorityLevel], &readyQueueTail[priorityLevel]);
                printf("preempt add to end of top priority queue\n");
                processPreempted = 0;
            }

            //if the current process priority level is less than 16...
            //16 is the cut off priority level for FCFS
            //First Come First Serve
            if (ready->iPriority < lowerPriorityRR) {

                //Run non preemptive job for process
                //Changes it's state to running, runs the job, sets the burst time to zero and finally updates the state to complete
                runNonPreemptiveJob(ready, &oStartTime, &oEndTime);

                //If the previous burst time is equal to the initial burst time of the process and the remaining burst time is not 0...
                //Means process is running for the first time and will not be completed within this one run
                if ((ready->iPreviousBurstTime == ready->iInitialBurstTime) && (ready->iRemainingBurstTime != 0)) {

                    //Print details about the process that is now running including the calculated response time
                    printf("TXT: Consumer %zu, Process Id = %zu (FCFS), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %ld\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime,
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oFirstTimeRunning));
                }

                    //Else if the previous burst time is not equal to the initial burst time of the process and the remaining burst time is 0...
                    //Means process is not running for the first time and will be completed within this one run
                else if ((ready->iPreviousBurstTime != ready->iInitialBurstTime) && (ready->iRemainingBurstTime == 0)) {

                    //Print details about the process that is now running including the calculated turnaround time
                    printf("TXT: Consumer %zu, Process Id = %zu (FCFS), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %ld\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime,
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oLastTimeRunning));
                }

                    //Else if the previous burst time is equal to the initial burst time of the process and the remaining burst time is 0...
                    //Means process is running for the first time and will be completed within this one run
                else if ((ready->iPreviousBurstTime == ready->iInitialBurstTime) && (ready->iRemainingBurstTime == 0)) {

                    //Print details about the process that is now running including the calculated response and turnaround time
                    printf("TXT: Consumer %zu, Process Id = %zu (FCFS), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %ld, Turnaround Time = %ld\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime,
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oFirstTimeRunning),
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oLastTimeRunning));
                }

                    //Else if the previous burst time is not equal to the initial burst time of the process and the remaining burst time is not 0...
                    // Means process is not running for the first time and will not be completed within this one run
                else  {

                    //Print details about the process that is now running
                    printf("TXT: Consumer %zu, Process Id = %zu (FCFS), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime);
                }

                //If the remaining burst time is zero...
                //Process will finish within this run
                if (ready->iRemainingBurstTime == 0) {

                    //Print SVG text
                    printProcessSVG((size_t) currentCPU, ready, oStartTime, oEndTime);

                    //Add finished process to the terminated queue along with it's PID
                    addLast(ready->pPID, &terminatedQueueHead, &terminatedQueueTail);

                    //Increment processes ran counter
                    processesRan++;
                }

                //Else if the remaining burst time is zero
                // Process will not finish within this run
                else if (ready->iRemainingBurstTime != 0) {

                    //Print SVG text
                    printProcessSVG((size_t) currentCPU, ready, oStartTime, oEndTime);

                    //Add process to the end of the corresponding ready queue array list based on it's priority level
                    addFirst(ready->pPID, &readyQueueHead[ready->iPriority], &readyQueueTail[ready->iPriority]);
                }
            }

            //Else if the process priority is greater than 15...
            //Means processes will be scheduled using a RR method
            //Round Robin
            else {

                //Run preemptive job for process
                //Calculates remaining burst time, changes it's state to running, runs the job
                //Reduce burst time to the time it was ran
                //Finally updates the state to complete if process complete else sets state to ready so process can be rerun
                runPreemptiveJob(ready, &oStartTime, &oEndTime);

                //If the previous burst time is equal to the initial burst time of the process and the remaining burst time is not 0...
                //Means process is running for the first time and will not be completed within this one run
                if ((ready->iPreviousBurstTime == ready->iInitialBurstTime) && (ready->iRemainingBurstTime != 0)) {

                    //Print details about the process that is now running including the calculated response time
                    printf("TXT: Consumer %zu, Process Id = %zu (RR), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %ld\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime,
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oFirstTimeRunning));
                }

                    //Else if the previous burst time is not equal to the initial burst time of the process and the remaining burst time is 0...
                    //Means process is not running for the first time and will be completed within this one run
                else if ((ready->iPreviousBurstTime != ready->iInitialBurstTime) && (ready->iRemainingBurstTime == 0)) {

                    //Print details about the process that is now running including the calculated turnaround time
                    printf("TXT: Consumer %zu, Process Id = %zu (RR), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %ld\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime,
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oMostRecentTime));
                }

                    //Else if the previous burst time is equal to the initial burst time of the process and the remaining burst time is 0...
                    //Means process is running for the first time and will be completed within this one run
                else if ((ready->iPreviousBurstTime == ready->iInitialBurstTime) && (ready->iRemainingBurstTime == 0)) {

                    //Print details about the process that is now running including the calculated response and turnaround time
                    printf("TXT: Consumer %zu, Process Id = %zu (RR), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %ld, Turnaround Time = %ld\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime,
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oFirstTimeRunning),
                           getDifferenceInMilliSeconds(ready->oTimeCreated, ready->oMostRecentTime));
                }

                    //Else if the previous burst time is not equal to the initial burst time of the process and the remaining burst time is not 0...
                    // Means process is not running for the first time and will not be completed within this one run
                else {

                    //Print details about the process that is now running
                    printf("TXT: Consumer %zu, Process Id = %zu (RR), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",
                           (size_t) currentCPU, (size_t) ready->pPID, ready->iPriority, ready->iPreviousBurstTime, ready->iRemainingBurstTime);
                }

                //If the remaining burst time is zero...
                //Process will finish within this run
                if (ready->iRemainingBurstTime == 0) {

                    //Print SVG text
                    printProcessSVG((size_t) currentCPU, ready, oStartTime, oEndTime);

                    //Add finished process to the terminated queue along with it's PID
                    addLast(ready->pPID, &terminatedQueueHead, &terminatedQueueTail);

                    //Increment processes ran counter
                    processesRan++;
                }

                    //Else if the remaining burst time is zero
                    //Process will not finish within this run
                else if (ready->iRemainingBurstTime != 0){

                    //Print SVG text
                    printProcessSVG((size_t) currentCPU, ready, oStartTime, oEndTime);

                    //Add process to the end of the corresponding ready queue array list based on it's priority level
                    addLast(ready->pPID, &readyQueueHead[ready->iPriority], &readyQueueTail[ready->iPriority]);
                }
            }

            //Set ready process to NULL so the if statement won't run until a new process has been taken from the ready queue
            ready = NULL;
        }
    }

    //Return NULL once thread is complete as function isn't void
    return NULL;
}

//Function boosts processes in the Round Robin bracket at regular intervals to give lower priority processes a chance to run
void *boosterDaemon() {

    //While the number of processes ran so far is less than the total number of processes...
    while (processesRan < NUMBER_OF_PROCESSES) {

        //Sleep for a predetermined time to only allow process boosting at certain intervals
        milliSleep(BOOST_INTERVAL / 1000);

        //For process priority is between the RR lowest priority + 1 and the max priority value
        //RR lowest priority + 1 as processes currently at the lowest priority level will not need to be boosted
        for (int i = lowerPriorityRR + 1; i < MAX_PRIORITY; i++) {

            //If the ready queue array list at point i is not empty
            if (readyQueueHead[i] != NULL && readyQueueTail[i] != NULL) {

                //Assign the struct process to the PCB data at the PID pointed by the first element of the specified ready queue array list
                struct process *boost = processTable[(size_t) removeFirst(&readyQueueHead[i], &readyQueueTail[i])];

                //Print details about the process that is now being boosted
                printf("TXT: Boost: Process Id = %zu, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",
                       (size_t) boost->pPID, boost->iPriority, boost->iPreviousBurstTime, boost->iRemainingBurstTime);

                //Add process to the end of the corresponding ready queue array list at the highest priority level for Round Robin
                addLast(boost->pPID, &readyQueueHead[lowerPriorityRR], &readyQueueTail[lowerPriorityRR]);
            }
        }
    }
    //Return NULL once thread is complete as function isn't void
    return NULL;
}

//Function terminates processes once they have finished their burst time, freeing memory and repopulating the free PID list
void *terminationDaemon() {

    //Stores the average response time and turnaround times calculated after a process has been terminated
    long averageResponseTime = 0, averageCycleTime = 0;

    //While the number of processes terminated so far is less than the total number of processes...
    while (processesTerm < NUMBER_OF_PROCESSES) {

        //Sleep for a predetermined time to allow for all processes to be finished before being accessed by the termination daemon
        milliSleep(TERMINATION_INTERVAL);

        //While the terminated queue is not empty...
        while (terminatedQueueHead != NULL && terminatedQueueTail != NULL) {

            //Assign the struct process to the PCB data at the PID pointed by the first element of the terminated queue
            struct process *terminate = processTable[(size_t) removeFirst(&terminatedQueueHead, &terminatedQueueTail)];

            //Adds turnaround time by adding the difference in time it took for all process to complete from once they were first run
            averageCycleTime += getDifferenceInMilliSeconds(terminate->oFirstTimeRunning, terminate->oLastTimeRunning);

            //Adds response time by adding the difference in time it took for all process to start from once they were first created
            averageResponseTime += getDifferenceInMilliSeconds(terminate->oTimeCreated, terminate->oFirstTimeRunning);

            //Print details about the process that is now being terminated
            printf("TXT: Terminated: Process Id = %zu, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",
                   (size_t) terminate->pPID, terminate->iPriority, terminate->iPreviousBurstTime, terminate->iRemainingBurstTime);

            //Add terminated process' PID to the end of the free PID list
            addLast(terminate->pPID, &freePidHead, &freePidTail);

            //Now that the process has gone through all process states it can now be freed in memory
            free(terminate);

            //Increment termination counter
            processesTerm++;
        }

        //Post semaphore to allow new processes to be generated by the process generator now that the free PID list has been populated
        sem_post(&freePID);
    }

    //Calculates average turnaround time by converting millisecond time difference to microseconds and dividing by the number of total processes
    averageCycleTime = (averageCycleTime * 1000) / NUMBER_OF_PROCESSES;

    //Calculates average turnaround time by converting millisecond time difference to microseconds and dividing by the number of total processes
    averageResponseTime = (averageResponseTime * 1000) / NUMBER_OF_PROCESSES;

    //Print final total averages for response time and turnaround time
    printf("TXT: Average response time = %f, Average turnaround time = %f\n", (double) averageResponseTime, (double) averageCycleTime);

    //Return NULL once thread is complete as function isn't void
    return NULL;
}

void printHeadersSVG() {
    printf("SVG: <!DOCTYPE html>\n");
    printf("SVG: <html>\n");
    printf("SVG: <body>\n");
    printf("SVG: <svg width=\"15000\" height=\"1100\">\n");
}

void printProcessSVG(int iCPUId, struct process *pProcess, struct timeval oStartTime, struct timeval oEndTime) {
    int iXOffset = getDifferenceInMilliSeconds(oBaseTime, oStartTime) + 30;
    int iYOffsetPriority = (pProcess->iPriority + 1) * 16 - 12;
    int iYOffsetCPU = (iCPUId - 1) * (480 + 50);
    int iWidth = getDifferenceInMilliSeconds(oStartTime, oEndTime);
    printf("SVG: <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"8\" style=\"fill:rgb(%zu,0,%zu);stroke-width:1;stroke:rgb(255,255,255)\"/>\n",
           iXOffset /* x */, iYOffsetCPU + iYOffsetPriority /* y */, iWidth,
           (size_t) (pProcess->pPID) - 1 /* rgb */, (size_t) (pProcess->pPID) - 1 /* rgb */);
}

void printPrioritiesSVG() {
    for (int iCPU = 1; iCPU <= NUMBER_OF_CPUS; iCPU++) {
        for (int iPriority = 0; iPriority < MAX_PRIORITY; iPriority++) {
            int iYOffsetPriority = (iPriority + 1) * 16 - 4;
            int iYOffsetCPU = (iCPU - 1) * (480 + 50);
            printf("SVG: <text x=\"0\" y=\"%d\" fill=\"black\">%d</text>\n", iYOffsetCPU + iYOffsetPriority, iPriority);
        }
    }
}

void printRasterSVG() {
    for (int iCPU = 1; iCPU <= NUMBER_OF_CPUS; iCPU++) {
        for (int iPriority = 0; iPriority < MAX_PRIORITY; iPriority++) {
            int iYOffsetPriority = (iPriority + 1) * 16 - 8;
            int iYOffsetCPU = (iCPU - 1) * (480 + 50);
            printf("SVG: <line x1=\"%d\" y1=\"%d\" x2=\"15000\" y2=\"%d\" style=\"stroke:rgb(125,125,125);"
                   "stroke-width:1\" />\n", 16, iYOffsetCPU + iYOffsetPriority, iYOffsetCPU + iYOffsetPriority);
        }
    }
}

void printFootersSVG() {
    printf("SVG: Sorry, your browser does not support inline SVG.\n");
    printf("SVG: </svg>\n");
    printf("SVG: </body>\n");
    printf("SVG: </html>\n");
}