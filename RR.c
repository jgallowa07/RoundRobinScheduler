/*

AUTHORSHIP STATEMENT 
DATE - 10/30/2017
NAME - JARED GREGORY GALLOWAY
DUCKID - jgallowa - 951 344 934

all of this code is original and was writtin by JARED GREGORY GALLOWAY.

citations:

1 ideas for architecture discussed with AMIE CORSO, and ANISHA MALYNUR. 
2 all p1* helper funtions and getfield() provided by Joe Sventek
3 example looked at for setitimer located at www.cs.cpi.edu/claypool/courses 

*/
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "p1fxns.h"	
#include "linkedList.h"

#define UNUSED __attribute__ ((unused))	

typedef struct data Data; 

Data *myData;
volatile int myWaitFlag = 0;
volatile int allThreadsFinished = 0;
static int getfield(char buf[], int i, int fs, char field[]);
static int getNumStringInArray(char *Strings);
static void printTime(int startSeconds, int startMicro, int endSeconds, int endMicro);
static void wakeUp(UNUSED int i);
static void threadWait();
static void switchContext(void);
static int pidTerminated(pid_t pid);
static void childHandler(UNUSED int sig);
static void UNUSED printProcessInfo(pid_t pid);
static void cleanUp();
void *malloc(size_t);
void *calloc(size_t,size_t);
void free(void *);
char *getenv(const char *);
void exit();

struct data{

	int quantum;
	int n_processes;
	int n_processors;
	int count;
	int activeProcesses;
	
	pid_t *pid_array;
	pid_t *terminatedPids;
	char **cmdargv;
 
	linkedList *readyQueue;		//Processes that are halted, but ready to run
	linkedList *runningQueue;  	//Processes that are currently executing
	pid_t *terminatedProcesses;	//array of finished child PID's

	int processInfoFD;
};

int main(int argc, char **argv){

	myData = (Data *)malloc(sizeof(Data));

	//SIGUSR1 HANDLER
	signal(SIGUSR1,wakeUp);
	//SIGCHLD HANDLER
	signal(SIGCHLD,childHandler);
	
	myData->quantum = 0;
	myData->n_processes = 0;
	myData->n_processors = 0;
	myData->count = 0;

	char *p;
	struct timeval start,end;
	struct itimerval it_val;	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 100;

	int commandSet = 0;
	int quantumSet = 0;
	int processesSet = 0;
	int processorsSet = 0;
	
	//PROCESS ALL ENVIRONMENT VARIABLES
	if((p = getenv("TH_QUANTUM_MSEC"))!=NULL){
		myData->quantum = p1atoi(p);
		quantumSet = 1;
	}
	if((p = getenv("TH_NPROCESSES"))!=NULL){
		myData->n_processes = p1atoi(p);
		processesSet = 1;
	}
	if((p = getenv("TH_NPROCESSORS"))!=NULL){
		myData->n_processors = p1atoi(p);
		processorsSet = 1;
	}
	
	//PROCESS ALL COMMAND LINE ARGUMENTS
	int i;
	for(i = 1; i < argc; i++){
		int index = 0;
		char argcmd[1000];
		char argval[100];
		index = getfield(argv[i],index,'=',argcmd);
		int cmdlen = p1strlen(argcmd);
		if(p1strneq(argcmd,"--quantum",cmdlen)==1){
			p1getword(argv[i],index,argval);
			myData->quantum = p1atoi(argval);
			quantumSet = 1;
		}
		if(p1strneq(argcmd,"--number",cmdlen)==1){
			p1getword(argv[i],index,argval);
			myData->n_processes = p1atoi(argval);
			processesSet = 1;
		}
		if(p1strneq(argcmd,"--processors",cmdlen)==1){
			p1getword(argv[i],index,argval);
			myData->n_processors = p1atoi(argval);
			processorsSet = 1;
		}
		if(p1strneq(argcmd,"--command",cmdlen)==1){
			int numargs = getNumStringInArray(argv[i]);
			myData->cmdargv = (char **)malloc(sizeof(char *)*numargs);
			
			int iter;
			for(iter = 0; iter < (numargs-1); iter++){	
				index = p1getword(argv[i],index,argval);
				myData->cmdargv[iter] = p1strdup(argval);
			}
			myData->cmdargv[iter] = NULL;
			commandSet = 1;
		}
	}

	//MAKE SURE ALL FLAGS HAVE BEEN SET
	if(!commandSet || !quantumSet || !processesSet || !processorsSet){
		p1putstr(1,"one of the flags was not set\n");
		return 0;
	}

	//MAKE SURE THE FLAGS ARE VALID 
	if(myData->quantum == 0 || myData->n_processes == 0 || myData->n_processors == 0){
		p1putstr(1,"one of the flags was zero\n");
		return 0;
	}	

	//MALLOCATE DATA
	myData->pid_array = (pid_t *)malloc(sizeof(pid_t)*myData->n_processes);
	myData->terminatedProcesses = (pid_t *)malloc(sizeof(pid_t)*myData->n_processes);
	myData->readyQueue = linkedListCreate();
	myData->runningQueue = linkedListCreate();
	myData->activeProcesses = myData->n_processes;

	//START TIMER
	gettimeofday(&start,NULL);
	
	//FORK ALL CHILD PROCESSES
	for (i = 0; i < myData->n_processes; i++){
		myData->pid_array[i] = fork();
		if(myData->pid_array[i] == 0){
			threadWait();
			execvp(myData->cmdargv[0],myData->cmdargv);
			p1putstr(1,"error with execvp\n");
			exit(1);
		}
		if (i < myData->n_processors){ 
			myData->runningQueue->en_queue(myData->runningQueue,myData->pid_array[i]);	
		}else{
			myData->readyQueue->en_queue(myData->readyQueue,myData->pid_array[i]);
		}
		
	}


	
	//WAKE UP ALL THREADS
	for (i = 0; i < myData->n_processes; i++){
		kill(myData->pid_array[i],SIGUSR1);
		if(i >= myData->n_processors){
			kill(myData->pid_array[i],SIGSTOP);
		}			
	}

	//SET SIGALRM SIGNAL HANDLER 	
	if (signal(SIGALRM, (void (*)(int)) switchContext) == SIG_ERR) {
   		perror("Unable to catch SIGALRM");
    		exit(1);
  	}
  	it_val.it_value.tv_usec = (myData->quantum*1000) % 1000000;	
  	it_val.it_value.tv_sec = myData->quantum/1000;
  	it_val.it_interval = it_val.it_value;
	
	//SETITIMER 
  	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    		perror("error calling setitimer()");
   	 	exit(1);
  	}

	//WAIT TILL THERE ARE NO MORE RUNNING CHILD PROCESSES
  	while (myData->activeProcesses){
		nanosleep(&ts,NULL);
	} 
	
	//END TIMER
	gettimeofday(&end,NULL);

	//PRINT ELAPSED TIME
	printTime(start.tv_sec,start.tv_usec,end.tv_sec,end.tv_usec);
	
	//FREE ALL MALLOC'S
	cleanUp();
	free(myData);	

	return 1;
}

//FREE MALLOCS
static void cleanUp(){

	int i;	
	for (i = 0; myData->cmdargv[i] != NULL; i++){
		free(myData->cmdargv[i]);
	}
	free(myData->cmdargv);
	free(myData->pid_array);
	free(myData->terminatedProcesses);
	myData->readyQueue->destroy(myData->readyQueue);
	myData->runningQueue->destroy(myData->runningQueue);

}


//RETURNS 1 IF PID PASSED IN HAS TERMINATED 
static int pidTerminated(pid_t pid){

	int i;
	for (i = 0; i < myData->count; i++){
		if (myData->terminatedProcesses[i] == pid){
			return 1;
		}			
	}
	return 0;
}

//SIGCHLD HANDLER
static void childHandler(UNUSED int sig){

	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
		if (WIFEXITED(status) || WIFSIGNALED(status)){
			myData->activeProcesses--;			
			myData->terminatedProcesses[myData->count] = pid;
			myData->count++;
		}
	}
}

//ROUND ROBIN THE NEXT SET OF PROCESSES 
static void switchContext(void){

	int i;
	
	//PULL N PROCESSORS THINGS FROM RUNNING QUEUE AND PUT THEM IN READY QUEUE
	for (i = 0; i < myData->n_processors; i++){	
		pid_t pidToStop = myData->runningQueue->de_queue(myData->runningQueue);		
		//DONT ENQUEUE A TERMINATED PROCESS	
		if(pidTerminated(pidToStop)){
			continue;
		}
		kill(pidToStop,SIGSTOP);
		myData->readyQueue->en_queue(myData->readyQueue,pidToStop);
	}
	
	//PULL N PROCESSORS THINGS FROM READY QUEUE AND PUT THEM IN RUNNING QUEUE
	for (i = 0; i < myData->n_processors; i++){
		pid_t pidToStart = myData->readyQueue->de_queue(myData->readyQueue);		
		//DON'T ENQUEUE A NULL PULLED FROM THE READY QUEUE. 
		if(pidToStart == -99){
			continue;
		}		
		kill(pidToStart,SIGCONT);
		myData->runningQueue->en_queue(myData->runningQueue,pidToStart);

		//printProcessInfo(pidToStart);			
		
	}
}

//PRINT PROC DATA
static void printProcessInfo(pid_t pid){


	char filePath[40];
	char buffer[40];
	char pidString[40];
	char VMsize[40];
	char info[1024];

	p1itoa(pid,buffer);
	p1strcpy(pidString,buffer);
	
	p1strcpy(filePath,"/proc/");
	p1strcat(filePath,pidString);	
	p1strcat(filePath,"/status");
	
	myData->processInfoFD = open(filePath,0);
	if(myData->processInfoFD == -1){
		p1putstr(1,"File ");
		p1putstr(1,filePath);
		p1putstr(1,"Does not exist \n");
		return;
	}

	
	p1getline(myData->processInfoFD,info,1000);
	p1getword(info,0,VMsize);
	int size = p1strlen("VmSize:");
	while(p1strneq(VMsize,"VmSize:",size) != 1){
		p1getline(myData->processInfoFD,info,1000);
		p1getword(info,0,VMsize);		
	}
	p1putstr(1,pidString);
	p1putstr(1,"\n");
	p1putstr(1,info);
	p1putstr(1,"\n");
	
	close(myData->processInfoFD);

	p1strcpy(filePath,"/proc/");
	p1strcat(filePath,pidString);
	p1strcat(filePath,"/io");

	myData->processInfoFD = open(filePath,0);
	if(myData->processInfoFD == -1){
		p1putstr(1,"File ");
		p1putstr(1,filePath);
		p1putstr(1,"Does not exist \n");
		return;
	}
	
	p1getline(myData->processInfoFD,info,1000);
	p1putstr(1,info);
	p1putstr(1,"\n");
	p1getline(myData->processInfoFD,info,1000);
	p1putstr(1,info);
	p1putstr(1,"\n");

	close(myData->processInfoFD);	
	
}

//WAKE UP ALL THREADS
static void wakeUp(UNUSED int i){
	myWaitFlag = 1;
}

//STOP THE EXECUTION OF THE THREADS 
static void threadWait(){
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 100;
	while(myWaitFlag == 0){
		nanosleep(&ts,NULL);
	}
}

//PRINT THE ELAPSED TIME
static void printTime(int startSeconds, int startMicro, int endSeconds, int endMicro){

	char printString[1000];
	char buffer[1000];
	char elapsedTime[15];

	char secBuffer[50];
	char micBuffer[50];
	char secChar[50];
	char micChar[50];

	int seconds;
	int remainder;

	if(endMicro < startMicro){
		
		endMicro = endMicro + 1000000;
		endSeconds = endSeconds - 1;
	
	}

	remainder = endMicro - startMicro;
	seconds = endSeconds - startSeconds;

	p1itoa(seconds,secChar);
	p1itoa(remainder,micChar);

	char * MicOut = calloc(1,sizeof(char)*4);
	p1strpack(secChar, -3, '0', secBuffer);
	p1strpack(micChar, 3, '0', micBuffer);

	int i;
	for (i = 0; i < 3; i++){
		MicOut[i] = micBuffer[i];
	}

	p1strcpy(elapsedTime, secChar);				
	p1strcat(elapsedTime, ".");
	p1strcat(elapsedTime, MicOut);				
	
	p1strcpy(printString,"The eleapsed time to execute ");
	p1itoa(myData->n_processes,buffer);
	p1strcat(printString, buffer);				
	p1strcat(printString, " copies of ");
	p1strcat(printString, myData->cmdargv[0]);		
	p1strcat(printString, " on ");
	p1itoa(myData->n_processors,buffer);
	p1strcat(printString, buffer);				
	p1strcat(printString, " processors is ");
	p1strcat(printString, elapsedTime);		
	p1strcat(printString, " sec\n");

	p1putstr(1,printString);
	free(MicOut);
}

//STRING PARSING HELPER
static int getfield(char buf[], int i, int fs, char field[]){
	char *p;
	if(buf[i] == '\0' || buf[i] == '\n'){
		return -1;
	}
	p = field;
	while(buf[i] != fs && buf[i] != '\0' && buf[i] != '\n'){
		*p++ = buf[i++];
	}
	if (buf[i] == fs){
		i++;
	}
	*p = '\0';
	return i;
}

//FIND THE NUMBER OF ARGUMENTS
static int getNumStringInArray(char *Strings){
	int count = 0; 
	int index = 0;
	char buf[1000];
	while(index != -1){
		index = p1getword(Strings,index,buf);
		count++;
	}
	return count;
}
