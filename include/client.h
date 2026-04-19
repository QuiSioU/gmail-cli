#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>


typedef struct {
    char **rcpts;
    char **files;
    char *msg;
    char *sbj;
    int n_rcpts;
    int n_files;
    char email[128];
    char passwd[128];
} config_t;

int send_email(config_t *config, bool verbose);

#endif
