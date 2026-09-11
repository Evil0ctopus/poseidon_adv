/*
 * sigdb_bt — Bluetooth SIG Assigned Numbers (curated subset).
 *
 * Embeds the manufacturer-ID + 16-bit member-service-UUID lookups
 * used by feat_ble_scan to label devices with human-readable
 * vendor / service names instead of raw hex.
 *
 * Subset chosen for size — top ~70 manufacturer IDs by real-world
 * BLE prevalence + ~30 most commonly advertised services. Full BT
 * SIG list is 3000+ entries; embedding all of it would burn 30 KB
 * of flash for marginal benefit.
 *
 * Source: Bluetooth SIG Assigned Numbers spec
 * (bluetooth.com/specifications/assigned-numbers) + GhostBLE
 * (SmonSE) curated subset.
 */
#pragma once

#include <stdint.h>

struct bt_mfr_t  { uint16_t cid;  const char *name; };
struct bt_svc_t  { uint16_t uuid; const char *name; };

/* ---- BT Company Identifiers (manufacturer-data prefix) ---- */
static const bt_mfr_t BT_MFR[] = {
    { 0x0001, "Nokia" },
    { 0x0002, "Intel" },
    { 0x0006, "Microsoft" },
    { 0x000A, "CSR" },
    { 0x000D, "Texas Instruments" },
    { 0x000F, "Broadcom" },
    { 0x0010, "Mitel" },
    { 0x001D, "Qualcomm" },
    { 0x0022, "Bandspeed" },
    { 0x002A, "Microchip" },
    { 0x0034, "Sony" },
    { 0x004C, "Apple" },
    { 0x004F, "Logitech" },
    { 0x0059, "Nordic Semi" },
    { 0x0065, "HP" },
    { 0x0075, "Samsung" },
    { 0x0078, "Nike" },
    { 0x0085, "BlueRadios" },
    { 0x0087, "Garmin" },
    { 0x008A, "Bose" },
    { 0x0099, "Plantronics" },
    { 0x009E, "Bose" },
    { 0x00B4, "Sonos" },
    { 0x00BB, "Garmin Sport" },
    { 0x00C4, "LG" },
    { 0x00D2, "Polar Electro" },
    { 0x00E0, "Google" },
    { 0x00F0, "Lenovo" },
    { 0x0118, "GoPro" },
    { 0x0131, "Cypress" },
    { 0x0157, "Anhui Huami (Mi Band)" },
    { 0x0171, "Amazon" },
    { 0x0193, "Roku" },
    { 0x01DA, "Logitech" },
    { 0x0224, "Cooler Master" },
    { 0x0258, "Allterco (Shelly)" },
    { 0x028D, "Mophie" },
    { 0x02E5, "Beats" },
    { 0x038F, "Xiaomi" },
    { 0x0590, "DJI" },
    { 0x05A7, "Sony Mobile" },
    { 0x05F1, "Telink Semi" },
    { 0x0822, "Govee" },
    { 0x0824, "Fitbit" },
    { 0x09C8, "XUNTONG (Raven)" },
    { 0x09CC, "Wyze" },
    { 0x0C68, "Anker" },
    { 0x0DFB, "TP-Link" },
    { 0x07E4, "Tile" },
    /* POS-AUDIT-223: dropped duplicate 0x0131 (Cypress at line 55 wins
     * first-match) and 0x0157 (Anhui Huami / Mi-Fit same registrant). */
    { 0x0E0A, "Ring" },
    { 0x0E1F, "Nest Labs" },
    { 0x011A, "Estimote" },
    { 0x00D7, "WiSilica" },
    { 0x0181, "Snapchat (Spectacles)" },
    { 0x0204, "Withings" },
};
static const int BT_MFR_N = sizeof(BT_MFR) / sizeof(BT_MFR[0]);

static inline const char *bt_mfr_name(uint16_t cid)
{
    for (int i = 0; i < BT_MFR_N; i++)
        if (BT_MFR[i].cid == cid) return BT_MFR[i].name;
    return nullptr;
}

/* ---- 16-bit Member Service UUIDs (most-advertised) ---- */
static const bt_svc_t BT_SVC[] = {
    /* Standard GATT services */
    { 0x1800, "Generic Access" },
    { 0x1801, "Generic Attribute" },
    { 0x1802, "Immediate Alert" },
    { 0x1803, "Link Loss" },
    { 0x1804, "Tx Power" },
    { 0x1805, "Current Time" },
    { 0x1808, "Glucose" },
    { 0x1809, "Health Thermometer" },
    { 0x180A, "Device Info" },
    { 0x180D, "Heart Rate" },
    { 0x180F, "Battery" },
    { 0x1810, "Blood Pressure" },
    { 0x1812, "HID" },
    { 0x1814, "Run Speed/Cadence" },
    { 0x1816, "Cycle Speed/Cadence" },
    { 0x1818, "Cycle Power" },
    { 0x1819, "Location/Navigation" },
    { 0x181A, "Env Sensing" },
    { 0x181B, "Body Composition" },
    { 0x181C, "User Data" },
    { 0x181D, "Weight Scale" },
    { 0x181E, "Bond Mgmt" },
    { 0x181F, "Continuous Glucose" },
    { 0x1822, "Pulse Oximeter" },
    { 0x1826, "Fitness Machine" },
    { 0x183B, "Binary Sensor" },
    /* Vendor SDO (member services) */
    { 0xFD6F, "Exposure Notif (A/G)" },
    { 0xFD3D, "Solum (label printer)" },
    { 0xFD81, "Apple Continuity" },
    { 0xFDD2, "Bose" },
    { 0xFDDF, "Harman Kardon" },
    { 0xFE07, "Sonos" },
    { 0xFE9F, "Google Fast Pair" },
    { 0xFEAA, "Eddystone" },
    { 0xFEFF, "GN ReSound" },
    { 0xFFFA, "ASTM Drone Remote ID" },
    { 0xFFFB, "Thread" },
    { 0xFFF6, "Matter" },
    { 0xFFF7, "Zigbee Direct" },
};
static const int BT_SVC_N = sizeof(BT_SVC) / sizeof(BT_SVC[0]);

static inline const char *bt_svc_name(uint16_t uuid)
{
    for (int i = 0; i < BT_SVC_N; i++)
        if (BT_SVC[i].uuid == uuid) return BT_SVC[i].name;
    return nullptr;
}

/* ---- Standard GATT Characteristic UUIDs (0x2Axx) ---- */
static const bt_svc_t BT_CHAR[] = {
    { 0x2A00, "Device Name" },
    { 0x2A01, "Appearance" },
    { 0x2A04, "Pref Conn Param" },
    { 0x2A05, "Service Changed" },
    { 0x2A19, "Battery Level" },
    { 0x2A24, "Model Number" },
    { 0x2A25, "Serial Number" },
    { 0x2A26, "Firmware Rev" },
    { 0x2A27, "Hardware Rev" },
    { 0x2A28, "Software Rev" },
    { 0x2A29, "Manufacturer" },
    { 0x2A37, "Heart Rate Measurement" },
    { 0x2A38, "Body Sensor Location" },
    { 0x2A4A, "HID Info" },
    { 0x2A4B, "Report Map" },
    { 0x2A4C, "HID Control Point" },
    { 0x2A4D, "Report" },
    { 0x2A4E, "Protocol Mode" },
    { 0x2A6E, "Temperature" },
    { 0x2A6F, "Humidity" },
};
static const int BT_CHAR_N = sizeof(BT_CHAR) / sizeof(BT_CHAR[0]);

static inline const char *bt_char_name(uint16_t uuid)
{
    for (int i = 0; i < BT_CHAR_N; i++)
        if (BT_CHAR[i].uuid == uuid) return BT_CHAR[i].name;
    return nullptr;
}

/* ---- Common appearance category prefixes (10-bit category, lower 6
 * are sub-type and ignored here for compactness) ---- */
static const bt_svc_t BT_APPEARANCE[] = {
    { 0x0040 >> 6, "Phone" },
    { 0x0080 >> 6, "Computer" },
    { 0x00C0 >> 6, "Watch" },
    { 0x0100 >> 6, "Clock" },
    { 0x0140 >> 6, "Display" },
    { 0x0180 >> 6, "Remote" },
    { 0x01C0 >> 6, "Eye-glasses" },
    { 0x0200 >> 6, "Tag" },
    { 0x0240 >> 6, "Keyring" },
    { 0x0280 >> 6, "Media Player" },
    { 0x02C0 >> 6, "Barcode Scanner" },
    { 0x0300 >> 6, "Thermometer" },
    { 0x0340 >> 6, "Heart Rate" },
    { 0x0380 >> 6, "Blood Pressure" },
    { 0x03C0 >> 6, "HID" },
    { 0x0400 >> 6, "Glucose" },
    { 0x0440 >> 6, "Sport/Activity" },
    { 0x0480 >> 6, "Cycle" },
    { 0x0500 >> 6, "Pulse Oximeter" },
    { 0x0540 >> 6, "Weight" },
    { 0x0580 >> 6, "Mobility" },
    { 0x05C0 >> 6, "Continuous Glucose" },
    { 0x0600 >> 6, "Insulin Pump" },
    { 0x0640 >> 6, "Med Imaging" },
    { 0x0680 >> 6, "Outdoor Sport" },
};
static const int BT_APPEARANCE_N = sizeof(BT_APPEARANCE) / sizeof(BT_APPEARANCE[0]);

static inline const char *bt_appearance_name(uint16_t appearance)
{
    uint16_t cat = appearance >> 6;
    for (int i = 0; i < BT_APPEARANCE_N; i++)
        if (BT_APPEARANCE[i].uuid == cat) return BT_APPEARANCE[i].name;
    return nullptr;
}
