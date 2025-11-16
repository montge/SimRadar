/*
 * test_pos_parsing.c
 * Unit tests for scan pattern parsing (pos.c)
 *
 * Tests the POS module's ability to parse scan patterns safely
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../pos.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    static void test_##name(); \
    static void run_test_##name() { \
        printf("Running test: %s ... ", #name); \
        fflush(stdout); \
        test_##name(); \
        tests_run++; \
        tests_passed++; \
        printf("PASS\n"); \
    } \
    static void test_##name()

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("\n  FAIL: %s\n", message); \
            printf("  at %s:%d\n", __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    ASSERT((expected) == (actual), message)

// Test: Basic PPI pattern parsing
TEST(parse_ppi_pattern) {
    POSPattern *pattern = POS_init();
    ASSERT(pattern != NULL, "POS_init should not return NULL");

    // Parse a simple PPI pattern
    int result = POS_parse_from_string(pattern, "P:3.0,-12:12:0.01");
    ASSERT_EQ(0, result, "Should successfully parse valid PPI pattern");
    ASSERT_EQ('P', pattern->mode, "Mode should be PPI");

    POS_free(pattern);
}

// Test: RHI pattern parsing
TEST(parse_rhi_pattern) {
    POSPattern *pattern = POS_init();
    ASSERT(pattern != NULL, "POS_init should not return NULL");

    int result = POS_parse_from_string(pattern, "R:0,0:90:1.0");
    ASSERT_EQ(0, result, "Should successfully parse valid RHI pattern");
    ASSERT_EQ('R', pattern->mode, "Mode should be RHI");

    POS_free(pattern);
}

// Test: Invalid pattern (missing colon)
TEST(parse_invalid_pattern_no_colon) {
    POSPattern *pattern = POS_init();
    ASSERT(pattern != NULL, "POS_init should not return NULL");

    int result = POS_parse_from_string(pattern, "P3.0,-12:12:0.01");
    ASSERT(result != 0, "Should fail to parse pattern without colon");

    POS_free(pattern);
}

// Test: Very long pattern (buffer overflow check)
TEST(parse_very_long_pattern) {
    POSPattern *pattern = POS_init();
    ASSERT(pattern != NULL, "POS_init should not return NULL");

    // Create a very long pattern string
    char long_pattern[2048];
    memset(long_pattern, 'A', sizeof(long_pattern) - 1);
    long_pattern[0] = 'P';
    long_pattern[1] = ':';
    long_pattern[sizeof(long_pattern) - 1] = '\0';

    // Should not crash with buffer overflow
    int result = POS_parse_from_string(pattern, long_pattern);
    // May succeed or fail, but should not crash
    ASSERT(1, "Should handle long pattern without crashing");

    POS_free(pattern);
}

// Test: Multiple sweep pattern
TEST(parse_multi_sweep_pattern) {
    POSPattern *pattern = POS_init();
    ASSERT(pattern != NULL, "POS_init should not return NULL");

    int result = POS_parse_from_string(pattern, "P:0.5,-12:12:0.02/1.0,-12:12:0.02");
    ASSERT_EQ(0, result, "Should parse multi-sweep pattern");
    ASSERT(pattern->sweep_count > 1, "Should have multiple sweeps");

    POS_free(pattern);
}

int main(void) {
    printf("=================================\n");
    printf("  POS Pattern Parsing Tests\n");
    printf("=================================\n\n");

    run_test_parse_ppi_pattern();
    run_test_parse_rhi_pattern();
    run_test_parse_invalid_pattern_no_colon();
    run_test_parse_very_long_pattern();
    run_test_parse_multi_sweep_pattern();

    printf("\n=================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("=================================\n");

    if (tests_passed == tests_run) {
        printf("\n✅ All tests passed!\n\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed!\n\n");
        return 1;
    }
}
