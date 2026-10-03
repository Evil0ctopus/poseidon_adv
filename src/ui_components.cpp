#include "ui.h"
#include "input.h"
#include "theme.h"
#include "ui_ambient.h"
#include <Preferences.h>
#include <ui_layout.h>

void ui_capture_frame(void)
{
    auto &d = M5Cardputer.Display;
    // The uint16_t overload returns byte-swapped pixels on this display driver.
    lgfx::rgb565_t row[SCR_W];
    char hex[SCR_W * 4 + 1];
    static constexpr char digits[] = "0123456789ABCDEF";
    uint32_t checksum = 2166136261u;
    Serial.printf("[FRAME_BEGIN] %d %d\n", SCR_W, SCR_H);
    for (int y = 0; y < SCR_H; ++y) {
        d.readRect(0, y, SCR_W, 1, row);
        for (int x = 0; x < SCR_W; ++x) {
            const uint16_t pixel = row[x].raw;
            checksum = (checksum ^ pixel) * 16777619u;
            for (int n = 0; n < 4; ++n)
                hex[x * 4 + n] = digits[(pixel >> (12 - n * 4)) & 15];
        }
        hex[SCR_W * 4] = 0;
        Serial.printf("[FRAME_ROW] %d %s\n", y, hex);
        delay(1);
    }
    Serial.printf("[FRAME_END] %08lX\n", (unsigned long)checksum);
}

static bool s_motion_loaded = false;
static bool s_motion = true;

bool ui_motion_enabled(void)
{
    if (!s_motion_loaded) {
        Preferences p;
        if (p.begin("pui", true)) {
            s_motion = p.getBool("motion", true);
            p.end();
        } else {
            Serial.println("[UI] motion preferences unavailable; using default");
        }
        s_motion_loaded = true;
    }
    return s_motion;
}

void ui_motion_enabled_set(bool on)
{
    Preferences p;
    if (!p.begin("pui", false)) {
        Serial.println("[UI] cannot save motion preference");
        ui_toast("Cannot save motion", T_BAD, 900);
        return;
    }
    const bool saved = p.putBool("motion", on) != 0;
    p.end();
    if (!saved) {
        Serial.println("[UI] motion preference write failed");
        ui_toast("Cannot save motion", T_BAD, 900);
        return;
    }
    s_motion = on;
    s_motion_loaded = true;
}

void ui_label(int x, int y, int width, uint16_t fg, uint16_t bg,
              const char *text, uint8_t scale)
{
    auto &d = M5Cardputer.Display;
    if (scale == 0) scale = 1;
    if (x < 0 || x >= SCR_W || width <= 0) return;
    if (width > SCR_W - x) width = SCR_W - x;
    char fitted[64];
    ui_layout::fit(text, fitted, sizeof(fitted), width / (6 * scale));
    d.fillRect(x, y, width, 8 * scale, bg);
    d.setTextSize(scale);
    d.setTextColor(fg, bg);
    d.setCursor(x, y);
    d.print(fitted);
    d.setTextSize(1);
}

void ui_header(const char *title, const char *detail)
{
    auto &d = M5Cardputer.Display;
    d.fillRect(0, BODY_Y, SCR_W, 15, T_BG);
    const int detail_width = detail && *detail
        ? min(90, static_cast<int>(strlen(detail)) * 6) : 0;
    ui_label(6, BODY_Y + 3, SCR_W - 12 - detail_width - (detail_width ? 8 : 0),
             T_FG, T_BG, title);
    if (detail_width) {
        ui_label(SCR_W - 6 - detail_width, BODY_Y + 3, detail_width,
                 T_DIM, T_BG, detail);
    }
    d.drawFastHLine(6, BODY_Y + 14, SCR_W - 12, theme().rule);
}

void ui_scrollbar(int x, int y, int height, int first, int visible, int total)
{
    auto &d = M5Cardputer.Display;
    d.fillRect(x, y, 2, height, T_BG);
    if (total <= visible || total <= 0 || height <= 0) return;
    const int thumb = max(4, height * visible / total);
    const int offset = (height - thumb) * first / (total - visible);
    d.fillRect(x, y, 2, height, theme().rule);
    d.fillRect(x, y + offset, 2, thumb, T_ACCENT);
}

void ui_show_text(const char *title, const char *text)
{
    const uint8_t scale = ui_big_text() ? 2 : 1;
    const int cells = scale == 2 ? 18 : 37;
    const int rows = scale == 2 ? 4 : 8;
    const int row_height = scale == 2 ? 21 : 11;
    const int total = static_cast<int>(ui_layout::line_count(text, cells));
    int first = 0;
    int previous = -1;
    while (true) {
        if (previous != first) {
            previous = first;
            ui_force_clear_body();
            char position[24];
            snprintf(position, sizeof(position), "%d/%d", total ? first + 1 : 0, total);
            ui_header(title, position);
            const char *p = text;
            char line[64];
            for (int i = 0; i < first; ++i)
                ui_layout::wrap_next(p, line, sizeof(line), cells);
            for (int r = 0; r < rows && ui_layout::wrap_next(p, line, sizeof(line), cells); ++r)
                ui_label(6, BODY_Y + 20 + r * row_height, 222, T_FG, T_BG, line, scale);
            ui_scrollbar(SCR_W - 3, BODY_Y + 20, 88, first, rows, total);
            ui_draw_footer(";/. scroll ENTER/` back");
            Serial.printf("[UI_TEXT] first=%d visible=%d total=%d\n", first, rows, total);
        }
        const uint16_t k = input_poll();
        if (k == PK_ESC || k == PK_ENTER) return;
        if (k == ';' || k == PK_UP) first = max(0, first - 1);
        if (k == '.' || k == PK_DOWN) first = min(max(0, total - rows), first + 1);
        delay(10);
    }
}

bool ui_confirm(const char *title, const char *detail)
{
    ui_force_clear_body();
    ui_header(title, "CONFIRM");
    const char *p = detail;
    char line[64];
    for (int r = 0; r < 6 && ui_layout::wrap_next(p, line, sizeof(line), 37); ++r)
        ui_label(6, BODY_Y + 24 + r * 11, 222, T_FG, T_BG, line);
    ui_draw_footer("ENTER confirm  ` cancel");
    while (true) {
        const uint16_t key = input_poll();
        if (key == PK_ENTER) return true;
        if (key == PK_ESC) return false;
        delay(10);
    }
}

void feat_ui_preferences(void)
{
    int previous = -1;
    while (true) {
        const int state = (ui_motion_enabled() ? 1 : 0)
                        | (ui_big_text() ? 2 : 0)
                        | (ui_ambient_enabled() ? 4 : 0);
        if (state != previous) {
            previous = state;
            ui_force_clear_body();
            ui_draw_status("display", "");
            ui_header("DISPLAY", "DEEPWATER");
            ui_label(8, BODY_Y + 24, 224, T_FG, T_BG,
                     state & 1 ? "[M] Motion          ON" : "[M] Motion          REDUCED");
            ui_label(8, BODY_Y + 42, 224, T_FG, T_BG,
                     state & 2 ? "[B] Large text      ON" : "[B] Large text      OFF");
            ui_label(8, BODY_Y + 60, 224, T_FG, T_BG,
                     state & 4 ? "[A] Ambient         ON" : "[A] Ambient         OFF");
            ui_label(8, BODY_Y + 85, 224, T_DIM, T_BG, "Reduced motion pauses decoration.");
            ui_draw_footer("M motion B text A ambient ` back");
        }
        uint16_t k = input_poll();
        if (k == PK_ESC) return;
        if (k == 'm' || k == 'M') ui_motion_enabled_set(!ui_motion_enabled());
        if (k == 'b' || k == 'B') ui_big_text_set(!ui_big_text());
        if (k == 'a' || k == 'A') ui_ambient_enabled_set(!ui_ambient_enabled());
        delay(10);
    }
}
