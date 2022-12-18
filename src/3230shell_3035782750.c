/*
FileName:    3230shell.c
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: Constains the main logic of the program, and work as the central coordinator of 3230shell.
Remark:      All the part including the bonus has been done.
             function implemented in this file:
             1. Process creation and execution – foreground: Should be able to print “$$ 3230shell ##  “ and accept user’s input
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "buffer.h"
#include "constant.h"
#include "linklist.h"
#include "signals.h"
#include "task.h"

// a global variable that store the message from sigchld
extern Buffer* sigBuffer;
// a global variable that store the PIDs and corresponding CMD.
Node** taskRecords;

/*
Main loop of 3230shell.
It allows user to input arguments into the buffer.
Then, preprocess the buffer for ease of further parsing.
Last, it start the task with arguments in the buffer(further parsing will be done in startTask()).
The above loop will always execute until user enter "exit".

@param argc Argument Count
@param argv Argument Vector

@return 0/1 status code of process
*/
int main(int argc, char* argv[]) {
	// Input Buffer for receiving user input
	Buffer* buffer = NULL;
	// Initialize the background process output buffer
	sigBuffer = initBuffer(-1);
	// Initialize the task record to record the PIDs and corresponding CMD
	taskRecords = (Node**) malloc(sizeof(Node*));
	(*taskRecords) = NULL;
	// exit status ( 0 -> not exit, 1-> exit)
	int exit = 0;
	
	// Flush standard output immediately.
	setbuf(stdout, NULL);
	
	while(exit == 0) {
		// register the signal handler of main process
		regMainSighandler();
		// display the input notification
		printf("$$ 3230shell ## ");
		// declare and initialize buffer
		buffer = initBuffer(-1);
		// allow user input to the buffer through command line
		getCommandLineInput(buffer);
		// avoid the empty input
		if (strlen(buffer->string) != 0) {
			// preprocess the input for ease of parsing.
			buffer = preprocessBuffer(buffer);
			// start all the tasks specify in the input string
			exit = startTasks(buffer->string);
		}
		// free the buffer
		buffer = freeBuffer(buffer);
		// print the exit message of background processes
		if (sigBuffer != NULL) {
			// output the message store in buffer
			printf("%s", sigBuffer->string);
			// empty the buffer
			memset(sigBuffer->string, 0, sigBuffer->capacity * sizeof(char));
		}
	}
	// release all child process
	killAll(taskRecords);
	// free the buffer
	freeBuffer(sigBuffer);
	// free the link list
	freeList(taskRecords);
	return 0;
}