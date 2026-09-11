/*
 * ui_ambient.cpp — see ui_ambient.h.
 *
 * Per-theme dispatch:
 *   POSEIDON  ->  combined cyberpunk: TRON purple grid + cyan/magenta
 *                 motes + magenta L-path packet (three layers).
 *   MATRIX    ->  souped-up phosphor rain at full T_FG brightness;
 *                 ui_matrix_rain handles the bright leading char +
 *                 fading green trail internally.
 *   E-INK     ->  no-op, paper aesthetic preserved.
 *
 * State invariants:
 *   - All per-frame state derives from millis() and esp_random(); no
 *     module-scope mutable state except the cached NVS flag.
 *   - Functions write directly to M5Cardputer.Display (no sprite buffer,
 *     no PSRAM — this unit's PSRAM is broken).
 *   - Functions are NOT marked IRAM_ATTR — IRAM is full.
 */
#include "ui_ambient.h"
#include "theme.h"
#include "app.h"
#include "ui.h"
#include <M5Cardputer.h>
#include <esp_random.h>
#include <Preferences.h>

/* ---- NVS-backed enable flag ---- */
static bool s_amb_loaded  = false;
static bool s_amb_enabled = true;

bool ui_ambient_enabled(void)
{
    if (!s_amb_loaded) {
        Preferences p;
        if (p.begin("pamb", false)) {
            s_amb_enabled = p.getBool("enabled", true);
            p.end();
        }
        s_amb_loaded = true;
    }
    return s_amb_enabled;
}

void ui_ambient_enabled_set(bool on)
{
    s_amb_enabled = on;
    s_amb_loaded  = true;
    Preferences p;
    if (p.begin("pamb", false)) {
        p.putBool("enabled", on);
        p.end();
    }
}

/* Intentional non-theme literal. AMB_GRID is the dim purple for the
 * TRON grid layer in the POSEIDON painter — has to sit BELOW T_DIM in
 * luminance so menu hint text on top stays the dominant visual element.
 * Not theme-derived because the grid color IS the cyberpunk-grid identity. */
#define AMB_GRID  0x1004u  /* very dark cyan-purple */

/* ---- POSEIDON: combined cyberpunk cyberscape ---- */
static void amb_poseidon(int x, int y, int w, int h)
{
    auto &d = M5Cardputer.Display;
    uint32_t now = millis();

    /* Layer 1: TRON grid scrolling diagonally. */
    int scroll = (int)((now % 6000u) * 30u / 6000u);
    for (int gx = -scroll; gx < w; gx += 30) {
        if (gx >= 0 && gx < w) d.drawFastVLine(x + gx, y, h, AMB_GRID);
    }
    for (int gy = -scroll; gy < h; gy += 30) {
        if (gy >= 0 && gy < h) d.drawFastHLine(x, y + gy, w, AMB_GRID);
    }

    /* Layer 2: POSEIDON motes drifting bottom -> top. */
    static const struct mote_t {
        uint8_t  col_pct;
        uint16_t period_ms;
        uint32_t seed;
        bool     alt;
    } MOTES[8] = {
        {  8, 5000, 0x1111u, false },
        { 22, 7000, 0x2222u, true  },
        { 36, 6000, 0x3333u, false },
        { 50, 8000, 0x4444u, true  },
        { 62, 5500, 0x5555u, false },
        { 78, 7000, 0x6666u, false },
        { 88, 6500, 0x7777u, true  },
        { 14, 9000, 0x8888u, false },
    };
    for (int i = 0; i < 8; ++i) {
        uint32_t period = MOTES[i].period_ms;
        uint32_t phase  = (now + MOTES[i].seed) % period;
        int my = (h - 2) - (int)((int64_t)phase * (h + 2) / (int64_t)period);
        int mx = (MOTES[i].col_pct * w) / 100;
        if (my < 0 || my >= h) continue;
        uint16_t color = MOTES[i].alt ? T_ACCENT2 : T_ACCENT;
        d.drawPixel(x + mx, y + my, color);
        if (my > 0)     d.drawPixel(x + mx, y + my - 1, color);
        if (my < h - 1) d.drawPixel(x + mx, y + my + 1, color);
    }

    /* Layer 3: magenta packet hopping the L-path on an 8 s loop. */
    uint32_t pkt_period = 8000u;
    uint32_t pphase     = now % pkt_period;
    uint32_t quarter    = pkt_period / 4u;
    int      seg        = (int)(pphase / quarter);
    if (seg > 3) seg = 3;
    int      seg_phase  = (int)(pphase - (uint32_t)seg * quarter);
    int      seg_pct    = (int)((int64_t)seg_phase * 100 / (int64_t)quarter);

    int px, py;
    switch (seg) {
    case 0:  px = (w * (10 + (30 - 10) * seg_pct / 100)) / 100;
             py = (h * 20) / 100; break;
    case 1:  px = (w * 30) / 100;
             py = (h * (20 + (75 - 20) * seg_pct / 100)) / 100; break;
    case 2:  px = (w * (30 + (75 - 30) * seg_pct / 100)) / 100;
             py = (h * 75) / 100; break;
    default: px = (w * 75) / 100;
             py = (h * (75 + (95 - 75) * seg_pct / 100)) / 100; break;
    }
    d.fillRect(x + px - 1, y + py - 1, 3, 3, T_ACCENT2);
    d.drawPixel(x + px - 2, y + py,     T_ACCENT2);
    d.drawPixel(x + px + 2, y + py,     T_ACCENT2);
    d.drawPixel(x + px,     y + py - 2, T_ACCENT2);
    d.drawPixel(x + px,     y + py + 2, T_ACCENT2);
}

/* ---- MATRIX: souped-up phosphor rain ----
 * Calls ui_matrix_rain at full T_FG brightness so the body chars are
 * full-bright green, the leading char is white, and the trail fades
 * through medium green to dark — straight out of the movie. */
static void amb_matrix(int x, int y, int w, int h)
{
    ui_matrix_rain(x, y, w, h, T_FG);
}

void ui_ambient_tick(int x, int y, int w, int h)
{
    if (!ui_ambient_enabled()) return;
    if (w <= 0 || h <= 0)      return;
    switch (theme_current_id()) {
    case THEME_POSEIDON:  amb_poseidon(x, y, w, h); break;
    case THEME_MATRIX:    amb_matrix  (x, y, w, h); break;
    case THEME_EINK:      /* paper aesthetic — no ambient */    break;
    case THEME_SYNTHWAVE: amb_poseidon(x, y, w, h); break;  /* cyberpunk lines repainted in vaporwave palette */
    case THEME_PHANTOM:   amb_poseidon(x, y, w, h); break;  /* same motion, violet repaint */
    case THEME_BLOOD:     /* fsociety tactical — no ambient, minimal */ break;
    default:              break;
    }
}
