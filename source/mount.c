#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <switch.h>
#include <nxsh.h>

#define MOUNT_USAGE "Usage: mount <option> <args>\r\n" \
                    "Options:\r\n\t--bis\tMount BIS partition\r\n" \
                    "\t--game-save\tMount game save\r\n" \
                    "\t--system-save\tMount system save\r\n"

#define MOUNT_BIS_SUCCESS "Mounted partition %s to \"%s\" successfully\r\n"
#define MOUNT_SYS_SUCCESS "Mounted system save %s to \"%s\" successfully\r\n"
#define MOUNT_GAME_SUCCESS "Mounted game save from user %s and game %s to \"%s\" successfully\r\n"

#define UMOUNT_SUCCESS "Unmounted \"%s\" successfully\r\n"

char *nxsh_mount(int argc, char **argv) {
    if (argc < 1)
        return error(MOUNT_USAGE);

    char *success = NULL;
    FsFileSystem dev;

    if (strcmp(argv[0], "--bis") == 0) {
        if (argc < 3)
            return error("Usage: mount --bis <partition> <device>\r\n");

        if (R_FAILED(fsOpenBisFileSystem(&dev, atoi(argv[1]), "")))
            return error("Opening file system failed\r\n");

        if (fsdevMountDevice(argv[2], dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (strlen(MOUNT_BIS_SUCCESS) - 4 + strlen(argv[1]) + strlen(argv[2])));
        sprintf(success, MOUNT_BIS_SUCCESS, argv[1], argv[2]);
    } else if (strcmp(argv[0], "--system-save") == 0) {
        if (argc < 3)
            return error("Usage: mount --system-save <save id> <device>\r\n");

        if (R_FAILED(fsMount_SystemSaveData(&dev, strtoul(argv[1], NULL, 16))))
            return error("Mounting save data failed\r\n");

        if (fsdevMountDevice(argv[2], dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (strlen(MOUNT_SYS_SUCCESS) - 4 + strlen(argv[1]) + strlen(argv[2])));
        sprintf(success, MOUNT_SYS_SUCCESS, argv[1], argv[2]);
    } else if (strcmp(argv[0], "--game-save") == 0) {
        if (argc < 4)
            return error("Usage: mount --game-save <user id> <title id> <device>\r\n");

        for (int i=0; i<strlen(argv[0]); i++) {
            argv[1][i] = tolower(argv[1][i]);
        }

        u128 user_id = 0;
        char id[33];
        memset(id, '0', sizeof(id));
        id[32] = '\0';
        memcpy(id + 32 - strlen(argv[1]), argv[1], strlen(argv[1])); // Pad to 32 characters
        for (int i=0; i<16; i++) {
            sscanf(id + i * 2, "%02hhx", (u8 *) &user_id + (15 - i));
        }

        Result rc = fsMount_SaveData(&dev, strtoul(argv[2], NULL, 16), user_id);
        if (R_FAILED(rc))
            return error("Mounting save data failed\r\n");

        if (fsdevMountDevice(argv[3], dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (strlen(MOUNT_GAME_SUCCESS) - 4 + strlen(argv[1]) + strlen(argv[2])));
        sprintf(success, MOUNT_GAME_SUCCESS, argv[1], argv[2], argv[3]);
    } else {
        return error(MOUNT_USAGE);
    }

    return success;
}

char *nxsh_umount(int argc, char **argv) {
    if (argc < 1)
        return error("Usage: umount <device>\r\n");
    if (R_FAILED(fsdevCommitDevice(argv[0])))
        return error("Committing failed\r\n");
    if (fsdevUnmountDevice(argv[0]) == -1)
        return error("Unmounting failed\r\n");
    char *success = malloc(sizeof(char) * (strlen(UMOUNT_SUCCESS) - 2 + strlen(argv[0])));
    sprintf(success, UMOUNT_SUCCESS, argv[0]);
    return success;
}

char *nxsh_commit_dev(int argc, char **argv) {
    if (argc < 1) {
        char *drive = nxsh_cwd();
        strtok(drive, ":/");
        if (R_FAILED(fsdevCommitDevice(drive)))
            return error("Committing failed\r\n");
    } else if (strcmp(argv[0], "--help") == 0) {
        return error("Usage: commit-dev [device]\r\n");
    } else {
        if(R_FAILED(fsdevCommitDevice(argv[0])))
            return error("Committing failed\r\n");
    }

    return NULL;
}