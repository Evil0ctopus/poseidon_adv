/*
 * screensaver — full-screen idle takeover with a pool of theme-tinting
 * painters.
 *
 * Wired into the menu loops via screensaver_check_idle() — call when
 * input_poll() returned PK_NONE. Internally checks the idle threshold
 * against input_last_input_ms() and, if exceeded, picks a painter from
 * the pool and runs it full-screen until any keypress arrives. The
 * triggering keypress is consumed (not surfaced to the menu) so the
 * user doesn't accidentally launch a feature on wake.
 *
 * Pool (all theme-tinting via T_FG / T_ACCENT / T_ACCENT2 / T_DIM):
 *   0  WARDRIVE       — fake AP scroll, channel hop, PMKID flash, packet streaks
 *   1  MATRIX RAIN    — full-screen phosphor cascade
 *   2  BREATHING      — POSEIDON wordmark fade-in/out, drifts to prevent burn-in
 *   3  DEEP SCAN      — sonar pulse from center, named contacts blip in/out
 *   4  PORT SCAN      — vertical port grid 0-65535, scanning beam, open/closed flashes
 *   5  HEX CASCADE    — falling hex bytes with occasional decoded ASCII reveals
 *   6  TERMINAL CRACK — fake hashcat / john output streaming, periodic CRACKED flash
 *   7  NEURAL ARC     — pulsing dot mesh with electric arcs jumping between nodes
 *   8  GLITCH BSOD    — periodic full-screen text glitches with chromatic shake
 *   9  TIDE WAVES     — overlapping sine waves drifting across the screen
 *   10 SAT SKYPLOT    — live orbital satellite tracking simulation with polar rings
 *   11 RF WATERFALL   — SDR spectrum waterfall with noise floor and transmission bursts
 *   12 STARFIELD WARP — 3D hyperspace star jump radiating outward
 *
 * Pick mode: -1 (SHUFFLE) picks randomly from the pool excluding the last-shown
 * one (persists across reboots in NVS); >=0 locks to that specific painter.
 *
 * NVS namespace "pscr": enabled (bool, default true), timeoutms (uint32,
 * default 120000), pick (int8, default -1 = SHUFFLE), last (int8, last
 * actually-run index — used to exclude in shuffle mode).
 */
#pragma once

#include <Arduino.h>

#define SCREENSAVER_PICK_SHUFFLE  (-1)

/* Returns true if the screensaver actually ran (caller should redraw
 * its UI on return). Returns false if still idle-but-not-yet or if the
 * screensaver is disabled. */
bool screensaver_check_idle(void);

bool     screensaver_enabled(void);
void     screensaver_enabled_set(bool on);
uint32_t screensaver_timeout_ms(void);
void     screensaver_timeout_set(uint32_t ms);

/* ---- Pool introspection + picker ---- */
int          screensaver_pool_count(void);
const char  *screensaver_pool_name(int idx);

/* Get the user's pick. Returns SCREENSAVER_PICK_SHUFFLE (-1) for random
 * rotation, or 0..pool_count-1 to lock to a specific painter. */
int          screensaver_pick_get(void);
void         screensaver_pick_set(int idx);

/* Force-run a specific painter (preview from the picker UI). Blocks
 * until a key is pressed. idx is clamped to 0..pool_count-1. */
void         screensaver_run_index(int idx);
