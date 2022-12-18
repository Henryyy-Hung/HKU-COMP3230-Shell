/*
FileName:    buffer.h
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: Hold the function header of buffer.c, provide self defined structure.
Remark:      None of function is implemented in this file.
*/

#ifndef BUFFER_H
#define BUFFER_H

// a buffer that holding the command line input and max capacity of itself.
typedef struct Buffer {
	char* string;
	int capacity;
} Buffer;

Buffer* initBuffer(int capacity);

Buffer* freeBuffer(Buffer* buffer);

Buffer* insertBuffer(Buffer* buffer, int pos, char ch);

Buffer* preprocessBuffer(Buffer* buffer);

int getCommandLineInput(Buffer* buffer);


#endif