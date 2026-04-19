#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdbool.h>


typedef struct {
    int     first;
    int     second;
} pair_t;

typedef struct {
    pair_t  files;
    pair_t  rcpts;
    bool    flag_help;
    bool    flag_verb;
    int     sbj;
    int     msg;
    int     acc;
} argv_t;

void    print_usage();
void    print_error(const char *msg);
int     parse_args(int argc, char **argv, argv_t *args);
int     get_keys(const char *account, char *email, size_t email_size, char *passwd, size_t passwd_size);
int     get_file_type(char *filename);
char *  get_mime_type(char *filename);

#endif
