#include "njvm.h"
#include "gc.h"
#include <string.h>

bool gcStatsFlag = false;
bool gcPurgeFlag = false;

unsigned int offset = 0;

unsigned int objectsCreatedSinceLastGC = 0;
unsigned int objectsCopiedDuringGC = 0;
unsigned int oldByteCount = 0;
unsigned int newByteCount = 0;

void* zielSpeicher;
void* quellSpeicher;
void* freePointer;

void gc(void){

    offset = 0;
    newByteCount = 0;

    //heap-haelften vertauschen
    void* tmpPointer = zielSpeicher;
    zielSpeicher = quellSpeicher;
    quellSpeicher = tmpPointer;
    freePointer = zielSpeicher;

    returnValueRegister  = relocate(returnValueRegister);

    bip.op1 = relocate(bip.op1);
    bip.op2 = relocate(bip.op2);
    bip.res = relocate(bip.res);
    bip.rem = relocate(bip.rem);

    for(int i = 0; i < staticDataAreaSize; i++){
        staticDataArea[i] = relocate(staticDataArea[i]);
    }

    for(int i = 0; i < stackPointer; i++){
        if(stack[i].isObjRef)
            stack[i].slotContent.objRef = relocate(stack[i].slotContent.objRef);
    }

    scan();

    if(gcPurgeFlag)
        memset(quellSpeicher, 0, heapSizeToAllocate/2);
    
    if(gcStatsFlag){
        printf("Garbage Collector:\n");
        printf("    %d objects (%d bytes) allocated since last collection\n", objectsCreatedSinceLastGC, oldByteCount);
        printf("    %d objects (%d bytes) copied during this collection\n", objectsCopiedDuringGC, newByteCount);
        printf("    %d of %d bytes free after this collection\n", (heapSizeToAllocate/2 * 1024)-newByteCount, (heapSizeToAllocate/2 * 1024));
    }

    objectsCreatedSinceLastGC = 0;
    objectsCopiedDuringGC = 0;
    oldByteCount = 0;
    newByteCount = 0;

}

void scan(){

    void* scan = zielSpeicher;
    while(scan!=freePointer){
        if(IS_PRIM((ObjRef)scan)){
            scan = (unsigned char *)scan + sizeof(unsigned int) + GET_SIZE((ObjRef) scan);
        }else{
            for(int i = 0; i < GET_SIZE((ObjRef)scan);i++){
                GET_REFS((ObjRef) scan)[i] = relocate(GET_REFS((ObjRef) scan)[i]);
            }
            scan=(unsigned char *)scan + sizeof(unsigned int) + (GET_SIZE((ObjRef) scan) * sizeof(ObjRef));
        }
    }

}

ObjRef relocate(ObjRef orig){
    ObjRef copy;

    if(orig == NULL)
        copy = NULL;
    else{
        if((orig->size & BROKEN_HEART_FLAG)){ 
            //Objekt ist bereits kopiert & Forward Pointer ist gesetzt
            copy = (ObjRef)((unsigned char *)zielSpeicher + (orig->size & (~(MSB | BROKEN_HEART_FLAG))));
        }else{
            //Objekt muss noch kopiert werden
            copy = copyObjtoFreeMem(orig);
        }
    }
    return copy;
}

ObjRef copyObjtoFreeMem(ObjRef orig){
    int bytes = 0;
    if(IS_PRIM(orig))
        bytes = sizeof(unsigned int) + GET_SIZE(orig);
    else
        bytes = sizeof(unsigned int) + GET_SIZE(orig) * sizeof(ObjRef);
    ObjRef cpy = memcpy(freePointer, orig, bytes);

    newByteCount += bytes;
    objectsCopiedDuringGC++;

    orig->size = 0;
    orig->size |= offset;
    orig->size |= BROKEN_HEART_FLAG;
    
    freePointer = (unsigned char *)freePointer + bytes;

    offset += bytes;
    return cpy;

}