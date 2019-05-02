#include <stdio.h>
#include <string.h>

#include <nxsh.h>
#include <switch.h>

#define USAGE_ERROR "Usage: %ssum <path>...\r\n"
#define DIR_ERROR ": Is a directory\r\n"
#define NONEXIST_ERROR ": No such file or directory\r\n"

char *nxsh_hash(int argc, char **argv, const char *type) {
    if (argc == 0) {
        char *out = malloc(sizeof(USAGE_ERROR) - 2 + strlen(type));
        sprintf(out, USAGE_ERROR, type);
        return out;
    }

    char *out = NULL, *dirs = NULL, *nonexist = NULL;

    out = malloc(1);
    out[0] = '\0';

    dirs = malloc(1);
    dirs[0] = '\0';

    nonexist = malloc(1);
    nonexist[0] = '\0';

    for (int i=0; i<argc; i++) {
        if (exists(argv[i])) {
            if (is_file(argv[i])) {
                FILE *fp = fopen(argv[i], "r");

                fseek(fp, 0, SEEK_END);
                size_t size = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                u8 *buf = malloc(size);
                if (fread(buf, size, 1, fp) != 1) {
                    free(buf);
                    return error("There was an error reading a file\r\n");
                }

                char *hash_str = NULL;
                if (strcmp(type, "sha1") == 0) {
                    u8 hash_bytes[20];
                    sha1CalculateHash(hash_bytes, buf, size);
                    hash_str = malloc(sizeof(hash_bytes) * 2 + 1);
                    for (int i=0; i<sizeof(hash_bytes); i++) {
                        sprintf(hash_str + i * 2, "%02x", hash_bytes[i]);
                    }
                } else if (strcmp(type, "sha256") == 0) {
                    u8 hash_bytes[32];
                    sha256CalculateHash(hash_bytes, buf, size);
                    hash_str = malloc(sizeof(hash_bytes) * 2 + 1);
                    for (int i=0; i<sizeof(hash_bytes); i++) {
                        sprintf(hash_str + i * 2, "%02x", hash_bytes[i]);
                    }
                } else if (strcmp(type, "md5") == 0) {
                    hash_str = md5_hash(buf, size);
                }

                out = realloc(out, strlen(out) + strlen(hash_str) + strlen(argv[i]) + 7);

                strcat(out, hash_str);
                strcat(out, "  ");
                strcat(out, argv[i]);
                strcat(out, "\r\n");

                free(hash_str);
                free(buf);

                fclose(fp);
            } else {
                dirs = realloc(dirs, strlen(dirs) + strlen(argv[i]) + sizeof(DIR_ERROR) + 2);
                strcat(dirs, argv[i]);
                strcat(dirs, DIR_ERROR);
            }
        } else {
            nonexist = realloc(nonexist, strlen(nonexist) + strlen(argv[i]) + sizeof(NONEXIST_ERROR) + 2);
            strcat(nonexist, argv[i]);
            strcat(nonexist, NONEXIST_ERROR);
        }
    }
    out = realloc(out, strlen(out) + strlen(dirs) + strlen(nonexist) + 3);
    strcat(out, dirs);
    free(dirs);
    strcat(out, nonexist);
    free(nonexist);

    return out;
}