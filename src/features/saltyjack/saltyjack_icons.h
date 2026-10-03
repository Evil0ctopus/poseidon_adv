#pragma once

#include <Arduino.h>
#include <M5Cardputer.h>

// Scale logical pixels, not line thickness, to keep every view sharp.
static inline void sj_icon_line(int x, int y, uint16_t color, int scale,
                                int x1, int y1, int x2, int y2)
{
    const int dx = abs(x2 - x1), dy = -abs(y2 - y1);
    const int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;
    while (true) {
        M5Cardputer.Display.fillRect(x + x1 * scale, y + y1 * scale, scale, scale, color);
        if (x1 == x2 && y1 == y2) break;
        const int twice = error * 2;
        if (twice >= dy) { error += dy; x1 += sx; }
        if (twice <= dx) { error += dx; y1 += sy; }
    }
}

static void icon_flag(int x, int y, uint16_t color, int s = 1)
{
    sj_icon_line(x, y, color, s, 3, 2, 3, 14);
    sj_icon_line(x, y, color, s, 4, 2, 12, 2);
    sj_icon_line(x, y, color, s, 12, 2, 10, 5);
    sj_icon_line(x, y, color, s, 10, 5, 12, 8);
    sj_icon_line(x, y, color, s, 4, 8, 12, 8);
    sj_icon_line(x, y, color, s, 6, 5, 8, 5);
}

static void icon_skull(int x, int y, uint16_t color, int s = 1)
{
    sj_icon_line(x, y, color, s, 5, 2, 10, 2);
    sj_icon_line(x, y, color, s, 5, 2, 3, 4);
    sj_icon_line(x, y, color, s, 10, 2, 12, 4);
    sj_icon_line(x, y, color, s, 3, 4, 3, 8);
    sj_icon_line(x, y, color, s, 12, 4, 12, 8);
    sj_icon_line(x, y, color, s, 3, 8, 5, 10);
    sj_icon_line(x, y, color, s, 12, 8, 10, 10);
    sj_icon_line(x, y, color, s, 5, 10, 5, 13);
    sj_icon_line(x, y, color, s, 10, 10, 10, 13);
    sj_icon_line(x, y, color, s, 5, 13, 10, 13);
    sj_icon_line(x, y, color, s, 5, 6, 6, 6);
    sj_icon_line(x, y, color, s, 9, 6, 10, 6);
    sj_icon_line(x, y, color, s, 7, 10, 7, 12);
    sj_icon_line(x, y, color, s, 8, 10, 8, 12);
}

static void icon_swords(int x, int y, uint16_t color, int s = 1)
{
    sj_icon_line(x, y, color, s, 2, 2, 13, 13);
    sj_icon_line(x, y, color, s, 13, 2, 2, 13);
    sj_icon_line(x, y, color, s, 2, 2, 5, 3);
    sj_icon_line(x, y, color, s, 13, 2, 10, 3);
    sj_icon_line(x, y, color, s, 9, 12, 12, 9);
    sj_icon_line(x, y, color, s, 3, 9, 6, 12);
}

static void icon_wheel(int x, int y, uint16_t color, int s = 1)
{
    sj_icon_line(x, y, color, s, 5, 3, 10, 3);
    sj_icon_line(x, y, color, s, 10, 3, 12, 5);
    sj_icon_line(x, y, color, s, 12, 5, 12, 10);
    sj_icon_line(x, y, color, s, 12, 10, 10, 12);
    sj_icon_line(x, y, color, s, 10, 12, 5, 12);
    sj_icon_line(x, y, color, s, 5, 12, 3, 10);
    sj_icon_line(x, y, color, s, 3, 10, 3, 5);
    sj_icon_line(x, y, color, s, 3, 5, 5, 3);
    sj_icon_line(x, y, color, s, 7, 1, 7, 14);
    sj_icon_line(x, y, color, s, 1, 7, 14, 7);
    sj_icon_line(x, y, color, s, 3, 3, 12, 12);
    sj_icon_line(x, y, color, s, 12, 3, 3, 12);
}

static void icon_horn(int x, int y, uint16_t color, int s = 1)
{
    sj_icon_line(x, y, color, s, 4, 6, 11, 3);
    sj_icon_line(x, y, color, s, 11, 3, 11, 11);
    sj_icon_line(x, y, color, s, 11, 11, 4, 8);
    sj_icon_line(x, y, color, s, 2, 6, 4, 6);
    sj_icon_line(x, y, color, s, 2, 6, 2, 8);
    sj_icon_line(x, y, color, s, 2, 8, 4, 8);
    sj_icon_line(x, y, color, s, 4, 9, 5, 13);
    sj_icon_line(x, y, color, s, 5, 13, 7, 13);
    sj_icon_line(x, y, color, s, 14, 5, 14, 9);
}

static void icon_web(int x, int y, uint16_t color, int s = 1)
{
    sj_icon_line(x, y, color, s, 7, 1, 7, 14);
    sj_icon_line(x, y, color, s, 1, 7, 14, 7);
    sj_icon_line(x, y, color, s, 2, 2, 13, 13);
    sj_icon_line(x, y, color, s, 13, 2, 2, 13);
    sj_icon_line(x, y, color, s, 7, 3, 11, 7);
    sj_icon_line(x, y, color, s, 11, 7, 7, 11);
    sj_icon_line(x, y, color, s, 7, 11, 3, 7);
    sj_icon_line(x, y, color, s, 3, 7, 7, 3);
    sj_icon_line(x, y, color, s, 7, 1, 13, 7);
    sj_icon_line(x, y, color, s, 13, 7, 7, 13);
    sj_icon_line(x, y, color, s, 7, 13, 1, 7);
    sj_icon_line(x, y, color, s, 1, 7, 7, 1);
}

static void icon_key(int x, int y, uint16_t color, int s = 1)
{
    sj_icon_line(x, y, color, s, 4, 2, 7, 2);
    sj_icon_line(x, y, color, s, 7, 2, 9, 4);
    sj_icon_line(x, y, color, s, 9, 4, 9, 7);
    sj_icon_line(x, y, color, s, 9, 7, 7, 9);
    sj_icon_line(x, y, color, s, 7, 9, 4, 9);
    sj_icon_line(x, y, color, s, 4, 9, 2, 7);
    sj_icon_line(x, y, color, s, 2, 7, 2, 4);
    sj_icon_line(x, y, color, s, 2, 4, 4, 2);
    sj_icon_line(x, y, color, s, 9, 8, 13, 12);
    sj_icon_line(x, y, color, s, 13, 12, 13, 14);
    sj_icon_line(x, y, color, s, 11, 10, 11, 12);
}

typedef void (*sj_icon_fn)(int, int, uint16_t, int);
