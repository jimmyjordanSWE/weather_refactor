#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include "weather_app.h"
#include "wrap_curl.h"

int main() {
    app_clear_screen();
    app_print_startup_message();

    /* inits and setup */
    weather_app* app = NULL;
    if (app_init(&app) != 0) {
        return -1;
    }

    wrap_curl* w_curl = NULL;
    if (w_curl_init(&w_curl) != 0) {
        return -1;
    }

    size_t selection = 0; /* todo integrate this variable somewhere */
    app_print_menu(app);

    /* Main program loop */
    do {
        selection = app_get_selection(app_get_nr_locations(app));
        if (selection == 0) {
            app_set_exit(app);
            break;
        }

        app_set_current_location_index(app, selection);
        w_curl_perform(app, w_curl);

        app_print_current_location_weather_all(app);

    } while (app_get_exit(app) == 0);

    /* cleanup and exit */
    w_curl_handle_destroy(&w_curl);
    w_curl_global_cleanup();
    app_destroy(&app);
    return 0;
}