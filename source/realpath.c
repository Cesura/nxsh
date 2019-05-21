/*  $OpenBSD: realpath.c,v 1.13 2005/08/08 08:05:37 espie Exp $ */
/*
 * Copyright (c) 2003 Constantin S. Svintsoff <kostik@iclub.nsu.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nxsh.h>

/*
 * char *realpath(const char *path, char resolved[PATH_MAX]);
 *
 * Find the real name of path, by removing all ".", ".." and symlink
 * components.  Returns (resolved) on success, or (NULL) on failure,
 * in which case the path which caused trouble is left in (resolved).
 */
char *
realpath(const char *path, char resolved[PATH_MAX])
{
    char *p, *q, *s;
    size_t left_len = 0, resolved_len = 0;
    char left[PATH_MAX], next_token[PATH_MAX];
    if (path[0] == '/') {
        char drive[PATH_MAX];
        if (getcwd(drive, PATH_MAX) == NULL) {
            strlcpy(resolved, ".", PATH_MAX);
            return (NULL);
        }
        strtok(drive, "/");
        resolved_len = strlcpy(resolved, drive, PATH_MAX);
        strlcat(resolved, "/", PATH_MAX);
        resolved_len++;
        if (path[1] == '\0')
            return (resolved);
        left_len = strlcpy(left, path + 1, sizeof(left));
    } else {
        int i, path_len = strlen(path);
        for (i=0; i<path_len; i++) {
            if (path[i] == '/') {
                if (path[i - 1] == ':') {
                    strncpy(resolved, path, i + 1);
                    resolved_len = i + 1;
                    left_len = strlcpy(left, path + i, sizeof(left));
                } else {
                    i = path_len;
                }
                break;
            }
        }

        if (i == path_len) {
            if (getcwd(resolved, PATH_MAX) == NULL) {
                strlcpy(resolved, ".", PATH_MAX);
                return (NULL);
            }
            resolved_len = strlen(resolved);
            left_len = strlcpy(left, path, sizeof(left));
        }
    }
    if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
        errno = ENAMETOOLONG;
        return (NULL);
    }
    /*
     * Iterate over path components in `left'.
     */
    while (left_len != 0) {
        /*
         * Extract the next path component and adjust `left'
         * and its length.
         */
        p = strchr(left, '/');
        s = p ? p : left + left_len;
        if (s - left >= sizeof(next_token)) {
            errno = ENAMETOOLONG;
            return (NULL);
        }
        memcpy(next_token, left, s - left);
        next_token[s - left] = '\0';
        left_len -= s - left;
        if (p != NULL)
            memmove(left, s + 1, left_len + 1);
        if (resolved[resolved_len - 1] != '/') {
            if (resolved_len + 1 >= PATH_MAX) {
                errno = ENAMETOOLONG;
                return (NULL);
            }
            resolved[resolved_len++] = '/';
            resolved[resolved_len] = '\0';
        }
        if (next_token[0] == '\0')
            continue;
        else if (strcmp(next_token, ".") == 0)
            continue;
        else if (strcmp(next_token, "..") == 0) {
            /*
             * Strip the last path component except when we have
             * single "/"
             */
            if (resolved_len > 1) {
                resolved[resolved_len - 1] = '\0';
                q = strrchr(resolved, '/') + 1;
                *q = '\0';
                resolved_len = q - resolved;
            }
            continue;
        }
        /*
         * Append the next path component and lstat() it. If
         * lstat() fails we still can return successfully if
         * there are no more path components left.
         */
        resolved_len = strlcat(resolved, next_token, PATH_MAX);
        if (resolved_len >= PATH_MAX) {
            errno = ENAMETOOLONG;
            return (NULL);
        }
    }
    /*
     * Remove trailing slash except when the resolved pathname
     * is a single "/".
     */
    int just_drive = 0;
    for (int i=0; i<resolved_len; i++) {
        if (resolved[i] == '/' && resolved[i - 1] == ':' && ++just_drive > 1)
            break;
    }
    if (resolved_len > 1 && resolved[resolved_len - 1] == '/' && just_drive > 1)
        resolved[resolved_len - 1] = '\0';
    return (resolved);
}

char *nxsh_realpath(int argc, char **argv) {
    if (argc == 0)
        return error("Usage: realpath <path>...\r\n");

    char *out = malloc(1);
    out[0] = '\0';

    for (int i=0; i<argc; i++) {
        char path[PATH_MAX];
        realpath(argv[i], path);
        out = realloc(out, strlen(out) + strlen(path) + 3);
        strcat(out, path);
        strcat(out, "\r\n");
    }

    return out;
}