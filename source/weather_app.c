#include "weather_app.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "jansson.h"
#include "linked_list.h"

/* ↓↓↓ INTERNAL ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ */
typedef struct {
    char time[32];
    int interval;
    double temperature_2m;
    int relative_humidity_2m;
    double apparent_temperature;
    int is_day;
    double precipitation;
    double rain;
    double showers;
    double snowfall;
    int weather_code;
    int cloud_cover;
    double pressure_msl;
    double surface_pressure;
    double wind_speed_10m;
    int wind_direction_10m;
    double wind_gusts_10m;
    int is_valid; /* 0 if json parsing had errors */
} weather_data;

typedef struct {
    char city_name[MAX_CITY_NAME];
    double latitude;
    double longitude;
    /* int elevation; */
    char timezone[64];
    weather_data current_weather;
} location;

struct weather_app {
    location current_location;
    LinkedList *locations;

    char *output_buffer;
    char *api_response;

    json_t *root;
    json_error_t error;

    char exit;
};

void app_reset_output_buffer(weather_app *_app) {
    memset(_app->output_buffer, 0, MAX_OUTPUT_BUFFER_LENGTH);
    return;
}
/* ↑↑↑ INTERNAL ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ */

/* ========================================
 * INITIALIZATION & CLEANUP
 * ======================================== */
int load_locations(weather_app **_app) {
    json_t *root;
    json_error_t error;

    root = json_load_file(LOCATIONS_FILE_PATH, 0, &error);

    if (!root) {
        fprintf(stderr,
                "JSON error in file %s:\n"
                "  line %d: %s\n",
                LOCATIONS_FILE_PATH, error.line, error.text);
        return -1;
    }

    json_t *val;
    val = json_object_get(root, "locations");

    size_t index;
    json_t *loc;

    json_array_foreach(val, index, loc) {
        json_t *name = json_object_get(loc, "name");
        json_t *latitude = json_object_get(loc, "latitude");
        json_t *longitude = json_object_get(loc, "longitude");

        if (json_is_string(name) && json_is_number(latitude) && json_is_number(longitude)) {
            location *tmp = (location *)calloc(1, sizeof(location));

            strncpy(tmp->city_name, json_string_value(name), MAX_CITY_NAME);
            tmp->latitude = json_number_value(latitude);
            tmp->longitude = json_number_value(longitude);
            json_number_value(latitude);
            /*  todo continue here */
            LinkedList_append((*_app)->locations, (void *)tmp);

        } else {
            fprintf(stderr, "Invalid data at index %zu\n", index);
        }
        /* todo finsih liked list so we can fil lthis data  */
    }

    return 0;
}

int app_init(weather_app **_app) {
    *_app = malloc(sizeof(weather_app));
    if (*_app == NULL) {
        fprintf(stderr, "Error: app_init. _app malloc failed.\n");
        return -1;
    }
    /*  Setting to NULL is not just a precaution, we must do it
    so that cleanup wont try to free garbage in case any malloc here fails*/
    (*_app)->output_buffer = NULL;
    (*_app)->api_response = NULL;
    (*_app)->exit = FALSE;

    (*_app)->output_buffer = malloc(MAX_RESPONSE_BUFFER_LENGTH);
    (*_app)->api_response = malloc(MAX_RESPONSE_BUFFER_LENGTH);

    if ((*_app)->api_response == NULL || (*_app)->output_buffer == NULL) {
        fprintf(stderr, "Error: app_init. _app members malloc failed.\n");
        return -1;
    }

    memset((*_app)->output_buffer, 0, MAX_RESPONSE_BUFFER_LENGTH);
    memset((*_app)->api_response, 0, MAX_RESPONSE_BUFFER_LENGTH);

    (*_app)->root = NULL;

    (*_app)->locations = LinkedList_create();

    load_locations(_app);

    return 0;
}

int app_destroy(weather_app **_app) {
    if (*_app == NULL) {
        fprintf(stderr, "Error: weather_app is NULL\n");
        return -1;
    }

    /* todo decref all jansson here */
    free((*_app)->output_buffer);
    free((*_app)->api_response);
    free(*_app);
    *_app = NULL;
    return 0;
}

/* ========================================
 * GETTERS
 * ======================================== */

int app_get_exit(weather_app *_app) {
    if (_app == NULL) {
        return -1;
    }
    return _app->exit;
}

LinkedList *app_get_locations(weather_app *_app) { return _app->locations; };

/* returns a copy placed in output_buffer  */
char *app_get_prev_api_response_raw(weather_app *_app) {
    app_reset_output_buffer(_app);
    strcpy(_app->output_buffer, _app->api_response);
    return _app->output_buffer;
}

double app_get_temp(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location.current_weather.temperature_2m;
}
int app_get_humidity(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location.current_weather.relative_humidity_2m;
}
double app_get_apparent_temp(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location.current_weather.apparent_temperature;
}
double app_get_wind_speed(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location.current_weather.wind_speed_10m;
}
int app_get_wind_direction(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location.current_weather.wind_direction_10m;
}
double app_get_pressure(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location.current_weather.pressure_msl;
}
int app_get_cloud_cover(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location.current_weather.cloud_cover;
}
int app_get_weather_code(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location.current_weather.weather_code;
}
const char *app_get_time(weather_app *_app) {
    if (_app == NULL) return "";
    return _app->current_location.current_weather.time;
}
const char *app_get_city_name(weather_app *_app) {
    if (_app == NULL) return "";
    return _app->current_location.city_name;
}

/* ========================================
 * SETTERS
 * ======================================== */
int app_set_prev_api_response(weather_app *_app, char *_response) {
    if (_app->api_response == NULL || _response == NULL) {
        return -1;
    }
    /* todo should we check if stncopy worked? */
    strncpy(_app->api_response, _response, MAX_RESPONSE_BUFFER_LENGTH);

    _app->api_response[MAX_RESPONSE_BUFFER_LENGTH - 1] = '\0';

    /* parse response with jansson and fill location and weather_data */
    _app->root = json_loads(_response, 0, &_app->error);
    if (!_app->root) {
        return -1;
    }

    json_t *current = json_object_get(_app->root, "current");
    if (!current) {
        _app->current_location.current_weather.is_valid = 0;
        return -1;
    }

    json_t *val;
    val = json_object_get(current, "time");
    if (json_is_string(val)) {
        strncpy(_app->current_location.current_weather.time, json_string_value(val), 31);
        _app->current_location.current_weather.time[31] = '\0';
    }

    val = json_object_get(current, "interval");
    _app->current_location.current_weather.interval = json_is_integer(val) ? json_integer_value(val) : 0;
    val = json_object_get(current, "temperature_2m");
    _app->current_location.current_weather.temperature_2m = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "relative_humidity_2m");
    _app->current_location.current_weather.relative_humidity_2m = json_is_integer(val) ? json_integer_value(val) : 0;
    val = json_object_get(current, "apparent_temperature");
    _app->current_location.current_weather.apparent_temperature = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "is_day");
    _app->current_location.current_weather.is_day = json_is_integer(val) ? json_integer_value(val) : 0;
    val = json_object_get(current, "precipitation");
    _app->current_location.current_weather.precipitation = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "rain");
    _app->current_location.current_weather.rain = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "showers");
    _app->current_location.current_weather.showers = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "snowfall");
    _app->current_location.current_weather.snowfall = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "weather_code");
    _app->current_location.current_weather.weather_code = json_is_integer(val) ? json_integer_value(val) : 0;
    val = json_object_get(current, "cloud_cover");
    _app->current_location.current_weather.cloud_cover = json_is_integer(val) ? json_integer_value(val) : 0;
    val = json_object_get(current, "pressure_msl");
    _app->current_location.current_weather.pressure_msl = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "surface_pressure");
    _app->current_location.current_weather.surface_pressure = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "wind_speed_10m");
    _app->current_location.current_weather.wind_speed_10m = json_is_number(val) ? json_number_value(val) : 0.0;
    val = json_object_get(current, "wind_direction_10m");
    _app->current_location.current_weather.wind_direction_10m = json_is_integer(val) ? json_integer_value(val) : 0;
    val = json_object_get(current, "wind_gusts_10m");
    _app->current_location.current_weather.wind_gusts_10m = json_is_number(val) ? json_number_value(val) : 0.0;

    _app->current_location.current_weather.is_valid = 1;

    json_decref(_app->root);
    _app->root = NULL;

    return 0;
}

int app_send_message(weather_app *_app, int _message_code) {
    switch (_message_code) {
        case APP_EXIT:
            _app->exit = TRUE;
            break;

        default:
    }

    return 0;
}
