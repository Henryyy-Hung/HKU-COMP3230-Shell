/*
FileName:    linklist.h
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: Hold the function header of linklist.c, provide self defined structure.
Remark:      None of function is implemented in this file.
*/

#include <sys/types.h>

#ifndef LINKLIST_H
#define LINKLIST_H

// a node of link list that storing pid and cmd
typedef struct Node
{
	pid_t pid;
	char* cmd;
	struct Node * next;
} Node;

void headInsert(Node** head, pid_t pid, char* cmd);

char* searchName(Node** head, pid_t pid);

void killAll(Node** head);

void freeList(Node** head);

#endif