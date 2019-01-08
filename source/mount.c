#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <switch.h>
#include <nxsh.h>

#define MOUNT_SUCCESS "Mounted partition %s to \"%s\" successfully\r\n"
#define UMOUNT_SUCCESS "Unmounted \"%s\" successfully\r\n"

char *nxsh_mount(int argc, char **argv) {
    if (argc < 2)
        return error("Usage: mount [partition id] [drive name]\r\n");
    FsFileSystem dev;
    if (R_FAILED(fsOpenBisFileSystem(&dev, atoi(argv[0]), "")))
        return error("Mounting failed\r\n");
    if (fsdevMountDevice(argv[1], dev) == -1)
        return error("Mounting failed\r\n");
    char *success = malloc(sizeof(char) * (strlen(MOUNT_SUCCESS) - 4 + strlen(argv[0]) + strlen(argv[1])));
    sprintf(success, MOUNT_SUCCESS, argv[0], argv[1]);
    return success;
}

char *nxsh_umount(int argc, char **argv) {
    if (argc < 1)
        return error("Usage: umount [drive name]\r\n");
    if (fsdevUnmountDevice(argv[0]) == -1)
        return error("Unmounting failed\r\n");
    char *success = malloc(sizeof(char) * (strlen(UMOUNT_SUCCESS) - 2 + strlen(argv[0])));
    sprintf(success, UMOUNT_SUCCESS, argv[0]);
    return success;
}
