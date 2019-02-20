#include <string.h>

#include <nxsh.h>

char *nxsh_echo(int argc, char **argv) {
    int out_length = 0;
    for (int i=0; i<argc; i++) {
        out_length += strlen(argv[i])+1;
    }
    
    char *out = malloc(sizeof(char) * (out_length + 2));
    memset(out, '\0', sizeof(char));
    for (int i=0; i<argc; i++) {
        strcat(out, argv[i]);
        strcat(out, " ");
    }
    strcat(out, "\r\n");
    
    return out;
}
