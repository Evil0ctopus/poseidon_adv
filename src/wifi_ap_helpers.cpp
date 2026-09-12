/*
 * wifi_ap_helpers.cpp — see header.
 *
 * Implementation cribbed from wifi_portal.cpp:417-481 (POS-AUDIT-003
 * proven pattern). Phase 2 / wifi-042 will migrate the older inlines
 * (wifi_portal, evil_twin, ap_signal_test, wifi_ciw) to call this.
 */
#include "wifi_ap_helpers.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_bt.h>
#include <esp_log.h>

bool wifi_raw_ap_up(const char *ssid, uint8_t channel,
                    bool hidden, uint8_t max_conn)
{
    if (!ssid || !*ssid) return false;

    /* Pre-AP teardown — if WiFi was already inited via raw-IDF
     * (Triton / portal / etc.) we MUST fully deinit before AP
     * bring-up. Otherwise esp_wifi_init below asserts (one-shot)
     * and esp_netif_create_default_wifi_ap conflicts with an
     * existing default STA netif. Idempotent: safe if WiFi was
     * never inited. */
    esp_wifi_set_promiscuous(false);
    esp_wifi_stop();
    esp_wifi_deinit();
    delay(300);
    esp_netif_t *sta_if = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_if) esp_netif_destroy_default_wifi(sta_if);
    /* Same reasoning for a leftover AP netif — a prior raw-AP feature
     * (Karma, Portal, Evil Twin, CIW, AP Signal Test) may not have torn
     * its own AP netif down before this call, and the
     * esp_netif_create_default_wifi_ap below asserts on a duplicate
     * ifkey (hard crash + reboot) instead of failing softly. Found via
     * full-menu sweep test 2026-09-12: chaining raw-AP features back
     * to back crashed with "assert failed: esp_netif_create_default_
     * wifi_ap ... duplicate key". */
    esp_netif_t *stale_ap_if = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (stale_ap_if) esp_netif_destroy_default_wifi(stale_ap_if);

    /* POS-AUDIT-007 (revised after on-device repro 2026-06-06):
     * force-shutdown BT before mem_release. The original IDLE gate
     * preserved BLE for the session but on Bruce libs the post-BLE
     * path then crashed in ieee80211_hostap_attach +0x2c during AP
     * bring-up. Sacrifice BLE for the session via explicit disable
     * + deinit so AP-mode always works. Caller owns the toast (the
     * 4 raw-IDF AP feature sites each have a UI gate above). */
    bool bt_was_inited =
        (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_IDLE);
    if (bt_was_inited) {
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
    }
    esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

    esp_log_level_set("wifi",      ESP_LOG_INFO);
    esp_log_level_set("wifi_init", ESP_LOG_INFO);
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    /* Shrunk buf pools to fit Cardputer's ~115 KB internal SRAM after
     * GFX claims its share. Same values that beacon_spam / portal
     * verified to actually beacon. */
    cfg.static_tx_buf_num  = 0;
    cfg.dynamic_tx_buf_num = 16;
    cfg.tx_buf_type        = 1;
    cfg.cache_tx_buf_num   = 4;
    cfg.static_rx_buf_num  = 4;
    cfg.dynamic_rx_buf_num = 16;
    cfg.ampdu_tx_enable    = 0;
    cfg.ampdu_rx_enable    = 0;
    if (esp_wifi_init(&cfg) != ESP_OK) return false;
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t apc = {};
    strncpy((char *)apc.ap.ssid, ssid, sizeof(apc.ap.ssid) - 1);
    apc.ap.ssid_len        = strlen(ssid);
    apc.ap.channel         = channel ? channel : 1;
    apc.ap.authmode        = WIFI_AUTH_OPEN;
    apc.ap.max_connection  = max_conn ? max_conn : 4;
    apc.ap.beacon_interval = 100;
    apc.ap.ssid_hidden     = hidden ? 1 : 0;
    if (esp_wifi_set_config(WIFI_IF_AP, &apc) != ESP_OK) {
        esp_wifi_deinit();
        return false;
    }
    if (esp_wifi_start() != ESP_OK) {
        esp_wifi_deinit();
        return false;
    }
    /* Bug 11: bump TX power so consumers (net_attacks dead-drop, WPAD,
     * autodiscover, rogue DHCP) all broadcast at realistic range. 78 =
     * 19.5 dBm in 0.25 dBm units. */
    esp_wifi_set_max_tx_power(84);
    /* Some builds ignore the channel in the config struct; force it. */
    esp_wifi_set_channel(apc.ap.channel, WIFI_SECOND_CHAN_NONE);
    /* Settle window — AP_START event needs to fully process before
     * the AP is actually beaconing. Matches wifi_portal. */
    delay(50);
    return true;
}

void wifi_raw_ap_down(void)
{
    esp_wifi_set_promiscuous(false);
    esp_wifi_stop();
    esp_wifi_deinit();
}

IPAddress wifi_raw_ap_ip(void)
{
    esp_netif_t *ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (!ap) return IPAddress();
    esp_netif_ip_info_t info = {};
    if (esp_netif_get_ip_info(ap, &info) != ESP_OK) return IPAddress();
    return IPAddress(info.ip.addr);
}
