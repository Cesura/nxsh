#ifdef __SYS__
#include <sysmodule.h>

// we aren't an applet
u32 __nx_applet_type = AppletType_None;

// we override libnx internals to do a minimal init
void __libnx_initheap(void)
{
    extern char *fake_heap_start;
    extern char *fake_heap_end;

    // setup newlib fake heap
    fake_heap_start = fake_heap;
    fake_heap_end = fake_heap + HEAP_SIZE;
}

void __appInit(void)
{
    Result rc;
    rc = smInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);
    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);
    rc = fsdevMountSdmc();
    if (R_FAILED(rc))
        fatalSimple(rc);
    rc = timeInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);
    __libnx_init_time();
    rc = socketInitializeDefault();
    if (R_FAILED(rc))
        fatalSimple(rc);
}

void __appExit(void)
{
    fsdevUnmountAll();
    fsExit();
    smExit();
    audoutExit();
    timeExit();
    socketExit();
}
#endif
