#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace ui_layout {

// Convert unsupported UTF-8 glyphs to one placeholder, not one per byte.
inline char glyph(const char *&text)
{
    const uint8_t c = static_cast<uint8_t>(*text++);
    if (c >= 0x20 && c < 0x7F) return static_cast<char>(c);
    if (c >= 0x80) {
        while ((static_cast<uint8_t>(*text) & 0xC0) == 0x80) ++text;
        return '?';
    }
    return ' ';
}

inline size_t fit(const char *text, char *out, size_t capacity, size_t cells)
{
    if (capacity == 0) return 0;
    if (cells >= capacity) cells = capacity - 1;
    if (!text) text = "";
    size_t n = 0;
    while (*text && n < cells) out[n++] = glyph(text);
    if (*text && n) {
        const size_t dots = n < 3 ? n : 3;
        for (size_t i = 0; i < dots; ++i) out[n - 1 - i] = '.';
    }
    out[n] = '\0';
    return n;
}

// Advance the source pointer so arbitrarily long help needs no line table.
inline bool wrap_next(const char *&text, char *out, size_t capacity, size_t cells)
{
    if (capacity == 0) return false;
    out[0] = '\0';
    if (!text || !*text || cells == 0) return false;
    if (cells >= capacity) cells = capacity - 1;
    if (cells == 0) return false;
    size_t n = 0;
    size_t break_at = 0;
    const char *break_next = nullptr;
    while (*text && *text != '\n' && n < cells) {
        if (*text == ' ') {
            break_at = n;
            break_next = text + 1;
        }
        out[n++] = glyph(text);
    }
    if (*text == '\n') {
        ++text;
    } else if (*text && *text != ' ' && break_next && break_at > 0) {
        text = break_next;
        n = break_at;
    }
    while (*text == ' ') ++text;
    while (n && out[n - 1] == ' ') --n;
    out[n] = '\0';
    return true;
}

inline size_t line_count(const char *text, size_t cells)
{
    char line[64];
    size_t count = 0;
    while (wrap_next(text, line, sizeof(line), cells)) ++count;
    return count;
}

inline int window_start(int cursor, int count, int rows)
{
    if (count <= rows) return 0;
    int first = cursor - rows / 2;
    if (first < 0) first = 0;
    if (first > count - rows) first = count - rows;
    return first;
}

inline uint16_t ease_out(uint32_t elapsed, uint32_t duration)
{
    if (duration == 0 || elapsed >= duration) return 256;
    const uint32_t remaining = 256 - elapsed * 256 / duration;
    return static_cast<uint16_t>(256 - remaining * remaining / 256);
}

} // namespace ui_layout
