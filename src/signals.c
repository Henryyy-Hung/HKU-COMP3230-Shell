/*
FileName:    signals.c
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: Signal handler of Main process and child process.
Remark:      function implemented in this file:
             1. Use of signals: All
             2. SIGCHLD signals: ALL
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"
#include "constant.h"
#include "linklist.h"
#include "signals.h"
#include "task.h"

// an global variable that indicate whether SIGUSER1 is received(1) or not(0).
int siguser1Received = 0;

// an buffer that store the termination message of background process
Buffer* sigBuffer = NULL;

// a lock that prevent 2 chldSighandler editing sigBuffer
pthread_mutex_t lock;

// a global variable that store the PIDs and corresponding CMD.
extern Node** taskRecords;

/*
Handler of SIGINT in the Main process.

@param signum Signal Number

@return void
*/
void intSighandlerMain(int signum) {
	printf("\n$$ 3230shell ## ");
}

/*
Handler of SIGINT in the Child process.
It display the type of signal that caused the program to terminate.

@param signum Signal Number

@return void
*/
void intSighandlerChild(int signum) {
	printf("Interrupt\n");
}

/*
Handler of SIGTERM in the Child process.
It display the type of signal that caused the program to terminate.

@param signum Signal Number

@return void
*/
void termSighandlerChild(int signum) {
	printf("software termination signal\n");
}

/*
Handler of SIGQUIT in the Child process.
It display the type of signal that caused the program to terminate.

@param signum Signal Number

@return void
*/
void quitSighandlerChild(int signum) {
	printf("quit\n");
}

/*
Handler of SIGKILL in the Child process.
It display the type of signal that caused the program to terminate.

@param signum Signal Number

@return void
*/
void killSighandlerChild(int signum) {
	printf("killed\n");
}

/*
Handler of SIGHUP in the Child process.
It display the type of signal that caused the program to terminate.

@param signum Signal Number

@return void
*/
void hupSighandlerChild(int signum) {
	printf("hangup\n");
}

/*
Handler of SIGUSER1 in the Child process.
It change the $(siguser1Received) of child process from 0 to 1.
Which allows the child process to exec();

@param signum Signal Number

@return void
*/
void user1Sighandler(int signum) {
	siguser1Received = 1;
}

/*
Handler of SIGCHLD in the Main process.
It terminates the background child process and send termination message to the buffer.

@param signum Signal Number
@param sig Information of signal
@param context Extra information (not used)

@return void
*/
void chldSighandler(int signum, siginfo_t* sig, void* context) {
	pid_t pid = sig->si_pid;
	// if this is a background process
	if (pid == getpgid(pid)) {
		// kill the process
		waitpid(pid, NULL, WNOHANG);
		
		// get the name of command by pid
		char* cmd = searchName(taskRecords, pid);

		// construct the output
		char output[max_length_of_command];
		memset(output, 0, max_length_of_command);
		snprintf(output, max_length_of_command * sizeof(char), "[%d] %s Done\n", pid, cmd);

		pthread_mutex_lock(&lock);
		// put the output into the buffer
		strcat(sigBuffer->string, output);
		pthread_mutex_unlock(&lock);
	}
}

/*
Register the signal handlers for the Main process.

@param void

@return void
*/
void regMainSighandler(void) {
	// reset the handlers to default
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGKILL, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	// disable SIGINT to terminate main program
	struct sigaction sa_int = {0};
	sa_int.sa_flags = SA_RESTART;
	sa_int.sa_handler = &intSighandlerMain;
	sigaction(SIGINT, &sa_int, NULL);
	// use SIGUSR1 to activate the child process
	struct sigaction sa_user1;
	sa_user1.sa_flags = SA_RESTART;
	sa_user1.sa_handler = &user1Sighandler;
	sigaction(SIGUSR1, &sa_user1, NULL);
	// use SIGCHLD to terminate background process
	struct sigaction sa_chld;
	sa_chld.sa_flags = SA_RESTART;
	sa_chld.sa_flags |= SA_SIGINFO;
	sa_chld.sa_sigaction = &chldSighandler;
	sigaction(SIGCHLD, &sa_chld, NULL);
}

/*
Register the signal handlers for the Child process.

@param void

@return void
*/
void regChildSighandler(void) {
	/*
	All these handler display the type of signal that caused the program to terminate.
	*/
	struct sigaction sa_int = {0};
	sa_int.sa_flags = SA_RESTART;
	sa_int.sa_handler = &intSighandlerChild;
	sigaction(SIGINT, &sa_int, NULL);
	
	struct sigaction sa_term = {0};
	sa_term.sa_flags = SA_RESTART;
	sa_term.sa_handler = &termSighandlerChild;
	sigaction(SIGTERM, &sa_term, NULL);
	
	struct sigaction sa_quit = {0};
	sa_quit.sa_flags = SA_RESTART;
	sa_quit.sa_handler = &quitSighandlerChild;
	sigaction(SIGQUIT, &sa_quit, NULL);
	
	struct sigaction sa_kill = {0};
	sa_kill.sa_flags = SA_RESTART;
	sa_kill.sa_handler = &killSighandlerChild;
	sigaction(SIGKILL, &sa_kill, NULL);
	
	struct sigaction sa_hup = {0};
	sa_hup.sa_flags = SA_RESTART;
	sa_hup.sa_handler = &hupSighandlerChild;
	sigaction(SIGHUP, &sa_hup, NULL);
}
