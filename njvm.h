#ifndef NJVM_INCLUDED
#define NJVM_INCLUDED

#include <stdbool.h>
#include "bigint.h"

#define IMMEDIATE(x) ((x) & 0x00FFFFFF)
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

#define MSB (1 << (8 * sizeof(unsigned int) -1))
#define IS_PRIM(objRef) (((objRef)->size & MSB) == 0)

#define GET_SIZE(objRef) ((objRef)->size & ~MSB)

#define GET_REFS(objRef) ((ObjRef*)(objRef)->data)

#define HALT 0
#define PUSHC 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6
#define RDINT 7
#define WRINT 8
#define RDCHR 9
#define WRCHR 10
#define PUSHG 11
#define POPG 12
#define ASF 13
#define RSF 14
#define PUSHL 15
#define POPL 16
#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22
#define JMP 23
#define BRF 24
#define BRT 25
#define CALL 26
#define RET 27
#define DROP 28
#define PUSHR 29
#define POPR 30
#define DUP 31
#define NEW 32
#define GETF 33
#define PUTF 34
#define NEWA 35
#define GETFA 36
#define PUTFA 37
#define GETSZ 38
#define PUSHN 39
#define REFEQ 40
#define REFNE 41

#define NJBF 1178749518

extern char *executeableName;

enum mode {LIST, RUN};

typedef struct{
    bool isObjRef;
    union
    {
        int value;
        ObjRef objRef;
    } slotContent;  
} StackSlot;

extern int staticDataAreaSize;
extern int programSize;

extern ObjRef* staticDataArea;


extern unsigned int *program;
extern int programPointer;

extern int halt;

extern int stackSlots;
extern StackSlot* stack;
extern int stackPointer;

extern int framePointer;

extern ObjRef returnValueRegister;

void handleArguments(int argc, char *arguments[]);
void runProgram(void);
void runInstruction(unsigned int instruction);
void executeInstruction(unsigned int instruction, int line, int mode);

void pushValueToStack(int number);
int pullValueFromStack(void);

void pushObjRefToStack(ObjRef ref);
ObjRef pullObjRefFromStack(void);

ObjRef newCompoundObject(int numObjRefs);

void* vm_alloc(int bytes);

void error(char* errorMsg);

void checkNullRefException(ObjRef ref);

#endif