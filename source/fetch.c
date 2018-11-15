#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

#include <nxsh.h>

#define NXSH_FETCH_BEGIN "Fetching file...\r\n"
#define NXSH_FETCH_END "]...done!\r\n"

int last_percent;
int response_fd;

/*
    Write function invoked by curl
*/
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

/*
    Progress bar function invoked by curl
*/
int progress_bar(void *bar, double total, double downloaded, double ultotal, double ulnow) {
    int percent = (int)((downloaded / total)*(float)100);

    if (percent % 4 == 0 && percent > last_percent) {
        if (percent == 0)
            send(response_fd, "[", 1, 0);

        send(response_fd, "=", 1, 0);
        last_percent = percent;
    }
    
    return 0;
}

/*
    Fetch file from remote source

    @returns - NULL on success, or error
*/
char *nxsh_fetch(int argc, char **argv, int connfd) {
    if (argc < 1 || argc > 2)
        return error("Usage: fetch [URL] [output name]\r\n" \
                    "\t(note that the second parameter is optional)\r\n");

    // Set the output file name
    char *output_file;
    if (argc == 2) {
        output_file = malloc(sizeof(char) * (strlen(argv[2]) + 1));
        strcpy(output_file, argv[1]);
    }
    else {
        char *basename = filename(argv[0]);
        if (basename == NULL || strlen(basename) == 0)
            return("Error: could not determine output file name\r\n");

        output_file = malloc(sizeof(char) * (strlen(basename) + 1));
        strcpy(output_file, basename);
    }

    CURL *handle;
    FILE *output;
 
    // Setup curl
    handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, argv[0]);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, progress_bar);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYSTATUS, 0L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);

    // Open the output file
    int error_code = 0;
    output = fopen(output_file, "wb");

    if (output) {

        // Set our globals so the progress bar function can see them
        last_percent = -1;
        response_fd = connfd;
        
        // Send the beginning message
        send(connfd, NXSH_FETCH_BEGIN, strlen(NXSH_FETCH_BEGIN)+1, 0);

        curl_easy_setopt(handle, CURLOPT_WRITEDATA, output);
        error_code = curl_easy_perform(handle);
        fclose(output);
        curl_easy_cleanup(handle);
    }
    else {
        free(output_file);
        return("Error: could not open output file for writing\r\n");
    }

    // Check to see if there was an error
    if (error_code != 0) {

        if (exists(output_file))
            remove(output_file);

        free(output_file);

        return error("Error: could not download specified file (note that HTTPS is not supported)\r\n");
    }
    
    // Send ending message
    send(connfd, NXSH_FETCH_END, strlen(NXSH_FETCH_END)+1, 0);

    free(output_file);
    return NULL;
}