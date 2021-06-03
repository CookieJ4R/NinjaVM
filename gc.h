#ifndef NJVM_GC_INCLUDED
#define NJVM_GC_INCLUDED

#include "njvm.h"

#define BROKEN_HEART_FLAG (1 << (8 * sizeof(unsigned int) - 2))

extern bool gcPurgeFlag;
extern bool gcStatsFlag;

extern int heapSizeToAllocate;

extern void* zielSpeicher;
extern void* quellSpeicher;
extern void* freePointer;

extern unsigned int offset;

extern unsigned int oldByteCount;
extern unsigned int newByteCount;
extern unsigned int objectsCreatedSinceLastGC;
extern unsigned int objectsCopiedDuringGC;

void gc(void);

ObjRef relocate(ObjRef orig);

ObjRef copyObjtoFreeMem(ObjRef copy);

void scan();


#endif