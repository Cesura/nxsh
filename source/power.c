#include <unistd.h>
#include <string.h>

#include <switch.h>
#include <nxsh.h>

char *nxsh_reboot(int argc, char **argv, int connfd) {
    if (argc == 0) {
        close(connfd);
        fsdevUnmountAll();
        bpcInitialize();
        bpcRebootSystem();
    } else if (argc == 1 && strcmp(argv[0], "--rcm") == 0) {
        close(connfd);
        svcSleepThread(1e+8L); // Sleep for a decisecond because otherwise the socket won't close correctly, not sure why
        splInitialize();
        fsdevUnmountAll();
        splSetConfig((SplConfigItem) 65001, 1);
    } else {
        return error("Usage: reboot [options]\r\n" \
                     "Options:\r\n\t--rcm\tReboot to RCM\r\n");
    }
    return "";
}

char *nxsh_shutdown(int connfd) {
    close(connfd);
    svcSleepThread(1e+8L);
    bpcInitialize();
    fsdevUnmountAll();
    bpcShutdownSystem();
    return "";
}
