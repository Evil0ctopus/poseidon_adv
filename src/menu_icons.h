/*
 * Pixel-aligned 24px domain icons. Submenus inherit their root domain;
 * hotkeys remain separate labels so pictographs never hide navigation.
 * The archived bitmap sheet is not used by the Deepwater renderer.
 */
#pragma once

#include "menu.h"

/* Returns true if it drew an icon. Returns false if the carousel
 * should render the hotkey letter instead. (cx, cy) is the center of
 * the badge interior — the dispatcher offsets the bitmap by half its
 * width / height to draw it centered. */
bool draw_menu_icon(int cx, int cy, uint16_t color,
                    const menu_node_t *parent, const menu_node_t *item);
