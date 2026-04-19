#include "utils.h"
#include "client.h"
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv) {
    int status_code = 0;
    argv_t args = {0};

    if (argc < 2) {
        print_usage();
        return 1;
    }

    if ((status_code = parse_args(argc, argv, &args)) != 0) {
        print_usage();
        return status_code;
    }

    if (args.flag_help) {
        print_usage();
        return status_code;
    }

    if (args.acc == 0) {
        print_error("You must specify an account with '-a'");
        print_usage();
        return 2;
    }

    if (args.rcpts.second == 0) {
        print_error("You must specify a recipient with '-r'");
        print_usage();
        return 3;
    }

    config_t client_config = {0};

    if (get_keys(argv[args.acc], client_config.email, sizeof(client_config.email),
                client_config.passwd, sizeof(client_config.passwd)) != 0)
    {
        print_error("Invalid Gmail account");
        return 4;
    }

    client_config.rcpts = malloc(args.rcpts.second * sizeof(char *));
    if (!client_config.rcpts) return 5;
    for (int i = 0; i < args.rcpts.second; i++) client_config.rcpts[i] = argv[args.rcpts.first + i];
    client_config.n_rcpts = args.rcpts.second;

    if (args.files.second) {
        client_config.files = malloc(args.files.second * sizeof(char *));
        if (!client_config.files) return 6;
        for (int i = 0; i < args.files.second; i++) client_config.files[i] = argv[args.files.first + i];
        client_config.n_files = args.files.second;
    }

    if (args.sbj) client_config.sbj = argv[args.sbj];
    if (args.msg) client_config.msg = argv[args.msg];

    status_code = send_email(&client_config, args.flag_verb);

    memset(client_config.email, 0, sizeof(client_config.email));
    memset(client_config.passwd, 0, sizeof(client_config.passwd));

    free(client_config.files);
    free(client_config.rcpts);

    return status_code;
}