/*
 * sigdb_surveillance — fingerprint database for passive surveillance
 * device detection (Flock Safety ALPR cameras, Raven/ShotSpotter
 * acoustic gunshot sensors).
 *
 * Authority: zmattmanz/Plume (Cardputer-Adv detector) is the most
 * actively maintained signature source as of pull date. Plume's
 * tier-1/tier-2 classification is used directly. Cross-references:
 *   - colonelpanichacks/flock-you (XIAO-ESP32-S3 detector)
 *   - 0xXyc/flock-you-wifi-recon
 *   - JesseCHale/HaleHound-CYD
 *   - DeFlockJoplin wildcard-probe research
 *   - NitekryDPaul receiver-MAC addr1 trick
 *
 * Anchor: b4:1e:52 (Flock direct, all sources agree).
 *
 * Tier definitions:
 *   T1 = high-confidence Flock-direct or Raven-direct (Plume's T1 list)
 *   T2 = contract manufacturer / lower-confidence (Plume's T2 list).
 *        Require corroborating SSID/UUID/probe match.
 *
 * NOTE: e4:aa:ea is Liteon, a contract manufacturer that produces
 * Flock hardware. Plume + 0xXyc both classify it as T1 (most-observed
 * Flock production OUI) — included here as T1.
 *
 * Speculative patterns (e.g. Axon body-cams) have been removed —
 * no public fingerprint exists, so including them as definitive
 * generated false positives on any Axon-named SSID.
 */
#pragma once

#include <stdint.h>
#include <ctype.h>

enum surv_class_t {
    SURV_UNKNOWN     = 0,
    SURV_FLOCK_T1    = 1,   /* Flock-direct OUI, high confidence */
    SURV_FLOCK_T2    = 2,   /* Contract manufacturer, need corroboration */
    SURV_FLOCK_SSID  = 3,   /* SSID pattern matched (Flock-XXXX etc.) */
    SURV_FLOCK_PROBE = 4,   /* Wildcard probe-req fingerprint (DeFlockJoplin) */
    SURV_RAVEN_BLE   = 5,   /* Raven/ShotSpotter via BLE manufacturer ID */
    SURV_RAVEN_UUID  = 6,   /* Raven via custom GATT service UUID */
};

/* ---- Flock OUI lists (3 octets each, lowercase form encoded as uint32) ---- */

/* Tier 1 — definitive when seen. Plume's T1 + entries cited as T1 by
 * 2+ upstream sources. */
static const uint32_t FLOCK_OUI_T1[] = {
    0xb41e52,  /* Flock anchor — universal agreement */
    0xe4aaea,  /* Liteon — most-observed Flock production OUI (Plume T1) */
    0x000901,  /* XUNTONG — Penguin battery (Plume T1) */
    0x4c6e44,  /* Plume T1 */
    0xd8a0d8,  /* Plume T1 */
    0xa0b765,  /* Plume T1 */
    0xf082c0,  /* Plume T1, also 0xXyc */
    0xb4e3f9,  /* Plume T1, also 0xXyc */
    0x040d84,  /* 0xXyc T1 */
    0x70c94e, 0x3c9180, 0xd8f3bc, 0x803049,
    0xb83532, 0x145afc, 0x744ca1, 0x083a88,
    0x9c2f9d, 0xc03532, 0x940853,
    0x1c34f1, 0x385b44, 0x943469,  /* 0xXyc T1 additions */
};
static const int FLOCK_OUI_T1_N = sizeof(FLOCK_OUI_T1) / sizeof(FLOCK_OUI_T1[0]);

/* Tier 2 — contract manufacturers / lower-confidence per Plume. */
static const uint32_t FLOCK_OUI_T2[] = {
    /* Plume T2 entries */
    0xe00af6, 0x00f48d, 0xd03957, 0xe8d0fc,
    0xe04f43, 0xb81ea4, 0xf46add, 0xf8a2d6,
    0x24b2b9, 0x700894, 0x588e81, 0xec1bbd,
    0x3c71bf, 0x5800e3, 0x9035ea, 0x5c93a2,
    0x646e69, 0x4827ea, 0xa4cf12, 0x826bf2,
    /* Plume T2 additions */
    0xe82a44, 0x30d16b, 0xb8ee65, 0xa4db30,
    0x40f02f,
};
static const int FLOCK_OUI_T2_N = sizeof(FLOCK_OUI_T2) / sizeof(FLOCK_OUI_T2[0]);

/* ---- Flock SSID patterns (case-INSENSITIVE substring match) ----
 * Plume's confirmed list. flock_classify_ssid() does its own case
 * folding so these can be written in their natural case. */
static const char *FLOCK_SSID_DEFINITIVE[] = {
    "Flock-",          /* Standard Flock SSID prefix */
    "FlockOS",         /* Internal management SSID */
    "flocksafety",     /* Plume — matched case-insensitively */
    "test_flck",       /* CVE-2025-59409 dev-mode SSID */
    "FS Ext Battery",  /* External battery pack accessory */
    "Penguin",         /* Internal codename for one Flock model */
    "Pigvision",       /* Plume internal codename */
    "OFS_IoT",         /* Plume — operations management */
    "PFS_",            /* Plume — production naming pattern */
    "FLCK-",           /* Newer naming pattern */
    "Raven-",          /* ShotSpotter Raven sensor */
};
static const int FLOCK_SSID_DEFINITIVE_N =
    sizeof(FLOCK_SSID_DEFINITIVE) / sizeof(FLOCK_SSID_DEFINITIVE[0]);

/* ---- BLE manufacturer-ID fingerprints ---- */

/* XUNTONG — Raven/ShotSpotter manufacturer-data company ID. */
#define BLE_CID_XUNTONG  0x09C8

/* ---- Raven custom GATT service UUIDs (16-bit form) ---- */

static const uint16_t RAVEN_UUID_16[] = {
    0x3100, 0x3200, 0x3300, 0x3400, 0x3500,
    0x180a, 0x1809, 0x1819,
};
static const int RAVEN_UUID_16_N = sizeof(RAVEN_UUID_16) / sizeof(RAVEN_UUID_16[0]);

/* ---- Lookup helpers ---- */

static inline uint32_t oui24(const uint8_t bssid[6])
{
    return ((uint32_t)bssid[0] << 16) | ((uint32_t)bssid[1] << 8) | bssid[2];
}

static inline surv_class_t flock_classify_oui(const uint8_t bssid[6])
{
    uint32_t v = oui24(bssid);
    for (int i = 0; i < FLOCK_OUI_T1_N; i++)
        if (v == FLOCK_OUI_T1[i]) return SURV_FLOCK_T1;
    for (int i = 0; i < FLOCK_OUI_T2_N; i++)
        if (v == FLOCK_OUI_T2[i]) return SURV_FLOCK_T2;
    return SURV_UNKNOWN;
}

/* Case-insensitive substring search. ESP-IDF doesn't ship strcasestr
 * so roll our own. */
static inline const char *istrstr_local(const char *hay, const char *needle)
{
    if (!hay || !needle || !*needle) return nullptr;
    for (; *hay; hay++) {
        const char *h = hay;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++; n++;
        }
        if (!*n) return hay;
    }
    return nullptr;
}

static inline surv_class_t flock_classify_ssid(const char *ssid)
{
    if (!ssid || !*ssid) return SURV_UNKNOWN;
    for (int i = 0; i < FLOCK_SSID_DEFINITIVE_N; i++) {
        if (istrstr_local(ssid, FLOCK_SSID_DEFINITIVE[i])) return SURV_FLOCK_SSID;
    }
    return SURV_UNKNOWN;
}

static inline bool raven_uuid_match(uint16_t uuid16)
{
    for (int i = 0; i < RAVEN_UUID_16_N; i++)
        if (uuid16 == RAVEN_UUID_16[i]) return true;
    return false;
}

static inline bool raven_company_match(uint16_t company_id)
{
    return company_id == BLE_CID_XUNTONG;
}

static inline const char *surv_class_name(surv_class_t c)
{
    switch (c) {
    case SURV_FLOCK_T1:    return "FLOCK-T1";
    case SURV_FLOCK_T2:    return "FLOCK-T2";
    case SURV_FLOCK_SSID:  return "FLOCK-SSID";
    case SURV_FLOCK_PROBE: return "FLOCK-PROBE";
    case SURV_RAVEN_BLE:   return "RAVEN-BLE";
    case SURV_RAVEN_UUID:  return "RAVEN-UUID";
    default:               return "UNKNOWN";
    }
}
