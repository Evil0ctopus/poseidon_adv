#include "menu_carousel.h"
#include "menu_icons.h"
#include "ui.h"
#include "input.h"
#include "radio.h"
#include "theme.h"
#include "screensaver.h"
#include <ui_layout.h>
#include <ctype.h>

static constexpr const char *FOOTER =
    "ENTER open = help ` back ;/. move letter jump";

static int child_count(const menu_node_t *parent)
{
    int count = 0;
    for (const menu_node_t *c = parent->children; c && c->hotkey; ++c) ++count;
    return count;
}

static void draw_card(const menu_node_t *parent, int cursor, int count)
{
    const uint32_t paint_started = millis();
    auto &d = M5Cardputer.Display;
    const menu_node_t *item = &parent->children[cursor];
    ui_force_clear_body();
    ui_draw_status(radio_name(), "");
    ui_draw_footer(FOOTER);
    char position[24];
    snprintf(position, sizeof(position), "%d/%d", cursor + 1, count);
    ui_header(parent->label, position);

    d.fillRoundRect(6, BODY_Y + 22, SCR_W - 12, 77, 5, T_SEL_BG);
    d.drawRoundRect(6, BODY_Y + 22, SCR_W - 12, 77, 5, theme().rule);
    draw_menu_icon(33, BODY_Y + 49, T_ACCENT, parent, item);
    const uint8_t scale = strlen(item->label) <= 13 ? 2 : 1;
    ui_label(64, BODY_Y + 32, 162, T_FG, T_SEL_BG, item->label, scale);

    const char *hint = item->hint;
    char line[64];
    for (int row = 0; row < 3 && ui_layout::wrap_next(hint, line, sizeof(line), 27); ++row)
        ui_label(64, BODY_Y + 55 + row * 11, 162, T_DIM, T_SEL_BG, line);

    char key[4];
    snprintf(key, sizeof(key), "[%c]", toupper(item->hotkey));
    ui_label(24, BODY_Y + 73, 24, T_FG, T_SEL_BG, key);
    ui_label(6, BODY_Y + 103, 18, T_ACCENT, T_BG, "<");
    ui_label(SCR_W - 12, BODY_Y + 103, 6, T_ACCENT, T_BG, ">");
    const int dots_width = count * 6;
    const int dots_x = (SCR_W - dots_width) / 2;
    for (int i = 0; i < count; ++i)
        d.fillRect(dots_x + i * 6, BODY_Y + 106, i == cursor ? 4 : 2, 2,
                   i == cursor ? T_ACCENT : theme().rule);
    Serial.printf("[UI_PAINT] card %lums\n", (unsigned long)(millis() - paint_started));
}

void carousel_run_submenu(const menu_node_t *parent)
{
    const int count = child_count(parent);
    if (count == 0) return;
    int cursor = 0;
    uint32_t selection_at = millis();
    int last_progress = -1;
    draw_card(parent, cursor, count);

    while (true) {
        if (menu_style_get() != MENU_STYLE_CAROUSEL) return;
        const uint32_t now = millis();
        if (ui_motion_enabled()) {
            const int progress = 216 * ui_layout::ease_out(now - selection_at, 140) / 256;
            if (progress != last_progress) {
                M5Cardputer.Display.drawFastHLine(12, BODY_Y + 97, progress, T_ACCENT);
                last_progress = progress;
            }
        }
        const uint16_t k = input_poll();
        if (k == PK_NONE) {
            if (screensaver_check_idle()) {
                draw_card(parent, cursor, count);
                selection_at = millis();
                last_progress = -1;
            }
            ui_draw_status(radio_name(), "");
            delay(8);
            continue;
        }
        if (k == PK_ESC) return;
        if (k == '=' || k == '?') {
            g_current_feature_item = &parent->children[cursor];
            ui_show_current_help();
            g_current_feature_item = nullptr;
            draw_card(parent, cursor, count);
            last_progress = -1;
            continue;
        }
        if (k == ';' || k == PK_LEFT || k == PK_UP ||
            k == '.' || k == PK_RIGHT || k == PK_DOWN) {
            const bool back = k == ';' || k == PK_LEFT || k == PK_UP;
            cursor = (cursor + (back ? count - 1 : 1)) % count;
            draw_card(parent, cursor, count);
            selection_at = millis();
            last_progress = -1;
            continue;
        }
        bool open = k == PK_ENTER;
        if (k >= 0x20 && k < 0x7F) {
            for (int i = 0; i < count; ++i) {
                if (parent->children[i].hotkey == tolower(static_cast<int>(k))) {
                    cursor = i;
                    open = true;
                    break;
                }
            }
        }
        if (open) {
            const menu_node_t *item = &parent->children[cursor];
            if (item->action) menu_execute_action(item);
            else if (item->children) carousel_run_submenu(item);
            draw_card(parent, cursor, count);
            selection_at = millis();
            last_progress = -1;
        }
    }
}
