#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

#include <nxsh.h>
#include <switch.h>
#include <md5.h>
 
/*
    Trim whitespace from head and tail
    @param input - pointer to be operated on
*/
void trim(char *input) {
   char *dst = input, *src = input;
   char *end;

   while (isspace((unsigned char)*src)){
      ++src;
   }

   end = src + strlen(src) - 1;
   while (end > src && isspace((unsigned char)*end)) {
      *end-- = 0;
   }

   if (src != dst) {
      while ((*dst++ = *src++));
   }
}

/*
    Allocates buffer for given message, and copies it in
    @param msg - pointer to error message

    @returns - pointer to allocated memory
*/
char *error(char *msg) {
    char *ptr = malloc(strlen(msg)+1);
    strcpy(ptr, msg);
    return ptr;
}

/*
    Check if directory is empty
    @param dir - directory in question

    @returns - 1 if empty, 0 if not
*/
int is_dir_empty(char *dir) {
    int n = 0;
    struct dirent *d;
    DIR *handle = opendir(dir);
    if (dir == NULL)
        return 1;
    while ((d = readdir(handle)) != NULL) {
        if (++n > 2)
            break;
    }
    closedir(handle);
    if (n <= 2)
        return 1;
    else
        return 0;
}

/*
    Check if specified path is a file
    @param name - path in question

    @returns - 1 if file, 0 if not
*/
int is_file(char *name) {
    struct stat buf;
    stat(name, &buf);
    return S_ISREG(buf.st_mode);
}

/*
    Check if file exists
    @param name - path in question

    @returns - 1 if it does, 0 if not
*/
int exists(char *name) {
    if (access(name, F_OK) == -1)
        return 0;
    return 1;
}


/*
    Similar to basename()
    @param path - full path of file

    @returns - the base file name of the given path
*/
char *filename(char *path) {
    char *fullpath = malloc(strlen(path)+1);
    strcpy(fullpath, path);
    char *filename = malloc(strlen(path)+1);

    char *tok = strtok(fullpath, "/");
    strcpy(filename, tok);

    while ((tok = strtok(NULL, "/")) != NULL) {
        strcpy(filename, tok);
    }

    free(fullpath);
    return filename;
}

/*
    Strip the SD card prefix from the path
    @param inpath - unstripped path

    @returns - either the input string (if no prefix present), or
                a pointer to the stripped version
*/
char *strip_prefix(char *inpath) {
    if (strlen(inpath) < 5)
        return inpath;

    char *test = malloc(sizeof(char) * 6);
    strncpy(test, inpath, 5);
    test[5] = '\0';

    if (strcmp(test, "sdmc:") == 0) {
        char *outpath = malloc(sizeof(char) * (strlen(inpath) - 4));
        memcpy(outpath, inpath+5, strlen(inpath)-4);

        free(inpath);
        free(test);
        return outpath;
    }
    else {
        free(test);
        return inpath;
    }
}

char *hash(void *input, size_t size, const char *type) {
    char *hash_str = malloc(1);
    hash_str[0] = '\0';

    if (strcmp(type, "md5") == 0) {
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, input, size);
        unsigned char digest[16];
        MD5_Final(digest, &ctx);

        hash_str = malloc(sizeof(char) * 33);
        for (int i = 0; i < 16; ++i)
            sprintf(&hash_str[i*2], "%02x", (unsigned int)digest[i]);

        hash_str[32] = '\0';
    } else if (strcmp(type, "sha1") == 0) {
        u8 hash_bytes[SHA1_HASH_SIZE];
        hash_str = malloc(sizeof(char) * (SHA1_HASH_SIZE * 2 + 1));

        sha1CalculateHash(hash_bytes, input, size);

        for (int i=0; i<sizeof(hash_bytes); i++)
            sprintf(hash_str + i * 2, "%02x", hash_bytes[i]);

        hash_str[SHA1_HASH_SIZE * 2] = '\0';
    } else if (strcmp(type, "sha256") == 0) {
        u8 hash_bytes[SHA256_HASH_SIZE];
        hash_str = malloc(sizeof(char) * (SHA256_HASH_SIZE * 2 + 1));

        sha256CalculateHash(hash_bytes, input, size);

        for (int i=0; i<sizeof(hash_bytes); i++)
            sprintf(hash_str + i * 2, "%02x", hash_bytes[i]);

        hash_str[SHA256_HASH_SIZE * 2] = '\0';
    }

    return hash_str;
}

char *format_u128_hex(u128 num) {
    u8 *ptr = (u8 *) &num;

    char *out = malloc(33);
    out[0] = '\0';

    for (int i=0; i<16; i++) {
        sprintf(out + i * 2, "%02x", ptr[15 - i]);
    }

    return out;
}