#include "ui_cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "stddef.h"
#include "weather_app.h"

void ui_print_startup_message() {
    printf("\n");
    printf("    ╔═════════════════════════╗\n");
    printf("    ║       CIRRUS  CLI       ║\n");
    printf("    ╚═════════════════════════╝\n");
    printf("\n");
}

void ui_print_response_raw(weather_app *_app) {
    printf("%s\n", app_get_prev_api_response_raw(_app));
    return;
}

void ui_print_menu(weather_app *_app) {
    int i = 0;
    size_t nr_loc = app_get_nr_locations(_app);

    for (; i < nr_loc; i++) {
        printf("[%d] %s", i + 1, app_get_location_name(_app, i));
    }
}

int ui_get_selection(int max) {
    int min = 0;
    size_t length = 0;
    long read_value = 0.0f;
    char *line = NULL;
    char *end_pointer = NULL;

    while (1) {
        printf("Select an option (0 to quit) (%d-%d): ", min + 1, max);

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