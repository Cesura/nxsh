#ifdef __SYS__
#include <sysmodule.h>

#include <unistd.h>

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

    rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }
    
    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalSimple(rc);
    rc = fsdevMountSdmc();
    if (R_FAILED(rc))
        fatalSimple(rc);

    /* The NRO begins in a folder under sdmc:/,
    so for consistency make the sysmodule change
    its directory to sdmc:/ instead of / */
    chdir("sdmc:/");

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
    socketExit();
    fsdevUnmountAll();
    timeExit();
    fsExit();
    smExit();
}
#endif
