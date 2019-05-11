#include <string.h>
#include <stdio.h>

#include <switch.h>
#include <nxsh.h>

#define PM_USAGE "Usage: pm [option] [args]\r\n" \
                 "Options:\r\n\ttid2pid\tFind the process ID for a title ID\r\n" \
                 "\tpid2tid\tFind the title ID for a process ID\r\n" \
                 "\tkill\tKill a process\r\n" \
                 "\tlaunch\tLaunch a process\r\n"

char *nxsh_pm(int argc, char **argv) {
    if (argc == 0)
        return error(PM_USAGE);
    
    char *out = malloc(sizeof(char) * 22); // The maximum length of a u64 converted to a string in base ten plus newline stuff
    memset(out, 0, sizeof(char));
    
    if (strcmp(argv[0], "tid2pid") == 0) {
        if (argc >= 2) {
            #ifdef __SYS__
            free(out);
            return error("This command doesn't work when running as a sysmodule\r\n");
            #endif

            pmdmntInitialize();
            
            u64 pid;
            u64 tid = strtoul(argv[1], NULL, 16);
            Result rc = pmdmntGetTitlePid(&pid, tid);
            if (R_FAILED(rc)) {
                free(out);
                pmdmntExit();
                return error("No process exists with that title ID\r\n");
            }
            sprintf(out, "%ld\r\n", pid);
            
            pmdmntExit();
        } else {
            free(out);
            return error("Usage: pm tid2pid <title id>\r\n");
        }
    } else if (strcmp(argv[0], "pid2tid") == 0) {
        if (argc >= 2) {
            pminfoInitialize();
            
            u64 tid;
            u64 pid = strtoul(argv[1], NULL, 10);
            
            Result rc = pminfoGetTitleId(&tid, pid);
            if (R_FAILED(rc)) {
                sprintf(out, "ERROR: 0x%x\r\n", rc);
                pminfoExit();
                return out;
            }
            sprintf(out, "%lx\r\n", tid);
            
            pminfoExit();
        } else {
            free(out);
            return error("Usage: pm pid2tid <process id>\r\n");
        }
    } else if (strcmp(argv[0], "kill") == 0) {
        if (argc >= 3) {
            if (strcmp(argv[1], "--pid") == 0) {
                pmshellInitialize();
                
                u64 pid = strtoul(argv[2], NULL, 10);
                Result rc = pmshellTerminateProcessByProcessId(pid);
                if (R_FAILED(rc)) {
                    free(out);
                    pmshellExit();
                    return error("No process with that process ID\r\n");
                }
                
                pmshellExit();
            } else if (strcmp(argv[1], "--tid") == 0) {
                pmshellInitialize();
                
                u64 tid = strtoul(argv[2], NULL, 16);
                Result rc = pmshellTerminateProcessByTitleId(tid);
                if (R_FAILED(rc)) {
                    free(out);
                    pmshellExit();
                    return error("No process with that title ID\r\n");
                }
                
                pmshellExit();
            } else {
                free(out);
                return error("Usage: pm kill <option> <id>\r\n" \
                        "Options:\r\n\t--pid\tKill the process based on a process ID\r\n" \
                        "\t--tid\tKill the process based on a title ID\r\n");
            }
        } else {
            free(out);
            return error("Usage: pm kill <option> <id>\r\n" \
                        "Options:\r\n\t--pid\tKill the process based on a process ID\r\n" \
                        "\t--tid\tKill the process based on a title ID\r\n");
        }
    } else if (strcmp(argv[0], "launch") == 0) {
        if (argc >= 2) {
            pmshellInitialize();
            
            u32 flags = PmLaunchFlag_None;
            if (argc == 3)
                flags = strtoul(argv[2], NULL, 2); // Input for flags in binary; currently should only need 6 digits at most
            
            u64 pid;
            u64 tid = strtoul(argv[1], NULL, 16);
            Result rc = pmshellLaunchProcess(flags, tid, FsStorageId_None, &pid);
            if (R_FAILED(rc)) {
                free(out);
                pmshellExit();
                return error("Launching failed\r\n");
            }
            sprintf(out, "%ld\n", pid);
            
            pmshellExit();
        } else {
            free(out);
            return error("Usage: pm launch <title id> <launch flags>\r\n");
        }
    } else {
        free(out);
        return error(PM_USAGE);
    }
    
    return out;
}
