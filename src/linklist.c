/*
FileName:    linklist.c
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: This file provides methods of link list.
Remark:      No function implemented in this file.
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
Insert a node to the head of linklist.

@param head The head of link list
@param pid The pid of node
@param cmd The command of the node

@return void
*/
void headInsert(Node** head, pid_t pid, char* cmd) {
	Node* p = (Node*) malloc(sizeof(Node));
	p->pid = pid;
	p->cmd = (char*) malloc(max_length_of_command*sizeof(char));
	memset(p->cmd, 0, max_length_of_command*sizeof(char));
	stpcpy(p->cmd, cmd);
	p->next = (*head);
	(*head) = p;
}

/*
Search the cmd of a pid.

@param head The head of link list
@param pid The pid of node

@return cmd The command of the node
*/
char* searchName(Node** head, pid_t pid)
{
    Node * current = (*head);
	while (current != NULL)
	{
		if (current->pid == pid) {
			return current->cmd;
		}
		current = current->next;
	}
	return NULL;
}

/*
kill all process recorded in the link list.

@param head The head of link list

@return void
*/
void killAll(Node** head)
{
	Node * current = (*head);
	while (current != NULL)
	{
		kill(current->pid, SIGKILL);
		current = current->next;
	}
}

/*
free the node.

@param node The node

@return void
*/
void freeNode(Node* node) {
	free(node->cmd);
	free(node);
}

/*
free the link list.

@param head The head of link list

@return void
*/
void freeList(Node** head) {
	Node * temp;
	while ((*head) != NULL)
	{
		temp = (*head)->next;
		freeNode(*head);
		(*head) = temp;
	}
	free(head);
	head = NULL;
}
		
	
