#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <nxsh.h>

/*
    Enable/disable logging (persists across sessions)
    Essentially checks if /logs/nxsh.log exists. If it does, and "disabled" is
    requested, then it will delete the log. Otherwise, it is created.

    @param argc - argument count
    @param argv - array of pointers to arguments

    @returns - status of logging, or error
*/
char *nxsh_log(int argc, char **argv) {

    if (argc != 1)
        return error("Usage: log [option]\r\n" \
                    "\tenable - enable logging and create log file\r\n" \
                    "\tdisable - disable logging and delete log file\r\n" \
                    "\tstatus - display status of logging\r\n");

    // Enable logging
    if (strcmp(argv[0], "enable") == 0) {

        if (!exists(NXSH_DIR))
            if (mkdir(NXSH_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                return error("Error: could not create log directory\r\n");
        
        char *log_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 11));
        sprintf(log_file, "%s/nxsh.log", NXSH_DIR);

        if (!exists(log_file)) {
            FILE *fp = fopen(log_file, "w+");
            if (!fp) {
                free(log_file);
                return error("Error: could not create log file\r\n");
            }
            fclose(fp);
        }
        else {
            return error("Logging already enabled (?)\r\n");
        }

        // Return message
        char *message = malloc(sizeof(char) * (strlen(log_file) + 21));
        sprintf(message, "Log file created: %s\r\n", log_file);
        free(log_file);
        NXSH_LOGGING_ENABLED = 1;

        return message;
    }

    // Disable logging
    else if (strcmp(argv[0], "disable") == 0) {
        if (!logging_enabled())
            return error("Logging is not enabled (?)\r\n");
        else {
            char *log_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 11));
            sprintf(log_file, "%s/nxsh.log", NXSH_DIR);
 
            if (remove(log_file) != 0) {
                free(log_file);
                return error("Error: could not delete log file\r\n");
            }

            // Return message
            char *message = malloc(sizeof(char) * (strlen(log_file) + 21));
            sprintf(message, "Log file deleted: %s\r\n", log_file);
            free(log_file);
            NXSH_LOGGING_ENABLED = 0;

            return message;
        }
    }

    // Get logging status
    else if (strcmp(argv[0], "status") == 0) {
        if (!exists(NXSH_DIR))
            return error("Logging status: disabled\r\n");
        
        char *log_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 12));
        sprintf(log_file, "%s/nxsh.log", NXSH_DIR);

        if (!exists(log_file)) {
            free(log_file);
            return error("Logging status: disabled\r\n");
        }

        free(log_file);
        return error("Logging status: enabled\r\n");
    }

    else {
        return error("Usage: log [option]\r\n" \
                    "\tenable - enable logging and create log file\r\n" \
                    "\tdisable - disable logging and delete log file\r\n" \
                    "\tstatus - print status of logging\r\n");
    }
}

/*
    Write to the log file, including date
    @param input - item to log
*/
void write_log(char *input) {
    if (input != NULL && strlen(input) > 0) {
        char *log_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 11));
        sprintf(log_file, "%s/nxsh.log", NXSH_DIR);

        FILE *log = fopen(log_file, "a");  
        if (log) {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            char s[64];
            strftime(s, sizeof(s), "%c", tm);

            fprintf(log, "[%s] %s\r\n", s, input);  
            fclose(log);  
        }

        free(log_file);
    }
}

/*
    Write to the log file
    @param input - item to log
*/
void write_log_raw(char *input) {
    if (input != NULL && strlen(input) > 0) {
        char *log_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 11));
        sprintf(log_file, "%s/nxsh.log", NXSH_DIR);

        FILE *log = fopen(log_file, "a");  
        if (log) {
            fprintf(log, "%s\r\n", input);  
            fclose(log);  
        }

        free(log_file);
    }
}

/*
    Check if logging is enabled
    
    @returns - 1 if it is, 0 if not
*/
int logging_enabled() {

    // Logging directory doesn't exist
    if (!exists(NXSH_DIR))
        return 0;
    else {
        char *log_file = malloc(sizeof(char) * (strlen(NXSH_DIR) + 11));
        sprintf(log_file, "%s/nxsh.log", NXSH_DIR);

        // Log file doesn't exist
        if (!exists(log_file)) {
            free(log_file);
            return 0;
        }

        free(log_file);
        return 1;
    }

}