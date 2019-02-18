#ifdef __SYS__
#include <switch.h>

#define TITLE_ID 0x43000000000000FF // This should be a purposeful number
#define HEAP_SIZE 0x280000

//setup a fake heap
char fake_heap[HEAP_SIZE];

void __libnx_initheap(void);

void __libnx_init_time(void);

void fatalLater(Result err);

void __appInit(void);

void __appExit(void);
#endif
