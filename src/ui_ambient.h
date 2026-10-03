/*
 * ui_ambient — theme-aware procedural ambient motion behind static screens.
 *
 * Call ui_ambient_tick() at the start of any refresh loop (or once per
 * menu redraw) BEFORE drawing content on top. Picks an animation per the
 * active theme: POSEIDON deep-sea motes + wave band, AMBER CRT scanline,
 * TRON grid + packet hop, MATRIX dimmed rain, PHANTOM cyberpunk glyphs.
 * No-op for E-INK and HI-CONTRAST (paper aesthetic + accessibility).
 *
 * Mood-aware speed modulation (HUNT 2.0x, STEALTH 0.5x, SURGICAL 1.0x,
 * STORM 3.5x) ships in a follow-up commit once the Triton-side accessor
 * (triton_current_mode_int) is wired up. For now the multiplier is 1.0
 * baseline regardless of any active feature.
 *
 * Persisted enable flag in NVS namespace "pamb", key "enabled" (default
 * false for new preferences). Saved choices are preserved.
 * ui_ambient_tick() also respects reduced motion; when disabled it returns
 * immediately so the menu hook is a true no-op and any visual regression
 * can be turned off without a re-flash.
 */
#pragma once

#include <Arduino.h>

void ui_ambient_tick(int x, int y, int w, int h);

bool ui_ambient_enabled(void);
void ui_ambient_enabled_set(bool on);
