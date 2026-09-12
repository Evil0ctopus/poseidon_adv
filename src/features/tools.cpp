/*
 * tools — handy utilities in the Flipper tradition.
 *
 *   SD format   : wipe + re-FAT the microSD card
 *   Flashlight  : full-screen white panic torch
 *   Screen test : RGB bars + color grid
 *   Stopwatch   : count up with start/stop/lap
 *   Dice        : 4d6 roller, coin flip, magic 8-ball
 *   Morse       : type text -> blink + beep in morse, or listen through
 *                 the mic and live-decode incoming morse to text
 *   MAC rand    : randomize the WiFi MAC (survives one session)
 *   Calc        : tiny RPN calculator
 */
#include "app.h"
#include "../theme.h"
#include "ui.h"
#include "input.h"
#include "radio.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <SD.h>
#include "../sd_helper.h"
#include <FS.h>
#include <esp_random.h>
#include <functional>

/* ================= SD format ================= */

void feat_tool_sd_format(void)
{
    ui_clear_body();
    auto &d = M5Cardputer.Display;
    d.setTextColor(T_BAD, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("SD FORMAT");
    d.drawFastHLine(4, BODY_Y + 12, 80, T_BAD);
    d.setTextColor(T_WARN, T_BG);
    d.setCursor(4, BODY_Y + 22); d.print("THIS WIPES EVERYTHING");
    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 36); d.print("Q = quick wipe (delete all)");
    d.setCursor(4, BODY_Y + 48); d.print("F = TRUE FAT32 reformat");
    d.setTextColor(T_DIM, T_BG);
    d.setCursor(4, BODY_Y + 62); d.print("F recovers exFAT/corrupt cards");
    ui_draw_footer("Q=quick  F=fat32  `=cancel");

    bool force = false;
    while (true) {
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(20); continue; }
        if (k == PK_ESC) return;
        if (k == 'q' || k == 'Q') { force = false; break; }
        if (k == 'f' || k == 'F') { force = true;  break; }
    }

    /* Type-YES gate on either path — destructive + irreversible. */
    char buf[8];
    if (!input_line(force ? "FAT32 reformat - type YES:" : "wipe - type YES:",
                    buf, sizeof(buf))) return;
    if (strcmp(buf, "YES") != 0) { ui_toast("cancelled", T_DIM, 500); return; }

    ui_toast(force ? "reformatting FAT32..." : "wiping...", T_WARN, 0);
    bool ok = force ? sd_force_format() : sd_format();
    if (!ok) {
        ui_toast(force ? "FAT32 format failed" : "SD wipe failed", T_BAD, 1500);
        return;
    }
    ui_toast(force ? "FAT32 done" : "wiped", T_GOOD, 900);
}

/* ================= Flashlight ================= */

void feat_tool_flashlight(void)
{
    auto &d = M5Cardputer.Display;
    d.fillScreen(0xFFFF);
    d.setTextColor(0x0000, 0xFFFF);
    d.setCursor(60, SCR_H - 10); d.print("any key to exit");
    while (true) {
        uint16_t k = input_poll();
        if (k != PK_NONE) break;
        delay(50);
    }
}

/* ================= Screen test ================= */

void feat_tool_screen_test(void)
{
    auto &d = M5Cardputer.Display;
    const uint16_t cols[] = { 0xF800, 0x07E0, 0x001F, 0xFFFF, 0x0000, T_ACCENT, T_WARN };
    int idx = 0;
    while (true) {
        d.fillScreen(cols[idx]);
        d.setTextColor(cols[idx] == 0xFFFF ? 0x0000 : 0xFFFF, cols[idx]);
        d.setCursor(4, 4);
        d.printf("color %d/%d  any=next `=back", idx + 1, (int)(sizeof(cols) / sizeof(cols[0])));
        /* Gradient band. */
        for (int x = 0; x < SCR_W; ++x) {
            uint8_t t = (uint8_t)(x * 31 / SCR_W);
            d.drawFastVLine(x, SCR_H - 20, 20, (t << 11));
        }
        uint16_t k = PK_NONE;
        while (k == PK_NONE) { k = input_poll(); delay(30); }
        if (k == PK_ESC) return;
        idx = (idx + 1) % (int)(sizeof(cols) / sizeof(cols[0]));
    }
}

/* ================= Stopwatch ================= */

void feat_tool_stopwatch(void)
{
    ui_clear_body();
    ui_draw_footer("SPACE=start/stop L=lap R=reset `=back");
    auto &d = M5Cardputer.Display;

    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("STOPWATCH");
    d.drawFastHLine(4, BODY_Y + 12, 80, T_ACCENT);

    uint32_t start_ms = 0;
    uint32_t elapsed  = 0;
    bool running = false;
    char laps[4][16] = {{0}};
    int  lap_n = 0;
    char shown_buf[16] = "";
    bool shown_running = false;
    int  shown_laps = -1;

    while (true) {
        uint32_t now = millis();
        uint32_t shown = running ? (elapsed + (now - start_ms)) : elapsed;

        char buf[16];
        uint32_t s  = shown / 1000;
        uint32_t ms = (shown % 1000) / 10;
        snprintf(buf, sizeof(buf), "%02lu:%02lu.%02lu",
                 (unsigned long)(s / 60), (unsigned long)(s % 60),
                 (unsigned long)ms);
        if (strcmp(buf, shown_buf) != 0 || running != shown_running) {
            strcpy(shown_buf, buf);
            shown_running = running;
            d.setTextColor(running ? T_GOOD : T_FG, T_BG);
            d.setTextSize(3);
            int w = d.textWidth(buf);
            d.setCursor((SCR_W - w) / 2, BODY_Y + 24);
            d.print(buf);
            d.setTextSize(1);
        }

        if (lap_n != shown_laps) {
            shown_laps = lap_n;
            d.fillRect(0, BODY_Y + 60, SCR_W, 40, T_BG);
            d.setTextColor(T_DIM, T_BG);
            for (int i = 0; i < lap_n; ++i) {
                d.setCursor(4, BODY_Y + 60 + i * 10);
                d.printf("L%d  %s", i + 1, laps[i]);
            }
        }

        uint16_t k = input_poll();
        if (k == PK_ESC) return;
        if (k == PK_SPACE || k == PK_ENTER) {
            if (running) { elapsed += now - start_ms; running = false; }
            else         { start_ms = now;            running = true;  }
        } else if (k == 'l' || k == 'L') {
            if (lap_n < 4) { strncpy(laps[lap_n++], buf, 15); }
        } else if (k == 'r' || k == 'R') {
            elapsed = 0; running = false; lap_n = 0;
        }
        delay(30);
    }
}

/* ================= Dice / 8-ball / coin ================= */

static const char *s_8ball[] = {
    "yes", "absolutely", "no", "doubtful", "ask again later",
    "signs point to yes", "very doubtful", "without a doubt",
    "my reply is no", "as i see it yes", "cannot predict",
    "outlook good", "outlook not so good"
};

void feat_tool_chance(void)
{
    auto &d = M5Cardputer.Display;
    int mode = 0;  /* 0=dice 1=coin 2=8ball */
    char last[48] = "roll...";

    auto draw_result = [&]() {
        d.fillRect(0, BODY_Y + 30, SCR_W, 16, T_BG);
        d.setTextColor(T_WARN, T_BG);
        d.setTextSize(2);
        int w = d.textWidth(last) * 2;
        d.setCursor((SCR_W - w) / 2, BODY_Y + 30);
        d.print(last);
        d.setTextSize(1);
    };
    auto draw_full = [&]() {
        ui_clear_body();
        d.setTextColor(T_ACCENT, T_BG);
        d.setCursor(4, BODY_Y + 2);
        d.print(mode == 0 ? "DICE (2d6)" : mode == 1 ? "COIN" : "8-BALL");
        d.drawFastHLine(4, BODY_Y + 12, 80, T_ACCENT);

        draw_result();

        d.setTextColor(T_DIM, T_BG);
        d.setCursor(4, BODY_Y + 70); d.print("SPACE=roll  M=mode");
        ui_draw_footer("SPACE=roll M=mode `=back");
    };
    draw_full();

    while (true) {
        uint16_t k = PK_NONE;
        while (k == PK_NONE) { k = input_poll(); delay(20); }
        if (k == PK_ESC) return;
        if (k == 'm' || k == 'M') { mode = (mode + 1) % 3; draw_full(); continue; }
        if (k == PK_SPACE || k == PK_ENTER) {
            if (mode == 0) {
                int a = 1 + (esp_random() % 6);
                int b = 1 + (esp_random() % 6);
                snprintf(last, sizeof(last), "%d + %d = %d", a, b, a + b);
            } else if (mode == 1) {
                snprintf(last, sizeof(last), "%s", (esp_random() & 1) ? "HEADS" : "TAILS");
            } else {
                int n = sizeof(s_8ball) / sizeof(s_8ball[0]);
                snprintf(last, sizeof(last), "%s", s_8ball[esp_random() % n]);
            }
            M5Cardputer.Speaker.tone(1500, 60);
            draw_result();
        }
    }
}

/* ================= Morse code sender + listener ================= */

static const char *s_morse[] = {
    /* A-Z */
    ".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..",
    "--","-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--..",
    /* 0-9 */
    "-----",".----","..---","...--","....-",".....","-....","--...","---..","----."
};

static void morse_send(const char *s)
{
    const int unit = 100;  /* ms per dot */
    auto &d = M5Cardputer.Display;
    for (const char *p = s; *p; ++p) {
        char c = toupper(*p);
        const char *m = nullptr;
        if (c >= 'A' && c <= 'Z') m = s_morse[c - 'A'];
        else if (c >= '0' && c <= '9') m = s_morse[26 + c - '0'];
        if (!m) { delay(unit * 7); continue; }
        for (const char *x = m; *x; ++x) {
            d.fillScreen(T_ACCENT);
            M5Cardputer.Speaker.tone(800, (*x == '-') ? unit * 3 : unit);
            delay((*x == '-') ? unit * 3 : unit);
            d.fillScreen(T_BG);
            delay(unit);
        }
        delay(unit * 3);
    }
}

/* Reverse lookup: dot/dash symbol string -> character. Returns 0 if the
 * symbol doesn't match any known letter/digit. */
static char morse_decode_symbol(const char *sym)
{
    if (!sym[0]) return 0;
    for (int i = 0; i < 26; ++i) if (!strcmp(sym, s_morse[i])) return (char)('A' + i);
    for (int i = 0; i < 10; ++i) if (!strcmp(sym, s_morse[26 + i])) return (char)('0' + i);
    return '?';
}

/* Listen through the mic, decode incoming on/off tone timing as Morse,
 * and show the live symbol buffer + decoded text. Unit length (dot ms)
 * is adaptive: it tracks the shortest recent "on" pulse so it keeps up
 * with whatever speed the source is sending at, after an initial fixed
 * guess. Dash/gap thresholds are derived from that unit (standard
 * Morse timing: dash = 3 units, intra-char gap = 1, letter gap = 3,
 * word gap = 7). */
void feat_tool_morse_listen(void)
{
    m5::mic_config_t mcfg = M5Cardputer.Mic.config();
    mcfg.sample_rate = 8000;
    mcfg.over_sampling = 2;
    mcfg.magnification = 24;
    M5Cardputer.Mic.config(mcfg);
    if (!M5Cardputer.Mic.begin()) {
        ui_toast("mic init failed", T_BAD, 1500);
        return;
    }

    ui_clear_body();
    auto &d = M5Cardputer.Display;
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("MORSE LISTEN");
    d.drawFastHLine(4, BODY_Y + 12, 80, T_ACCENT);
    ui_draw_footer("listening...  ESC=stop");

    /* 20ms chunks at 8kHz = 160 samples. */
    const int    CHUNK_N = 160;
    static int16_t buf[CHUNK_N];

    /* Brief noise-floor calibration: average RMS over ~300ms of silence. */
    long noise_sum = 0;
    int  noise_n = 0;
    for (int i = 0; i < 15; ++i) {
        if (!M5Cardputer.Mic.record(buf, CHUNK_N, mcfg.sample_rate)) { delay(20); continue; }
        long sq = 0;
        for (int j = 0; j < CHUNK_N; ++j) sq += (long)buf[j] * buf[j];
        noise_sum += (long)sqrt((double)sq / CHUNK_N);
        noise_n++;
    }
    long noise_floor = noise_n ? (noise_sum / noise_n) : 40;
    long on_thresh   = noise_floor * 4 + 200;

    uint32_t unit_ms   = 100;   /* adaptive dot length, seeded with a guess */
    bool     tone_on   = false;
    uint32_t edge_ms    = millis();
    bool     gap_handled = true;   /* avoids double-counting the same silence gap */

    char sym[8]  = {0}; int sym_n = 0;
    char text[48] = {0}; int text_n = 0;

    char shown_sym[8] = "";
    char shown_text[48] = "";

    while (true) {
        uint16_t k = input_poll();
        if (k == PK_ESC) break;
        if (k == 'c' || k == 'C') { text_n = 0; text[0] = 0; sym_n = 0; sym[0] = 0; }

        if (!M5Cardputer.Mic.record(buf, CHUNK_N, mcfg.sample_rate)) { delay(10); continue; }
        long sq = 0;
        for (int j = 0; j < CHUNK_N; ++j) sq += (long)buf[j] * buf[j];
        long rms = (long)sqrt((double)sq / CHUNK_N);
        bool now_on = rms > on_thresh;
        uint32_t now = millis();

        if (now_on && !tone_on) {
            /* rising edge: the silence before it just ended */
            uint32_t gap = now - edge_ms;
            if (!gap_handled && sym_n > 0) {
                if (gap > unit_ms * 5) {                 /* word gap */
                    char c = morse_decode_symbol(sym);
                    if (c && text_n < (int)sizeof(text) - 2) text[text_n++] = c;
                    if (text_n < (int)sizeof(text) - 2) text[text_n++] = ' ';
                    text[text_n] = 0;
                    sym_n = 0; sym[0] = 0;
                } else if (gap > unit_ms * 2) {           /* letter gap */
                    char c = morse_decode_symbol(sym);
                    if (c && text_n < (int)sizeof(text) - 1) text[text_n++] = c;
                    text[text_n] = 0;
                    sym_n = 0; sym[0] = 0;
                }
            }
            tone_on = true;
            edge_ms = now;
            gap_handled = true;
        } else if (!now_on && tone_on) {
            /* falling edge: the tone before it just ended -> classify dot/dash */
            uint32_t dur = now - edge_ms;
            if (sym_n < (int)sizeof(sym) - 1) {
                sym[sym_n++] = (dur > unit_ms * 2) ? '-' : '.';
                sym[sym_n] = 0;
            }
            /* Track the shortest recent pulse as the running unit length so
             * decode speed follows the sender instead of a fixed guess. */
            if (dur < unit_ms * 2 && dur > 20) {
                unit_ms = (unit_ms * 3 + dur) / 4;
                if (unit_ms < 40) unit_ms = 40;
            }
            tone_on = false;
            edge_ms = now;
            gap_handled = false;
        }

        if (strcmp(sym, shown_sym) != 0 || strcmp(text, shown_text) != 0) {
            strncpy(shown_sym, sym, sizeof(shown_sym) - 1);
            strncpy(shown_text, text, sizeof(shown_text) - 1);
            d.fillRect(0, BODY_Y + 20, SCR_W, 60, T_BG);
            d.setTextColor(now_on ? T_GOOD : T_DIM, T_BG);
            d.setCursor(4, BODY_Y + 22); d.printf("[%s]", now_on ? "TONE" : "----");
            d.setTextColor(T_WARN, T_BG);
            d.setTextSize(2);
            d.setCursor(4, BODY_Y + 34); d.print(sym[0] ? sym : "_");
            d.setTextSize(1);
            d.setTextColor(T_FG, T_BG);
            d.setCursor(4, BODY_Y + 58);
            /* last ~30 chars so the newest decoded text stays on screen */
            const char *tail = text_n > 30 ? text + (text_n - 30) : text;
            d.print(tail);
        }
        delay(5);
    }
    M5Cardputer.Mic.end();
    ui_toast("stopped", T_DIM, 400);
}

void feat_tool_morse_send(void)
{
    char msg[64];
    if (!input_line("text:", msg, sizeof(msg))) return;
    if (!msg[0]) return;
    morse_send(msg);
    ui_toast("sent", T_GOOD, 500);
}

void feat_tool_morse(void)
{
    ui_clear_body();
    auto &d = M5Cardputer.Display;
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("MORSE");
    d.drawFastHLine(4, BODY_Y + 12, 80, T_ACCENT);
    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 26); d.print("S = send text as morse");
    d.setCursor(4, BODY_Y + 40); d.print("L = listen + decode morse");
    ui_draw_footer("S=send  L=listen  `=back");

    while (true) {
        uint16_t k = PK_NONE;
        while (k == PK_NONE) { k = input_poll(); delay(20); }
        if (k == PK_ESC) return;
        if (k == 's' || k == 'S') { feat_tool_morse_send(); return; }
        if (k == 'l' || k == 'L') { feat_tool_morse_listen(); return; }
    }
}

/* ================= MAC randomizer ================= */

void feat_tool_mac_rand(void)
{
    uint8_t mac[6];
    for (int i = 0; i < 6; ++i) mac[i] = (uint8_t)esp_random();
    mac[0] &= 0xFE;  /* unicast */
    mac[0] |= 0x02;  /* locally administered */

    wifi_lean_sta_init();   /* raw-safe STA init; Arduino WiFi.mode() asserts on a raw-inited driver. */
    esp_wifi_set_mac(WIFI_IF_STA, mac);

    ui_clear_body();
    auto &d = M5Cardputer.Display;
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("MAC RANDOMIZED");
    d.drawFastHLine(4, BODY_Y + 12, 130, T_ACCENT);
    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 26); d.printf("%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    d.setTextColor(T_DIM, T_BG);
    d.setCursor(4, BODY_Y + 46); d.print("resets on reboot");
    ui_draw_footer("`=back");
    while (true) {
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(40); continue; }
        if (k == PK_ESC) break;
    }
}

/* ================= Tiny calculator ================= */

void feat_tool_calc(void)
{
    /* Supports +, -, *, / on a single expression. */
    char expr[32];
    if (!input_line("expr (e.g. 12*7+3):", expr, sizeof(expr))) return;

    /* Simplest evaluator: left-to-right, no precedence. */
    double acc = 0;
    char op = '+';
    const char *p = expr;
    while (*p) {
        while (*p == ' ') ++p;
        if (!*p) break;
        double num = 0;
        bool have = false;
        while (*p >= '0' && *p <= '9') { num = num * 10 + (*p - '0'); ++p; have = true; }
        if (*p == '.') { ++p; double s = 0.1; while (*p >= '0' && *p <= '9') { num += (*p - '0') * s; s *= 0.1; ++p; have = true; } }
        if (!have) break;
        switch (op) {
        case '+': acc += num; break;
        case '-': acc -= num; break;
        case '*': acc *= num; break;
        case '/': if (num != 0) acc /= num; break;
        }
        while (*p == ' ') ++p;
        if (!*p) break;
        op = *p++;
    }

    ui_clear_body();
    auto &d = M5Cardputer.Display;
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("CALC");
    d.drawFastHLine(4, BODY_Y + 12, 50, T_ACCENT);
    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 22); d.printf("%s", expr);
    d.setTextColor(T_GOOD, T_BG);
    d.setTextSize(2);
    char out[24];
    if (acc == (long)acc) snprintf(out, sizeof(out), "= %ld", (long)acc);
    else                  snprintf(out, sizeof(out), "= %.4f", acc);
    d.setCursor(4, BODY_Y + 40); d.print(out);
    d.setTextSize(1);
    ui_draw_footer("`=back");
    while (true) {
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(40); continue; }
        if (k == PK_ESC) break;
    }
}
