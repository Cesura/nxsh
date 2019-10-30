#include <string.h>
#include <stdio.h>

#include <nxsh.h>
#include <switch.h>

#define ACC_USAGE "Usage: acc <option> [args]\r\n" \
                  "Options:\r\n\tlist\tLists all accounts\r\n" \
                  "\tfind\tFinds an account based on username\r\n"
#define ACC_ENTRY "%s\t%s\r\n" // User id, username

char *nxsh_acc(int argc, char **argv) {
    if (argc < 1)
        return error(ACC_USAGE);

    char *out = NULL;

    if (strcmp(argv[0], "list") == 0) {
        accountInitialize();

        s32 num_accounts;
        accountGetUserCount(&num_accounts);

        AccountUid account_ids[num_accounts];
        s32 total;
        accountListAllUsers(account_ids, INT32_MAX, &total);

        out = malloc(num_accounts * (sizeof(ACC_ENTRY) + 60)); // Enough room for max length u128 in base 16 (32) and max length username (32)
        out[0] = '\0';

        for (int i=0; i<num_accounts; i++) {
            AccountProfile profile;
            accountGetProfile(&profile, account_ids[i]);

            AccountProfileBase profile_base;
            accountProfileGet(&profile, NULL, &profile_base);

            char entry[sizeof(ACC_ENTRY) + 60];
            char *id = format_u128_hex(*(u128*)&account_ids[i]);
            sprintf(entry, ACC_ENTRY, id, profile_base.nickname);
            strcat(out, entry);
            free(id);
        }

        accountExit();
    } else if (strcmp(argv[0], "find") == 0) {
        if (argc < 2)
            return error("Usage: acc find <username>\r\n");

        accountInitialize();

        s32 num_accounts;
        accountGetUserCount(&num_accounts);

        AccountUid account_ids[num_accounts];
        s32 total;
        accountListAllUsers(account_ids, INT32_MAX, &total);

        int i;
        for (i=0; i<num_accounts; i++) {
            AccountProfile profile;
            accountGetProfile(&profile, account_ids[i]);

            AccountProfileBase profile_base;
            accountProfileGet(&profile, NULL, &profile_base);

            if (strcmp(profile_base.nickname, argv[1]) == 0) {
                out = malloc(sizeof(ACC_ENTRY) + 60); // Enough room for max length u128 in base 16 (32) and max length username (32)
                out[0] = '\0';

                char *id = format_u128_hex(*(u128*)&account_ids[i]);
                sprintf(out, ACC_ENTRY, id, profile_base.nickname);
                free(id);
                break;
            }
        }

        accountExit();

        if (i == num_accounts)
            return error("No account found with that name\r\n");
    } else {
        return error(ACC_USAGE);
    }

    return out;
}