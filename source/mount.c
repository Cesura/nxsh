#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>

#include <switch.h>
#include <nxsh.h>

#define MOUNT_USAGE "Usage: mount <option> <args>\r\n" \
                    "Options:\r\n\t--bis\tMount BIS partition\r\n" \
                    "\t--game-save\tMount game save\r\n" \
                    "\t--system-save\tMount system save\r\n" \
                    "\t--pfs0\tMount PFS0\r\n" \
                    "\t--sd-card\tMount SD card\r\n" \
                    "\t--gamecard\tMount gamecard\r\n"

#define MOUNT_BIS_SUCCESS "Mounted partition %s to \"%s\" successfully\r\n"
#define MOUNT_SYS_SUCCESS "Mounted system save %s to \"%s\" successfully\r\n"
#define MOUNT_GAME_SUCCESS "Mounted game save from user %s and game %s to \"%s\" successfully\r\n"
#define MOUNT_PFS0_SUCCESS "Mounted PFS0 %s to \"%s\" successfully\r\n"
#define MOUNT_SD_SUCCESS "Mounted SD card to \"%s\" successfully\r\n"
#define MOUNT_GC_SUCCESS "Mounted gamecard partiton %s to \"%s\" successfully\r\n"

#define UMOUNT_SUCCESS "Unmounted \"%s\" successfully\r\n"
#define UMOUNT_FAIL "Unmounting \"%s\" failed\r\n"

#define COMMIT_FAIL "Commiting \"%s\" failed\r\n"

Result fsOpenGameCardFileSystem(FsFileSystem *out, FsGameCardHandle handle, u32 partition) {
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u32 handle;
        u32 partition;
    } *raw;

    raw = serviceIpcPrepareHeader(fsGetServiceSession(), &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 31;
    raw->handle = handle.value;
    raw->partition = partition;

    Result rc = serviceIpcDispatch(fsGetServiceSession());

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(fsGetServiceSession(), &r, sizeof(*resp));

        resp = r.Raw;

        rc = resp->result;

        if (R_SUCCEEDED(rc)) {
            serviceCreateSubservice(&out->s, fsGetServiceSession(), &r, 0);
        }
    }

    return rc;
}

char *nxsh_mount(int argc, char **argv) {
    if (argc < 1)
        return error(MOUNT_USAGE);

    char *success = NULL;
    FsFileSystem dev;

    if (strcmp(argv[0], "--bis") == 0) {
        if (argc < 3)
            return error("Usage: mount --bis <partition id> <device>\r\n");

        if (R_FAILED(fsOpenBisFileSystem(&dev, atoi(argv[1]), "")))
            return error("Opening file system failed\r\n");

        if (fsdevMountDevice(argv[2], dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (sizeof(MOUNT_BIS_SUCCESS) - 4 + strlen(argv[1]) + strlen(argv[2])));
        success[0] = '\0';
        sprintf(success, MOUNT_BIS_SUCCESS, argv[1], argv[2]);
    } else if (strcmp(argv[0], "--system-save") == 0) {
        if (argc < 3)
            return error("Usage: mount --system-save <save id> <device>\r\n");

        if (R_FAILED(fsMount_SystemSaveData(&dev, strtoul(argv[1], NULL, 16))))
            return error("Mounting save data failed\r\n");

        if (fsdevMountDevice(argv[2], dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (sizeof(MOUNT_SYS_SUCCESS) - 4 + strlen(argv[1]) + strlen(argv[2])));
        success[0] = '\0';
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

        success = malloc(sizeof(char) * (sizeof(MOUNT_GAME_SUCCESS) - 6 + strlen(argv[1]) + strlen(argv[2]) + strlen(argv[3])));
        success[0] = '\0';
        sprintf(success, MOUNT_GAME_SUCCESS, argv[1], argv[2], argv[3]);
    } else if (strcmp(argv[0], "--pfs0") == 0) {
        if (argc < 3)
            return error("Usage: mount --pfs0 <PFS0 file> <device>\r\n");

        char *path = NULL;
        if (argv[1][0] != '@') {
            char abs_path[PATH_MAX];
            char *ptr = realpath(argv[1], abs_path);
            if (ptr == NULL || strncmp("sdmc:/", abs_path, strlen("sdmc:/")) != 0) {
                path = malloc(strlen(argv[1]) + 1);
                strcpy(path, argv[1]);
            } else {
                path = malloc(sizeof("@Sdcard:/") + strlen(abs_path) - strlen("sdmc:/"));
                path[0] = '\0';
                strcpy(path, "@Sdcard:/");
                strcat(path, abs_path + strlen("sdmc:/"));
            }
        } else {
            path = malloc(strlen(argv[1]) + 1);
            path[0] = '\0';
            strcpy(path, argv[1]);
        }

        Result rc = fsOpenFileSystem(&dev, FsFileSystemType_ApplicationPackage, path);
        free(path);
        if (R_FAILED(rc)) {
            return error("Opening file system failed\r\n");
        }

        if (fsdevMountDevice(argv[2], dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (sizeof(MOUNT_PFS0_SUCCESS) - 4 + strlen(argv[1]) + strlen(argv[2])));
        success[0] = '\0';
        sprintf(success, MOUNT_PFS0_SUCCESS, argv[1], argv[2]);
    } else if (strcmp(argv[0], "--sd-card") == 0) {
        if (R_FAILED(fsMountSdcard(&dev)))
            return error("Mounting SD card failed\r\n");

        char *device;
        if (argc < 2)
            device = "sdmc";
        else
            device = argv[1];

        if (fsdevMountDevice(device, dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (sizeof(MOUNT_SD_SUCCESS) - 2 + strlen(device)));
        success[0] = '\0';
        sprintf(success, MOUNT_SD_SUCCESS, device);
    } else if (strcmp(argv[0], "--gamecard") == 0) {
        if (argc < 3)
            return error("Usage: mount --gamecard <partition id> <device>\r\n");

        FsDeviceOperator op;
        if (R_FAILED(fsOpenDeviceOperator(&op)))
            return error("Opening device operator failed\r\n");

        bool gc_inserted = 0;
        if (R_FAILED(fsDeviceOperatorIsGameCardInserted(&op, &gc_inserted)))
            return error("Detecting whether the gamecard is inserted failed\r\n");

        if (!gc_inserted)
            return error("Gamecard must be inserted\r\n");

        FsGameCardHandle gc_handle;
        if (R_FAILED(fsDeviceOperatorGetGameCardHandle(&op, &gc_handle)))
            return error("Getting gamecard handle failed\r\n");

        if (R_FAILED(fsOpenGameCardFileSystem(&dev, gc_handle, atoi(argv[1]))))
            return error("Opening filesytem failed\r\n");

        if (fsdevMountDevice(argv[2], dev) == -1)
            return error("Mounting failed\r\n");

        success = malloc(sizeof(char) * (sizeof(MOUNT_GC_SUCCESS) - 4 + strlen(argv[1]) + strlen(argv[2])));
        success[0] = '\0';
        sprintf(success, MOUNT_GC_SUCCESS, argv[1], argv[2]);
    } else {
        return error(MOUNT_USAGE);
    }

    return success;
}

char *nxsh_umount(int argc, char **argv) {
    if (argc < 1)
        return error("Usage: umount <device>\r\n");

    char *out = malloc(1);
    out[0] = '\0';

    for (int i=0; i<argc; i++) {
        if (R_FAILED(fsdevCommitDevice(argv[i]))) {
            char error[sizeof(COMMIT_FAIL) - 2 + strlen(argv[i])];
            sprintf(error, COMMIT_FAIL, argv[i]);
            out = realloc(out, strlen(out) + strlen(error) + 1);
            strcat(out, error);
        }

        if (fsdevUnmountDevice(argv[i]) == -1) {
            char error[sizeof(UMOUNT_FAIL) - 2 + strlen(argv[i])];
            sprintf(error, UMOUNT_FAIL, argv[i]);
            out = realloc(out, strlen(out) + strlen(error) + 1);
            strcat(out, error);
            continue;
        }

        char success[sizeof(UMOUNT_SUCCESS) - 2 + strlen(argv[i])];
        sprintf(success, UMOUNT_SUCCESS, argv[i]);
        out = realloc(out, strlen(out) + strlen(success) + 1);
        strcat(out, success);
    }

    return out;
}

char *nxsh_commit_dev(int argc, char **argv) {
    char device[PATH_MAX];
    device[0] = '\0';

    if (argc < 1) {
        getcwd(device, PATH_MAX);
        strtok(device, ":/");
    } else {
        strcpy(device, argv[0]);
    }

    if (strcmp(device, "--help") == 0) {
        return error("Usage: commit-dev [device]\r\n");
    } else {
        if(R_FAILED(fsdevCommitDevice(device))) {
            char *error = malloc(sizeof(COMMIT_FAIL) - 2 + strlen(device));
            sprintf(error, COMMIT_FAIL, device);
            return error;
        }
    }

    return NULL;
}