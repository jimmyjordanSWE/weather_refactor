#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "weather_app.h"
#include "wrap_curl.h"

/* TODO UNTIL FINISHED:
- Write API responses to file
- check cache before calling API
- add smart cool down to API calls, data refreshed
    every 00,15,30,45 minutes. Check how far away we are.

- bonus: perhaps integrate w_curl into weather_app?
*/

/* todo temporary globals. Create some kind of URL handler code */
char* meteo_url = "https://api.open-meteo.com/v1/forecast?latitude=59.3293&longitude=18.0686&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,rain,showers,snowfall,weather_code,cloud_cover,pressure_msl,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m";

int main() {
    /* inits and setup */
    weather_app* app = NULL;
    if (app_init(&app) != 0) {
        return -1;
    }

    wrap_curl* w_curl = NULL;
    if (w_curl_init(&w_curl) != 0) {
        return -1;
    }
    w_curl_set_url(w_curl, meteo_url);

    app_print_startup_message();
    /* Main program loop */
    do {
        app_print_menu(app);

        if (app_get_selection(app_get_nr_locations(app)) == 0) {
            app_set_exit(app);
            break;
        }

        w_curl_perform(app, w_curl);

        printf("Temp: %.1f°C | Humidity: %d%% | Wind: %.1f km/h @ %d°\n", app_get_temp(app), app_get_humidity(app), app_get_wind_speed(app), app_get_wind_direction(app));

    } while (app_get_exit(app) == 0);

    /* cleanup and exit */
    w_curl_handle_destroy(&w_curl);
    w_curl_global_cleanup();
    app_destroy(&app);
    return 0;
}
