#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#include <nxsh.h>

/*
    Backend for file copies
    @param from - source path
    @param in_to - destination path

    @returns - 0 if success, -1 if fail
*/
int do_file_copy(char *from, char *in_to) {

    char *to;
    
    if (exists(in_to) && !is_file(in_to)) {
        char *basename = filename(from);
        to = malloc(strlen(in_to) + strlen(basename) + 2);
        sprintf(to, "%s/%s", in_to, basename);
        free(basename);
    }
    else {
        to = malloc(strlen(in_to) + 1);
        strcpy(to, in_to);
    }

    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0) {
        close(fd_from);
        if (fd_to >= 0)
            close(fd_to);
        return -1;
    }

    while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR) {
                close(fd_from);
                if (fd_to >= 0)
                    close(fd_to);
                return -1;
            }
        } while (nread > 0);
    }

    close(fd_to);
    close(fd_from);

    free(to);

    return 0;
}

/*
    Recurse into directories
    @param src - source path
    @param dest - destination path

    @returns - 0 if success, -1 if fail
*/
int recursive_copy(char *src, char *dest) {

    int status = 0;

    // Check if src is a directory, and recurse through if it is
    if (!is_file(src)) {

        // Create the directory if it doesn't exists
        if (access(dest, F_OK) == -1) {
            if (mkdir(dest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                return -1;
        }

        DIR *handle = opendir(src);
        char *real_src, *real_dest;

        if (handle) {
            struct dirent *p;

            while ((p = readdir(handle))) {
                
                // Skip . and ..
                if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                    continue;
                }

                // Construct real paths
                real_src = malloc(strlen(p->d_name) + strlen(src) + 2);
                sprintf(real_src, "%s/%s", src, p->d_name);
                real_dest = malloc(strlen(p->d_name) + strlen(dest) + 2);
                sprintf(real_dest, "%s/%s", dest, p->d_name);

                // Copy a file
                if (is_file(real_src)) {
                    if (do_file_copy(real_src, real_dest) != 0)
                        return -1;
                }

                // Create dest directory and recurse
                else {
                    if (mkdir(real_dest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                        return -1;

                    // Recursively copy
                    status = recursive_copy(real_src, real_dest);

                    free(real_src);
                    free(real_dest);
                }
            }

            closedir(handle);
        }
    }

    // Simple file copy
    else {
        if (do_file_copy(src, dest) != 0)
            return -1;
    }

    return status;
}

/*
    Removes a file or directory
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - NULL if everything worked, otherwise error message
*/
char *nxsh_cp(int argc, char **argv) {
    if (argc == 0)
        return error("Error: please specify a file or directory\r\n");
    else if (argc == 1)
        return error("Error: please specify a destination\r\n");

    int FLAG_R = 0;
    char *targets[128];
    char *dest;
    int count = 0;

    // Parse the arguments
    for (int i=0; i<argc; i++) {

        // It's an option
        if (argv[i][0] == '-') {
            for (int j=1; j<strlen(argv[i]); j++) {
                switch (argv[i][j]) {
                    case 'r':
                        FLAG_R = 1;
                        break;
                    default:
                        return error("Usage: cp [options] [source] [destination]\r\n" \
                                    "Options:\r\n\t-r\tcopy directories recursively\r\n");
                }
            }
        }
        else {
            targets[count++] = argv[i];
        }
    }

    dest = targets[count-1];
    if (is_file(dest) && count > 2) {
        return error("Error: destination must be a directory\r\n");
    }

    // Simple file copy, destination should be file
    if (count == 2 && is_file(targets[0])) {
        if (do_file_copy(targets[0], dest) != 0)
            return error("Error: could not copy file\r\n");
    }

    // Something more complicated
    else {
        int dest_exists = 1;
        if (access(dest, F_OK) == -1) {
            dest_exists = 0;
            if (mkdir(dest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                return error("Error: could not create destination directory\r\n");
        }

        char *real_src, *real_dest;

        // Loop through each file/directory
        for (int i=0; i<count-1; i++) {

            if (access(targets[i], F_OK) == -1) {

                // Remove the directory we created if the copy failed and the 
                // destination is empty
                if (!dest_exists && is_dir_empty(dest))
                    remove(dest);

                return error("Error: specified source does not exist\r\n");
            }

            if (!is_file(targets[i]) && !FLAG_R) {
                return error("Error: -r omitted, not recursively copying\r\n");
            }

            if (!dest_exists) {
                DIR *handle = opendir(targets[i]);

                if (handle) {
                    struct dirent *p;

                    while ((p = readdir(handle))) {
                        
                        // Skip . and ..
                        if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                            continue;
                        }

                        // Create the real path to copy to/from
                        real_src = malloc(strlen(targets[i]) + strlen(p->d_name) + 2);
                        sprintf(real_src, "%s/%s", targets[i], p->d_name);
                        real_dest = malloc(strlen(targets[i]) + strlen(dest) + strlen(p->d_name) + 2);
                        sprintf(real_dest, "%s/%s", dest, p->d_name);

                        recursive_copy(real_src, real_dest);
                        free(real_src);
                        free(real_dest);
                    }

                    closedir(handle);
                }
            }
            else {
                real_dest = malloc(strlen(targets[i]) + strlen(dest) + 2);
                sprintf(real_dest, "%s/%s", dest, targets[i]);
                recursive_copy(targets[i], real_dest);
                free(real_dest);
            }
        }
    }

    return NULL;
}

