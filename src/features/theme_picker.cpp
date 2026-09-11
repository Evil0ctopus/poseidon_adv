/*
 * theme_picker.cpp — visual theme selector with live preview.
 *
 * Browses POSEIDON / MATRIX / E-INK with arrow keys, previewing each
 * in-RAM only (no NVS thrash). ENTER commits the choice to NVS; ESC
 * restores the original.
 */
#include "../app.h"
#include "../ui.h"
#include "../input.h"
#include "../theme.h"

void feat_theme_picker(void)
{
    auto &d = M5Cardputer.Display;
    theme_id_t original = theme_current_id();  /* for ESC-restore */
    int sel = (int)original;

    int prev_sel = -1;
    while (true) {
        /* Preview only — no NVS writes during browsing. */
        theme_preview((theme_id_t)sel);
        if (sel != prev_sel) { ui_force_clear_body(); prev_sel = sel; }
        ui_draw_status("theme", "");

        d.setTextColor(T_ACCENT, T_BG);
        d.setCursor(4, BODY_Y + 2); d.print("THEME");
        d.drawFastHLine(4, BODY_Y + 12, SCR_W - 8, T_ACCENT2);

        char cnt_str[12];
        snprintf(cnt_str, sizeof(cnt_str), "%d/%d", sel + 1, (int)THEME__COUNT);
        d.setTextColor(T_DIM, T_BG);
        int tw = d.textWidth(cnt_str);
        d.setCursor(SCR_W - tw - 6, BODY_Y + 2);
        d.print(cnt_str);

        const int rows = 6;
        const int row_h = 14;
        const int first_y = BODY_Y + 16;
        int first = sel - rows / 2;
        if (first < 0) first = 0;
        if (first + rows > (int)THEME__COUNT) first = (int)THEME__COUNT - rows;
        if (first < 0) first = 0;

        for (int r = 0; r < rows && first + r < (int)THEME__COUNT; r++) {
            int i = first + r;
            int y = first_y + r * row_h;
            /* Flip palette just to render this row's swatches. Still RAM-only. */
            theme_preview((theme_id_t)i);
            bool s = (i == sel);
            bool is_curr = (i == (int)original);
            if (s) {
                d.fillRoundRect(2, y - 1, SCR_W - 4, 13, 2, T_SEL_BG);
                d.drawRoundRect(2, y - 1, SCR_W - 4, 13, 2, T_SEL_BD);
            }
            uint16_t bg = s ? T_SEL_BG : theme().bg;
            d.setTextColor(is_curr ? T_ACCENT2 : T_DIM, bg);
            d.setCursor(6, y + 1);
            d.print(is_curr ? ">" : " ");

            d.setTextColor(s ? T_FG : T_ACCENT, bg);
            d.setCursor(16, y + 1);
            d.printf("%-14.14s", theme().name);

            /* Color swatches */
            d.fillRect(138, y + 2, 12, 8, theme().accent);
            d.fillRect(152, y + 2, 12, 8, theme().accent2);
            d.fillRect(166, y + 2, 12, 8, theme().good);
            d.fillRect(180, y + 2, 12, 8, theme().bad);
            d.fillRect(194, y + 2, 12, 8, theme().dim);
        }
        theme_preview((theme_id_t)sel);   /* back to browsed theme after swatch loop */

        /* Scroll arrows */
        if (first > 0) {
            d.fillTriangle(SCR_W - 7, first_y - 2, SCR_W - 3, first_y - 2, SCR_W - 5, first_y - 5, T_ACCENT2);
        }
        if (first + rows < (int)THEME__COUNT) {
            int ay = first_y + rows * row_h - 2;
            d.fillTriangle(SCR_W - 7, ay, SCR_W - 3, ay, SCR_W - 5, ay + 3, T_ACCENT2);
        }

        ui_draw_footer(";/.=browse  ENTER=apply  ESC=back");

        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(20); continue; }
        if (k == PK_ESC) {
            /* Restore original in RAM — NVS still holds original, nothing to write. */
            theme_preview(original);
            return;
        }
        if (k == ';' || k == PK_UP)   sel = (sel - 1 + THEME__COUNT) % THEME__COUNT;
        if (k == '.' || k == PK_DOWN) sel = (sel + 1) % THEME__COUNT;
        if (k == PK_ENTER) {
            theme_set((theme_id_t)sel);    /* THE ONE NVS write — commit */
            d.fillScreen(T_BG);
            ui_toast("theme applied", T_GOOD, 600);
            return;
        }
    }
}
