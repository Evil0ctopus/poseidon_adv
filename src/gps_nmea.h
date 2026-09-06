#pragma once

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static inline int gps_nmea_split_csv(char *line, char **fields, int max)
{
    int n = 0;
    if (!line || !fields || max <= 0) return 0;
    fields[n++] = line;
    for (char *p = line; *p && n < max; ++p) {
        if (*p == ',') {
            *p = '\0';
            fields[n++] = p + 1;
        } else if (*p == '*') {
            *p = '\0';
            break;
        }
    }
    return n;
}

static inline bool gps_nmea_sentence_has_fix(const char *sentence)
{
    if (!sentence || !*sentence) return false;

    char line[128];
    size_t len = strlen(sentence);
    if (len >= sizeof(line)) return false;
    memcpy(line, sentence, len + 1);

    char *fields[16] = {0};
    int n = gps_nmea_split_csv(line, fields, 16);
    if (n < 10) return false;

    if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
        return atoi(fields[6]) >= 1;
    }
    if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
        return fields[2][0] == 'A';
    }
    return false;
}
