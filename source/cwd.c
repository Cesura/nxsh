#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <nxsh.h>

/*
    Get current working directory

    @returns - pointer to buffer with current working directory
*/
char *nxsh_cwd() {

    char *output;
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        output = malloc(sizeof(char) * strlen(cwd)+1);
        strcpy(output, cwd);
        return output;
    }
    
    return NULL;
}