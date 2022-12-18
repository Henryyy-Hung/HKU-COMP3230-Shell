/*
FileName:    buffer.c
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: It contains methods for the self defined structure "Buffer", which is responsible for receiving input and pre-process input.
Remark:      function implemented in this file:
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

/*
initializer of structure "Buffer".
It declare a buffer with $(capaciy) size, and return the pointer to this buffer.

@param capacity Capacity of the buffer to be created.

@return buffer Pointer to this buffer.
*/
Buffer* initBuffer(int capacity) {
	// set the minimum capacity
	if (capacity < max_length_of_command) {
		capacity = max_length_of_command;
	}
	// declare and initialize buffer, then clear the space
	Buffer* buffer = (Buffer*)malloc(sizeof(Buffer));
	memset(buffer, 0, sizeof(Buffer));
	// initialize the $(buffer->capacity)
	buffer->capacity = capacity;
	// initialize $(buffer->string) and clear the space.
	buffer->string = (char*) malloc((buffer->capacity)*sizeof(char));
	memset(buffer->string, 0, buffer->capacity*sizeof(char));
	return buffer;
}

/*
Free the structure "Buffer".

@param buffer Pointer of the buffer to be free.

@return Null to NULL the buffer.
*/
Buffer* freeBuffer(Buffer* buffer) {
	free(buffer->string);
	free(buffer);
	return NULL;
}

/*
Insert char $(ch) at position $(pos) in $(buffer->string).
If the capacity is not enough, increase the capacity.

@param buffer The pointer to the buffer that need insertion
@param pos Postion to insert char
@param ch Char to be inserted

@return buffer The pointer to the buffer that finish the insertion
*/
Buffer* insertBuffer(Buffer* buffer, int pos, char ch) {
	// extend buffer->string if capacity is full
	if ((strlen(buffer->string)+1) == buffer->capacity) {
		Buffer* temp = initBuffer(buffer->capacity + 1);
		strcpy(temp->string, buffer->string); 
		buffer = freeBuffer(buffer);
		buffer = temp;
	}
	// ensure the insert position will not cause invalid memory access
	if (pos >= (buffer->capacity)) {
		return buffer;
	}
	// insert char $(ch) into postion $(pos) and shift the subsequent char to right by 1 
	char prev, current;
	for (int i = 0; i < buffer->capacity; i++) {
		// keep all char before position $(pos)
		if (i < pos) {
			continue;
		}
		// change char at position $(pos) to char $(ch)
		else if (i == pos) {
			current = buffer->string[i];
			buffer->string[i] = ch;
		}
		// shift the subsequent char to right by 1 
		else {
			prev = current;
			current = buffer->string[i];
			buffer->string[i] = prev;
		}
	}
	return buffer;
}

/*
It convert all white space char into a space.
It insert space around char "|" and "&"

@param buffer The pointer to the buffer that need preprocess

@return buffer The pointer to the buffer that finish the preprocess
*/
Buffer* preprocessBuffer(Buffer* buffer) {
	for (int i = 0; i < buffer->capacity; i++) {
		char ch = buffer->string[i];
		// convert all space into white space
		if (ch == '\v' || ch == '\t' || ch == '\r' || ch == '\n') {
			buffer->string[i] = ' ';
			continue;
		}
		// insert space around '|' and '&'
		if (ch == '|' || ch == '&') {
			buffer = insertBuffer(buffer, i, ' ');
			buffer = insertBuffer(buffer, i+2, ' ');
			i = i+2;
		}
	}
	return buffer;
}

/*
Get at most $(buffer->capacity) chars from the command line.
The minimum capacity will be 1024.

@param buffer The pointer to the buffer that need input.

@return buffer The pointer to the buffer that finish the input.
*/
int getCommandLineInput(Buffer* buffer) {
	// allow the user to input $(buffer->capacity - 2) chars
	// the last 2 char is reserved for '\10' and '\0'
	fgets((buffer->string), (buffer->capacity), stdin);
	// change the '\10' to '\0'
	buffer->string[strlen((buffer->string))-1] = '\0';
	return 0;
}



	
	

