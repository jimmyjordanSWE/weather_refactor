#ifndef CURL_WRAPPER
#define CURL_WRAPPER
#include "weather_app.h"

typedef struct wrap_curl wrap_curl;

int w_curl_init(wrap_curl **_w_curl);
int w_curl_global_cleanup();
int w_curl_handle_destroy(wrap_curl **_w_curl);

int w_curl_perform(weather_app *_app, wrap_curl *_w_curl);
int w_curl_set_url(wrap_curl *_w_curl, char *_url);

#endif /* CURL_WRAPPER */