/*
 * wifi_wardrive — public globals.
 *
 * Triton (and other WiFi-centric features) can seed their internal
 * BSSID→SSID resolver from wardrive's accumulated AP table instead of
 * waiting to catch beacons from scratch. Single-producer / many-reader;
 * safe because radio mutex prevents wardrive + consumers running
 * concurrently.
 */
#pragma once

#include <Arduino.h>

/* This is a rolling cache, not a capture limit: flushed entries are recycled
 * when it fills. Keep it small enough to leave runtime headroom for WiFi,
 * GPS, C5, the SFX task, and the wardrive UI on the no-PSRAM Cardputer.
 * A 200-entry cache left only ~3.7 KB free on hardware after startup. */
#define WARDRIVE_MAX_APS 96

/* Field order matters: 8-byte members first avoids padding that would
 * otherwise inflate sizeof() well past the 88 bytes this packs down to
 * (256 * 96-with-padding once didn't fit the fragmented heap at all). */
struct wdr_ap_t {
    double   lat;
    double   lon;
    float    alt;
    uint32_t first_seen_ms;
    uint32_t last_seen;
    uint8_t  bssid[6];
    char     ssid[33];
    char     first_seen_utc[16]; /* YYYYMMDD-HHMMSS for WiGLE, empty if no UTC fix */
    int8_t   rssi;
    uint8_t  channel;
    uint8_t  auth;
    bool     has_gps;
    bool     dirty;
};

static_assert(sizeof(wdr_ap_t) == 88, "wdr_ap_t packing changed");

/* Lazily allocated on first wardrive use, then resident (shared session state:
 * triton and pmkid read it after wardrive populates it, so it is never freed
 * on wardrive exit). Null until the first wardrive; keeps roughly 8.5 KB free
 * otherwise. When full, the capture rolls through entries already flushed to
 * SD so the CSV can continue beyond the in-memory cache size. */
extern wdr_ap_t *g_wdr_aps;
bool wdr_aps_ensure(void);   /* allocs g_wdr_aps if needed; false on OOM */
extern int      g_wdr_ap_count;
