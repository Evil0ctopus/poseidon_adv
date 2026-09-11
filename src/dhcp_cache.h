/*
 * dhcp_cache — shared (MAC → hostname) table.
 *
 * Populated by any feature that sees a DHCP DISCOVER/REQUEST:
 *   - wifi_portal.cpp (we're the DHCP server; plaintext)
 *   - wifi_clients / wifi_clients_all (open-network promiscuous)
 *   - anywhere we parse raw WiFi frames and hit option 12
 *
 * Consulted by client display code to upgrade MAC → real device name.
 */
#pragma once
#include <stdint.h>

/* Learn a (mac, hostname) pair. mac is 6 bytes in the order they
 * appear in 802.11 headers (display / big-endian). hostname is
 * null-terminated but may be truncated.
 *
 * POS-AUDIT-265 / net-015: previous header text claimed this was
 * "safe to call from an ISR or WiFi callback" — that's FALSE. find()
 * + s_n++ in dhcp_learn are not atomic; concurrent learns from
 * promisc_cb and DHCP server context can interleave and corrupt the
 * table (lost entries, duplicated MACs). Callers today happen to be
 * single-threaded by virtue of running from a single WiFi context at
 * a time, but adding portMUX here is a Phase-2 followup (net-015
 * extended ticket). For now: DO NOT call from an ISR; DO NOT call
 * from two concurrent WiFi contexts. */
void dhcp_learn(const uint8_t mac[6], const char *hostname);

/* Look up a previously-seen hostname. Returns nullptr if not cached. */
const char *dhcp_hostname(const uint8_t mac[6]);

/* Look up inferred operating system from DHCP fingerprints. Returns nullptr if not detected. */
const char *dhcp_os(const uint8_t mac[6]);

/* Learn full details (mac, hostname, os, vendor_class). */
void dhcp_learn_full(const uint8_t mac[6], const char *hostname, const char *os, const char *vendor_class);

/* Try to decode a DHCP Option 12 hostname out of a raw 802.11 data
 * frame payload (the whole promisc pkt.payload + sig_len). If the frame
 * is a DHCP DISCOVER/REQUEST, learns the hostname and returns true.
 *
 * This is for use inside promisc callbacks — it's cheap: bails as soon
 * as the frame fails a check. Safe on encrypted networks (encrypted
 * payload will fail the LLC/SNAP sanity check and return fast). */
bool dhcp_try_parse_802_11(const uint8_t *pkt, int len);

/* Stats. */
int  dhcp_cache_count(void);
void dhcp_cache_clear(void);
