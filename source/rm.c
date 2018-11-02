#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include <nxsh.h>

/*
    Recursively delete directory
    @param path - path to desired directory

    @returns - status of deletion
*/
int recursive_delete(const char *path) {
    DIR *handle = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (handle) {
        struct dirent *p;
        r = 0;

        while (!r && (p = readdir(handle))) {
            int r2 = -1;
            char *buf;
            size_t len;

            // Skip . and ..
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                continue;
            }

            len = path_len + strlen(p->d_name) + 2; 
            buf = malloc(len);

            if (buf) {
                struct stat s;
                snprintf(buf, len, "%s/%s", path, p->d_name);

                if (!stat(buf, &s)) {
                    if (S_ISDIR(s.st_mode))
                        r2 = recursive_delete(buf);
                    else {
                        r2 = remove(buf);
                    }
                }

                free(buf);
            }

            r = r2;
        }

        closedir(handle);
    }

    if (!r) {
        r = rmdir(path);
    }

    return r;
}

/*
    Removes a file or directory
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - NULL if everything worked, otherwise error message
*/
char *nxsh_rm(int argc, char **argv) {
    if (argc == 0)
        return error("Error: please specify a file or directory\r\n");

    int FLAG_R = 0;
    char *targets[128];
    int count = 0;

    // Parse the arguments
    for (int i=0; i<argc; i++) {

        // It's an option
        if (argv[i][0] == '-') {
            for (int j=1; j<strlen(argv[i]); j++) {
                switch (argv[i][j]) {
                    case 'r':
                        FLAG_R = 1;
                        break;
                    default:
                        return error("Usage: rm [options] [file/directory]\r\n" \
                                    "Options:\r\n\t-r\trecurse into directory.\r\n");
                }
            }
        }
        else {
            targets[count++] = argv[i];
        }
    }

    // Loop through each file/directory
    for (int i=0; i<count; i++) {
        
        // Check if it's a directory
        if (!is_file(targets[i])) {

            // Recursive flag not set
            if (!FLAG_R) {
                char *error = malloc(sizeof(char) * (strlen(targets[i] + 128)));
                sprintf(error, "Error: cannot recurse into directory '%s'\r\n", targets[i]);
                return error;
            }

            // Recursively delete the directory
            if (recursive_delete(targets[i]) != 0)
                return error("Error: could not delete specified directory.\r\n");
        }
        else {

            // Delete the regular file
            if (remove(targets[i]) != 0)
                return error("Error: could not delete specified file.\r\n");
        }

    }

    return NULL;
}

