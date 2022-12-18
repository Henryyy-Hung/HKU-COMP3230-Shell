/*
FileName:    constant.h
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: Define all the constant that will be use in the program.
Remark:      None of function is implemented in this file.
*/

#ifndef CONSTANT_H
#define CONSTANT_H

// the maximum length of command (reserve 2 extra space for holding '\10' and '\0")
static const int max_length_of_command = (1024 + 2);

// the maximum number of arguments (reserve 1 extra space for holding 'NULL' as a end point marker)
static const int max_num_of_arguments = (30 + 1);

#endif