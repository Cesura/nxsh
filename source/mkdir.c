#include <sys/stat.h>
#include <sys/types.h>

#include <nxsh.h>

/*
    Make directory
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - NULL if everything worked, otherwise error message
*/
char *nxsh_mkdir(int argc, char **argv) {
    if (argc == 0)
        return error("Error: please specify a directory name\r\n");

    if (mkdir(argv[0], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
        return NULL;
    else
        return error("Error: could not create directory\r\n");
}