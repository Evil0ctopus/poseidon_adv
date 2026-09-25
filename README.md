<div align="center">

  <h1>POSEIDON: ADVANCED</h1>
  <p><b>Commander of the Deep</b></p>
  <p><i>Educational RF / embedded security lab for the M5Stack Cardputer-Advance — authorized use on owned hardware only</i></p>

  [![target](https://img.shields.io/badge/target-Cardputer--Adv-blue)](https://m5stack.com)
  [![framework](https://img.shields.io/badge/framework-Arduino%2FPlatformIO-green)](https://platformio.org/)
  [![license](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)
  [![modules](https://img.shields.io/badge/research%20modules-170%2B-orange)]()
  [![release](https://img.shields.io/badge/release-v0.7.0-blueviolet)]()
  [![version](https://img.shields.io/badge/version-Community%20Edition-success)]()
  [![use](https://img.shields.io/badge/use-authorized%20%2F%20owned%20hardware-important)]()

</div>

---

## Safety / Authorized Use

**POSEIDON is an educational wireless auditing lab for security research on hardware and networks you own or have explicit permission to test.**

- Use only on **your own equipment** or systems where you have **written authorization**.
- Do **not** use this firmware for unauthorized access, interference, jamming, or disruption of third-party networks or devices.
- You are responsible for complying with local radio and computer-misuse laws (e.g. FCC / Ofcom / ETSI and equivalents).
- Captured data (handshakes, probes, logs) may include personal information — handle it lawfully.

If you are unsure whether a use is allowed, treat it as **not allowed**. See the upstream [Legal & Responsible Use](https://generaldussduss.github.io/poseidon/legal.html) page for fuller guidance.

---

## About POSEIDON (Community Edition)

**POSEIDON** is a pocket-sized, keyboard-first **wireless auditing and security-research lab** built for the **M5Stack Cardputer-Advance** (ESP32-S3, QWERTY keyboard, 240×135 color display). Research modules cover Wi-Fi, Bluetooth Low Energy (BLE), Sub-GHz, 2.4 GHz, LoRa, Infrared, and LAN — intended for learning RF/embedded security concepts and testing **owned** gear.

This fork (`poseidon_adv`) builds on the GeneralDussDuss release with stability work, performance tweaks, and features aimed at lab / field evaluation on authorized hardware.

* **170+ research modules:** Broad coverage across Wi-Fi, BLE, Sub-GHz, 2.4 GHz, LoRa, Infrared, and LAN for recon and defensive assessment in authorized environments.
* **QWERTY-driven interface:** Letter-mnemonic menus and direct parameter typing (no thumb-stick scrolling).
* **Argus autonomous Gotchi:** A reactive 96×96 mood sprite that can hunt handshakes on owned networks using an SD-persisted reinforcement-learning brain.
* **Multi-theme engine:** Six core aesthetics (POSEIDON, Matrix, E-Ink, Synthwave, Phantom, Blood) with procedural ambient motion.

---

## What's New & Custom Changes in `poseidon_adv`

This Community Edition adds several upgrades over upstream:

* **Real-time sensor awareness:** Dedicated monitoring helpers for Flock ALPR and ShotSpotter Raven fingerprints (lab / awareness tooling — not for unauthorized targeting).
* **Triton RPG Gotchi leveling:** Persistent XP tracking and interactive leveling tied to Argus.
* **Expanded UI palettes & screensavers:** Broader theme palette (11 themes) and 13 ambient screensavers.
* **On-device data management:** Integrated hex and log viewers on the display.
* **Advanced connectivity:** GATT notification streaming, richer device/OS fingerprinting, and expanded support for hardware hat expansion modules.

---

## Quick Start & Installation

Two fast, browser-based methods — no local toolchain required:

### Method 1: Web Flasher (Recommended)
1. Plug your Cardputer-Advance into your PC via USB-C.
2. Open the official [Web Installer](https://generaldussduss.github.io/poseidon/install.html) in a WebSerial-compatible browser (Chrome, Edge, or Opera).
3. Click **Connect**, select your device's serial port, and click **Install**.

### Method 2: M5Burner
1. Open the desktop [M5Burner](https://m5burner.com/) application.
2. Search for **POSEIDON**, plug in your device, and hit **Burn**.

---

## Support My Work

If you find this firmware or related open-source security tools helpful, consider supporting future development:

[![PayPal](https://img.shields.io/badge/PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://paypal.me/Evil0ctopus)

---

## Acknowledgments & Shoutouts

Thanks to the open-source creators whose work made this lab possible:

* **[@7h30th3r0n3](https://github.com/7h30th3r0n3)** — Evil-M5Project and RaspyJack (inspiration for the SaltyJack LAN toolkit).
* **[@justcallmekoko](https://github.com/justcallmekoko)** — ESP32Marauder wireless research / promiscuous-mode frameworks.
* **[@bmorcelli](https://github.com/bmorcelli)** / **[@pr3y](https://github.com/pr3y)** — Bruce hardware reference implementations.
* **[@UberGuidoZ](https://github.com/UberGuidoZ)** — Flipper Sub-GHz signal libraries.
* **[@M5Stack](https://github.com/M5Stack)** — Hardware design and SDKs.

---

</div>
