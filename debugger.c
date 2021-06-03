#include <stdio.h>
#include <string.h>

#include "njvm.h"
#include "gc.h"
#include "debugger.h"

int breakpoint = -1;

void runDebugger(void){
    printf("Instructions: %d, Data: %d, Stackslots: %d, Heapsize: 2 * %d\n", programSize, staticDataAreaSize, stackSlots, heapSizeToAllocate/2 * 1024);
    printf("\nprogramPointer -> ");
    executeInstruction(program[programPointer], programPointer, LIST);
    printf("\nDEBUG MENU: list, inspect, step, run, breakpoint, gc, quit\n");
    char debugInstruction[255];
    while(!halt){
        scanf("%s", debugInstruction);
        if(strcmp("list", debugInstruction) == 0){
            listProgram();
        }else if(strcmp("step", debugInstruction) == 0){
            unsigned int instruction = program[programPointer++];
            executeInstruction(instruction, -1, RUN);
        }else if(strcmp("run", debugInstruction) == 0){
            runProgram();
        }else if(strcmp("quit", debugInstruction) == 0){
            halt = 1;
            return;
        }else if(strcmp("breakpoint", debugInstruction) == 0){
            scanf("%d", &breakpoint);
            printf("\nBreakpoint set to: %d\n", breakpoint);
        }else if(strcmp("inspect", debugInstruction) == 0){
            printf("\nDEBUG MENU: stack, data, ret, object, bip\n");
            scanf("%s", debugInstruction);
            if(strcmp("stack", debugInstruction) == 0){
                inspectStack();
            }else if(strcmp("data", debugInstruction) == 0){
                inspectData();
            }else if(strcmp("ret", debugInstruction) == 0){
                inspectReturnValue();
            }else if(strcmp("object", debugInstruction) == 0){
                inspectObject();
            }else if(strcmp("bip", debugInstruction) == 0){
                inspectBip();
            }
        }else if(strcmp("gc", debugInstruction) == 0){
            gc();
        }else{
            printf("Unknown debug instruction...\n");
        }
        if(halt) return;
        printf("\nprogramPointer -> ");
        executeInstruction(program[programPointer], programPointer, LIST);
        printf("\nDEBUG MENU: list, inspect, step, run, breakpoint, gc, quit\n");
    }
}


void listProgram(void){
    for(int i = 0; i < programSize; i++){
        listInstruction(program[i], i);
    }
}

void listInstruction(unsigned int instruction, int line){
    executeInstruction(instruction,line, LIST);
}

void inspectStack(void){
    printf("\n----Start of the Stack----\n");
    for(int i = stackPointer; i >= 0; i--){
        if(i == stackPointer && i == framePointer){
                printf("sp, fp       ->  %05d     (xxxxxx) xxx\n", i);
        }else if(i == stackPointer){
                printf("stackPointer ->  %05d     (xxxxxx) xxx\n", i);
        }else if(i == framePointer){
            if(!stack[i].isObjRef)
                printf("framePointer ->  %05d     (number) %d\n", i, stack[i].slotContent.value);
            else
                printf("framePointer ->  %05d     (objref) %p\n", i, (void *)stack[i].slotContent.objRef);
        }else{
            if(!stack[i].isObjRef)
                printf("                 %05d     (number) %d\n", i, stack[i].slotContent.value);
            else
                printf("                 %05d     (objref) %p\n", i, (void *)stack[i].slotContent.objRef);
        }
    }
    printf("----Bottom of the Stack----\n");
}

void inspectData(void){
    printf("\n----Start of static data----\n");
    for(int i = staticDataAreaSize - 1; i >= 0; i--){
        printf("data[%05d]:     (objref) %p\n", i, (void *)staticDataArea[i]);
    }
    printf("----End of static data----\n");
}

void inspectReturnValue(void){
    printf("----Return Value Register----\n");
    printf("%p\n", (void *)returnValueRegister);
}

void inspectObject(void){
    printf("Enter object reference:\n");
    void *address;
    scanf("%p", &address);
    if(address != NULL){
        ObjRef objReference = (ObjRef)address;
        if(IS_PRIM(objReference)){
            printf("Primitive Object\n");
            bip.op1 = objReference;
            printf("value=");
            bigPrint(stdout);
            printf("\n");
        }
        else{
            printf("Compound Object\n");
            for(int i = 0; i < GET_SIZE(objReference); i++)
                printf("field[%05d]  (objRef) %p\n", i, (void *)GET_REFS(objReference)[i]);
        }
    }
}


void inspectBip(void){
    printf("----Big Integer Processor----\n");
    printf("op1: %p\n", (void *)bip.op1);
    printf("op2: %p\n", (void *)bip.op2);
    printf("rem: %p\n", (void *)bip.rem);
    printf("res: %p\n", (void *)bip.res);

}