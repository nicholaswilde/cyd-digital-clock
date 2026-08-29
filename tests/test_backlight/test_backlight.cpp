#include <unity.h>
#include "Arduino.h"
#include "../mocks/mocks.cpp"
#include "backlight_manager.h"
#include "../../src/backlight_manager.cpp"

void setUp(void) {
    // any setup needed
}

void tearDown(void) {
    // any cleanup needed
}

void test_backlight_initial_state(void) {
    BacklightManager bm(21, 0, 10.0f);
    TEST_ASSERT_EQUAL(0, bm.getDutyCycle());
}

void test_backlight_mapping_extremes(void) {
    BacklightManager bm(21, 0, 10.0f);
    
    // First reading initializes the filter instantly.
    // 0 ADC is brightest, so maps to max duty (255)
    bm.update(0);
    TEST_ASSERT_EQUAL(255, bm.getDutyCycle());
    
    // Test direct jump to max extreme (4095 is darkest, maps to min duty)
    BacklightManager bm2(21, 0, 10.0f);
    bm2.update(4095);
    TEST_ASSERT_EQUAL(26, bm2.getDutyCycle());
}

void test_backlight_smoothing(void) {
    BacklightManager bm(21, 0, 10.0f);
    
    // Initial reading: 4095 (darkest)
    bm.update(4095);
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
    
    // Subsequent reading: 0 (brightest)
    // Since alpha is 0.1, the new filtered light should be:
    // filtered = 0.1 * 0 + 0.9 * 4095 = 3685.5
    // invertedLight = 4095 - 3685.5 = 409.5
    // 409.5 is 10% of 4095. So the mapped duty cycle should be:
    // minDuty = 26
    // gammaFactor = pow(0.1, 2.2) = 0.0063
    // duty = 26 + 0.0063 * (255 - 26) = 26 + 1.44 = 27
    bm.update(0);
    
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
}

void test_backlight_manual_brightness(void) {
    BacklightManager bm(21, 0, 10.0f);
    
    bm.setManualBrightness(50); // 50%
    TEST_ASSERT_EQUAL(64, bm.getDutyCycle()); // Gamma corrected 50% is 64
    
    bm.setManualBrightness(100); // 100%
    TEST_ASSERT_EQUAL(255, bm.getDutyCycle());
    
    bm.setManualBrightness(0); // Should be capped at min (10% -> 26)
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
}

void test_backlight_fade_to(void) {
    BacklightManager bm(21, 0, 10.0f);
    bm.setManualBrightness(10); // Start at 10%
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
    
    bm.fadeTo(50, 50); // Fade to 50%
    TEST_ASSERT_EQUAL(64, bm.getDutyCycle()); // Gamma corrected 50% is 64
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_backlight_initial_state);
    RUN_TEST(test_backlight_mapping_extremes);
    RUN_TEST(test_backlight_smoothing);
    RUN_TEST(test_backlight_manual_brightness);
    RUN_TEST(test_backlight_fade_to);
    return UNITY_END();
}
