#include "weather_app.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "defines.h"
#include "jansson.h"
#include "linked_list.h"
#include "stddef.h"
#include "time.h"
#include "weather_app.h"

/* ↓↓↓ INTERNAL ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ */
typedef struct {
    char timezone[16];
    char time[32];
    int utc_offset_seconds;
    int interval;
    int weather_code;
    double elevation;
    double temperature_2m;
    double relative_humidity_2m;
    double cloud_cover;
    double wind_direction_10m;
    double apparent_temperature;
    double precipitation;
    double rain;
    double showers;
    double snowfall;
    double pressure_msl;
    double surface_pressure;
    double wind_speed_10m;
    double wind_gusts_10m;
} weather_data;

typedef struct location {
    char name[MAX_CITY_NAME];
    double latitude;
    double longitude;
    time_t timestamp_last_api_call;
    weather_data current_weather;
} location;

struct weather_app {
    location *current_location; /* points to any location in the locations list  */
    LinkedList *locations;
    char *api_response;
    json_t *root;
    json_error_t error;

    char exit;
};

/* ↑↑↑ INTERNAL ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ */

/* ========================================
 * INITIALIZATION & CLEANUP
 * ======================================== */

int app_set_current_location(weather_app **_app, location *_new_location) {
    (*_app)->current_location = _new_location;
    return 0;
}

void app_set_current_location_index(weather_app *_app, size_t _selection) {
    _selection--; /* index starts at 0, user menu at 1 */
    _app->current_location = LinkedList_get_index(_app->locations, _selection)->item;
    return;
}

int app_load_locations(weather_app **_app) {
    size_t index;
    json_t *val = NULL;
    json_t *loc = NULL;
    json_t *name = NULL;
    json_t *latitude = NULL;
    json_t *longitude = NULL;
    json_t *root = NULL;
    json_error_t error;
    int error_parsing_file_data = 0;

    root = json_load_file(LOCATIONS_FILE_PATH, 0, &error);

    /* File found and read. Try to load from file */
    if (root != NULL) {
        val = json_object_get(root, "locations");

        if (val != NULL && json_is_array(val)) {
            json_array_foreach(val, index, loc) {
                name = json_object_get(loc, "name");
                latitude = json_object_get(loc, "latitude");
                longitude = json_object_get(loc, "longitude");

                if (json_is_string(name) && json_is_number(latitude) && json_is_number(longitude)) {
                    location *tmp = (location *)calloc(1, sizeof(location));

                    strlcpy(tmp->name, json_string_value(name), MAX_CITY_NAME);
                    tmp->latitude = json_number_value(latitude);
                    tmp->longitude = json_number_value(longitude);

                    LinkedList_append((*_app)->locations, (void *)tmp);

                } else {
                    fprintf(stderr, "Invalid data at index %zu. Loading hard coded locations instead.\n", index);
                    error_parsing_file_data = 1;
                    /* clean up and stop parsing on any error */
                    json_decref(root);
                    root = NULL;
                    LinkedList_free_nodes_and_payloads((*_app)->locations);
                    break;
                }
            }

            fprintf(stderr, "%zu locations loaded from \"%s\"\n\n", (*_app)->locations->size, LOCATIONS_FILE_PATH);
        }
    }

    /* file not found or there where errors reading file, load from hard coded locations */
    if (root == NULL || error_parsing_file_data != 0) {
        /* todo hard coded path, make defines */
        mkdir(LOCATIONS_FILE_FOLDER, 0777);
        root = json_loads(HARD_CODED_LOCATIONS, 0, &error);
        val = json_object_get(root, "locations");
        if (val != NULL && json_is_array(val)) {
            json_array_foreach(val, index, loc) {
                /* same code as above but we load from a string on the stack */
                name = json_object_get(loc, "name");
                latitude = json_object_get(loc, "latitude");
                longitude = json_object_get(loc, "longitude");

                location *tmp = (location *)calloc(1, sizeof(location));

                strlcpy(tmp->name, json_string_value(name), MAX_CITY_NAME);
                tmp->latitude = json_number_value(latitude);
                tmp->longitude = json_number_value(longitude);

                LinkedList_append((*_app)->locations, (void *)tmp);
            }
            fprintf(stderr, "\n\"%s\" not found.\n%zu hard coded locations loaded.\n", LOCATIONS_FILE_PATH, (*_app)->locations->size);
            fprintf(stderr, "\"%s\" was created.\n\n", LOCATIONS_FILE_PATH);
            app_write_locations_to_file(*_app);
        }
    }
    json_decref(root);
    root = NULL;

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
    (*_app)->api_response = NULL;
    (*_app)->exit = FALSE;

    (*_app)->api_response = malloc(MAX_RESPONSE_BUFFER_LENGTH);

    if ((*_app)->api_response == NULL) {
        fprintf(stderr, "Error: app_init. _app members malloc failed.\n");
        return -1;
    }

    memset((*_app)->api_response, 0, MAX_RESPONSE_BUFFER_LENGTH);

    (*_app)->root = NULL;

    (*_app)->locations = LinkedList_create();

    app_load_locations(_app);
    app_set_current_location(_app, (location *)(*_app)->locations->head->item);

    return 0;
}

int app_destroy(weather_app **_app) {
    if (*_app == NULL) {
        fprintf(stderr, "Error: weather_app is NULL\n");
        return -1;
    }

    /* todo decref all jansson here? */
    free((*_app)->api_response);
    free(*_app);
    *_app = NULL;
    return 0;
}

/* ========================================
 * GETTERS
 * ======================================== */

size_t app_is_cache_stale(weather_app *_app) {
    time_t now = time(NULL);
    size_t prev_call = (size_t)_app->current_location->timestamp_last_api_call;
    size_t interval = (size_t)_app->current_location->current_weather.interval;

    if ((size_t)(now - prev_call) >= interval) return 0;

    return interval - (size_t)(now - prev_call);
}

double app_get_current_location_latitude(weather_app *_app) { return _app->current_location->latitude; }
double app_get_current_location_longitude(weather_app *_app) { return _app->current_location->longitude; }

size_t app_get_nr_locations(weather_app *_app) {
    if (_app == NULL) {
        return -1;
    }
    return _app->locations->size;
}
int app_get_exit(weather_app *_app) {
    if (_app == NULL) {
        return -1;
    }
    return _app->exit;
}

LinkedList *app_get_locations(weather_app *_app) { return _app->locations; };

double app_get_temp(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location->current_weather.temperature_2m;
}
int app_get_humidity(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location->current_weather.relative_humidity_2m;
}
double app_get_apparent_temp(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location->current_weather.apparent_temperature;
}
double app_get_wind_speed(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location->current_weather.wind_speed_10m;
}
int app_get_wind_direction(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location->current_weather.wind_direction_10m;
}
double app_get_pressure(weather_app *_app) {
    if (_app == NULL) return 0.0;
    return _app->current_location->current_weather.pressure_msl;
}
int app_get_cloud_cover(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location->current_weather.cloud_cover;
}
int app_get_weather_code(weather_app *_app) {
    if (_app == NULL) return 0;
    return _app->current_location->current_weather.weather_code;
}
const char *app_get_time(weather_app *_app) {
    if (_app == NULL) return "";
    return _app->current_location->current_weather.time;
}
const char *app_get_name(weather_app *_app) {
    if (_app == NULL) return "";
    return _app->current_location->name;
}

/* ========================================
 * SETTERS
 * ======================================== */
void app_set_current_location_weather(weather_app *_app, char *_api_response) {
    _app->current_location->timestamp_last_api_call = time(NULL);
    /* check if long and lat line up so its the right location */

    json_error_t error;
    json_t *root = json_loads(_api_response, 0, &error);

    if (root != NULL) {
        json_t *latitude = json_object_get(root, "latitude");
        json_t *longitude = json_object_get(root, "longitude");
        json_t *utc_offset_seconds = json_object_get(root, "utc_offset_seconds");
        json_t *timezone = json_object_get(root, "timezone");
        json_t *elevation = json_object_get(root, "elevation");

        json_t *current = json_object_get(root, "current");

        json_t *time = json_object_get(current, "time");
        json_t *interval = json_object_get(current, "interval");
        json_t *temperature_2m = json_object_get(current, "temperature_2m");
        json_t *relative_humidity_2m = json_object_get(current, "relative_humidity_2m");
        json_t *apparent_temperature = json_object_get(current, "apparent_temperature");
        json_t *precipitation = json_object_get(current, "precipitation");
        json_t *rain = json_object_get(current, "rain");
        json_t *showers = json_object_get(current, "showers");
        json_t *snowfall = json_object_get(current, "snowfall");
        json_t *weather_code = json_object_get(current, "weather_code");
        json_t *cloud_cover = json_object_get(current, "cloud_cover");
        json_t *pressure_msl = json_object_get(current, "pressure_msl");
        json_t *surface_pressure = json_object_get(current, "surface_pressure");
        json_t *wind_speed_10m = json_object_get(current, "wind_speed_10m");
        json_t *wind_direction_10m = json_object_get(current, "wind_direction_10m");
        json_t *wind_gusts_10m = json_object_get(current, "wind_gusts_10m");

        if (latitude != NULL && json_is_number(latitude)) _app->current_location->latitude = json_number_value(latitude);
        if (longitude != NULL && json_is_number(longitude)) _app->current_location->longitude = json_number_value(longitude);
        if (utc_offset_seconds != NULL && json_is_integer(utc_offset_seconds)) _app->current_location->current_weather.utc_offset_seconds = (int)json_integer_value(utc_offset_seconds);
        if (timezone != NULL && json_is_string(timezone)) strlcpy(_app->current_location->current_weather.timezone, json_string_value(timezone), sizeof(_app->current_location->current_weather.timezone));
        if (elevation != NULL && json_is_number(elevation)) _app->current_location->current_weather.elevation = json_number_value(elevation);
        if (time != NULL && json_is_string(time)) strlcpy(_app->current_location->current_weather.time, json_string_value(time), sizeof(_app->current_location->current_weather.time));
        if (interval != NULL && json_is_integer(interval)) _app->current_location->current_weather.interval = (int)json_integer_value(interval);
        if (temperature_2m != NULL && json_is_number(temperature_2m)) _app->current_location->current_weather.temperature_2m = json_number_value(temperature_2m);
        if (relative_humidity_2m != NULL && json_is_number(relative_humidity_2m)) _app->current_location->current_weather.relative_humidity_2m = json_number_value(relative_humidity_2m);
        if (apparent_temperature != NULL && json_is_number(apparent_temperature)) _app->current_location->current_weather.apparent_temperature = json_number_value(apparent_temperature);
        if (precipitation != NULL && json_is_number(precipitation)) _app->current_location->current_weather.precipitation = json_number_value(precipitation);
        if (rain != NULL && json_is_number(rain)) _app->current_location->current_weather.rain = json_number_value(rain);
        if (showers != NULL && json_is_number(showers)) _app->current_location->current_weather.showers = json_number_value(showers);
        if (snowfall != NULL && json_is_number(snowfall)) _app->current_location->current_weather.snowfall = json_number_value(snowfall);
        if (weather_code != NULL && json_is_integer(weather_code)) _app->current_location->current_weather.weather_code = (int)json_integer_value(weather_code);
        if (cloud_cover != NULL && json_is_number(cloud_cover)) _app->current_location->current_weather.cloud_cover = json_number_value(cloud_cover);
        if (pressure_msl != NULL && json_is_number(pressure_msl)) _app->current_location->current_weather.pressure_msl = json_number_value(pressure_msl);
        if (surface_pressure != NULL && json_is_number(surface_pressure)) _app->current_location->current_weather.surface_pressure = json_number_value(surface_pressure);
        if (wind_speed_10m != NULL && json_is_number(wind_speed_10m)) _app->current_location->current_weather.wind_speed_10m = json_number_value(wind_speed_10m);
        if (wind_direction_10m != NULL && json_is_number(wind_direction_10m)) _app->current_location->current_weather.wind_direction_10m = json_number_value(wind_direction_10m);
        if (wind_gusts_10m != NULL && json_is_number(wind_gusts_10m)) _app->current_location->current_weather.wind_gusts_10m = json_number_value(wind_gusts_10m);
    }

    return;
}
int app_set_exit(weather_app *_app) {
    if (_app == NULL) {
        return -1;
    }
    _app->exit = 1;
    return 0;
}

/* UI */

void app_clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void app_print_startup_message() {
    printf("\n");
    printf("    ╔═════════════════════════╗\n");
    printf("    ║       CIRRUS  CLI       ║\n");
    printf("    ╚═════════════════════════╝\n");
    printf("\n");
}
void app_print_menu(weather_app *_app) {
    for (size_t i = 0; i < _app->locations->size; i++) {
        printf("%3zu: %s\n", i + 1, ((location *)LinkedList_get_index(_app->locations, i)->item)->name);
    }
    printf("  0: EXIT\n");
    return;
}

void app_print_current_location_weather_all(weather_app *_app) {
    location *current_location = _app->current_location;
    weather_data w = current_location->current_weather;

    printf("\n---------------------------------------------\n");
    printf("📍 Location: %s\n", current_location->name);
    printf("   Latitude: %.4f   Longitude: %.4f\n", current_location->latitude, current_location->longitude);
    printf("   Time: %s (UTC offset %d sec)\n", w.time, w.utc_offset_seconds);
    printf("   Elevation: %.0f m   update interval: %d minutes\n", (double)w.elevation, w.interval / 60);

    printf("   Weather Code: %d\n", w.weather_code);
    printf("   Temperature: %.1f °C (feels like %.1f °C)\n", w.temperature_2m, w.apparent_temperature);
    printf("   Humidity: %.0f %%   Cloud Cover: %.0f %%\n", w.relative_humidity_2m, w.cloud_cover);
    printf("   Wind: %.1f m/s from %.0f° (gusts up to %.1f m/s)\n", w.wind_speed_10m, w.wind_direction_10m, w.wind_gusts_10m);

    printf("   Precipitation: %.1f mm (Rain: %.1f, Showers: %.1f, Snowfall: %.1f)\n", w.precipitation, w.rain, w.showers, w.snowfall);

    printf("   Pressure: %.1f hPa (MSL: %.1f)\n", w.surface_pressure, w.pressure_msl);

    printf("---------------------------------------------\n\n");
}

int app_get_selection(int max) {
    int min = 0;
    size_t length = 0;
    long read_value = 0.0f;
    char *line = NULL;
    char *end_pointer = NULL;

    while (1) {
        printf("Select an option (%d-%d): ", min + 1, max);

        if ((ssize_t)getline(&line, &length, stdin) == -1) {
            free(line);
            return 0; /* todo only works because 0 is quit in main() */
        }

        line[strcspn(line, "\n")] = '\0';
        read_value = strtol(line, &end_pointer, 10);

        if (end_pointer == line || read_value < min || read_value > max) {
            continue;
        }

        free(line); /* getline() mallocs */
        return (int)read_value;
    }
}

/* Files */
int app_write_locations_to_file(weather_app *_app) {
    /* todo needs error handling */
    LinkedList *list = _app->locations;
    location *current_location = NULL;
    weather_data current_weather;

    FILE *file = fopen("./data/locations.json", "w");

    json_t *root = json_object();
    json_t *locations_obj = json_array();

    for (size_t i = 0; i < _app->locations->size; i++) {
        current_location = (location *)LinkedList_get_index(list, i)->item;
        current_weather = current_location->current_weather;

        /* write weather_data object */
        json_t *current_weather_obj = json_object();
        json_object_set_new(current_weather_obj, "utc_offset_seconds", json_integer(current_weather.utc_offset_seconds));
        json_object_set_new(current_weather_obj, "timezone", json_string(current_weather.timezone));
        json_object_set_new(current_weather_obj, "elevation", json_integer(current_weather.elevation));
        json_object_set_new(current_weather_obj, "time", json_string(current_weather.time));
        json_object_set_new(current_weather_obj, "interval", json_integer(current_weather.interval));
        json_object_set_new(current_weather_obj, "weather_code", json_integer(current_weather.weather_code));
        json_object_set_new(current_weather_obj, "temperature_2m", json_real(current_weather.temperature_2m));
        json_object_set_new(current_weather_obj, "relative_humidity_2m", json_real(current_weather.relative_humidity_2m));
        json_object_set_new(current_weather_obj, "cloud_cover", json_real(current_weather.cloud_cover));
        json_object_set_new(current_weather_obj, "wind_direction_10m", json_real(current_weather.wind_direction_10m));
        json_object_set_new(current_weather_obj, "apparent_temperature", json_real(current_weather.apparent_temperature));
        json_object_set_new(current_weather_obj, "precipitation", json_real(current_weather.precipitation));
        json_object_set_new(current_weather_obj, "rain", json_real(current_weather.rain));
        json_object_set_new(current_weather_obj, "showers", json_real(current_weather.showers));
        json_object_set_new(current_weather_obj, "snowfall", json_real(current_weather.snowfall));
        json_object_set_new(current_weather_obj, "pressure_msl", json_real(current_weather.pressure_msl));
        json_object_set_new(current_weather_obj, "surface_pressure", json_real(current_weather.surface_pressure));
        json_object_set_new(current_weather_obj, "wind_speed_10m", json_real(current_weather.wind_speed_10m));
        json_object_set_new(current_weather_obj, "wind_gusts_10m", json_real(current_weather.wind_gusts_10m));

        /* write location object */
        json_t *location = json_object();
        json_object_set_new(location, "name", json_string(current_location->name));
        json_object_set_new(location, "latitude", json_real(current_location->latitude));
        json_object_set_new(location, "longitude", json_real(current_location->longitude));
        json_object_set_new(location, "timestamp_last_api_call", json_integer(current_location->timestamp_last_api_call));

        /* add weather_data to location */
        json_object_set_new(location, "current_weather", current_weather_obj);
        /* add location object to locations array */
        json_array_append_new(locations_obj, location);
    }
    /* add locations array to root */
    json_object_set_new(root, "locations", locations_obj);

    /* save file with 4 decimal places for floats */
    json_dumpf(root, file, JSON_INDENT(2) | JSON_REAL_PRECISION(6));

    fclose(file); /* this is where the file gets written. buffer->disk */
    json_decref(root);
    return 0;
}
