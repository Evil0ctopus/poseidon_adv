#include <unity.h>
#include <ui_layout.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static void test_fit_exact_and_overflow()
{
    char out[40];
    TEST_ASSERT_EQUAL_UINT(8, ui_layout::fit("POSEIDON", out, sizeof(out), 8));
    TEST_ASSERT_EQUAL_STRING("POSEIDON", out);
    ui_layout::fit("Bluetooth devices", out, sizeof(out), 9);
    TEST_ASSERT_EQUAL_STRING("Blueto...", out);
    ui_layout::fit("long", out, sizeof(out), 2);
    TEST_ASSERT_EQUAL_STRING("..", out);
}

static void test_fit_capacity_and_empty()
{
    char out[5] = "xxxx";
    ui_layout::fit("12345678", out, sizeof(out), 38);
    TEST_ASSERT_EQUAL_STRING("1...", out);
    ui_layout::fit(nullptr, out, sizeof(out), 38);
    TEST_ASSERT_EQUAL_STRING("", out);
    out[0] = 'x';
    TEST_ASSERT_EQUAL_UINT(0, ui_layout::fit("abc", out, 0, 5));
    TEST_ASSERT_EQUAL_CHAR('x', out[0]);
    ui_layout::fit("abc", out, 1, 8);
    TEST_ASSERT_EQUAL_CHAR('\0', out[0]);
}

static void test_utf8_is_one_cell()
{
    char out[40];
    ui_layout::fit("A\xE2\x86\x92" "B", out, sizeof(out), 3);
    TEST_ASSERT_EQUAL_STRING("A?B", out);
    ui_layout::fit("ab\tcd", out, sizeof(out), 10);
    TEST_ASSERT_EQUAL_STRING("ab cd", out);
}

static void test_wrap_words_and_newlines()
{
    const char *text = "one two three\n\nfour";
    char out[40];
    TEST_ASSERT_TRUE(ui_layout::wrap_next(text, out, sizeof(out), 7));
    TEST_ASSERT_EQUAL_STRING("one two", out);
    TEST_ASSERT_TRUE(ui_layout::wrap_next(text, out, sizeof(out), 7));
    TEST_ASSERT_EQUAL_STRING("three", out);
    TEST_ASSERT_TRUE(ui_layout::wrap_next(text, out, sizeof(out), 7));
    TEST_ASSERT_EQUAL_STRING("", out);
    TEST_ASSERT_TRUE(ui_layout::wrap_next(text, out, sizeof(out), 7));
    TEST_ASSERT_EQUAL_STRING("four", out);
    TEST_ASSERT_FALSE(ui_layout::wrap_next(text, out, sizeof(out), 7));
}

static void test_wrap_long_token_never_loses_content()
{
    const char *text = "123456789012345";
    char line[8], reconstructed[32] = "";
    while (ui_layout::wrap_next(text, line, sizeof(line), 6)) strcat(reconstructed, line);
    TEST_ASSERT_EQUAL_STRING("123456789012345", reconstructed);
}

static void test_entire_long_help_is_reachable()
{
    char text[2401];
    for (int i = 0; i < 2400; ++i) text[i] = i % 12 == 11 ? ' ' : 'a';
    text[2400] = '\0';
    const char *p = text;
    char line[64];
    size_t letters = 0, lines = 0;
    while (ui_layout::wrap_next(p, line, sizeof(line), 37)) {
        for (const char *c = line; *c; ++c) if (*c == 'a') ++letters;
        ++lines;
    }
    TEST_ASSERT_EQUAL_UINT(2200, letters);
    TEST_ASSERT_GREATER_THAN(8, lines);
    TEST_ASSERT_EQUAL_UINT(lines, ui_layout::line_count(text, 37));
}

static void test_footer_paging_preserves_all_controls()
{
    const char *p = "ENTER open = help ` back ;/. move letter jump";
    char line[64];
    char joined[128] = "";
    while (ui_layout::wrap_next(p, line, sizeof(line), 25)) {
        TEST_ASSERT_LESS_OR_EQUAL_UINT(25, strlen(line));
        if (joined[0]) strcat(joined, " ");
        strcat(joined, line);
    }
    TEST_ASSERT_EQUAL_STRING("ENTER open = help ` back ;/. move letter jump", joined);
}

static void test_wrap_tiny_buffers_makes_progress()
{
    const char *p = "abcd";
    char line[2];
    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_TRUE(ui_layout::wrap_next(p, line, sizeof(line), 38));
        TEST_ASSERT_EQUAL_CHAR('a' + i, line[0]);
    }
    TEST_ASSERT_FALSE(ui_layout::wrap_next(p, line, sizeof(line), 38));
    p = "abc";
    TEST_ASSERT_FALSE(ui_layout::wrap_next(p, line, 0, 38));
    TEST_ASSERT_EQUAL_STRING("abc", p);
}

static void test_menu_windows()
{
    for (int rows = 4; rows <= 6; rows += 2) {
        for (int count = 1; count < 80; ++count) {
            for (int cursor = 0; cursor < count; ++cursor) {
                const int first = ui_layout::window_start(cursor, count, rows);
                TEST_ASSERT_GREATER_OR_EQUAL(0, first);
                TEST_ASSERT_TRUE(cursor >= first && cursor < first + rows);
                TEST_ASSERT_LESS_OR_EQUAL(count <= rows ? 0 : count - rows, first);
            }
        }
    }
}

static void test_motion_is_bounded_and_finishes()
{
    uint16_t previous = 0;
    for (uint32_t ms = 0; ms <= 140; ++ms) {
        const uint16_t value = ui_layout::ease_out(ms, 140);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT16(previous, value);
        TEST_ASSERT_LESS_OR_EQUAL_UINT16(256, value);
        previous = value;
    }
    TEST_ASSERT_EQUAL_UINT16(0, ui_layout::ease_out(0, 140));
    TEST_ASSERT_EQUAL_UINT16(256, ui_layout::ease_out(140, 140));
    TEST_ASSERT_EQUAL_UINT16(256, ui_layout::ease_out(UINT32_MAX, 140));
    TEST_ASSERT_EQUAL_UINT16(256, ui_layout::ease_out(0, 0));
}

static void test_screen_geometry()
{
    TEST_ASSERT_LESS_OR_EQUAL(125, 12 + 104 + 8); // Menu hint above footer.
    TEST_ASSERT_LESS_OR_EQUAL(125, 12 + 20 + 7 * 11 + 8); // Eight scan rows.
    TEST_ASSERT_LESS_OR_EQUAL(125, 12 + 20 + 2 * 29 + 16 + 10); // Large scan rows.
    TEST_ASSERT_LESS_OR_EQUAL(125, 12 + 20 + 3 * 21 + 16); // Large help rows.
    TEST_ASSERT_LESS_OR_EQUAL(124, 12 + 1 + 3 + 4 + 2 + 2 * 31 + 28); // Three grid rows.
    for (int cursor = 0; cursor < 7; ++cursor) {
        const int first = cursor / 6 * 6;
        TEST_ASSERT_TRUE(cursor >= first && cursor < first + 6);
    }
    TEST_ASSERT_LESS_OR_EQUAL(240, 196 + 24); // Battery label.
    TEST_ASSERT_LESS_OR_EQUAL(150, 108 + 36); // Extra before C5 slot.
    TEST_ASSERT_LESS_OR_EQUAL(196, 150 + 42); // C5 before battery.
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_fit_exact_and_overflow);
    RUN_TEST(test_fit_capacity_and_empty);
    RUN_TEST(test_utf8_is_one_cell);
    RUN_TEST(test_wrap_words_and_newlines);
    RUN_TEST(test_wrap_long_token_never_loses_content);
    RUN_TEST(test_entire_long_help_is_reachable);
    RUN_TEST(test_footer_paging_preserves_all_controls);
    RUN_TEST(test_wrap_tiny_buffers_makes_progress);
    RUN_TEST(test_menu_windows);
    RUN_TEST(test_motion_is_bounded_and_finishes);
    RUN_TEST(test_screen_geometry);
    return UNITY_END();
}
