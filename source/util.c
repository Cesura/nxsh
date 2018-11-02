#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

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