#include "../app.h"
#include "../ui.h"
#include "../input.h"
#include "../theme.h"
#include <ui_layout.h>

void feat_theme_picker(void)
{
    auto &d = M5Cardputer.Display;
    const theme_id_t original = theme_current_id();
    int selected = static_cast<int>(original);
    int previous = -1;
    while (true) {
        if (selected != previous) {
            previous = selected;
            theme_preview(static_cast<theme_id_t>(selected));
            ui_force_clear_body();
            ui_draw_status("theme", "preview");
            char position[16];
            snprintf(position, sizeof(position), "%d/%d", selected + 1, THEME__COUNT);
            ui_header("THEME", position);
            constexpr int rows = 6;
            const int first = ui_layout::window_start(selected, THEME__COUNT, rows);
            for (int r = 0; r < rows; ++r) {
                const int i = first + r;
                const int y = BODY_Y + 20 + r * 14;
                theme_preview(static_cast<theme_id_t>(i));
                const poseidon_theme_t candidate = theme();
                theme_preview(static_cast<theme_id_t>(selected));
                const bool focused = i == selected;
                const uint16_t bg = focused ? T_SEL_BG : T_BG;
                d.fillRoundRect(4, y - 1, 230, 13, 2, bg);
                ui_label(10, y + 1, 6, T_ACCENT, bg, i == original ? "*" : "");
                ui_label(24, y + 1, 108, focused ? T_FG : T_DIM, bg, candidate.name);
                const uint16_t swatches[] = {
                    candidate.bg, candidate.accent, candidate.fg, candidate.good, candidate.bad
                };
                for (int s = 0; s < 5; ++s) {
                    d.fillRect(144 + s * 16, y + 2, 12, 8, swatches[s]);
                    d.drawRect(144 + s * 16, y + 2, 12, 8, theme().rule);
                }
            }
            ui_scrollbar(237, BODY_Y + 20, 84, first, rows, THEME__COUNT);
            ui_label(6, BODY_Y + 104, 228, T_DIM, T_BG, "* saved theme / preview only");
            ui_draw_footer(";/. preview ENTER save ` cancel");
        }
        const uint16_t k = input_poll();
        if (k == PK_ESC) { theme_preview(original); ui_status_invalidate(); return; }
        if (k == ';' || k == PK_UP) selected = (selected + THEME__COUNT - 1) % THEME__COUNT;
        if (k == '.' || k == PK_DOWN) selected = (selected + 1) % THEME__COUNT;
        if (k == PK_ENTER) {
            if (!theme_set(static_cast<theme_id_t>(selected))) {
                ui_toast("Theme could not be saved", T_BAD, 900);
                previous = -1;
                continue;
            }
            ui_force_clear_body();
            ui_toast("Theme saved", T_GOOD, 600);
            return;
        }
        delay(10);
    }
}
