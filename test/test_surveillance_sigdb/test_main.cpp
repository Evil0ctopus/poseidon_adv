#include <unity.h>
#include <string.h>
#include "../../src/sigdb_surveillance.h"

void setUp(void) {}
void tearDown(void) {}

static void test_raven_company_id_fixture(void)
{
    TEST_ASSERT_TRUE(raven_company_match(0x09C8));
    TEST_ASSERT_FALSE(raven_company_match(0x004C));
    TEST_ASSERT_FALSE(raven_company_match(0xFFFF));
}

static void test_raven_uuid_fixtures(void)
{
    TEST_ASSERT_TRUE(raven_uuid_match(0x3100));
    TEST_ASSERT_TRUE(raven_uuid_match(0x3500));
    TEST_ASSERT_TRUE(raven_uuid_match(0x180A));
    TEST_ASSERT_FALSE(raven_uuid_match(0x2A00));
}

static void test_flock_oui_fixtures(void)
{
    const uint8_t direct[6] = {0xB4, 0x1E, 0x52, 0, 0, 0};
    const uint8_t contract[6] = {0xE0, 0x0A, 0xF6, 0, 0, 0};
    const uint8_t unknown[6] = {0xDE, 0xAD, 0xBE, 0, 0, 0};
    TEST_ASSERT_EQUAL(SURV_FLOCK_T1, flock_classify_oui(direct));
    TEST_ASSERT_EQUAL(SURV_FLOCK_T2, flock_classify_oui(contract));
    TEST_ASSERT_EQUAL(SURV_UNKNOWN, flock_classify_oui(unknown));
}

static void test_flock_ssid_fixtures(void)
{
    TEST_ASSERT_EQUAL(SURV_FLOCK_SSID, flock_classify_ssid("Flock-1234"));
    TEST_ASSERT_EQUAL(SURV_FLOCK_SSID, flock_classify_ssid("pFs_camera"));
    TEST_ASSERT_EQUAL(SURV_UNKNOWN, flock_classify_ssid("ordinary-network"));
}

static void test_rtsp_status_line_is_service_presence(void)
{
    TEST_ASSERT_EQUAL_INT(0, strncmp("RTSP/1.0 200 OK", "RTSP/", 5));
    TEST_ASSERT_EQUAL_INT(0, strncmp("RTSP/1.0 404 Not Found", "RTSP/", 5));
    TEST_ASSERT_NOT_EQUAL(0, strncmp("HTTP/1.1 404 Not Found", "RTSP/", 5));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_raven_company_id_fixture);
    RUN_TEST(test_raven_uuid_fixtures);
    RUN_TEST(test_flock_oui_fixtures);
    RUN_TEST(test_flock_ssid_fixtures);
    RUN_TEST(test_rtsp_status_line_is_service_presence);
    return UNITY_END();
}
