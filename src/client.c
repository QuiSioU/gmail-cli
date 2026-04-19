#define _POSIX_C_SOURCE 200809L

#include "client.h"
#include "utils.h"
#include <curl/curl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>


void _client_set_config(CURL *curl, config_t *config, struct curl_slist **recipients, curl_mime **mime) {
    curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
    curl_easy_setopt(curl, CURLOPT_USERNAME, config->email);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, config->passwd);

    char from_email[150];
    snprintf(from_email, sizeof(from_email), "<%s>", config->email);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_email);

    for (int i = 0; i < config->n_rcpts; i++)
        *recipients = curl_slist_append(*recipients, config->rcpts[i]);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, *recipients);

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, *mime);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
}


void _client_set_headers(CURL *curl, config_t *config, struct curl_slist **headers) {
    
    char                subj_hdr[256];
    char                from_hdr[256];
    size_t              total_len       = 5;
    char                *to_line        = NULL;

    snprintf(subj_hdr, sizeof(subj_hdr), "Subject: %s", config->sbj ? config->sbj : "parangaricutirimicuaro");
    *headers = curl_slist_append(*headers, subj_hdr);

    snprintf(from_hdr, sizeof(from_hdr), "From: <%s>", config->email);
    *headers = curl_slist_append(*headers, from_hdr);

    
    for (int i = 0; i < config->n_rcpts; i++)
        total_len += strlen(config->rcpts[i]) + 2;

    if ((to_line = malloc(total_len)) != NULL) {
        strcpy(to_line, "To: ");
        for (int i = 0; i < config->n_rcpts; i++) {
            strcat(to_line, config->rcpts[i]);
            if (i < config->n_rcpts - 1) strcat(to_line, ", ");
        }

        *headers = curl_slist_append(*headers, to_line);
        free(to_line); 
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *headers);
}


void _client_set_body(config_t *config, curl_mime **mime, curl_mimepart **part) {
    *part = curl_mime_addpart(*mime);
    curl_mime_data(
        *part,
        (config->msg && strlen(config->msg) > 0) ? config->msg : "esternocleidomastoideo.\r\n", CURL_ZERO_TERMINATED
    );
    curl_mime_type(*part, "text/plain");
}


int _client_add_files(config_t *config, curl_mime **mime, curl_mimepart **part) {
    for (int i = 0; i < config->n_files; i++) {
        switch (get_file_type(config->files[i])) {
            case -1:
                fprintf(stderr, "[ERROR]: File not found: %s\n", config->files[i]);
                return -1;

            case -2:
                fprintf(stderr, "[ERROR]: Cannot read file: %s.\n", config->files[i]);
                return -1;

            case S_IFREG:
                break;

            default:
                fprintf(stderr, "[ERROR]: Unsupported file type for '%s'\n", config->files[i]);
                return -1;
        }

        *part = curl_mime_addpart(*mime);
        
        if (curl_mime_filedata(*part, config->files[i]) != CURLE_OK) {
            fprintf(stderr, "[ERROR]: Failed to attach file %s.\n", config->files[i]);
            return -1;
        }

        char *display_name = strrchr(config->files[i], '/');
        curl_mime_filename(*part, display_name ? display_name + 1 : config->files[i]);

        char *mime_type = get_mime_type(config->files[i]);

        curl_mime_type(*part, mime_type);
        curl_mime_encoder(*part, "base64");

        free(mime_type);

        printf("File '%s' attached correcly.\n", config->files[i]);
    }

    return 0;
}


int send_email(config_t *config, bool verbose) {
    CURL                *curl       = NULL;
    CURLcode             res        = CURLE_OK;
    struct curl_slist   *recipients = NULL;
    struct curl_slist   *headers    = NULL;
    curl_mime           *mime       = NULL;
    curl_mimepart       *part       = NULL;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (!curl) return -1;

    curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose ? 1L : 0L);

    mime = curl_mime_init(curl);

    _client_set_headers(curl, config, &headers);
    _client_set_body(config, &mime, &part);
    if (_client_add_files(config, &mime, &part) != 0) return -1;
    _client_set_config(curl, config, &recipients, &mime);

    if ((res = curl_easy_perform(curl)) == CURLE_OK) puts("Mail sent correctly!");
    else printf("Error while trying to send mail! (error %d)\n", (int)res);

    curl_slist_free_all(headers);
    curl_slist_free_all(recipients);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return (int)res;
}
