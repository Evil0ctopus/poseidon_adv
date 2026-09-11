/*
 * net_cctv — CCTV / IP-camera reconnaissance toolkit.
 *
 * Credit: architecture + probe set ported from @7h30th3r0n3's
 * Evil-M5Project (Evil-Cardputer-v1-5-2.ino `scanCCTVCameras()` block)
 * and RaspyJack (`payloads/reconnaissance/cctv_scanner.py`). Their SD
 * layout (`/evil/CCTV/`) is preserved under `/poseidon/captures/surveillance/` so
 * downstream tooling can still consume the output.
 *
 * What it does on each target IP:
 *   1. Fast port probe (80, 443, 554, 8080-8083, 8443, 8554).
 *   2. HTTP banner grab on any open HTTP port → brand fingerprint
 *      (Hikvision, Dahua, Axis, Vivotek, Panasonic, CPPlus, generic).
 *   3. If HTTP auth is required (401), spray a 10-entry default-cred
 *      list (admin/admin, admin/12345, admin/888888, root/root, etc.).
 *      First 2xx wins and gets logged.
 *   4. If RTSP (554/8554) is open, `OPTIONS * RTSP/1.0` presence
 *      probe, then `DESCRIBE` against a shortlist of vendor-common
 *      stream paths (/Streaming/Channels/1, /cam/realmonitor, /live
 *      etc). First 200 = confirmed stream URL.
 *   5. Write hit to `/poseidon/captures/surveillance/cctv-<ts>.csv` and stream a scrolling
 *      hit list on-screen. ESC bails out at any time.
 *
 * Three entry modes mirror Evil-M5:
 *   - LAN sweep: walks the current STA subnet (/24 around our IP).
 *   - Single IP:   user types one IP.
 *   - From file:   reads `/poseidon/cctv-targets.txt`, one IP per line.
 *
 * Discovery begins with a bounded ONVIF WS-Discovery probe on UDP 3702.
 * ProbeMatches seed the candidate list and preserve their device-service
 * XAddr, then the TCP sweep fills in ports, vendor evidence and RTSP paths.
 *
 * Deliberately NOT included:
 *   - Hikvision CVE-2017-7921 / CVE-2021-36260 payloads.
 *   - Dahua CVE-2021-33044/45.
 *   - On-device MJPEG viewer (display-heavy; separate feature).
 *
 * Output columns (CSV): ip,open_ports,brand,creds,endpoint
 */
#include "app.h"
#include "../theme.h"
#include "ui.h"
#include "input.h"
#include "radio.h"
#include <esp_heap_caps.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <esp_netif.h>
#include <SD.h>
#include "../sd_helper.h"
#include "../net_helpers.h"
#include <mbedtls/base64.h>

/* ---- probe tables (PROGMEM-ish; the compiler'll put them in flash) ---- */

/* Primary ports are probed on every address. Secondary ports are checked only
 * after a host answers on a primary port, which cuts dead-host socket churn by
 * more than half. HTTPS-only 443/8443 are intentionally omitted: this scanner
 * has no TLS client, so treating them as plaintext HTTP produced connection
 * resets and false camera hits without yielding a usable fingerprint. */
static const uint16_t CAM_PORTS[] = { 80, 554, 8080, 8554, 8081, 8082, 8083 };
static const int CAM_PORTS_N = sizeof(CAM_PORTS) / sizeof(CAM_PORTS[0]);
static const int CAM_PRIMARY_PORTS_N = 4;

static const char *RTSP_PATHS[] = {
    "/Streaming/Channels/1",
    "/cam/realmonitor?channel=1&subtype=0",
    "/live",
    "/live.sdp",
    "/h264/ch1/main/av_stream",
    "/ch0_0.264",
    "/live/ch00_0",
    "/videoMain",
};
static const int RTSP_PATHS_N = sizeof(RTSP_PATHS) / sizeof(RTSP_PATHS[0]);

struct cred_t { const char *user; const char *pass; };
static const cred_t DEFAULT_CREDS[] = {
    {"admin", "admin"},
    {"admin", "12345"},
    {"admin", "888888"},
    {"admin", "666666"},
    {"admin", "password"},
    {"admin", ""},
    {"root",  "root"},
    {"root",  "admin"},
    {"root",  "pass"},
    {"user",  "user"},
};
static const int CRED_N = sizeof(DEFAULT_CREDS) / sizeof(DEFAULT_CREDS[0]);

struct brand_sig_t { const char *brand; const char *needle; };
static const brand_sig_t BRAND_SIGS[] = {
    {"hikvision", "ISAPI"},
    {"hikvision", "DNVRS-Webs"},
    {"dahua",     "magicBox.cgi"},
    {"dahua",     "DH-"},
    {"axis",      "axis-cgi"},
    {"vivotek",   "VS-"},
    {"panasonic", "panasonic"},
    {"cpplus",    "cpplus"},
};
static const int BRAND_N = sizeof(BRAND_SIGS) / sizeof(BRAND_SIGS[0]);

/* ---- result list ---- */

#define CCTV_MAX_HITS 32
struct cctv_hit_t {
    char ip[16];
    uint16_t ports_mask;   /* bitmask into CAM_PORTS */
    char brand[12];
    char creds[24];        /* "user:pass" or "" */
    char stream[96];       /* "rtsp://host:554/path" or "" */
};
/* Lazy-allocated once on first CCTV scan (rarely used; keeps ~4.7 KB free
 * until then). Readers gate on s_hits_n, which is 0 until this is allocated. */
static cctv_hit_t *s_hits = nullptr;
static int s_hits_n = 0;

static bool cctv_hits_ensure(void)
{
    if (s_hits) return true;
    s_hits = (cctv_hit_t *)heap_caps_calloc(CCTV_MAX_HITS, sizeof(cctv_hit_t),
                                            MALLOC_CAP_INTERNAL);
    if (!s_hits) { ui_toast("Low memory", T_BAD, 1500); return false; }
    return true;
}

static volatile bool s_abort = false;

/* ---- small helpers ---- */

static void format_ip(IPAddress ip, char *out, size_t out_size)
{
    snprintf(out, out_size, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

static void hit_add(const cctv_hit_t &h)
{
    for (int i = 0; i < s_hits_n; ++i) {
        if (strcmp(s_hits[i].ip, h.ip) != 0) continue;
        s_hits[i].ports_mask |= h.ports_mask;
        if ((!s_hits[i].brand[0] || strcmp(s_hits[i].brand, "unknown") == 0)
            && h.brand[0]) {
            strncpy(s_hits[i].brand, h.brand, sizeof(s_hits[i].brand) - 1);
        }
        if (h.creds[0]) strncpy(s_hits[i].creds, h.creds, sizeof(s_hits[i].creds) - 1);
        if (h.stream[0]) strncpy(s_hits[i].stream, h.stream, sizeof(s_hits[i].stream) - 1);
        return;
    }
    if (s_hits_n < CCTV_MAX_HITS) s_hits[s_hits_n++] = h;
}

static const char *onvif_brand(const char *xml)
{
    if (strcasestr(xml, "hikvision")) return "hikvision";
    if (strcasestr(xml, "dahua"))     return "dahua";
    if (strcasestr(xml, "axis"))      return "axis";
    if (strcasestr(xml, "vivotek"))   return "vivotek";
    if (strcasestr(xml, "panasonic")) return "panasonic";
    if (strcasestr(xml, "reolink"))   return "reolink";
    if (strcasestr(xml, "amcrest"))   return "amcrest";
    return "onvif";
}

static void extract_onvif_xaddr(const char *xml, char *out, size_t out_size)
{
    out[0] = '\0';
    const char *tag = strstr(xml, "XAddrs>");
    if (!tag) return;
    const char *value = tag + 7;
    while (*value == ' ' || *value == '\t') ++value;
    size_t length = 0;
    while (value[length] && value[length] != '<'
           && value[length] != ' ' && value[length] != '\r'
           && value[length] != '\n' && length + 1 < out_size) {
        out[length] = value[length];
        length++;
    }
    out[length] = '\0';
}

static void onvif_discover(void)
{
    static const char *probe =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<e:Envelope xmlns:e=\"http://www.w3.org/2003/05/soap-envelope\" "
        "xmlns:w=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
        "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
        "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
        "<e:Header><w:MessageID>uuid:poseidon-cctv</w:MessageID>"
        "<w:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>"
        "<w:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</w:Action>"
        "</e:Header><e:Body><d:Probe><d:Types>dn:NetworkVideoTransmitter</d:Types>"
        "</d:Probe></e:Body></e:Envelope>";

    WiFiUDP udp;
    if (!udp.begin(3702)) {
        Serial.println("[cctv] ONVIF UDP bind failed");
        return;
    }
    IPAddress multicast(239, 255, 255, 250);
    for (int burst = 0; burst < 3; ++burst) {
        udp.beginPacket(multicast, 3702);
        udp.write((const uint8_t *)probe, strlen(probe));
        udp.endPacket();
        delay(150);
    }

    uint32_t deadline = millis() + 3500;
    char response[1400];
    while (millis() < deadline && !s_abort) {
        int packet_size = udp.parsePacket();
        if (packet_size <= 0) {
            ui_scanning_indicator("ONVIF discovery", s_hits_n);
            if (input_poll() == PK_ESC) s_abort = true;
            delay(5);
            continue;
        }
        int length = udp.read((uint8_t *)response, sizeof(response) - 1);
        if (length <= 0) continue;
        response[length] = '\0';
        if (!strstr(response, "ProbeMatch")) continue;

        cctv_hit_t hit = {};
        format_ip(udp.remoteIP(), hit.ip, sizeof(hit.ip));
        strncpy(hit.brand, onvif_brand(response), sizeof(hit.brand) - 1);
        extract_onvif_xaddr(response, hit.stream, sizeof(hit.stream));
        hit_add(hit);
        Serial.printf("[cctv] ONVIF ip=%s brand=%s endpoint=%s\n",
                      hit.ip, hit.brand, hit.stream[0] ? hit.stream : "-");
    }
    udp.stop();
}

/* Basic auth header: "Authorization: Basic <base64(user:pass)>". Bounded
 * so a long user:pass can't overflow the output buffer — mbedtls returns
 * MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL without null-terminating when the
 * destination is undersized. We treat that as failure and blank the
 * header. Output buffer is 128 B everywhere we call this. */
static bool build_basic_auth(const char *user, const char *pass, char *out, size_t out_sz)
{
    char up[64];
    int n = snprintf(up, sizeof(up), "%s:%s", user ? user : "", pass ? pass : "");
    if (n < 0 || (size_t)n >= sizeof(up)) { out[0] = 0; return false; }

    unsigned char b64[128];
    size_t olen = 0;
    int rc = mbedtls_base64_encode(b64, sizeof(b64), &olen,
                                   (const unsigned char *)up, (size_t)n);
    if (rc != 0 || olen >= sizeof(b64)) { out[0] = 0; return false; }
    b64[olen] = 0;
    snprintf(out, out_sz, "Basic %s", (const char *)b64);
    return true;
}

/* Fingerprint a brand from an HTTP response body. */
static const char *brand_of(const String &body, const String &headers)
{
    for (int i = 0; i < BRAND_N; ++i) {
        if (headers.indexOf(BRAND_SIGS[i].needle) >= 0
            || body.indexOf(BRAND_SIGS[i].needle) >= 0)
            return BRAND_SIGS[i].brand;
    }
    return "generic";
}

static bool looks_like_camera_page(const String &body, const String &headers)
{
    static const char *markers[] = {
        "camera", "Camera", "webcam", "Webcam", "IPCam", "ipcam",
        "ONVIF", "onvif", "NVR", "DVR", "rtsp", "RTSP",
    };
    for (const char *marker : markers)
        if (headers.indexOf(marker) >= 0 || body.indexOf(marker) >= 0) return true;
    return false;
}

/* ---- HTTP probe ---- */

struct http_result_t {
    int code;
    String headers;
    String body;
};

static bool http_get(IPAddress ip, uint16_t port, const char *path,
                     const char *auth_header, http_result_t &out,
                     uint32_t timeout_ms)
{
    /* Forwards to the canonical net_http_get so the status-line + body
     * parsing lives in one place. Keep this thin wrapper because the
     * struct return-by-ref + .code=0-on-fail convention is used by the
     * cred-spray loop. */
    out.code = net_http_get(ip, port, path, auth_header,
                            &out.body, &out.headers, timeout_ms);
    return out.code != 0;
}

/* Is this 2xx body actually authed content, or just a login page served
 * to unauthenticated clients? Heuristic: known "please sign in" markers
 * in the body. Cheap but kills most false positives. */
static bool looks_like_login_page(const String &body)
{
    if (body.length() == 0) return false;
    return body.indexOf("<title>Login") >= 0
        || body.indexOf("name=\"password\"") >= 0
        || body.indexOf("id=\"password\"") >= 0
        || body.indexOf("type=\"password\"") >= 0
        || body.indexOf("login-form") >= 0;
}

/* Try default credentials on a path that returned 401. Returns the
 * matching cred index, or -1. Stops on the first 2xx whose body doesn't
 * look like an unauthenticated login form. */
static int try_creds(IPAddress ip, uint16_t port, const char *path)
{
    for (int i = 0; i < CRED_N && !s_abort; ++i) {
        char auth[140];
        if (!build_basic_auth(DEFAULT_CREDS[i].user, DEFAULT_CREDS[i].pass,
                              auth, sizeof(auth))) continue;
        http_result_t r;
        if (!http_get(ip, port, path, auth, r, 1500)) continue;
        if (r.code >= 200 && r.code < 300 && !looks_like_login_page(r.body))
            return i;
    }
    return -1;
}

/* ---- RTSP DESCRIBE ---- */

static bool rtsp_options(IPAddress ip, uint16_t port)
{
    WiFiClient c;
    c.setTimeout(2);    /* read timeout: 2 s in seconds units (matches connect budget below) */
    if (!c.connect(ip, port, 2000)) return false;
    c.print("OPTIONS * RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: poseidon\r\n\r\n");
    uint32_t deadline = millis() + 1500;
    char line[64];
    size_t line_len = 0;
    while (c.connected() && millis() < deadline) {
        if (c.available()) {
            int value = c.read();
            if (value == '\n' || line_len + 1 >= sizeof(line)) {
                line[line_len] = '\0';
                c.stop();
                /* Any RTSP status proves the service is alive; DESCRIBE
                 * below decides whether a known stream path is usable. */
                return strncmp(line, "RTSP/", 5) == 0;
            }
            if (value >= 0 && value != '\r') line[line_len++] = (char)value;
            continue;
        }
        delay(5);
    }
    c.stop();
    return false;
}

static bool rtsp_describe(IPAddress ip, uint16_t port, const char *path,
                          char *out_url, size_t out_sz)
{
    WiFiClient c;
    c.setTimeout(2);
    if (!c.connect(ip, port, 2000)) return false;
    char ip_text[16];
    format_ip(ip, ip_text, sizeof(ip_text));
    char url[128];
    snprintf(url, sizeof(url), "rtsp://%s:%u%s", ip_text, port, path);
    c.printf("DESCRIBE %s RTSP/1.0\r\n", url);
    c.print("CSeq: 2\r\nUser-Agent: poseidon\r\n");
    c.print("Accept: application/sdp\r\n\r\n");

    uint32_t deadline = millis() + 1500;
    bool ok = false;
    char line[64];
    size_t line_len = 0;
    while (c.connected() && millis() < deadline) {
        if (c.available()) {
            int value = c.read();
            if (value != '\n' && line_len + 1 < sizeof(line)) {
                if (value >= 0 && value != '\r') line[line_len++] = (char)value;
                continue;
            }
            line[line_len] = '\0';
            if (strncmp(line, "RTSP/", 5) == 0) {
                const char *space = strchr(line, ' ');
                int code = space ? atoi(space + 1) : 0;
                ok = code >= 200 && code < 300;
            }
            break;
        }
        delay(5);
    }
    c.stop();
    if (ok) {
        strncpy(out_url, url, out_sz - 1);
        out_url[out_sz - 1] = 0;
    }
    return ok;
}

/* ---- per-host scan pipeline ---- */

static void scan_host(IPAddress ip)
{
    if (s_abort) return;

    cctv_hit_t h = {};
    format_ip(ip, h.ip, sizeof(h.ip));
    bool any_open = false;

    /* 1. Probe common camera ports on every host. */
    for (int i = 0; i < CAM_PRIMARY_PORTS_N && !s_abort; ++i) {
        if (input_poll() == PK_ESC) { s_abort = true; break; }
        if (net_tcp_open(ip, CAM_PORTS[i], 350)) {
            h.ports_mask |= (1u << i);
            any_open = true;
        }
    }
    if (!any_open) return;

    /* A live candidate earns the less-common alternate HTTP probes. */
    for (int i = CAM_PRIMARY_PORTS_N; i < CAM_PORTS_N && !s_abort; ++i) {
        if (input_poll() == PK_ESC) { s_abort = true; break; }
        if (net_tcp_open(ip, CAM_PORTS[i], 350)) h.ports_mask |= (1u << i);
    }

    bool camera_evidence = false;
    bool has_rtsp_port = (h.ports_mask & (1u << 1)) || (h.ports_mask & (1u << 3));

    /* 2. HTTP fingerprint on whichever HTTP port is open. */
    uint16_t http_port = 0;
    for (int i = 0; i < CAM_PORTS_N; ++i) {
        uint16_t p = CAM_PORTS[i];
        if (!(h.ports_mask & (1u << i))) continue;
        if (p == 80 || p == 8080 || p == 8081 || p == 8082 || p == 8083) {
            http_port = p; break;
        }
    }
    if (http_port) {
        http_result_t r;
        if (http_get(ip, http_port, "/", nullptr, r, 1500)) {
            strncpy(h.brand, brand_of(r.body, r.headers), sizeof(h.brand) - 1);
            camera_evidence = strcmp(h.brand, "generic") != 0
                           || looks_like_camera_page(r.body, r.headers);
            if (r.code == 401 && (camera_evidence || has_rtsp_port)) {
                int ci = try_creds(ip, http_port, "/");
                if (ci >= 0) {
                    snprintf(h.creds, sizeof(h.creds), "%s:%s",
                             DEFAULT_CREDS[ci].user, DEFAULT_CREDS[ci].pass);
                }
            }
        }
    }
    if (!h.brand[0]) strncpy(h.brand, "unknown", sizeof(h.brand) - 1);

    /* 3. RTSP. Walk common path shortlist; first 200 wins. */
    uint16_t rtsp_port = 0;
    for (int i = 0; i < CAM_PORTS_N; ++i) {
        uint16_t p = CAM_PORTS[i];
        if (!(h.ports_mask & (1u << i))) continue;
        if (p == 554 || p == 8554) { rtsp_port = p; break; }
    }
    if (rtsp_port && rtsp_options(ip, rtsp_port)) {
        camera_evidence = true;
        for (int i = 0; i < RTSP_PATHS_N && !h.stream[0] && !s_abort; ++i) {
            if (input_poll() == PK_ESC) { s_abort = true; break; }
            rtsp_describe(ip, rtsp_port, RTSP_PATHS[i], h.stream, sizeof(h.stream));
        }
    }

    if (!camera_evidence || s_abort) return;
    hit_add(h);
    Serial.printf("[cctv] hit ip=%s ports=0x%04X brand=%s stream=%s\n",
                  h.ip, h.ports_mask, h.brand, h.stream[0] ? "yes" : "no");
}

/* ---- SD output ---- */

static char s_log_path[64];

static void open_log(void)
{
    File log = sdlog_open_in(SD_SURVEILLANCE_DIR, "cctv",
                             "ip,ports_mask,brand,creds,endpoint",
                             s_log_path, sizeof(s_log_path));
    if (log) log.close();
}

static void log_hit(const cctv_hit_t &h)
{
    if (!s_log_path[0]) return;
    File log = SD.open(s_log_path, FILE_APPEND);
    if (!log) return;
    log.printf("%s,0x%04x,%s,%s,%s\n",
               h.ip, h.ports_mask, h.brand, h.creds, h.stream);
    log.close();
}

static void close_log(void)
{
    /* Logs are opened only around individual writes to keep FATFS buffers
     * out of the heap during the long socket-heavy subnet sweep. */
}

static void log_all_hits(void)
{
    for (int i = 0; i < s_hits_n; ++i) log_hit(s_hits[i]);
}

/* ---- UI ---- */

/* Track scan start so we can estimate remaining time. */
static uint32_t s_scan_start_ms = 0;

static void fmt_eta(uint32_t secs, char *out, size_t sz)
{
    if (secs >= 60) snprintf(out, sz, "%lum%02lus", (unsigned long)(secs / 60),
                                                     (unsigned long)(secs % 60));
    else            snprintf(out, sz, "%lus",       (unsigned long)secs);
}

static uint32_t s_prog_last = 0;

static void draw_cctv_chrome(void)
{
    auto &d = M5Cardputer.Display;
    ui_clear_body();
    ui_draw_status(radio_name(), "cctv");
    d.setTextColor(T_ACCENT, T_BG);
    d.setCursor(4, BODY_Y + 2);
    d.print("CCTV TOOLKIT");
    d.drawFastHLine(4, BODY_Y + 12, SCR_W - 8, T_ACCENT);
    d.drawRect(4, BODY_Y + 38, SCR_W - 12, 5, T_ACCENT);
    ui_draw_footer("`=abort");
    s_prog_last = 0;
}

static void draw_progress(int done, int total, int hits,
                          const char *phase, const char *current)
{
    bool final = (total <= 0) || (done >= total);
    uint32_t now = millis();
    if (!final && now - s_prog_last < 150) return;
    s_prog_last = now;

    auto &d = M5Cardputer.Display;

    /* Phase + current target IP, so the user knows we're actually working. */
    ui_text(4, BODY_Y + 16, T_FG, "%s", phase);
    if (current && *current) ui_text(4, BODY_Y + 26, T_DIM, "-> %s", current);
    else                     ui_text_w(4, BODY_Y + 26, SCR_W - 8, T_DIM, "");

    /* Progress bar + counts + ETA. */
    int bar_w = SCR_W - 12;
    int filled = total ? (bar_w * done / total) : 0;
    d.fillRect(5, BODY_Y + 39, bar_w - 2, 3, T_BG);
    d.fillRect(5, BODY_Y + 39, filled, 3, T_ACCENT2);

    char eta[16] = "";
    if (s_scan_start_ms && done > 0 && done < total) {
        uint32_t elapsed = (millis() - s_scan_start_ms) / 1000;
        uint32_t remaining = (elapsed * (uint32_t)(total - done)) / (uint32_t)done;
        fmt_eta(remaining, eta, sizeof(eta));
    }
    ui_text(4, BODY_Y + 46, T_DIM, "%d / %d   eta %s", done, total, eta);
    ui_text(4, BODY_Y + 56, T_GOOD, "hits %d", hits);

    /* 3 most recent hits — the 4th row overlapped the footer. */
    int first = s_hits_n > 3 ? s_hits_n - 3 : 0;
    for (int i = first; i < s_hits_n; ++i) {
        int y = BODY_Y + 68 + (i - first) * 10;
        const cctv_hit_t &h = s_hits[i];
        uint16_t col = h.creds[0] ? T_BAD : (h.stream[0] ? T_WARN : T_GOOD);
        ui_text(4, y, col, "%s %.7s %.14s", h.ip, h.brand,
                h.creds[0] ? h.creds : (h.stream[0] ? h.stream : "ports"));
    }
}

/* ---- three entry modes ---- */

static void reset_state(void)
{
    s_hits_n = 0;
    s_abort = false;
    memset(s_log_path, 0, sizeof(s_log_path));
}

static void wait_for_exit_key(void)
{
    while (true) {
        uint16_t k = input_poll();
        if (k == PK_ESC || k == PK_ENTER) break;
        delay(20);
    }
}

static bool cctv_wifi_info(IPAddress &local_ip)
{
    wifi_ap_record_t ap = {};
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) return false;
    esp_netif_t *sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!sta) return false;
    esp_netif_ip_info_t info = {};
    if (esp_netif_get_ip_info(sta, &info) != ESP_OK || info.ip.addr == 0) return false;
    local_ip = IPAddress(info.ip.addr);
    return true;
}

static void scan_lan(void)
{
    IPAddress local_ip;
    if (!cctv_wifi_info(local_ip)) {
        ui_toast("no WiFi", T_BAD, 1200);
        return;
    }
    reset_state();
    open_log();
    s_scan_start_ms = millis();

    IPAddress me  = local_ip;
    uint32_t base = ((uint32_t)me[0] << 24) | ((uint32_t)me[1] << 16)
                  | ((uint32_t)me[2] << 8);
    int total = 254;
    char cur[16] = "";

    draw_cctv_chrome();
    draw_progress(0, total, s_hits_n, "ONVIF discovery", "multicast");
    onvif_discover();
    for (int host = 1; host <= 254 && !s_abort; ++host) {
        if ((host & 15) == 0) {
            size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
            size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
            Serial.printf("[cctv] progress=%d/254 hits=%d free=%u largest=%u\n",
                          host, s_hits_n, (unsigned)free_heap, (unsigned)largest);
            if (free_heap < 20 * 1024 || largest < 12 * 1024) {
                Serial.println("[cctv] stopping: low internal heap");
                ui_toast("low memory - scan stopped", T_WARN, 1200);
                s_abort = true;
                break;
            }
        }
        IPAddress ip((base >> 24) & 0xFF,
                     (base >> 16) & 0xFF,
                     (base >> 8)  & 0xFF, host);
        if (ip == me) continue;   /* don't probe ourselves */
        format_ip(ip, cur, sizeof(cur));
        /* Redraw before each probe so the current IP is always visible. */
        draw_progress(host, total, s_hits_n, "LAN /24 sweep", cur);
        ui_scanning_indicator("probing", s_hits_n);
        scan_host(ip);
        uint16_t k = input_poll();
        if (k == PK_ESC) { s_abort = true; break; }
        delay(2);  /* let lwIP reclaim closed sockets between hosts */
    }

    log_all_hits();
    close_log();
    draw_progress(total, total, s_hits_n, "done", s_log_path);
    ui_toast(s_hits_n ? "hits saved to SD" : "no cams found",
             s_hits_n ? T_GOOD : T_WARN, 1500);
    wait_for_exit_key();
}

static void scan_single(void)
{
    char buf[20] = {0};
    if (!input_line("Target IP:", buf, sizeof(buf))) return;
    IPAddress ip;
    if (!ip.fromString(buf)) {
        ui_toast("invalid IP", T_BAD, 900);
        return;
    }
    reset_state();
    open_log();
    s_scan_start_ms = millis();
    draw_cctv_chrome();
    draw_progress(0, 1, 0, "single host", buf);
    ui_scanning_indicator("probing", s_hits_n);
    scan_host(ip);
    log_all_hits();
    close_log();
    draw_progress(1, 1, s_hits_n, "done", s_log_path);
    wait_for_exit_key();
}

static void scan_file(void)
{
    if (!sd_mount()) { ui_toast("no SD card", T_BAD, 1000); return; }
    File f = SD.open("/poseidon/cctv-targets.txt", FILE_READ);
    if (!f) {
        ui_toast("put IPs in /poseidon/cctv-targets.txt", T_BAD, 1800);
        return;
    }
    reset_state();
    open_log();
    s_scan_start_ms = millis();

    /* Count lines first so progress bar is accurate. */
    int total = 0;
    while (f.available()) {
        String l = f.readStringUntil('\n');
        if (l.length() > 0) total++;
    }
    f.seek(0);

    draw_cctv_chrome();
    int done = 0;
    char cur[16] = "";
    while (f.available() && !s_abort) {
        String l = f.readStringUntil('\n');
        l.trim();
        if (l.length() == 0 || l.startsWith("#")) { done++; continue; }
        IPAddress ip;
        if (ip.fromString(l)) {
            snprintf(cur, sizeof(cur), "%s", l.c_str());
            draw_progress(done, total, s_hits_n, "from file", cur);
            ui_scanning_indicator("probing", s_hits_n);
            scan_host(ip);
        }
        done++;
        uint16_t k = input_poll();
        if (k == PK_ESC) { s_abort = true; break; }
    }
    f.close();
    log_all_hits();
    close_log();
    draw_progress(total, total, s_hits_n, "done", s_log_path);
    wait_for_exit_key();
}

/* ---- top-level dispatcher ---- */

void feat_cctv_scan(void)
{
    if (!cctv_hits_ensure()) return;
    radio_switch(RADIO_WIFI);

    auto &d = M5Cardputer.Display;
    int sel = 0;
    const char *items[] = {
        "Scan LAN /24",
        "Single IP",
        "From /poseidon/cctv-targets.txt",
    };
    const int n = 3;

    while (true) {
        ui_clear_body();
        ui_draw_status(radio_name(), "cctv");
        d.setTextColor(T_ACCENT, T_BG);
        d.setCursor(4, BODY_Y + 2); d.print("CCTV TOOLKIT");
        d.drawFastHLine(4, BODY_Y + 12, SCR_W - 8, T_ACCENT);
        d.setTextColor(T_DIM, T_BG);
        d.setCursor(4, BODY_Y + 18);
        d.print("WiFi required  LAN sweep ~3-5 min  ESC=abort");

        for (int i = 0; i < n; ++i) {
            int y = BODY_Y + 34 + i * 14;
            bool s = (i == sel);
            if (s) d.fillRoundRect(4, y - 2, SCR_W - 8, 13, 2, 0x3007);
            d.setTextColor(s ? T_ACCENT2 : T_FG, s ? 0x3007 : T_BG);
            d.setCursor(10, y);
            d.printf("%s", items[i]);
        }
        ui_draw_footer(";/.=move ENTER=go `=back");

        uint16_t k = input_poll();
        if (k == PK_NONE) { delay(20); continue; }
        if (k == PK_ESC) return;
        if (k == ';' || k == PK_UP)   sel = (sel - 1 + n) % n;
        if (k == '.' || k == PK_DOWN) sel = (sel + 1) % n;
        if (k == PK_ENTER) {
            switch (sel) {
            case 0: scan_lan();    break;
            case 1: scan_single(); break;
            case 2: scan_file();   break;
            }
        }
    }
}
