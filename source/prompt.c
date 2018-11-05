#include <stdlib.h>
#include <string.h>

#include <nxsh.h>

/*
    Print prompt for shell
    NOTE: user/hostname are difficult to obtain, so dummy values are used

    @returns - pointer to buffer with prompt string
*/
char *nxsh_prompt() {
	char *prompt;
    char *cwd = strip_prefix(nxsh_cwd());
    char *user = "root";
    char *host = "switch";

    prompt = malloc(sizeof(char) * (strlen(cwd) + strlen(user) + strlen(host) + 6));

    strcpy(prompt, user);
    strcat(prompt, "@");
    strcat(prompt, host);
    strcat(prompt, ":");
    strcat(prompt, cwd);
    strcat(prompt, " # ");
    
    free(cwd);
    return prompt;
}