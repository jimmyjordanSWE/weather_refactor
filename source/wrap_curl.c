#include "wrap_curl.h"
#include <curl/curl.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "defines.h"
#include "weather_app.h"

/* ↓↓↓ INTERNAL ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ */
static int curl_global_init_has_been_run = 0;

struct wrap_curl {
    CURL *handle;
    CURLcode error;
    time_t call_cooldown;
    char *response_storage;
    char *url;

    /* todo list of prev URLs */
};
/* reset all for handle reuse */
int w_curl_reset_handle(wrap_curl *_w_curl) {
    _w_curl->call_cooldown = 0;
    _w_curl->error = CURLE_OK;
    memset(_w_curl->response_storage, 0, MAX_RESPONSE_BUFFER_LENGTH);
    /* todo check if this segfaults at second run (first is during init)*/
    memset(_w_curl->url, 0, MAX_URL_LENGTH);
    return 0;
}

/* reset error code and response_storage */
int w_curl_reload_handle(wrap_curl *_w_curl) {
    _w_curl->error = CURLE_OK;
    memset(_w_curl->response_storage, 0, MAX_RESPONSE_BUFFER_LENGTH);
    /* segfault second time */
    return 0;
}

size_t write_callback(void *response_data, size_t size, size_t nmemb, void *response_storage) {
    size_t total_size = size * nmemb;

    /* todo check that we dont overfill buffer */
    strncat((char *)response_storage, (char *)response_data, total_size);

    return total_size;
}
/* ↑↑↑ INTERNAL ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ */

int w_curl_perform(weather_app *_app, wrap_curl *_w_curl) {
    if (_w_curl == NULL) {
        fprintf(stderr, "Error: curl_perform(). _w_curl = NULL");
        return 1;
    }

    w_curl_reload_handle(_w_curl);

    if (time(NULL) - _w_curl->call_cooldown < SECONDS_BETWEEN_CALLS) {
        printf("Cooldown active. Waiting for %d seconds...\n", SECONDS_BETWEEN_CALLS);
        sleep(time(NULL) - _w_curl->call_cooldown + 1);
    }

    _w_curl->error = curl_easy_perform(_w_curl->handle);
    _w_curl->call_cooldown = time(NULL);

    if (_w_curl->error != CURLE_OK) {
        fprintf(stderr, "Error: curl_perform(). CURLcode: %s\n", curl_easy_strerror(_w_curl->error));
        return 1;
    }

    if (app_set_prev_api_response(_app, _w_curl->response_storage) != 0) {
        fprintf(stderr, "Error: curl_perform(). could not write response to _app->prev-api_response");
        return 1;
    }

    /* write response to file */

    return 0;
}

int w_curl_init(wrap_curl **_w_curl) {
    if (*_w_curl != NULL) {
        fprintf(stderr, "Error: cant init null pointer");
        return 1;
    }
    wrap_curl *tmp = malloc(sizeof(wrap_curl));
    if (tmp == NULL) {
        fprintf(stderr, "Error: malloc failed in w_curl_init");
        return -1;
    }
    *_w_curl = malloc(sizeof(wrap_curl));

    if (curl_global_init_has_been_run == 0) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            fprintf(stderr, "Error: curl_global_init()");
            return 1;
        }
        curl_global_init_has_been_run = 1;
    }

    /* todo check for malloc error */
    (*_w_curl)->response_storage = malloc(MAX_RESPONSE_BUFFER_LENGTH);

    /* todo check for malloc error */
    (*_w_curl)->url = malloc(MAX_URL_LENGTH);

    (*_w_curl)->handle = curl_easy_init();
    /* check for error return */
    w_curl_reset_handle(*_w_curl);

    curl_easy_setopt((*_w_curl)->handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt((*_w_curl)->handle, CURLOPT_WRITEDATA, (*_w_curl)->response_storage);

    return 0;
}

int w_curl_set_url(wrap_curl *_w_curl, char *_url) {
    _w_curl->url = _url;
    _w_curl->error = curl_easy_setopt(_w_curl->handle, CURLOPT_URL, _url);
    if (_w_curl->error == CURLE_OK) {
        return 0;
    }
    fprintf(stderr, "Error: w_curl_set_url(). CURLcode: %s\n", curl_easy_strerror(_w_curl->error));
    return 1;
}

int w_curl_handle_destroy(wrap_curl **_w_curl) {
    if (*_w_curl == NULL) {
        fprintf(stderr, "Error: *_w_curl is NULL. Could not destroy.\n");
        return 1;
    }

    curl_easy_cleanup(*_w_curl);
    free(*_w_curl);
    *_w_curl = NULL;

    free((*_w_curl));
    return 0;
}

int w_curl_global_cleanup() {
    curl_global_cleanup();
    return 0;
}