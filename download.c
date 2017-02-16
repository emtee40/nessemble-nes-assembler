#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "nessemble.h"
#include "download.h"

#define CONTENT_TYPE "Content-Type: "

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream) {
    struct write_result *result = (struct write_result *)stream;

    if (result->pos + size * nmemb >= BUFFER_SIZE - 1) {
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

unsigned int get_request(char **request, size_t *request_length, char *url, char *mime_type) {
    unsigned int rc = RETURN_OK;
    long code = 0;
    size_t mime_type_length = 0;
    char *data = NULL, *content_type = NULL;
    struct curl_slist *headers = NULL;
    CURL *curl = NULL;
    CURLcode status;

    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    curl = curl_easy_init();

    if (!curl) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    data = (char *)malloc(sizeof(char) * BUFFER_SIZE);

    if (!data) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);

    mime_type_length = strlen(mime_type);
    content_type = (char *)malloc(sizeof(char) * (mime_type_length + 15));

    if (!content_type) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    sprintf(content_type, CONTENT_TYPE "%s", mime_type);

    headers = curl_slist_append(headers, content_type);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    status = curl_easy_perform(curl);

    if (status != 0) {
        fprintf(stderr, "Could not reach the registry\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

    if (code != 200) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    data[write_result.pos] = '\0';

    *request = data;
    *request_length = write_result.pos;

cleanup:
    if (content_type) {
        free(content_type);
    }

    return rc;
}