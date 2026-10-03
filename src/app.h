/*
 * POSEIDON — shared types, colors, constants.
 */
#pragma once

#include <Arduino.h>
#include <M5Cardputer.h>
#include "theme.h"

/* ---- palette (16-bit 565, via M5Cardputer.Display) ---- */
#define COL_BG       T_BG
#define COL_FG       T_FG
#define COL_ACCENT   T_ACCENT
#define COL_WARN     T_WARN
#define COL_BAD      T_BAD
#define COL_GOOD     T_GOOD
#define COL_DIM      T_DIM
#define COL_MAGENTA  T_ACCENT2

/* ---- display geometry ---- */
#define SCR_W 240
#define SCR_H 135
#define STATUS_H 12
#define FOOTER_H 10
#define BODY_Y   (STATUS_H)
#define BODY_H   (SCR_H - STATUS_H - FOOTER_H)
#define FOOTER_Y (SCR_H - FOOTER_H)

/* ---- build info ---- */
/* POSEIDON_VERSION comes from -D in platformio.ini; src/version.h
 * provides an #ifndef-guarded fallback. Defining it here too caused
 * a compiler "redefined" warning on every build. */
