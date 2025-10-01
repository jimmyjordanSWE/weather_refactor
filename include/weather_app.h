#ifndef WEATHER_APP_H
#define WEATHER_APP_H
#include "stddef.h"

typedef struct location location;
typedef struct weather_app weather_app;

void app_print_menu(weather_app *_app);

/* init / destroy */
int app_init(weather_app **_app);
int app_destroy(weather_app **_app);

/* getters */

/* returns seconds left until next interval */
size_t app_is_cache_stale(weather_app *_app);

char *app_get_location_name(weather_app *_app, int index);
double app_get_current_location_latitude(weather_app *_app);
double app_get_current_location_longitude(weather_app *_app);

size_t app_get_nr_locations(weather_app *_app);

int app_get_exit(weather_app *_app);
char *app_get_prev_api_response_raw(weather_app *_app);

int app_get_cloud_cover(weather_app *_app);
int app_get_weather_code(weather_app *_app);
int app_get_humidity(weather_app *_app);
int app_get_wind_direction(weather_app *_app);
double app_get_temp(weather_app *_app);
double app_get_apparent_temp(weather_app *_app);
double app_get_wind_speed(weather_app *_app);
double app_get_pressure(weather_app *_app);
const char *app_get_time(weather_app *_app);
const char *app_get_name(weather_app *_app);

/* setters */
void app_set_current_location_weather(weather_app *_app, char *api_response);
int app_set_exit(weather_app *_app);
int app_set_prev_api_response(weather_app *_app, char *_response);
int app_set_current_location(weather_app **_app, location *_new_location);
void app_set_current_location_index(weather_app *_app, size_t _selection);

/* UI */
void app_clear_screen();
void app_print_startup_message();
void app_print_menu(weather_app *_app);
void app_print_current_location_weather_all(weather_app *_app);

int app_get_selection(int max);

/* Files */
int app_write_locations_to_file(weather_app *_app);

#endif /* WEATHER_APP_H */
