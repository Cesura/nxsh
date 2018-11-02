#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>
#include <errno.h>

#include <nxsh.h>
 
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
    DIR* handle = opendir(name);

    if (handle != NULL) {
        closedir(handle);
        return 0;
    }

    if (errno == ENOTDIR) {
        return 1;
    }

    return -1;
}