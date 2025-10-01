#ifndef DEFINES_H
#define DEFINES_H

#define TRUE 1
#define FALSE 0

#define SECONDS_BETWEEN_CALLS 1

#define MAX_CITY_NAME 128

#define MAX_RESPONSE_BUFFER_LENGTH 16384
#define MAX_URL_LENGTH 4096

#define APP_EXIT 1

#define LOCATIONS_FILE_PATH "./data/locations.json"
#define LOCATIONS_FILE_FOLDER "data"

#define HARD_CODED_LOCATIONS                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
    "{\"locations\":[{\"name\":\"Stockholm\",\"latitude\":59.3293,\"longitude\":18.0686},{\"name\":\"Göteborg\",\"latitude\":57.7089,\"longitude\":11.9746},{\"name\":\"Malmö\",\"latitude\":55.6050,\"longitude\":13.0038},{\"name\":\"Uppsala\",\"latitude\":59.8586,\"longitude\":17.6389},{\"name\":\"Västerås\",\"latitude\":59.6099,\"longitude\":16.5448},{\"name\":\"Örebro\",\"latitude\":59.2741,\"longitude\":15.2066},{\"name\":\"Linköping\",\"latitude\":58.4109,\"longitude\":15.6216},{\"name\":\"Helsingborg\",\"latitude\":56.0465,\"longitude\":12.6945},{\"name\":\"Jönköping\",\"latitude\":57.7815,\"longitude\":14.1562},{\"name\":\"Norrköping\",\"latitude\":58.5877,\"longitude\":16.1924},{\"name\":\"Lund\",\"latitude\":55.7047,\"longitude\":13.1910},{\"name\":\"Gävle\",\"latitude\":60.6749,\"longitude\":17.1413},{\"name\":\"Sundsvall\",\"latitude\":62.3908,\"longitude\":17.3069},{\"name\":\"Umeå\",\"latitude\":63.8258,\"longitude\":20.2630},{\"name\":\"Luleå\",\"latitude\":65.5848," \
    "\"longitude\":22.1567},{\"name\":\"Kiruna\",\"latitude\":67.8558,\"longitude\":20.2253}]}"

#define URL_TEMPLATE_METEO "https://api.open-meteo.com/v1/forecast?latitude=%.4lf&longitude=%.4lf&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,rain,showers,snowfall,weather_code,cloud_cover,pressure_msl,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m"

#endif /* DEFINES_H */
