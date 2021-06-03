#ifndef NJVM_DEBUGGER_INCLUDED
#define NJVM_DEBUGGER_INCLUDED

#include "bigint.h"

extern int breakpoint;

void runDebugger(void);
void listProgram(void);
void listInstruction(unsigned int instruction, int line);
void inspectData(void);
void inspectStack(void);
void inspectObject(void);
void inspectReturnValue(void);
void inspectBip(void);
#endif