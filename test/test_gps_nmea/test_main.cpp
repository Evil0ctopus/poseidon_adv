#include <unity.h>
#include "../../src/gps_nmea.h"

void setUp(void) {}
void tearDown(void) {}

static void test_gga_fix_quality_zero_invalidates(void) {
    const char *line = "$GPGGA,123519,4807.038,N,01131.000,E,0,08,0.9,545.4,M,46.9,M,,*47";
    TEST_ASSERT_FALSE(gps_nmea_sentence_has_fix(line));
}

static void test_gga_fix_quality_one_validates(void) {
    const char *line = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    TEST_ASSERT_TRUE(gps_nmea_sentence_has_fix(line));
}

static void test_rmc_void_invalidates(void) {
    const char *line = "$GPRMC,123519,V,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";
    TEST_ASSERT_FALSE(gps_nmea_sentence_has_fix(line));
}

static void test_rmc_active_validates(void) {
    const char *line = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";
    TEST_ASSERT_TRUE(gps_nmea_sentence_has_fix(line));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_gga_fix_quality_zero_invalidates);
    RUN_TEST(test_gga_fix_quality_one_validates);
    RUN_TEST(test_rmc_void_invalidates);
    RUN_TEST(test_rmc_active_validates);
    return UNITY_END();
}
