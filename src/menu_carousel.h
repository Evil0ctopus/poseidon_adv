/*
 * menu_carousel — secondary menu render style.
 *
 * Single-focus domain card with pixel icons, wrapped description,
 * a separate hotkey, position counter and interruptible focus underline.
 * Keyboard semantics match the terminal
 * mode exactly (letter mnemonics, ENTER, ESC, `=` for help) so a user
 * can flip between the two layouts at any time.
 *
 * Activated via menu_style_set(MENU_STYLE_CAROUSEL). Until then, the
 * terminal renderer in menu.cpp handles everything.
 */
#pragma once

#include "menu.h"

/* Recursive render+input loop for one submenu. Mirrors run_submenu() in
 * menu.cpp but with the card-style visuals + animation. Returns when
 * the user hits ESC (or `, the alternate back key). */
void carousel_run_submenu(const menu_node_t *parent);
