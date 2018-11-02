#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <nxsh.h>

/*
    Print file contents to standard output
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - contents of file(s) in a buffer, or error messsage
*/
char *nxsh_cat(int argc, char **argv) {
    if (argc == 0)
        return error("Usage: cat [file] ([file2] [file3] ...)\r\n");

    // Parse the arguments
    FILE *fp;
    char *output = NULL;
    char *contents;
    size_t prevlen = 0;

    for (int i=0; i<argc; i++) {

        // Ensure the file exists
        if (access(argv[i], F_OK) == -1)
            return error("Error: specified file does not exist\r\n");

        // Ensure it is a file
        if (!is_file(argv[i]))
            return error("Error: cannot operate on directories\r\n");


        fp = fopen(argv[i], "r");
        if (fp != NULL) {
            if (fseek(fp, 0L, SEEK_END) == 0) {

                // Get file size
                long bufsize = ftell(fp);
                contents = malloc(sizeof(char) * (bufsize + 3));

                if (fseek(fp, 0L, SEEK_SET) != 0) { }

                // Read the file into memory
                size_t newlen = fread(contents, sizeof(char), bufsize, fp);
                if (ferror(fp) != 0) {
                    fclose(fp);
                    return error("Error: could not read specified file\r\n");
                }
                else {
                    contents[newlen++] = '\r';
                    contents[newlen++] = '\n';
                    contents[newlen++] = '\0';

                    if (prevlen == 0) {
                        output = malloc(sizeof(char) * (bufsize + 3));
                        strcpy(output, contents);
                    }

                    // For concatinating multiple files
                    else {
                        output = realloc(output, prevlen + bufsize + 4);
                        strcat(output, "\n");
                        strcat(output, contents);
                    }

                    free(contents);
                    prevlen += bufsize + 3;
                }
            }
            fclose(fp);
        }
        else {
            fclose(fp);
            return error("Error: could not read specified file\r\n");
        }
    }

    return output;
}

