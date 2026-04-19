#define _POSIX_C_SOURCE 200809L

#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <magic.h>
#include <sys/stat.h>
#include <fcntl.h>


void _get_args_list(int argc, char **argv, pair_t *pair) {
    pair->first = optind - 1;
    while (optind < argc && argv[optind][0] != '-') optind++;
    pair->second = optind - pair->first;
}


void print_usage() {
    puts("Usage: ./gmail-cli -a ACCOUNT -r RCPT1 RCP2 ... [OPTIONS]\n");
    puts("Options:");
    
    printf("    %-5s%-20s%s\n", "-f", "FILE1 FILE2 ...",    "File(s) to send.");
    printf("    %-5s%-20s%s\n", "-h", "",                   "Show this help message.");
    printf("    %-5s%-20s%s\n", "-m", "MESSAGE",            "The email's body data.");
    printf("    %-5s%-20s%s\n", "-s", "SUBJECT",            "The email's subject.");
    printf("    %-5s%-20s%s\n", "-v", "",                   "Display SMTP logs on stdout (verbose)");
}


void print_error(const char *msg) {
    fprintf(stderr, "[ERROR]: %s\n", msg);
}


int parse_args(int argc, char **argv, argv_t *args) {
    int opt, status_code = 0;

    while ((opt = getopt(argc, argv, "a:f:hm:r:s:v")) != -1) {
        switch (opt){
            case 'a':
                args->acc = optind - 1;
                break;

            case 'f':
                _get_args_list(argc, argv, &args->files);
                break;

            case 'h':
                args->flag_help = true;
                break;

            case 'm':
                args->msg = optind - 1;
                break;

            case 'r':
                _get_args_list(argc, argv, &args->rcpts);
                break;

            case 's':
                args->sbj = optind - 1;
                break;

            case 'v':
                args->flag_verb = true;
                break;

            default:
                status_code = -1;
                break;
        }
    }

    return status_code;
}


int get_keys(const char *account, char *email, size_t email_size, char *passwd, size_t passwd_size) {
    const char  *home           = NULL;
    char        *key_file       = NULL;
    char        *key_path       = NULL;
    char        command[512];
    FILE        *fp             = NULL;

    if (!strcmp(account, "personal")) key_file = ".keys/.gmail_personal.gpg";
    else if (!strcmp(account, "ucm")) key_file = ".keys/.gmail_ucm.gpg";
    else return 1;

    
    if ((home = getenv("HOME")) == NULL) return 2;

    key_path = malloc(strlen(home) + strlen(key_file) + 2);
    sprintf(key_path, "%s/%s", home, key_file);

    snprintf(command, sizeof(command), "gpg -d -q %s 2>/dev/null", key_path);
    if ((fp = popen(command, "r")) == NULL) {
        perror("popen failed");
        free(key_path);
        return 3;
    }

    if (fgets(email, email_size, fp)) email[strcspn(email, "\n")] = 0;
    if (fgets(passwd, passwd_size, fp)) passwd[strcspn(passwd, "\n")] = 0;

    int status = pclose(fp);
    free(key_path);

    if (status != 0) return 4;

    return 0;
}


int get_file_type(char *filename) {
    if (access(filename, F_OK) != 0) return -1;
    if (access(filename, R_OK) != 0) return -2;

    struct stat file_stats;
    stat(filename, &file_stats);

    return (file_stats.st_mode & S_IFMT);
}


char *get_mime_type(char *filename) {
    magic_t magic_cookie;
    const char *mime;
    char *result = NULL;

    if ((magic_cookie = magic_open(MAGIC_MIME_TYPE)) == NULL) return "application/octet-stream";

    if (magic_load(magic_cookie, NULL) != 0) {
        magic_close(magic_cookie);
        return "application/octet-stream";
    }

    if ((mime = magic_file(magic_cookie, filename)) != NULL) result = strdup(mime);
    else result = strdup("application/octet-stream");

    magic_close(magic_cookie);
    return result;
}
