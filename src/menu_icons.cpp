#include "menu_icons.h"
#include <M5Cardputer.h>

extern const menu_node_t MENU_ROOT;

static bool contains(const menu_node_t *node, const menu_node_t *wanted)
{
    if (node == wanted) return true;
    for (const menu_node_t *c = node->children; c && c->hotkey; ++c)
        if (contains(c, wanted)) return true;
    return false;
}

bool draw_menu_icon(int cx, int cy, uint16_t color,
                    const menu_node_t *parent, const menu_node_t *item)
{
    if (!parent || !item) return false;
    char domain = item->hotkey;
    if (parent != &MENU_ROOT) {
        domain = 0;
        for (const menu_node_t *root = MENU_ROOT.children; root && root->hotkey; ++root) {
            if (contains(root, parent)) { domain = root->hotkey; break; }
        }
    }
    auto &d = M5Cardputer.Display;
    auto line = [&](int x1, int y1, int x2, int y2) {
        d.drawLine(cx + x1, cy + y1, cx + x2, cy + y2, color);
    };
    auto rect = [&](int x, int y, int w, int h) {
        d.drawRect(cx + x, cy + y, w, h, color);
    };
    auto circle = [&](int x, int y, int r) {
        d.drawCircle(cx + x, cy + y, r, color);
    };

    switch (domain) {
    case 'w':
        line(-10, -5, -6, -9); line(-6, -9, 6, -9); line(6, -9, 10, -5);
        line(-7, -2, -4, -5); line(-4, -5, 4, -5); line(4, -5, 7, -2);
        line(-4, 1, -2, -1); line(-2, -1, 2, -1); line(2, -1, 4, 1);
        circle(0, 6, 2);
        break;
    case 'b':
        line(0, -10, 0, 10); line(0, -10, 7, -4);
        line(7, -4, -6, 6); line(-6, -6, 7, 4); line(7, 4, 0, 10);
        break;
    case 'i':
        rect(-5, -2, 11, 9); line(-3, 7, -3, 11); line(3, 7, 3, 11);
        line(0, -5, 0, -10); line(-7, -4, -10, -7); line(7, -4, 10, -7);
        break;
    case 't':
        circle(0, 0, 10); circle(-3, -1, 2); circle(4, -1, 2);
        line(-4, 5, 4, 5); line(-4, 5, -5, 3); line(4, 5, 5, 3);
        break;
    case 'u':
        rect(-5, -10, 11, 8); rect(-7, -2, 15, 11);
        line(-3, -7, -3, -5); line(3, -7, 3, -5); line(0, 9, 0, 11);
        break;
    case 'n':
    case 'm':
        circle(0, 0, 3); circle(-8, -7, 2); circle(8, -7, 2); circle(0, 9, 2);
        line(-6, -5, -2, -2); line(6, -5, 2, -2); line(0, 3, 0, 7);
        if (domain == 'm') { line(-6, -7, 6, -7); line(-7, -5, -1, 7); line(7, -5, 1, 7); }
        break;
    case 'j':
        circle(0, -1, 8); rect(-4, 5, 9, 5);
        rect(-5, -3, 3, 3); rect(3, -3, 3, 3); line(0, 1, 0, 3);
        break;
    case 'r':
        line(0, -7, -7, 10); line(-7, 10, 7, 10); line(7, 10, 0, -7);
        line(-4, 3, 4, 3); line(-4, 3, 5, 8); line(4, 3, -5, 8);
        circle(0, -9, 2); line(-6, -10, -9, -6); line(6, -10, 9, -6);
        break;
    case '5':
        rect(-3, -3, 7, 7); rect(-11, -7, 6, 8); rect(6, 1, 6, 8);
        line(-5, -3, -3, -1); line(4, 2, 6, 4); line(1, -3, 5, -7);
        circle(7, -9, 2);
        break;
    case 'x':
        line(-11, 0, -5, -6); line(-5, -6, 5, -6); line(5, -6, 11, 0);
        line(-11, 0, -5, 6); line(-5, 6, 5, 6); line(5, 6, 11, 0); circle(0, 0, 3);
        break;
    case 'p':
        rect(-9, -8, 19, 13); line(-11, 8, 11, 8);
        line(-9, 5, -11, 8); line(9, 5, 11, 8);
        break;
    case 's':
        circle(0, 0, 7); circle(0, 0, 3);
        line(0, -7, 0, -11); line(0, 7, 0, 11);
        line(-7, 0, -11, 0); line(7, 0, 11, 0);
        line(-5, -5, -8, -8); line(5, -5, 8, -8);
        line(-5, 5, -8, 8); line(5, 5, 8, 8);
        break;
    case 'f':
        line(-8, 9, 7, -9); line(7, -9, 9, -2); line(9, -2, 1, 6);
        line(1, 6, -6, 6); line(-3, 3, 4, 3); line(1, -2, 7, -2);
        break;
    case 'k':
        circle(-5, -3, 5); line(-1, 1, 8, 10);
        line(3, 5, 6, 2); line(6, 8, 9, 5);
        break;
    case 'o':
        line(-8, 9, 5, -4); circle(5, -5, 5);
        line(6, -10, 6, -5); line(6, -5, 10, -5);
        circle(-7, 8, 1);
        break;
    default:
        rect(-9, -8, 19, 17); line(-5, -3, -1, 0); line(-1, 0, -5, 3);
        line(2, 4, 6, 4);
        break;
    }
    return true;
}
