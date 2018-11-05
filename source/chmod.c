#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>

#include <nxsh.h>

/*
    Backend to chmod
    @param mode - permissions
    @param path - path to file/directory

    @returns - 0 if success, -1 if failure
*/
int do_chmod(int mode, char *path) {
    if (chmod(path, mode) < 0)
        return -1;

    return 0;
}

/*
    Recurse into directories
    @param mode - permissions
    @param target - file/directory to recurse into

    @returns - 0 if success, -1 if fail
*/
int recursive_chmod(int mode, char *target) {

    int status = 0;

    // Check if argument is actually a directory, and recurse through if it is
    if (!is_file(target)) {
        DIR *handle = opendir(target);
        char *real_path;

        if (handle) {
            struct dirent *p;
            while ((p = readdir(handle))) {

                // Skip . and ..
                if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                    continue;
                }

                // Target is a file
                if (is_file(p->d_name)) {
                    if (do_chmod(mode, p->d_name) != 0)
                        return -1;
                }

                // Target is a directory
                else {

                    // Construct real path
                    real_path = malloc(strlen(p->d_name) + strlen(target) + 2);
                    sprintf(real_path, "%s/%s", target, p->d_name);
                    
                    if (do_chmod(mode, real_path) != 0)
                        return -1;

                    // Recurse
                    status = recursive_chmod(mode, real_path);
                    free(real_path);
                }
            }

            closedir(handle);
        }
    }

    return status;
}


/*
    Change permissions
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - NULL if everything worked, otherwise error message
*/
char *nxsh_chmod(int argc, char **argv) {
    if (argc < 2)
        return error("Usage: chmod [permissions] [files]\r\n" \
            "Options:\r\n\t-r\trecursively set permissions\r\n");

    int FLAG_R = 0;
    char *targets[128];
    int count = 0;

    if (strlen(argv[0]) != 3) {
        return error("Error: invalid permissions\r\n");
    }

    // Convert permissions into usable format
    char *buf = malloc(sizeof(char) * 5);
    strcpy(buf, "0");
    strcat(buf, argv[0]);

    if (!isdigit(argv[0][0]) || !isdigit(argv[0][1]) || !isdigit(argv[0][2]))
        return error("Error: invalid permissions (check your parameters were passed in order)\r\n");

    if ((int)(argv[0][0] - '0') > 7 || (int)(argv[0][1] - '0') > 7 || (int)(argv[0][2] - '0') > 7)
        return error("Error: invalid permissions (check your parameters were passed in order)\r\n");

    int mode = strtol(buf, 0, 8);
    free(buf);
 
    // Parse the arguments
    for (int i=1; i<argc; i++) {

        // It's an option
        if (argv[i][0] == '-') {
            for (int j=1; j<strlen(argv[i]); j++) {
                switch (argv[i][j]) {
                    case 'r':
                        FLAG_R = 1;
                        break;
                    default:
                        return error("Usage: chmod [permissions] [options] [files]\r\n" \
                            "Options:\r\n\t-r\trecursively set permissions\r\n");
                }
            }
        }
        else {
            targets[count++] = argv[i];
        }
    }

    // Loop through each file/directory
    for (int i=0; i<count; i++) {
        if (!exists(targets[i]))
            return error("Error: no such file or directory\r\n");
        
        if (do_chmod(mode, targets[i]) != 0)
            return error("Error: could not set permissions\r\n");

        // If it's a directory, recurse
        if (!is_file(targets[i])) {
            if (FLAG_R) {
                if (recursive_chmod(mode, targets[i]) != 0) {
                    return error("Error: could not set permissions\r\n");
                }
            }
        }
    }

    return NULL;
}