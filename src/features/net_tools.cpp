/*
 * net_tools — basic network utilities run from STA mode.
 *
 * Port scan: iterate a target host's TCP ports, log which connect.
 * Ping: ICMP echo to a host, round-trip time.
 * DNS: lookup a hostname's A records.
 *
 * All require you to be associated to a WiFi AP first — via a saved
 * network (feat_wifi_connect) or by adding one in settings.
 */
#include "app.h"
#include "../theme.h"
#include "ui.h"
#include "input.h"
#include "radio.h"
#include "../sfx.h"
#include <WiFi.h>
#include <ESP32Ping.h>

static void draw_waiting(const char *title, const char *sub)
{
    auto &d = M5Cardputer.Display;
    ui_clear_body();
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print(title);
    d.drawFastHLine(4, BODY_Y + 12, 100, T_ACCENT);
    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 24); d.print(sub);
}

static bool require_sta(void)
{
    if (WiFi.status() != WL_CONNECTED) {
        ui_toast("connect to WiFi first", T_WARN, 1500);
        return false;
    }
    return true;
}

static void grab_banner(WiFiClient &c, int port, char *out, size_t out_sz)
{
    out[0] = 0;
    if (port == 80 || port == 8080 || port == 8000) {
        c.print("HEAD / HTTP/1.0\r\n\r\n");
    }
    uint32_t dl = millis() + 400;
    int idx = 0;
    while (millis() < dl && c.connected() && idx + 1 < (int)out_sz) {
        while (c.available() && idx + 1 < (int)out_sz) {
            char ch = (char)c.read();
            if (ch == '\r' || ch == '\n') {
                if (idx > 0) { out[idx] = 0; return; }
            } else if (ch >= 32 && ch < 127) {
                out[idx++] = ch;
            }
        }
        delay(10);
    }
    out[idx] = 0;
}

void feat_net_portscan(void)
{
    radio_switch(RADIO_WIFI);
    if (!require_sta()) return;
    char host[64], range_buf[16];
    if (!input_line("host (ip or name):", host, sizeof(host))) return;
    if (!input_line("port range (22-443):", range_buf, sizeof(range_buf))) return;
    int lo = 1, hi = 1024;
    sscanf(range_buf, "%d-%d", &lo, &hi);
    if (lo < 1) lo = 1;
    if (hi > 65535) hi = 65535;
    if (hi < lo) hi = lo;

    IPAddress ip;
    if (!WiFi.hostByName(host, ip)) {
        ui_toast("dns fail", T_BAD, 1200);
        return;
    }

    draw_waiting("PORT SCAN", host);
    ui_draw_footer("`=stop");
    int y = BODY_Y + 36;

    int open_count = 0;
    uint32_t last_draw = 0;
    for (int p = lo; p <= hi; ++p) {
        if (millis() - last_draw >= 150) {
            ui_text_w(4, BODY_Y + 22, SCR_W - 8, T_DIM, "port %d...", p);
            last_draw = millis();
        }
        WiFiClient c;
        c.setTimeout(300);
        if (c.connect(ip, p)) {
            char banner[36] = {0};
            grab_banner(c, p, banner, sizeof(banner));
            if (banner[0]) {
                ui_text(4, y, T_GOOD, "OPEN %-5d %.24s", p, banner);
            } else {
                ui_text(4, y, T_GOOD, "OPEN %d", p);
            }
            y += 10;
            if (y > FOOTER_Y - 12) y = BODY_Y + 36;
            open_count++;
            sfx_click();
            c.stop();
        }
        uint16_t k = input_poll();
        if (k == PK_ESC) break;
    }
    ui_text_w(4, BODY_Y + 22, SCR_W - 8, T_ACCENT, "done. %d open.", open_count);
    sfx_scan_hit();
    while (true) {
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(20); continue; }
        if (k == PK_ESC) break;
    }
}

void feat_net_pingsweep(void)
{
    radio_switch(RADIO_WIFI);
    if (!require_sta()) return;

    IPAddress myIP = WiFi.localIP();
    IPAddress baseIP = myIP;

    auto &d = M5Cardputer.Display;
    ui_clear_body();
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("SUBNET PING SWEEP (/24)");
    d.drawFastHLine(4, BODY_Y + 12, SCR_W - 8, T_ACCENT);
    ui_draw_footer("`=stop");

    int alive_count = 0;
    int grid_w = 16, grid_h = 16;
    int cell_w = 12, cell_h = 5;
    int start_x = (SCR_W - grid_w * cell_w) / 2;
    int start_y = BODY_Y + 16;

    /* Draw grid background */
    for (int gy = 0; gy < grid_h; gy++) {
        for (int gx = 0; gx < grid_w; gx++) {
            d.drawRect(start_x + gx * cell_w, start_y + gy * cell_h, cell_w - 1, cell_h - 1, 0x18C3);
        }
    }

    for (int host = 1; host <= 254; host++) {
        IPAddress target(baseIP[0], baseIP[1], baseIP[2], host);
        int gx = (host - 1) % grid_w;
        int gy = (host - 1) / grid_w;

        bool is_me = (target == myIP);
        bool alive = false;
        if (is_me) {
            alive = true;
        } else {
            alive = Ping.ping(target, 1);
        }

        uint16_t col = is_me ? T_ACCENT2 : (alive ? T_GOOD : 0x0820);
        d.fillRect(start_x + gx * cell_w, start_y + gy * cell_h, cell_w - 1, cell_h - 1, col);

        if (alive) {
            alive_count++;
            sfx_click();
        }

        d.setTextColor(T_DIM, T_BG);
        d.fillRect(4, FOOTER_Y - 14, SCR_W - 8, 8, T_BG);
        d.setCursor(4, FOOTER_Y - 14);
        d.printf(".%-3d | Alive: %d", host, alive_count);

        uint16_t k = input_poll();
        if (k == PK_ESC) break;
    }

    d.setTextColor(T_ACCENT, T_BG);
    d.fillRect(4, FOOTER_Y - 14, SCR_W - 8, 8, T_BG);
    d.setCursor(4, FOOTER_Y - 14);
    d.printf("Done: %d active on /24", alive_count);
    sfx_scan_hit();

    while (true) {
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(20); continue; }
        if (k == PK_ESC) break;
    }
}

void feat_net_ping(void)
{
    radio_switch(RADIO_WIFI);
    if (!require_sta()) return;
    char host[64];
    if (!input_line("host:", host, sizeof(host))) return;

    draw_waiting("PING", host);
    ui_draw_footer("`=stop");
    auto &d = M5Cardputer.Display;
    int y = BODY_Y + 36;
    int seq = 0;
    while (true) {
        bool ok = Ping.ping(host, 1);
        int rtt = ok ? (int)Ping.averageTime() : -1;
        if (ok) ui_text_w(4, y, SCR_W - 8, T_GOOD, "seq=%d rtt=%dms", seq, rtt);
        else    ui_text_w(4, y, SCR_W - 8, T_BAD, "seq=%d TIMEOUT", seq);
        y += 10;
        if (y > FOOTER_Y - 12) { d.fillRect(0, BODY_Y + 36, SCR_W, FOOTER_Y - BODY_Y - 36, T_BG); y = BODY_Y + 36; }
        ++seq;

        for (int i = 0; i < 20; ++i) {
            uint16_t k = input_poll();
            if (k == PK_ESC) return;
            delay(50);
        }
    }
}

void feat_net_dns(void)
{
    radio_switch(RADIO_WIFI);
    if (!require_sta()) return;
    char host[64];
    if (!input_line("hostname:", host, sizeof(host))) return;

    ui_clear_body();
    ui_text(4, BODY_Y + 2, T_ACCENT, "DNS LOOKUP");
    ui_scanning_indicator("resolving", -1);
    IPAddress ip;
    bool ok = WiFi.hostByName(host, ip);
    ui_clear_body();
    auto &d = M5Cardputer.Display;
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("DNS LOOKUP");
    d.drawFastHLine(4, BODY_Y + 12, 90, T_ACCENT);
    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 22); d.printf("%s", host);
    d.setTextColor(ok ? T_GOOD : T_BAD, T_BG);
    d.setCursor(4, BODY_Y + 36);
    d.print(ok ? ip.toString().c_str() : "FAILED");
    ui_draw_footer("`=back");
    while (true) {
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(20); continue; }
        if (k == PK_ESC) break;
    }
}
