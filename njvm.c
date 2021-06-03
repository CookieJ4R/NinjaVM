#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "njvm.h"
#include "debugger.h"
#include "bigint.h"
#include "gc.h"

int version = 8;
int programPointer = 0;
int stackPointer = 0;
int framePointer = 0;

int halt = 0;

int debuggerEnabled = 0;

int codeFileSpecified = 0;

bool skipNextArg = false;
int stackSizeToAllocate = 64;
int heapSizeToAllocate = 8192;

StackSlot* stack;
int stackSlots = 0;

char *executeableName;

int programSize = 0;
unsigned int *program;

int staticDataAreaSize = 0;
ObjRef* staticDataArea;

ObjRef returnValueRegister;

int main(int argc, char *argv[]){
    executeableName = argv[0];
    handleArguments(argc, argv);
    if(codeFileSpecified != 1)
        error("No codefile specified...\n");

    stack = malloc(stackSizeToAllocate * 1024);
    stackSlots = (stackSizeToAllocate * 1024) / sizeof(StackSlot);
    
    zielSpeicher = malloc(heapSizeToAllocate * 1024);
    quellSpeicher = (unsigned char* )zielSpeicher + (heapSizeToAllocate/2 * 1024);
    freePointer = zielSpeicher;

    printf("Ninja Virtual Machine started\n");
    if(debuggerEnabled == 0)
        runProgram();
    else
        runDebugger();
    printf("Ninja Virtual Machine stopped\n");
}

void handleArguments(int argc, char *arguments[]){
    for(int i = 1; i < argc; i++){
        if(skipNextArg){
            skipNextArg = false;
            continue;
        }           
        if(strncmp("--", arguments[i], 2) == 0){
            if(strcmp("--version", arguments[i]) == 0){
                printf("Ninja Virtual Machine Version %i\n", version);
                exit(0);
            }else if(strcmp("--help", arguments[i]) == 0){
                printf("Usage: %s [option] [option] ...\n", executeableName);
                printf("  --version     show version\n");
                printf("  --help        show this message\n");
                printf("  --debug       starts the virtual machine in debug mode\n");
                exit(0);
            }else if(strcmp("--debug", arguments[i]) == 0){
                debuggerEnabled = 1;
            }else  if(strcmp("--stack", arguments[i]) == 0){
                skipNextArg = true;
                stackSizeToAllocate = atoi(arguments[i + 1]);
            }else if(strcmp("--heap", arguments[i]) == 0){
                skipNextArg = true;
                heapSizeToAllocate = atoi(arguments[i+1]);
            }else if(strcmp("--gcpurge", arguments[i]) == 0){
                gcPurgeFlag = true;
            }else if(strcmp("--gcstats", arguments[i]) == 0){
                gcStatsFlag = true;
            }else{
                printf("Unknown command line argument: '%s'. Try '%s --help'\n", arguments[i], executeableName);
                exit(-1);
            }
        }else{
            FILE* codeFile;
            codeFile = fopen(arguments[i], "rb");
            if(codeFile == NULL){
                error("No such codefile...\n");
            }else{
                int bytes[4];
                fread(bytes, sizeof(unsigned int), 4, codeFile);
                if(bytes[0] != NJBF){
                    error("Unknown fileformat...\nNJVM shutting down...\n");
                }
                if(bytes[1] != version){
                    error("File version and NJVM version dont match...\nNJVM shutting down...\n");
                }
                programSize = bytes[2];
                program = malloc(programSize * sizeof(unsigned int));
                staticDataAreaSize = bytes[3];
                staticDataArea = malloc(bytes[3] * (sizeof(ObjRef)));
                for(int i = 0; i < staticDataAreaSize; i++)
                    staticDataArea[i] = NULL; 
                fread(program, 4, programSize, codeFile);
                fclose(codeFile);
                codeFileSpecified = 1;
            }
        }
    }
}

void runProgram(void){
    while(!halt){
        if(programPointer == breakpoint){
            breakpoint = -1;
            runDebugger();
            return;
        }
        unsigned int instruction = program[programPointer++];
        runInstruction(instruction);
    }
    gc();
}


void runInstruction(unsigned int instruction){
    executeInstruction(instruction,-1, RUN);
}

void executeInstruction(unsigned int instruction, int line, int mode){
    switch(instruction >> 24){
        case HALT:
            if(mode == LIST)
                printf("%05d:  HALT\n", line);
            else if(mode == RUN)
                halt = 1;
        break;
        case PUSHC:
            if(mode == LIST)
                printf("%05d:  PUSHC   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                bigFromInt(SIGN_EXTEND(IMMEDIATE(instruction)));
                pushObjRefToStack(bip.res);
            } 
        break;
        case ADD:
            if(mode == LIST)
                printf("%05d:  ADD\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                checkNullRefException(bip.op2);
                bigAdd();
                pushObjRefToStack(bip.res);
            }
        break;
        case SUB:
            if(mode == LIST)
                printf("%05d:  SUB\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                checkNullRefException(bip.op2);
                bigSub();
                pushObjRefToStack(bip.res);
            }
        break;
        case MUL:
            if(mode == LIST)
                printf("%05d:  MUL\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                checkNullRefException(bip.op2);
                bigMul();
                pushObjRefToStack(bip.res);
            }
        break;
        case DIV:
            if(mode == LIST)
                printf("%05d:  DIV\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                checkNullRefException(bip.op2);
                bigDiv();
                pushObjRefToStack(bip.res);
            }
        break;
        case MOD:
            if(mode == LIST)
                printf("%05d:  MOD\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                checkNullRefException(bip.op2);
                bigDiv();
                pushObjRefToStack(bip.rem);
            }
        break;
        case RDINT:
            if(mode == LIST)
                printf("%05d:  RDINT\n", line);
            else if(mode == RUN){
                bigRead(stdin);
                pushObjRefToStack(bip.res);
            }
        break;
        case WRINT:
            if(mode == LIST)
                printf("%05d:  WRINT\n", line);
            else if(mode == RUN){
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                bigPrint(stdout);
            }
        break;
        case RDCHR:
            if(mode == LIST)
                printf("%05d:  RDCHR\n", line);
            else if(mode == RUN){
                char input = getchar();
                runInstruction((PUSHC << 24) | SIGN_EXTEND(IMMEDIATE(input)));
            }
        break;
        case WRCHR:
            if(mode == LIST)
                printf("%05d:  WRCHR\n", line);
            else if(mode == RUN){
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                printf("%c", bigToInt());
            }
        break;
        case PUSHG:
            if(mode == LIST)
                printf("%05d:  PUSHG   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                pushObjRefToStack(staticDataArea[SIGN_EXTEND(IMMEDIATE(instruction))]);
            }
        break;
        case POPG:
            if(mode == LIST)
                printf("%05d:  POPG   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                staticDataArea[SIGN_EXTEND(IMMEDIATE(instruction))] = pullObjRefFromStack();
            }
        break;
        case ASF:
            if(mode == LIST)
                printf("%05d:  ASF   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                pushValueToStack(framePointer);
                framePointer = stackPointer;
                stackPointer += SIGN_EXTEND(IMMEDIATE(instruction));
                for(int i = framePointer; i < stackPointer; i++){
                    stack[i].isObjRef = true;
                    stack[i].slotContent.objRef = NULL;
                }
            }
        break;
        case RSF:
            if(mode == LIST)
                printf("%05d:  RSF   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                stackPointer = framePointer;
                framePointer = pullValueFromStack();
            }
        break;
        case PUSHL:
            if(mode == LIST)
                printf("%05d:  PUSHL   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                pushObjRefToStack(stack[framePointer + SIGN_EXTEND(IMMEDIATE(instruction))].slotContent.objRef);
            }
        break;
        case POPL:
            if(mode == LIST)
                printf("%05d:  POPL   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                stack[framePointer + SIGN_EXTEND(IMMEDIATE(instruction))].isObjRef = true;
                stack[framePointer + SIGN_EXTEND(IMMEDIATE(instruction))].slotContent.objRef = pullObjRefFromStack();
            }
        break;
        case EQ:
            if(mode == LIST)
                printf("%05d:  EQ\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                bigCmp()==0 ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);
            }
        break;
        case NE:
            if(mode == LIST)
                printf("%05d:  NE\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                bigCmp()!=0 ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);
            }
        break;
        case LT:
            if(mode == LIST)
                printf("%05d:  LT\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                bigCmp() < 0 ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);
            }
        break;
        case LE:
            if(mode == LIST)
                printf("%05d:  LE\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                bigCmp() <= 0 ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);
            }
        break;
        case GT:
            if(mode == LIST)
                printf("%05d:  GT\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                bigCmp() > 0 ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);
            }
        break;
        case GE:
            if(mode == LIST)
                printf("%05d:  GE\n", line);
            else if(mode == RUN){
                bip.op2 = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                bigCmp() >= 0 ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);
            }
        break;
        case JMP:
            if(mode == LIST)
                printf("%05d:  JMP   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                programPointer = SIGN_EXTEND(IMMEDIATE(instruction));
            }
        break;
        case BRF:
            if(mode == LIST)
                printf("%05d:  BRF   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                bip.op1 = pullObjRefFromStack();
                if(bigToInt() == 0)
                    executeInstruction((JMP << 24) | SIGN_EXTEND(IMMEDIATE(instruction)), -1, RUN);
            }
        break;
        case BRT:
            if(mode == LIST)
                printf("%05d:  BRT   %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                bip.op1 = pullObjRefFromStack();
                if(bigToInt() == 1)
                    executeInstruction((JMP << 24) | SIGN_EXTEND(IMMEDIATE(instruction)), -1, RUN);
            }
        break;
        case CALL:
            if(mode == LIST)
                printf("%05d:  CALL  %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                pushValueToStack(programPointer);
                executeInstruction((JMP << 24) | SIGN_EXTEND(IMMEDIATE(instruction)), -1, RUN);
            }
        break;
        case RET:
            if(mode == LIST)
                printf("%05d:  RET\n", line);
            else if(mode == RUN){
                executeInstruction((JMP << 24) | pullValueFromStack(), -1, RUN);
            }
        break;
        case DROP:
            if(mode == LIST)
                printf("%05d:  DROP\n", line);
            else if(mode == RUN){
                stackPointer -= SIGN_EXTEND(IMMEDIATE(instruction));
            }
        break;
        case PUSHR:
            if(mode == LIST)
                printf("%05d:  PUSHR\n", line);
            else if(mode == RUN){
                pushObjRefToStack(returnValueRegister);
            }
        break;
        case POPR:
            if(mode == LIST)
                printf("%05d:  POPR\n", line);
            else if(mode == RUN){
                returnValueRegister = pullObjRefFromStack();
            }
        break;
        case DUP:
            if(mode == LIST)
                printf("%05d:  DUP\n", line);
            else if(mode == RUN){
                if(stack[stackPointer-1].isObjRef){
                    ObjRef refToDuplicate = pullObjRefFromStack();
                    pushObjRefToStack(refToDuplicate);
                    pushObjRefToStack(refToDuplicate);
                }
                else if(!stack[stackPointer-1].isObjRef){
                    int value = pullValueFromStack();
                    pushValueToStack(value);
                    pushValueToStack(value);
                }
            }
        break;
        case NEW:
            if(mode == LIST)
                printf("%05d:  NEW  %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                pushObjRefToStack(newCompoundObject(SIGN_EXTEND(IMMEDIATE(instruction))));
            }
        break;
        case GETF:
            if(mode == LIST)
                printf("%05d  GETF  %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                pushObjRefToStack(GET_REFS(pullObjRefFromStack())[SIGN_EXTEND(IMMEDIATE(instruction))]);
            }
        break;
        case PUTF:
            if(mode == LIST)
                printf("%05d  PUTF  %d\n", line, SIGN_EXTEND(IMMEDIATE(instruction)));
            else if(mode == RUN){
                ObjRef value = pullObjRefFromStack();
                ObjRef object = pullObjRefFromStack();
                GET_REFS(object)[SIGN_EXTEND(IMMEDIATE(instruction))] = value;
            }
        break;
        case NEWA:
            if(mode == LIST)
                printf("%05d  NEWA\n", line);
            else if(mode == RUN){
                bip.op1 = pullObjRefFromStack();                
                checkNullRefException(bip.op1);
                pushObjRefToStack(newCompoundObject(bigToInt()));
            }
        break;
        case GETFA:
            if(mode == LIST)
                printf("%05d  GETFA\n", line);
            else if(mode == RUN){
                bip.op1 = pullObjRefFromStack();
                checkNullRefException(bip.op1);
                ObjRef arr = pullObjRefFromStack();
                checkNullRefException(arr);
                pushObjRefToStack(GET_REFS(arr)[bigToInt()]);
            }
        break;
        case PUTFA:
            if(mode == LIST)
                printf("%05d  PUTFA\n", line);
            else if(mode == RUN){
                ObjRef value = pullObjRefFromStack();
                bip.op1 = pullObjRefFromStack();
                ObjRef arr = pullObjRefFromStack();
                checkNullRefException(value);
                checkNullRefException(bip.op1);
                checkNullRefException(arr);
                GET_REFS(arr)[bigToInt()] = value;
            }
        break;
        case GETSZ:
            if(mode == LIST)
                printf("%05d  GETSZ\n", line);
            else if(mode == RUN){
                ObjRef obj = pullObjRefFromStack();
                checkNullRefException(obj);
                if(IS_PRIM(obj)){
                    executeInstruction((PUSHC << 24) | IMMEDIATE(-1), -1, RUN);
                }
                else{
                    executeInstruction((PUSHC << 24) | IMMEDIATE(GET_SIZE(obj)), -1, RUN);
                }
            }
        break;
        case PUSHN:
            if(mode == LIST)
                printf("%05d  PUSHN\n", line);
            else if(mode == RUN){
                pushObjRefToStack(NULL);
            }
        break;
        case REFEQ:
            if(mode == LIST)
                printf("%05d  REFEQ\n", line);
            else if(mode == RUN){
                pullObjRefFromStack() == pullObjRefFromStack() ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);
            }
        break;
        case REFNE:
            if(mode == LIST)
                printf("%05d  REFNE\n", line);
            else if(mode == RUN){
                pullObjRefFromStack() != pullObjRefFromStack() ? executeInstruction((PUSHC << 24) | IMMEDIATE(1), -1, RUN) : executeInstruction((PUSHC << 24) | IMMEDIATE(0), -1, RUN);

            }
    }
}

void pushValueToStack(int number){
    if(stackPointer >= stackSlots)
        error("Stack overflow!\n");
    if(stackPointer < 0)
        error("Stack pointer pointing at negative stack element!\n");
    stack[stackPointer].isObjRef = false;
    stack[stackPointer].slotContent.value = number;
    stackPointer++;
}

int pullValueFromStack(void){
    stackPointer--;
    if(stackPointer < 0)
        error("Stack underflow!\n");
    if(stack[stackPointer].isObjRef)
        error("Stack Element is not a Value!\n");
    return stack[stackPointer].slotContent.value;
}

void pushObjRefToStack(ObjRef objRef){
    if(stackPointer >= stackSlots)
        error("Stack overflow!\n");
    if(stackPointer < 0)
        error("Stack pointer pointing at negative stack element!\n");
    stack[stackPointer].isObjRef = true;
    stack[stackPointer].slotContent.objRef = objRef;
    stackPointer++;
}

ObjRef pullObjRefFromStack(){
    stackPointer--;
    if(stackPointer < 0)
        error("Stack underflow!\n");
    if(!stack[stackPointer].isObjRef)
        error("Stack Element is not a ObjRef!\n");
    return stack[stackPointer].slotContent.objRef;
}

void* vm_alloc(int bytes){
    if((unsigned char *)freePointer + bytes > ((unsigned char *)zielSpeicher + heapSizeToAllocate/2 * 1024)){//{
        gc();
        if((unsigned char *)freePointer + bytes > ((unsigned char *)zielSpeicher + heapSizeToAllocate/2 * 1024))//zielSpeicher + heapSizeToAllocate/2 * 1024)
            error("Not enough Memory available!\n");
    }
    void* pointer = freePointer;
    oldByteCount += bytes;
    freePointer = (unsigned char *)freePointer + bytes;
    return pointer;
}

ObjRef newPrimObject(int dataSize){
    ObjRef primObj = vm_alloc(sizeof(unsigned int) + dataSize * sizeof(unsigned char));      
    if(primObj == NULL)
        error("No memory available!\n");
    primObj->size = (unsigned int)dataSize;
    objectsCreatedSinceLastGC++;
    return primObj;
}

ObjRef newCompoundObject(int numObjRefs){
    ObjRef compoundObj = vm_alloc(sizeof(unsigned int) + numObjRefs * sizeof(ObjRef));
    if(compoundObj == NULL)
        error("No memory available!\n");
    compoundObj->size = (unsigned int)(MSB | numObjRefs);
    for(int i = 0; i < numObjRefs; i++)
        GET_REFS(compoundObj)[i] = NULL; 
        objectsCreatedSinceLastGC++;
    return compoundObj;
}

void checkNullRefException(ObjRef ref){
    if(ref == NULL)
        error("Null reference exception\n");
}

void fatalError(char* msg){
    error(msg);
}

void error(char* errorMsg){
    printf("ERROR: %s", errorMsg);
    exit(-1);
}
