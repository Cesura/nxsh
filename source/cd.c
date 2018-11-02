#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <nxsh.h>

/*
    Change directory
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - NULL if everything worked, otherwise error message
*/
char *nxsh_cd(int argc, char **argv) {
    char *dir;

    // If no argument is passed, assume the root directory
    if (argc == 0)
        dir = "/";
    else
        dir = argv[0];

    // Ensure the given path is a directory
    if (is_file(dir) == 1)
        return error("Error: not a directory\r\n");

    // Change directory, returning error if failure
    if (chdir(dir) == 0) {
        return NULL;
    }
    else {
        return error("Error: specified directory does not exist\r\n");
    }
}