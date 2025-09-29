#ifndef WEATHER_APP_H
#define WEATHER_APP_H
#include "stddef.h"

typedef struct weather_app weather_app;

/* init / destroy */
int app_init(weather_app **_app);
int app_destroy(weather_app **_app);

/* messages */
int app_send_message(weather_app *_app, int _message);

/* getters */
char *app_get_location_name(weather_app *_app, int index);
size_t app_get_nr_locations(weather_app *_app);
int app_get_exit(weather_app *_app);
char *app_get_prev_api_response_raw(weather_app *_app);

double app_get_temp(weather_app *_app);
int app_get_humidity(weather_app *_app);
double app_get_apparent_temp(weather_app *_app);
double app_get_wind_speed(weather_app *_app);
int app_get_wind_direction(weather_app *_app);
double app_get_pressure(weather_app *_app);
int app_get_cloud_cover(weather_app *_app);
int app_get_weather_code(weather_app *_app);
const char *app_get_time(weather_app *_app);
const char *app_get_city_name(weather_app *_app);

/* setters */
int app_set_prev_api_response(weather_app *_app, char *_response);

#endif /* WEATHER_APP_H */
