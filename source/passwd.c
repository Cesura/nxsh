#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <nxsh.h>

/*
    Authenticate the user with the given password
    @param password - password to test

    @returns 1 if correct password, 0 if incorrect
*/
int nxsh_authenticate(char *password) {
    char *pw_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 10));
    sprintf(pw_file, "%s/nxsh.pw", NXSH_DIR);

    if (!exists(pw_file)) {
        free(pw_file);
        return 1;
    }
    else {
        FILE *fp = fopen(pw_file, "r");
        free(pw_file);
        
        if (fp) {
            // Get a hash of the passed password, and the system password 
            char *sys_pw = malloc(sizeof(char) * 33);
            char *user_pw = md5_hash(password, strlen(password));
            fread(sys_pw, sizeof(char), 32, fp);
            fclose(fp);

            sys_pw[32] = '\0';

            // Compare the passwords
            if (strcmp(user_pw, sys_pw) == 0) {
                free(user_pw);
                free(sys_pw);
                return 1;
            }
            
            free(user_pw);
            free(sys_pw);
            return 0;
        }
        else {
            return 0;
        }
    }
}

/*
    Set/modify/clear password
    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - NULL if everything worked, otherwise error message
*/
char *nxsh_passwd(int argc, char **argv) {
    if (argc < 1 || argc > 2)
        return error("Usage: passwd [option] [value]\r\n" \
                    "\tset - set the password with the given parameter\r\n" \
                    "\tclear - clear the password\r\n");

    char *pw_file;

    if (strcmp(argv[0], "set") == 0 && argc == 2) {

        // Make sure it's at least 6 characters
        if (strlen(argv[1]) < 6)
            return error("Error: password must be at least 6 characters\r\n");

        if (!exists(NXSH_DIR))
            if (mkdir(NXSH_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                return error("Error: could not create password directory\r\n");

        pw_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 10));
        sprintf(pw_file, "%s/nxsh.pw", NXSH_DIR);

        // Set the new password
        FILE *fp = fopen(pw_file, "w+");
        if (fp) {
            char *hashed_pw = md5_hash(argv[1], strlen(argv[1]));
            fwrite(hashed_pw, sizeof(char), 32, fp);
            free(hashed_pw);
            fclose(fp);
            free(pw_file);
            return error("Password set successfully. You will be prompted for it upon next login.\r\n");
        }
        else {
            return error("Error: could not open password file\r\n");
        }
    }
    else if (strcmp(argv[0], "clear") == 0) {
        pw_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 10));
        sprintf(pw_file, "%s/nxsh.pw", NXSH_DIR);

        // Delete the password file, if it exists
        if (exists(pw_file)) {
            remove(pw_file);
            free(pw_file);
            return error("Password cleared successfully\r\n");
        }
    }
    else {
        return error("Usage: passwd [option] [value]\r\n" \
                    "\tset - set the password with the given parameter\r\n" \
                    "\tclear - clear the password\r\n");
    }

    return NULL;
}