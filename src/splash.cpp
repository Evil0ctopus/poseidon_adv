#include "app.h"
#include "ui.h"
#include "input.h"
#include "sfx.h"
#include "version.h"
#include <ui_layout.h>

static uint16_t blend565(uint16_t a, uint16_t b, uint16_t t)
{
    const uint16_t r = (((a >> 11) & 31) * (256 - t) + ((b >> 11) & 31) * t) / 256;
    const uint16_t g = (((a >> 5) & 63) * (256 - t) + ((b >> 5) & 63) * t) / 256;
    const uint16_t blue = ((a & 31) * (256 - t) + (b & 31) * t) / 256;
    return (r << 11) | (g << 5) | blue;
}

static void emblem(uint16_t color)
{
    auto &d = M5Cardputer.Display;
    d.fillRoundRect(118, 20, 4, 37, 1, color);
    d.fillRoundRect(102, 22, 4, 20, 1, color);
    d.fillRoundRect(134, 22, 4, 20, 1, color);
    d.drawLine(104, 41, 120, 48, color);
    d.drawLine(105, 40, 120, 47, color);
    d.drawLine(120, 48, 136, 41, color);
    d.drawLine(120, 47, 135, 40, color);
    d.fillTriangle(120, 14, 115, 23, 125, 23, color);
    d.fillTriangle(104, 17, 99, 26, 109, 26, color);
    d.fillTriangle(136, 17, 131, 26, 141, 26, color);
}

void ui_splash(void)
{
    const uint32_t boot_started = millis();
    auto &d = M5Cardputer.Display;
    d.fillScreen(T_BG);
    d.setTextWrap(false, false);
    ui_label(72, 67, 96, T_FG, T_BG, "POSEIDON", 2);
    ui_label(69, 90, 108, T_DIM, T_BG, "COMMAND THE DEEP");
    char version[40];
    snprintf(version, sizeof(version), "v%s  /  DEEPWATER", poseidon_version());
    const int width = min(228, static_cast<int>(strlen(version)) * 6);
    ui_label((SCR_W - width) / 2, 119, width, T_DIM, T_BG, version);
    d.drawFastHLine(12, 108, 216, theme().rule);
    sfx_boot(); // The existing SFX queue already plays asynchronously.

    if (ui_motion_enabled()) {
        const uint32_t start = millis();
        uint32_t last_frame = 0;
        while (millis() - start < 900) {
            if (input_poll() != PK_NONE) break;
            const uint32_t elapsed = millis() - start;
            if (elapsed - last_frame >= 30) {
                last_frame = elapsed;
                const uint16_t amount = ui_layout::ease_out(elapsed, 650);
                emblem(blend565(T_BG, T_ACCENT, amount));
                d.drawFastHLine(12, 108, 216 * amount / 256, T_ACCENT);
            }
            delay(5);
        }
    } else {
        emblem(T_ACCENT);
    }
    d.setTextSize(1);
    ui_status_invalidate();
    Serial.println("[SPLASH_EXIT]");
    Serial.printf("[UI_BOOT] reveal=%lums\n", (unsigned long)(millis() - boot_started));
}
