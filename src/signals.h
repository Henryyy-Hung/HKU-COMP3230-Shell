/*
FileName:    signals.h
Author:      Hung Ka Hing
UID:         3035782750
Platform:    Linux Debian & Linux Ubuntu
Description: Hold the function header of signals.c.
Remark:      None of function is implemented in this file.
*/

#ifndef SIGNALS_H
#define SIGNALS_H

void regMainSighandler(void);

void regChildSighandler(void);

#endif