#include <stdio.h>
#include <unistd.h>

#include <nxsh.h>

char *nxsh_touch(int argc, char **argv) {
    if (argc == 0)
        return error("Usage: touch FILE...\r\n");

    for (int i=0; i<argc; i++) {
        if (!is_file(argv[i])) {
            FILE *fp = fopen(argv[i], "w");
            fclose(fp);
        } else {
            FILE *fp = fopen(argv[i], "a+");
            fseek(fp, 0L, SEEK_END);
            size_t len = ftell(fp);
            rewind(fp);
            fprintf(fp, "\n");
            ftruncate(fileno(fp), len-1);
            fclose(fp);
        }
    }

    return NULL;
}
