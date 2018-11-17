#include <switch.h>

#define TITLE_ID 0x420420420420420 //This should be a purposeful number, but mine isn't
#define HEAP_SIZE 0x540000 //I think this has to be a multiple of 2MB

//setup a fake heap
char fake_heap[HEAP_SIZE];

void __libnx_initheap(void);

void fatalLater(Result err);

void registerFspLr();

void __appInit(void);

void __appExit(void);
