#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include <nxsh.h>

/*
    Get directory listing
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - director listing, or error message
*/
char *nxsh_ls(int argc, char **argv) {
    DIR *handle;
    char *current_dir = nxsh_cwd();
    char *dir = current_dir;
    struct dirent *entry;

    int FLAG_A = 0;
    int FLAG_L = 0;

    // Parse the arguments
    for (int i=0; i<argc; i++) {

        // It's an option
        if (argv[i][0] == '-') {
            for (int j=1; j<strlen(argv[i]); j++) {
                switch (argv[i][j]) {
                    case 'a':
                        FLAG_A = 1;
                        break;
                    case 'l':
                        FLAG_L = 1;
                        break;
                    default:
                        return error("Usage: ls [options] [directory]\r\n" \
                                    "Options:\r\n\t-l\tuse a long listing format\r\n" \
                                    "\t-a\tdo not ignore entries starting with .\r\n");
                }
            }
        }

        // Set the desired directory
        else {
            dir = argv[i];
        }
    }

    // Check if it is indeed a directory
    if (is_file(dir))
        return error("Error: not a directory\r\n");

    handle = opendir(dir);
    if (handle) {

        // Calculate the length of the output
        size_t length = 0;
        while ((entry = readdir(handle))) {
            length += strlen(entry->d_name) + 2;
        }

        // Create the output buffer
        char *output = malloc((sizeof(char) + 27) * length);            // 27 <- permissions size + max file size
        memset(output, '\0', sizeof(char));
        
        // Re-read the directory and append appropriate entries to the output buffer
        closedir(handle);
        handle = opendir(dir);

        char permissions[11];
        char size[16];
        struct stat st;
        mode_t mode;

        chdir(dir);

        // Step through all items in the directory
        while ((entry = readdir(handle))) {
            if ((entry->d_name[0] == '.' && FLAG_A) || (entry->d_name[0] != '.')) {

                if (FLAG_L) {
                    stat(entry->d_name, &st);

                    // Permissions
                    mode = st.st_mode;

                    permissions[0] = ((mode & 0040000) == 16384) ? 'd' : '-';   // is directory?
                    permissions[1] = ((mode & 00400) == 256) ? 'r' : '-';        // owner readable
                    permissions[2] = ((mode & 00200) == 128) ? 'w' : '-';       // owner writable
                    permissions[3] = ((mode & 00100) == 64) ? 'x' : '-';        // owner executable
                    permissions[4] = ((mode & 00040) == 32) ? 'r' : '-';        // group readable
                    permissions[5] = ((mode & 00020) == 16) ? 'w' : '-';       // group writable
                    permissions[6] = ((mode & 00010) == 8) ? 'x' : '-';        // group executable
                    permissions[7] = ((mode & 00004) == 4) ? 'r' : '-';        // other readable
                    permissions[8] = ((mode & 00002) == 2) ? 'w' : '-';       // other writable
                    permissions[9] = ((mode & 00001) == 1) ? 'x' : '-';        // other executable
                    permissions[10] = '\0';

                    strcat(output, permissions);
                    strcat(output, "\t");

                    // Size
                    sprintf(size, "%i", (int)st.st_size);
                    strcat(output, size);
                    strcat(output, "\t\t");
                }

                // File name
                strcat(output, entry->d_name);
                strcat(output, "\r\n");
            }
        }

        closedir(handle);
        chdir(current_dir);
        return output;
    }
    
    return error("Error: specified directory does not exist\r\n");
}