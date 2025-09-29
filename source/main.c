#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "ui_cli.h"
#include "weather_app.h"
#include "wrap_curl.h"

/* todo temporary globals */
char* meteo_url = "https://api.open-meteo.com/v1/forecast?latitude=59.3293&longitude=18.0686&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,rain,showers,snowfall,weather_code,cloud_cover,pressure_msl,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m";

int main() {
    weather_app* app = NULL;
    if (app_init(&app) != 0) {
        return -1;
    }

    wrap_curl* w_curl = NULL;
    if (w_curl_init(&w_curl) != 0) {
        return -1;
    }
    w_curl_set_url(w_curl, meteo_url);

    ui_print_startup_message();

    do {
        ui_print_menu(app);

        if (ui_get_selection(app_get_nr_locations(app)) == 0) {
            app_send_message(app, APP_EXIT);
            break;
        }

        w_curl_perform(app, w_curl);
        printf("Temp: %.1f°C | Humidity: %d%% | Wind: %.1f km/h @ %d°\n", app_get_temp(app), app_get_humidity(app), app_get_wind_speed(app), app_get_wind_direction(app));
    } while (app_get_exit(app) == 0);

    w_curl_handle_destroy(&w_curl);
    w_curl_global_cleanup();
    app_destroy(&app);
    return 0;
}
