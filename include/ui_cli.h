#ifndef UI_CLI_H
#define UI_CLI_H
#include "weather_app.h"

void ui_print_startup_message();
void ui_print_menu(weather_app *app);
void ui_print_response_raw(weather_app *_app);
void ui_print_response(weather_app *_app);

int ui_get_selection(int _max);

#endif /* UI_CLI_H */
