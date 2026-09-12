/*
 * radio — lazy domain management.
 *
 * Only one radio stack runs at a time. Switching domains tears down
 * the old one first to free heap. Copied from Davey Jones's proven
 * architecture — keeps BLE init from starving out of RAM when WiFi
 * is already eating 100KB.
 */
#pragma once

#include <Arduino.h>

enum radio_domain_t {
    RADIO_NONE = 0,
    RADIO_WIFI,
    RADIO_BLE,
    RADIO_LORA,
    RADIO_SUBGHZ,
    RADIO_NRF24,
};

/* Switch domains. Tears down the current one, brings up the new one.
 * Call with RADIO_NONE to drop all radios and return heap. */
bool radio_switch(radio_domain_t target);
radio_domain_t radio_current(void);

/* Short name for status bar ("wifi", "ble", "idle"). */
const char *radio_name(void);

/* Force the WiFi driver to a clean STA + non-promiscuous state, but
 * ONLY if it was previously inited. Calling esp_wifi_disconnect /
 * esp_wifi_set_mode on a fresh boot where WiFi has never been inited
 * returns ESP_ERR_WIFI_NOT_INIT and leaves Bruce's libs in a
 * half-state that makes subsequent WiFi.mode(WIFI_STA) init fail
 * NO_MEM with "Expected to init 4 rx buffer, actual is 3". Use this
 * everywhere instead of inlining the raw IDF calls. */
void wifi_force_clean_sta(void);

/* Fully deinit + destroy the WiFi driver and its netifs. Normally the
 * driver stays resident-but-stopped after a WiFi feature exits (see
 * teardown_current) to avoid deinit/init churn fragmenting the heap.
 * That resident state can itself be the fragmentation source for a
 * feature that needs a large contiguous block right after a prior WiFi
 * session (e.g. wardrive's 24 KB AP table) — call this first in that
 * case, then re-init via wifi_lean_sta_init(). No-op if never inited. */
void wifi_release_driver(void);

/* Idempotent raw-IDF lean WiFi init in STA mode. Bypasses Arduino's
 * WiFi.mode(WIFI_STA) which uses DEFAULT buffer counts that won't fit
 * in DMA-capable RAM after the M5GFX framebuffer takes its ~60 KB.
 * Returns true if WiFi is up (either already, or successfully started
 * by this call). Every WiFi-feature should call this BEFORE
 * esp_wifi_set_promiscuous / scan_start / etc. */
bool wifi_lean_sta_init(void);
