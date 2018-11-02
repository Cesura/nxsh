#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <nxsh.h>

/*
    Move file from source to dest (actually just a copy then remove for safety)
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - NULL if everything worked, otherwise error message
*/
char *nxsh_mv(int argc, char **argv) {

    if (argc < 2)
        return error("Usage: mv [source] [target]\r\n");
    
    argv[2] = malloc(sizeof(char) * 3);
    strcpy(argv[2], "-r");
    argc++;

    // First, do the copy
    char *cp_status = nxsh_cp(argc, argv);
    if (cp_status != NULL)
        return cp_status;
    free(cp_status);

    // Now, delete the original
    argv[1] = argv[2];
    argc = 2;
    char *rm_status = nxsh_rm(argc, argv);
    if (rm_status != NULL)
        return rm_status;
    
    return NULL;
}