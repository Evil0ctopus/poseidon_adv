/*
 * net_ssdp — UPnP / SSDP discovery scanner.
 *
 * Sends M-SEARCH to the SSDP multicast group (239.255.255.250:1900)
 * and collects responses from UPnP devices on the LAN. Pulls the
 * LOCATION URL from each, fetches the XML description, extracts
 * friendlyName + modelName + serialNumber. Logs to SD.
 *
 * Great for mapping internal IoT — routers, printers, IP cams, NAS,
 * smart TVs, Sonos, Chromecasts, and most smart plugs expose SSDP.
 */
#include "app.h"
#include "../theme.h"
#include "ui.h"
#include "input.h"
#include "radio.h"
#include "../sfx.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <SD.h>
#include "../sd_helper.h"

#define MAX_DEV 16

struct ssdp_dev_t {
    char ip[16];
    char location[96];
    char friendly[40];
    char model[32];
    char manufacturer[32];
    char model_number[24];
    char device_type[32];
    char pres_url[64];
};

static ssdp_dev_t s_dev[MAX_DEV];
static int s_dev_n = 0;

static const char *MSEARCH =
    "M-SEARCH * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "MAN: \"ssdp:discover\"\r\n"
    "MX: 2\r\n"
    "ST: ssdp:all\r\n"
    "USER-AGENT: POSEIDON/1.0\r\n"
    "\r\n";

static void extract_tag(const String &xml, const char *tag, char *out, int out_sz)
{
    char open[32], close[32];
    snprintf(open, sizeof(open), "<%s>", tag);
    snprintf(close, sizeof(close), "</%s>", tag);
    int a = xml.indexOf(open);
    if (a < 0) { out[0] = '\0'; return; }
    a += strlen(open);
    int b = xml.indexOf(close, a);
    if (b < 0) { out[0] = '\0'; return; }
    String s = xml.substring(a, b);
    strncpy(out, s.c_str(), out_sz - 1);
    out[out_sz - 1] = '\0';
}

static void fetch_desc(ssdp_dev_t *d)
{
    HTTPClient http;
    http.setConnectTimeout(1500);
    http.setTimeout(2500);
    if (!http.begin(d->location)) return;
    int code = http.GET();
    if (code == 200) {
        String body = http.getString();
        extract_tag(body, "friendlyName",    d->friendly,     sizeof(d->friendly));
        extract_tag(body, "modelName",       d->model,        sizeof(d->model));
        extract_tag(body, "manufacturer",    d->manufacturer, sizeof(d->manufacturer));
        extract_tag(body, "modelNumber",     d->model_number, sizeof(d->model_number));
        extract_tag(body, "deviceType",      d->device_type,  sizeof(d->device_type));
        extract_tag(body, "presentationURL", d->pres_url,     sizeof(d->pres_url));
    }
    http.end();
}

static void show_device_detail(const ssdp_dev_t &e)
{
    auto &d = M5Cardputer.Display;
    ui_clear_body();
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("UPnP DEVICE DETAILS");
    d.drawFastHLine(4, BODY_Y + 12, SCR_W - 8, T_ACCENT);

    d.setTextColor(T_GOOD, T_BG);
    d.setCursor(4, BODY_Y + 16);
    d.printf("NAME : %.28s", e.friendly[0] ? e.friendly : "(unnamed)");

    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 28);
    d.printf("IP   : %s", e.ip);

    d.setTextColor(T_ACCENT2, T_BG);
    d.setCursor(4, BODY_Y + 40);
    d.printf("MFR  : %.28s", e.manufacturer[0] ? e.manufacturer : "Unknown");

    d.setTextColor(T_FG, T_BG);
    d.setCursor(4, BODY_Y + 52);
    d.printf("MODEL: %.20s %s", e.model[0] ? e.model : "?", e.model_number[0] ? e.model_number : "");

    d.setTextColor(T_DIM, T_BG);
    d.setCursor(4, BODY_Y + 64);
    d.printf("TYPE : %.28s", e.device_type[0] ? e.device_type : "urn:schemas-upnp-org");

    d.setTextColor(T_WARN, T_BG);
    d.setCursor(4, BODY_Y + 76);
    if (e.pres_url[0]) d.printf("WEB  : %.28s", e.pres_url);
    else               d.printf("LOC  : %.28s", e.location);

    ui_draw_footer("`=back to list");
    sfx_select();

    while (true) {
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(20); continue; }
        if (k == PK_ESC) break;
    }
}

void feat_net_ssdp(void)
{
    radio_switch(RADIO_WIFI);
    if (WiFi.status() != WL_CONNECTED) {
        ui_toast("connect to WiFi first", T_WARN, 1500);
        return;
    }
    s_dev_n = 0;

    WiFiUDP udp;
    udp.begin(1900);

    ui_clear_body();
    auto &d = M5Cardputer.Display;
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2); d.print("SSDP / UPnP");
    d.drawFastHLine(4, BODY_Y + 12, 90, T_ACCENT);
    d.setTextColor(T_DIM, T_BG);
    d.setCursor(4, BODY_Y + 22); d.print("sending M-SEARCH...");
    ui_draw_footer("`=stop");

    /* Three M-SEARCH bursts for good coverage. */
    IPAddress mcast(239, 255, 255, 250);
    for (int burst = 0; burst < 3; ++burst) {
        udp.beginPacket(mcast, 1900);
        udp.write((const uint8_t *)MSEARCH, strlen(MSEARCH));
        udp.endPacket();
        delay(200);
    }

    uint32_t deadline = millis() + 5000;
    while (millis() < deadline) {
        int len = udp.parsePacket();
        if (len > 0) {
            char buf[600];
            int n = udp.read((uint8_t *)buf, sizeof(buf) - 1);
            if (n <= 0) continue;
            buf[n] = '\0';
            /* Parse LOCATION header. */
            char *loc = strcasestr(buf, "LOCATION:");
            if (!loc) continue;
            loc += 9;
            while (*loc == ' ' || *loc == '\t') ++loc;
            char *eol = strstr(loc, "\r\n");
            if (!eol) continue;
            *eol = '\0';

            /* Dedup by location. */
            bool dup = false;
            for (int i = 0; i < s_dev_n; ++i)
                if (strcmp(s_dev[i].location, loc) == 0) { dup = true; break; }
            if (dup) continue;
            if (s_dev_n >= MAX_DEV) break;

            ssdp_dev_t &e = s_dev[s_dev_n];
            IPAddress rip = udp.remoteIP();
            snprintf(e.ip, sizeof(e.ip), "%u.%u.%u.%u", rip[0], rip[1], rip[2], rip[3]);
            strncpy(e.location, loc, sizeof(e.location) - 1);
            e.location[sizeof(e.location) - 1] = '\0';
            e.friendly[0] = '\0';
            e.model[0] = '\0';
            s_dev_n++;

            d.fillRect(0, BODY_Y + 22, SCR_W, 12, T_BG);
            d.setTextColor(T_FG, T_BG);
            d.setCursor(4, BODY_Y + 22);
            d.printf("found %d  %s", s_dev_n, e.ip);
        }
        /* Animate the whole listen window so a silent LAN (zero responses)
         * never leaves the screen frozen on "sending M-SEARCH...". */
        ui_scanning_indicator("listening", s_dev_n);
        if (input_poll() == PK_ESC) break;
    }
    udp.stop();

    /* Fetch each description. */
    for (int i = 0; i < s_dev_n; ++i) {
        d.fillRect(0, BODY_Y + 22, SCR_W, 12, T_BG);
        d.setTextColor(T_ACCENT, T_BG);
        d.setCursor(4, BODY_Y + 22);
        d.printf("probing %d/%d", i + 1, s_dev_n);
        ui_scanning_indicator("probing", i + 1);
        fetch_desc(&s_dev[i]);
    }

    /* Save log. */
    if (sd_mount()) {
        sd_ensure_layout();
        File f = SD.open(SD_WIFI_CAPTURE_DIR "/ssdp.csv", FILE_APPEND);
        if (f) {
            f.println("ip,friendly,model,location");
            for (int i = 0; i < s_dev_n; ++i)
                f.printf("%s,%s,%s,%s\n", s_dev[i].ip,
                         s_dev[i].friendly, s_dev[i].model, s_dev[i].location);
            f.close();
        }
    }

    /* Scroll list. */
    int cursor = 0;
    int last_cursor = -1;
    ui_draw_footer(";/.=move ENTER=info S=save `=back");
    while (true) {
      if (cursor != last_cursor) {
        last_cursor = cursor;
        ui_clear_body();
        d.setTextColor(T_ACCENT, T_BG);
        d.setCursor(4, BODY_Y + 2);
        d.printf("UPnP / SSDP  %d devices", s_dev_n);
        d.drawFastHLine(4, BODY_Y + 12, SCR_W - 8, T_ACCENT);
        if (s_dev_n == 0) {
            d.setTextColor(T_DIM, T_BG);
            d.setCursor(4, BODY_Y + 24);
            d.print("no UPnP devices found on LAN");
        } else {
            int rows = 7;
            if (cursor < 0) cursor = 0;
            if (cursor >= s_dev_n) cursor = s_dev_n - 1;
            int first = cursor - rows / 2;
            if (first < 0) first = 0;
            if (first + rows > s_dev_n) first = max(0, s_dev_n - rows);
            for (int r = 0; r < rows && first + r < s_dev_n; ++r) {
                int y = BODY_Y + 18 + r * 11;
                bool sel = (first + r == cursor);
                if (sel) d.fillRect(0, y - 1, SCR_W, 11, 0x18C7);
                const ssdp_dev_t &e = s_dev[first + r];
                d.setTextColor(sel ? T_ACCENT : T_FG, sel ? 0x18C7 : T_BG);
                d.setCursor(4, y);
                d.printf("%-15s", e.ip);
                d.setTextColor(sel ? 0xFFFF : (e.manufacturer[0] ? T_GOOD : T_ACCENT), sel ? 0x18C7 : T_BG);
                d.setCursor(102, y);
                d.printf("%.22s", e.friendly[0] ? e.friendly : (e.model[0] ? e.model : "?"));
            }
        }
      }
        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(40); continue; }
        if (k == PK_ESC) return;
        if (k == ';' || k == PK_UP)   { if (cursor > 0) cursor--; }
        if (k == '.' || k == PK_DOWN) { if (cursor + 1 < s_dev_n) cursor++; }
        if (k == PK_ENTER && s_dev_n > 0 && cursor < s_dev_n) {
            show_device_detail(s_dev[cursor]);
            last_cursor = -1;  /* force redraw list */
            ui_draw_footer(";/.=move ENTER=info S=save `=back");
        }
        if ((k == 's' || k == 'S') && s_dev_n > 0) {
            if (sd_mount()) {
                sd_ensure_layout();
                char path[64];
                snprintf(path, sizeof(path), SD_WIFI_CAPTURE_DIR "/ssdp-%lu.csv", (unsigned long)(millis() / 1000));
                SD.mkdir(SD_WIFI_CAPTURE_DIR);
                File f = SD.open(path, FILE_WRITE);
                if (f) {
                    f.println("ip,friendly,manufacturer,model,model_number,device_type,location,presentation_url");
                    for (int i = 0; i < s_dev_n; ++i) {
                        f.printf("%s,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
                                 s_dev[i].ip, s_dev[i].friendly, s_dev[i].manufacturer,
                                 s_dev[i].model, s_dev[i].model_number, s_dev[i].device_type,
                                 s_dev[i].location, s_dev[i].pres_url);
                    }
                    f.close();
                    sfx_capture();
                    ui_toast("Saved UPnP CSV", T_GOOD, 800);
                } else {
                    ui_toast("File write failed", T_BAD, 800);
                }
            } else {
                ui_toast("No SD card", T_WARN, 800);
            }
            last_cursor = -1;
            ui_draw_footer(";/.=move ENTER=info S=save `=back");
        }
    }
}
