#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <nxsh.h>
#include <duktape.h>

int NXSH_REPLY_FD;
int NXSH_FD_LOCK;

/*
    Native print() js function
    @param ctx - duktape context

    @returns - number of return values
    
    Script usage:
    print(message)
*/
static duk_ret_t native_print(duk_context *ctx) {
    duk_push_string(ctx, " ");
    duk_insert(ctx, 0);
    duk_join(ctx, duk_get_top(ctx) - 1);
    const char *result = duk_safe_to_string(ctx, -1);
    send(NXSH_REPLY_FD, result, strlen(result)+1, 0);
    send(NXSH_REPLY_FD, "\r\n", 2, 0);
    return 0;
}

/*
    Native readFile() js function
    @param ctx - duktape context

    @returns - number of return values

    Script usage:
    readFile(filename)
*/
static duk_ret_t native_readFile(duk_context *ctx) {

    char *contents = NULL;
    const char *file = duk_to_string(ctx, 0);
    
    // Ensure the file exists
    if (access(file, F_OK) == -1)
        goto error;

    // Ensure it is a file
    if (!is_file((char *)file))
        goto error;

    FILE *fp = fopen(file, "r");
    long bufsize = 0;
    if (fp) {
        if (fseek(fp, 0L, SEEK_END) == 0) {

            // Get file size
            bufsize = ftell(fp);
            contents = malloc(sizeof(char) * (bufsize + 1));

            if (fseek(fp, 0L, SEEK_SET) != 0) { }

            // Read the file into memory
            fread(contents, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0) {
                fclose(fp);
                goto error;
            }
        }

        fclose(fp);
        contents[bufsize] = '\0';
        duk_push_string(ctx, contents);
    }

    return 1;

error:
    duk_push_string(ctx, "Error: could not read file");
    return 1;
}

/*
    Native writeFile() js function
    @param ctx - duktape context

    @returns - number of return values

    Script usage:
    writeFile(filename, content)
*/
static duk_ret_t native_writeFile(duk_context *ctx) {

    const char *file = duk_to_string(ctx, 0);
    const char *to_write = duk_to_string(ctx, 1);

    FILE *fp = fopen(file, "w+");
    if (fp) {
        if (fseek(fp, 0L, SEEK_END) == 0) {

            // Write to the desired file
            fwrite(to_write, sizeof(char), strlen(to_write), fp);
            if (ferror(fp) != 0) {
                fclose(fp);
                goto error;
            }
        }
        fclose(fp);
    }

    return 0;

error:
    duk_push_string(ctx, "Error: could not write to desired file");
    return 1;
}

/*
    Initialize duktape with custom nxsh functions
*/
duk_context *nxsh_duktape_init() {

    duk_context *ctx = duk_create_heap_default();

    // Set the print function
    duk_push_c_function(ctx, native_print, DUK_VARARGS);
    duk_put_global_string(ctx, "print");

    // Set the readFile function
    duk_push_c_function(ctx, native_readFile, 1);
    duk_put_global_string(ctx, "readFile");

    // Set the writeFile function
    duk_push_c_function(ctx, native_writeFile, 2);
    duk_put_global_string(ctx, "writeFile");

    return ctx;
}


/*
    Execute a script via duktape
    @param path - path to the desired script
    @param argc - argument count
    @param argv - array of pointers to arguments
    @param argv - socket to respond to

    @returns - pointer to output
*/
char *nxsh_script(char *path, int argc, char **argv, int connfd) {
    NXSH_REPLY_FD = connfd;

    // Initialize duktape with custom nxsh functions
    duk_context *ctx = nxsh_duktape_init();

    // Open the desired script
    FILE *fp;
    duk_size_t len;
    fp = fopen(path, "rb");
    if (fp) {
        if (fseek(fp, 0L, SEEK_END) == 0) {

            // Calculate how large of a buffer we need
            long bufsize = ftell(fp);
            char *buf = malloc(sizeof(char) * (bufsize + 3));
            if (fseek(fp, 0L, SEEK_SET) != 0) { }

            // Read the file into memory and push it to the duktape stack
            len = fread(buf, sizeof(char), bufsize, fp);
            fclose(fp);
            duk_push_lstring(ctx, (const char *) buf, (duk_size_t) len);
        }
    }
    else {
        duk_push_undefined(ctx);
    }

    // If an error occurred, print it out and skip destroying the context
    if (duk_peval(ctx) != 0) {
        const char *error = duk_safe_to_string(ctx, -1);
        send(NXSH_REPLY_FD, error, strlen(error)+1, 0);
        send(NXSH_REPLY_FD, "\r\n", 2, 0);
        goto finished;
    }

    duk_pop(ctx);
    duk_destroy_heap(ctx);

finished:
    return NULL;
}