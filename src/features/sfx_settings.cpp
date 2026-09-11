/*
 * sfx_settings — volume + mute UI.
 *
 * Under System → Sound. `+`/`-` adjusts volume, `m` toggles mute.
 * Both persist to NVS immediately.
 */
#include "../app.h"
#include "../theme.h"
#include "../ui.h"
#include "../input.h"
#include "../sfx.h"
#include <stdio.h>

void feat_sfx_settings(void)
{
    auto &d = M5Cardputer.Display;
    ui_draw_footer("+/-=vol  M=mute  P=prof  T=test  `=back");

    while (true) {
        ui_clear_body();
        d.setTextColor(T_ACCENT, T_BG);
        d.setCursor(4, BODY_Y + 2); d.print("SOUND SETTINGS");
        d.drawFastHLine(4, BODY_Y + 12, SCR_W - 8, T_ACCENT);

        uint8_t vol = sfx_get_volume();
        bool muted = sfx_is_muted();
        sfx_profile_t prof = sfx_get_profile();

        /* Volume bar */
        d.setTextColor(muted ? T_DIM : T_FG, T_BG);
        d.setCursor(4, BODY_Y + 18); d.printf("Volume  : %d/10", vol);

        int bx = 4, by = BODY_Y + 30, bw = SCR_W - 20, bh = 10;
        d.drawRect(bx, by, bw, bh, T_DIM);
        int fill = (vol * (bw - 2)) / 10;
        if (fill > 0) {
            uint16_t fillc = muted ? T_DIM : (vol <= 3 ? T_ACCENT : vol <= 7 ? T_ACCENT2 : T_WARN);
            d.fillRect(bx + 1, by + 1, fill, bh - 2, fillc);
        }

        d.setTextColor(muted ? T_BAD : T_GOOD, T_BG);
        d.setCursor(4, BODY_Y + 46);
        d.printf("Mute    : %s", muted ? "MUTED" : "UNMUTED");

        d.setTextColor(T_ACCENT2, T_BG);
        d.setCursor(4, BODY_Y + 58);
        d.printf("Profile : %s", sfx_profile_name(prof));

        d.setTextColor(T_DIM, T_BG);
        d.setCursor(4, BODY_Y + 74); d.print("P = cycle audio profile");
        d.setCursor(4, BODY_Y + 86); d.print("T = test sample  ` = back");

        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(30); continue; }
        if (k == PK_ESC) { sfx_back(); return; }
        if (k == '+' || k == '=') {
            if (vol < 10) sfx_set_volume(vol + 1);
            sfx_click();
        }
        if (k == '-' || k == '_') {
            if (vol > 0) sfx_set_volume(vol - 1);
            sfx_click();
        }
        if (k == 'm' || k == 'M') {
            sfx_set_mute(!muted);
            sfx_click();
        }
        if (k == 'p' || k == 'P') {
            sfx_profile_t next = (sfx_profile_t)(((int)prof + 1) % SFX_PROFILE_COUNT);
            sfx_set_profile(next);
            sfx_select();
        }
        if (k == 't' || k == 'T') {
            sfx_capture();  /* a rich, multi-note SFX so the user can hear curve */
        }
    }
}
