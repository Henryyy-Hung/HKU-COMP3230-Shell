/*
FileName:    task.c
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: This file provides methods of parsing the arguments and execute the arguments(such arguments including build-in exit, timeX, &, |).
Remark:      function implemented in this file:
             1. Process creation and execution – foreground: All  
             2. Process creation and execution – use of ‘|’: ALL
             3. Built-in command: timeX: ALL
             4. Built-in command: exit: ALL
             5. Process creation and execution – background: ALL
             6. SIGCHLD signaL: ALL (Another part is in signals.c)
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

// a global variable that indicate whether SIGUSER1 is received(1) or not(0).
extern int siguser1Received;
// a global variable that store the PIDs and corresponding CMD.
extern Node** taskRecords;

/*
Initialize the argument vector $(argv), which contains argument string.
i.e. it is like {"ls", "-l", "-a"}.

@param capacity The capacity of argument vector

@return argv The pointer to the argument vector that finish initialization.
*/
char** initArgv(int capacity) {
	// set minimum capacity for argv
	if (capacity < max_num_of_arguments) {
		capacity = max_num_of_arguments;
	}
	// declare and initialize an argument vector,  and clear the space for it.
	char** argv = (char**) malloc(capacity*sizeof(char*));
	memset(argv, 0, capacity*sizeof(char*));
	for (int i = 0; i < capacity; i++) {
		argv[i] = (char*) malloc(max_length_of_command*sizeof(char));
		memset(argv[i], 0, max_length_of_command*sizeof(char));
	}
	return argv;
}

/*
Split the input string by space and form an argument vector.
i.e. it split "/bin/ls -l -a | grep .c" into {"/bin/ls", "-l", "-a", "|", "grep", ".c", NULL}

@param string The pre-processed command line input(i.e. "|" and "&" are surrounded by space).

@return argv The pointer to the argument vector that contains arguments.
*/
char** constructArgv(char* string) {
	// initialize a argument vector with default capacity
	char** argv = initArgv(-1);
	// split the input string by " " and transfer it into argv
	int i = 0;
	char* token = strtok(string, " ");
	while (token != NULL) {
		strcpy(argv[i], token);
		token = strtok(NULL, " ");
		i++;
	}
	// label the end of arguments with NULL pointer
	free(argv[i]);
	argv[i] = NULL;
	return argv;
}

/*
Remove the path of the argument in $(string)(e.g. "/bin/ls" to "ls")

@param string A argument which may consist path(e.g. /bin/ls)

@return void
*/
void removePath(char* string) {
	// the position contains "/"
	int slashPos = -1;
	// the postion contains '\0'
	int endPos = 0;
	// find $(slashPos) and $(endPos)
	for (int i = 0; i < max_length_of_command; i++) {
		if (string[i] == '\0') {
			endPos = i;
			break;
		} 
		else if (string[i] == '/') {
			slashPos = i;
		}
		else {
			continue;
		}
	}
	// If the argument consist of path, remove the path
	if (slashPos != -1) {
		char* temp = (char*) malloc(max_length_of_command*sizeof(char));
		memset(temp, 0, max_length_of_command*sizeof(char));
		strncpy(temp, &string[slashPos+1], endPos - slashPos);
		memset(string, 0, max_length_of_command*sizeof(char));
		strcpy(string, temp);
		free(temp);
	}
		
	return;
}

/*
Parse the arguments and execute arguments.
There are 5 stages when start a task:

Stage 0: Declaration of  all necessary variables
    I guess I don't need to explain?
Stage 1: Initialization of argument vector
    convert the command line input $(string) into argument vector.
    e.g. "ls -l -a |grep c$" -> {"ls", "-l", "-a", "|", "grep", "c$"}
Stage 2: Paring argument vector and detect input errors.
    Detect the exit, timeX, & and perform corresponding behavior.
	e.g. exit the program, set the mode indicator to be 1 and etc.
	Check the input error of exit, timeX, &, |.
	s.t. if any error, pop err message and enter next loop.
Stage 3: Allocation of task
    split argument vector into sub-vectors, which could be put into exec() directly.
	e.g. {"timeX", "ls", "-la", "|", "grep", "c$"} -> {("ls", "-la"), ("grep", "c$")}.
Stage 4: Execution of task
    Execute tasks one by one, if there is pipe, it will redirect stdout of 
	previous task to stdin of current task.
	It also register a different set of signal handler for child process.
	It will print error message when exec fail.
	It perform timeX function.
    It allow child process to execute in background.
	Its child process will wait for USR1 to activate.
	It kills the child process after child process enter zombie state.
If there is any error in any stage, the function will free all memory and quit. (I guess?)
I tried to do my best on memory management but I am not so familiar with c :(

@param string The pre-processed command line input(i.e. "|" and "&" are surrounded by space).

@return output The status code of exit, if 1, then quit main process.
*/
int startTasks(char* string) {
	
	/* Stage 0: Declare variables */
	
	// return value ( 0 -> enter next loop, 1 -> exit the main program)
	int output = 0;
	
	// indicator of background(&) mode
	int backgroundMode = 0;
	// indicator of timeX mode
	int timeXMode = 0;    
	
	// state indicators: Initialization of argument vector
	int iniStage = 0;    
	// state indicators: Paring argument vector and detect input errors.
	int parStage = 0;
	// state indicators: Allocation of task
	int allStage = 0;
	// state indicators: Execution of task
	int exeStage = 0;
	
	// containers
	char** rawArgs;    // an string array holding all arguments (e.g. ["timeX", "ls", "-l", "-a", "|", "cat", "|", "grep", ".*.c"] )
	char*** argvs;    // an vector of string array (e.g. [("ls", "-l", "-a"), ("cat"), ("grep", ".*.c")] )
	
	// variables
	int argvsPos = 0;
	int argPos = 0;
	
	/* Stage 1: Initialization of Argument Vector */
		
	// split the string into fragments by space, extract all arguments into rawArgs vector
	if (iniStage == 0) {
		rawArgs = constructArgv(string);
		if (rawArgs == NULL) {
			printf("3230shell: Fail to construct argument vector.\n");
			iniStage = 1;
		}
	}
	// quit if error occurs in Stage 1.
	if (iniStage == 1) {
		return output;
	}
	
	/* Stage 2: Paring argument vector and detect input errors.*/
	
	if (parStage == 0) {
		// handle exit command
		if (strcmp(rawArgs[0], "exit") == 0 && rawArgs[1] == NULL) {
			printf("3230shell: Terminated\n");
			parStage = 1;
			output = 1;
		}
		else if (strcmp(rawArgs[0], "exit") == 0 && rawArgs[1] != NULL) {
			printf("3230shell: \"exit\" with other arguments!!!\n");
			parStage = 1;
			output = 0;
		}
		// handle pipe command
		for (int i = 0; rawArgs[i] != NULL && parStage == 0; i++) {
			if (strcmp(rawArgs[i], "|") == 0 && i == 0) {
				printf("3230shell: syntax error near unexpected token `|'\n");
				parStage = 1;
				output = 0;
			}
			else if (strcmp(rawArgs[i], "|") == 0 && strcmp(rawArgs[i-1], "timeX") == 0 && i == 1) {
				printf("3230shell: syntax error near unexpected token `|'\n");
				parStage = 1;
				output = 0;
			}
			else if (strcmp(rawArgs[i], "|") == 0 && rawArgs[i+1] == NULL) {
				printf("3230shell: '|' should not appear in the last of the command line\n");
				parStage = 1;
				output = 0;
			}
			else if (strcmp(rawArgs[i], "|") == 0 && strcmp(rawArgs[i+1], "&") == 0) {
				printf("3230shell: syntax error near unexpected token `|'\n");
				parStage = 1;
				output = 0;
			}
			else if (strcmp(rawArgs[i], "|") == 0 && strcmp(rawArgs[i+1], "|") == 0) {
				printf("3230shell: should not have two consecutive | without in-between command\n");
				parStage = 1;
				output = 0;	
			}
		}
		// handle & command
		for (int i = 0; rawArgs[i] != NULL && parStage == 0; i++) {
			if (strcmp(rawArgs[i], "&") == 0 && rawArgs[i+1] == NULL  && i == 0) {
				printf("3230shell: '&' cannot be a standalone command\n");
				parStage = 1;
				output = 0;
			}
			else if (strcmp(rawArgs[i], "&") == 0 && rawArgs[i+1] != NULL  && i == 0) {
				printf("3230shell: '&' should not appear in the begin of the command line\n");
				parStage = 1;
				output = 0;
			}
			else if (strcmp(rawArgs[i], "&") == 0 && rawArgs[i+1] != NULL) {
				printf("3230shell: '&' should not appear in the middle of the command line\n");
				parStage = 1;
				output = 0;
			}
			else if (strcmp(rawArgs[i], "&") == 0 && rawArgs[i+1] == NULL) {
				backgroundMode = 1;
			}
		}
		// handle timeX command
		if (strcmp(rawArgs[0], "timeX") == 0 && rawArgs[1] == NULL) {
			printf("3230shell: \"timeX\" cannot be a standalone command\n");
			parStage = 1;
			output = 0;
		}
		else if (strcmp(rawArgs[0], "timeX") == 0 && backgroundMode == 1) {
			printf("3230shell: \"timeX\" cannot be run in background mode\n");
			parStage = 1;
			output = 0;
		}
		else if (strcmp(rawArgs[0], "timeX") == 0) {
			timeXMode = 1;
		}
		else {
			timeXMode = 0;
		}
	}
	// quit if error occurs in Stage 2.
	if (parStage == 1) {
		// free char** rawArgs
		for (int i = 0; i < max_num_of_arguments; i++) {
			free(rawArgs[i]);
		}
		free(rawArgs);
		return output;
	}
	
	/* Stage 3: Allocation of task */
	
	// further split the arguments into independent command vector, and store in argvs
	if (allStage == 0) {
		// declare and initialize an vector that could contains argument vectors
		argvs = (char***) malloc(max_num_of_arguments*sizeof(char**));
		memset(argvs, 0, max_num_of_arguments*sizeof(char**));
		for (int i = 0; i < max_num_of_arguments; i++) {
			argvs[i] = initArgv(-1);
		}
		// variable that indicates the position of argvs and argv
		argvsPos = 0;
		argPos = 0;
		// allocate the arguments in rawArgs into vector of argument vector, the $(argvs).
		for (int i = 0; rawArgs[i] != NULL; i++) {
			// ignore the & and timeX at beginning
			if (strcmp(rawArgs[i], "&") == 0) {
				continue;
			}
			else if (strcmp(rawArgs[i], "timeX") == 0 && i == 0) {
				continue;
			}
			// split the commands by "|"
			else if (strcmp(rawArgs[i], "|") == 0) {
				argvs[argvsPos][argPos] = NULL;
				argvsPos += 1;
				argPos = 0;
				continue;
			}
			else {
				strcpy(argvs[argvsPos][argPos], rawArgs[i]);
				argPos+=1;
				continue;
			}
		}
		// Label the end of the array by NULL
		free(argvs[argvsPos][argPos]);
		for (int i = 0; i < max_num_of_arguments; i++) {
			free(argvs[argvsPos+1][i]);
		}
		free(argvs[argvsPos+1]);
		argvs[argvsPos][argPos] = NULL;
		argvs[argvsPos+1] = NULL;
	}
	// quit if error occurs in Stage 3.
	if (allStage == 1) {
		// free char** rawArgs
		for (int i = 0; i < max_num_of_arguments; i++) {
			free(rawArgs[i]);
		}
		free(rawArgs);
		// free char*** argvs
		for (int i = 0; i < max_num_of_arguments; i++) {
			for (int j = 0; j < max_num_of_arguments; j++) {
				if (argvs[i] == NULL) {
					break;
				}
				free(argvs[i][j]);
			}
		}
		for (int i = 0; i < max_num_of_arguments; i++) {
			free(argvs[i]);
		}
		free(argvs);
		return output;
	}
	
	/* Stage 4: Execution of task */
	
	// execute the command vectors (single command, multiple command in pipe, or in background)
	if (exeStage == 0) {
		// Number of Process to be executed
		int processNum = argvsPos + 1;
		// Number of pipe needed
		int pipeNum = processNum - 1;
		// container of pipes
		int pipes[pipeNum][2];
		// container of pids
		int pids[processNum];
		// container of timeX output
		char timeXOutput[max_length_of_command];
		memset(timeXOutput, 0, sizeof(timeXOutput));
		// initialize all pipes
		for (int i = 0; i < pipeNum && exeStage == 0; i++) {
			if (pipe(pipes[i]) == -1) {
				printf("3230shell: error with creating pipe\n");
				exeStage = 1;
			}
		}
		// execute the commands in child process one by one
		for (int i = 0; i < processNum && exeStage == 0; i++) {
			// register the signal handler to child process
			regChildSighandler();
			// store the full path of current command in $(fullPath)
			char* fullPath = (char*) malloc(max_length_of_command*sizeof(char));
			memset(fullPath, 0, max_length_of_command*sizeof(char));
			strcpy(fullPath, argvs[i][0]);
			// remove the full path from argv
			removePath(argvs[i][0]);
			// fork the child process and record its pid
			pids[i] = fork();
			/* Situation 1: failed to fork child process */
			if (pids[i] == -1) {
				printf("3230shell: error with creating porcess");
				exeStage = 1;
				continue;
			}
			/* Situation 2: in child process */
			else if (pids[i] == 0) {
				// turn the current child process into background mode before doing anything
				if (backgroundMode == 1) {
					setpgid(pids[i], pids[i]);
				}
				// wait for SIGUSER1
				while (siguser1Received != 1) {
					continue;
				}
				// close the unused pipe for current child process
				for (int j = 0; j < pipeNum; j++) {
					// First process: close all except the write port of itself
					if (i == 0) {
						close(pipes[j][0]);
						if (j != i) {
							close(pipes[j][1]);
						}
					}
					// Last process: close all except read port of previous process
					else if (i == processNum - 1) {
						if (j != i-1) {
							close(pipes[j][0]);
						}
						close(pipes[j][1]);
					}
					// Middle process: close all r/w port except write port of itself and read port of previous process
					else {
						if (j != i-1) {
							close(pipes[j][0]);
						}
						if (j != i) {
							close(pipes[j][1]);
						}
					}
				}
				// redirect the I/O
				if (processNum == 1) {
					// if only one process, don't do any redirection of output
				}
				else if (i == 0) {
					// First process: pass std output toward pipe
					dup2(pipes[i][1], STDOUT_FILENO);
					close(pipes[i][1]);
				}
				else if (i == processNum - 1) {
					// Last process: read std input from pipe
					dup2(pipes[i-1][0], STDIN_FILENO);
					close(pipes[i-1][0]);
				}
				else {
					// Middle process: read std input from pipe and pass std output toward pipe
					dup2(pipes[i][1], STDOUT_FILENO);
					close(pipes[i][1]);
					dup2(pipes[i-1][0], STDIN_FILENO);
					close(pipes[i-1][0]);
				}
				
				// execute the program	
				execvp(fullPath, argvs[i]);
				
				// if the program fail to execute, print err message
				char* temp = (char*) malloc(max_length_of_command*sizeof(char));
				memset(temp, 0, (max_length_of_command*sizeof(char)));
				strcat(temp, "3230shell: '");
				strcat(temp, fullPath);
				strcat(temp, "'");
				perror(temp);
				free(temp);
				
				// break the loop and terminate the program (because it is in child process).
				exeStage = 1;
				output = 1;
			}
			/* Situation 3: in parent process */
			else {
				// insert the task into the task record
				headInsert(taskRecords, pids[i], argvs[i][0]);
				// close the unused pipe
				if (processNum == 1) {
				}
				else if (i == 0) {
					close(pipes[i][1]);
				}
				else if (i == processNum - 1) {
					close(pipes[i-1][0]);
				}
				else {
					close(pipes[i-1][0]);
					close(pipes[i][1]);
				}
				// signal the child process to start
				kill(pids[i], SIGUSR1);
				// not wait for child process
				if (backgroundMode == 1) {
					// do nothing in parent process
				}
				// wait the process to finish
				else if (timeXMode == 1) {
					// wait for child process to finish
					int status;
					struct rusage usage;
					wait4(pids[i], &status, 0, &usage);
					// append the timeX message to the buffer
					char temp[max_length_of_command];
					snprintf(temp, sizeof(temp), "(PID)%d  (CMD)%s    (user)%ld.%03ld s  (sys)%ld.%03ld s\n", pids[i], argvs[i][0], usage.ru_utime.tv_sec, usage.ru_utime.tv_usec/1000, usage.ru_stime.tv_sec, usage.ru_stime.tv_usec/1000);
					strcat(timeXOutput, temp);
					// kill the child process after everthing has done.
					kill(pids[i], SIGKILL);
				}
				else {
					// wait for child process to finish
					waitpid(pids[i], NULL, 0);
					// kill the child process after everthing has done.
					kill(pids[i], SIGKILL);
				}
				// restore the full path back to argv
				strcpy(argvs[i][0], fullPath);
				// free the temp variable
				free(fullPath);
			}
		}
		// print the timeX message
		if (timeXMode == 1) {
			printf("%s", timeXOutput);
			memset(timeXOutput, 0, sizeof(timeXOutput));
		}

	}
	
	// free char** rawArgs
	for (int i = 0; i < max_num_of_arguments; i++) {
		free(rawArgs[i]);
	}
	free(rawArgs);

	// free char*** argvs
	for (int i = 0; i < max_num_of_arguments; i++) {
		for (int j = 0; j < max_num_of_arguments; j++) {
			if (argvs[i] == NULL) {
				break;
			}
			free(argvs[i][j]);
		}
	}
	for (int i = 0; i < max_num_of_arguments; i++) {
		free(argvs[i]);
	}
	free(argvs);
	
	return output;
}

	