/*
 * test_data_integration.c
 * Integration tests for LES/ADM/RCS data loading
 *
 * Tests actual data loader functions with data availability checks.
 * Skips tests if required data files are not available.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include "les.h"
#include "adm.h"
#include "rcs.h"

// Test counter
static int tests_run = 0;
static int tests_passed = 0;
static int tests_skipped = 0;

// Data availability flags
static bool les_data_available = false;
static bool adm_data_available = false;
static bool rcs_data_available = false;

// Simple assertion macro
#define ASSERT(test, message) \
    do { \
        if (!(test)) { \
            printf("FAIL\n  %s\n", message); \
            return; \
        } \
    } while(0)

// Skip macros for data tests
#define SKIP_IF_NO_LES_DATA() \
    do { \
        if (!les_data_available) { \
            printf("SKIP (no LES data)\n"); \
            tests_skipped++; \
            return; \
        } \
    } while(0)

#define SKIP_IF_NO_ADM_DATA() \
    do { \
        if (!adm_data_available) { \
            printf("SKIP (no ADM data)\n"); \
            tests_skipped++; \
            return; \
        } \
    } while(0)

#define SKIP_IF_NO_RCS_DATA() \
    do { \
        if (!rcs_data_available) { \
            printf("SKIP (no RCS data)\n"); \
            tests_skipped++; \
            return; \
        } \
    } while(0)

// Test wrapper macro
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

//
// Helper Functions
//

static bool check_directory_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static bool check_data_available(void) {
    // Check common locations for data tables
    const char *home = getenv("HOME");
    if (!home) return false;

    char path[2048];

    // Check LES data
    snprintf(path, sizeof(path), "%s/Downloads/tables/les", home);
    if (check_directory_exists(path)) {
        les_data_available = true;
    } else {
        snprintf(path, sizeof(path), "%s/Documents/tables/les", home);
        if (check_directory_exists(path)) {
            les_data_available = true;
        } else {
            snprintf(path, sizeof(path), "%s/Desktop/tables/les", home);
            if (check_directory_exists(path)) {
                les_data_available = true;
            }
        }
    }

    // Check ADM data
    snprintf(path, sizeof(path), "%s/Downloads/tables/adm", home);
    if (check_directory_exists(path)) {
        adm_data_available = true;
    } else {
        snprintf(path, sizeof(path), "%s/Documents/tables/adm", home);
        if (check_directory_exists(path)) {
            adm_data_available = true;
        } else {
            snprintf(path, sizeof(path), "%s/Desktop/tables/adm", home);
            if (check_directory_exists(path)) {
                adm_data_available = true;
            }
        }
    }

    // Check RCS data
    snprintf(path, sizeof(path), "%s/Downloads/tables/rcs", home);
    if (check_directory_exists(path)) {
        rcs_data_available = true;
    } else {
        snprintf(path, sizeof(path), "%s/Documents/tables/rcs", home);
        if (check_directory_exists(path)) {
            rcs_data_available = true;
        } else {
            snprintf(path, sizeof(path), "%s/Desktop/tables/rcs", home);
            if (check_directory_exists(path)) {
                rcs_data_available = true;
            }
        }
    }

    return les_data_available || adm_data_available || rcs_data_available;
}

//
// LES Tests
//

TEST(les_init_default) {
    SKIP_IF_NO_LES_DATA();

    LESHandle handle = LES_init();
    ASSERT(handle != NULL, "LES_init() should return valid handle");

    LES_free(handle);
}

TEST(les_data_path) {
    SKIP_IF_NO_LES_DATA();

    LESHandle handle = LES_init();
    ASSERT(handle != NULL, "LES_init() should succeed");

    char *path = LES_data_path(handle);
    ASSERT(path != NULL, "LES_data_path() should return valid path");
    ASSERT(strlen(path) > 0, "Path should not be empty");
    ASSERT(check_directory_exists(path), "Path should exist");

    LES_free(handle);
}

TEST(les_table_count) {
    SKIP_IF_NO_LES_DATA();

    LESHandle handle = LES_init();
    ASSERT(handle != NULL, "LES_init() should succeed");

    size_t count = LES_get_table_count(handle);
    ASSERT(count > 0, "Should have at least one LES table");

    LES_free(handle);
}

TEST(les_table_period) {
    SKIP_IF_NO_LES_DATA();

    LESHandle handle = LES_init();
    ASSERT(handle != NULL, "LES_init() should succeed");

    float period = LES_get_table_period(handle);
    ASSERT(period > 0.0f, "Table period should be positive");

    LES_free(handle);
}

TEST(les_get_frame) {
    SKIP_IF_NO_LES_DATA();

    LESHandle handle = LES_init();
    ASSERT(handle != NULL, "LES_init() should succeed");

    // Try to get first frame
    LESTable *table = LES_get_frame(handle, 0);
    ASSERT(table != NULL, "Should be able to get first frame");

    // Validate table structure
    ASSERT(table->nx > 0, "Grid should have positive x dimension");
    ASSERT(table->ny > 0, "Grid should have positive y dimension");
    ASSERT(table->nz > 0, "Grid should have positive z dimension");
    ASSERT(table->nn > 0, "Total grid points should be positive");

    // Data arrays should be allocated
    ASSERT(table->data.u != NULL, "u velocity data should be allocated");
    ASSERT(table->data.v != NULL, "v velocity data should be allocated");
    ASSERT(table->data.w != NULL, "w velocity data should be allocated");

    LES_free(handle);
}

TEST(les_multiple_frames) {
    SKIP_IF_NO_LES_DATA();

    LESHandle handle = LES_init();
    ASSERT(handle != NULL, "LES_init() should succeed");

    size_t count = LES_get_table_count(handle);

    // Get first and last frames
    LESTable *first = LES_get_frame(handle, 0);
    ASSERT(first != NULL, "Should get first frame");

    if (count > 1) {
        LESTable *second = LES_get_frame(handle, 1);
        ASSERT(second != NULL, "Should get second frame");

        // Dimensions should be the same across frames
        ASSERT(first->nx == second->nx, "Frame dimensions should match");
        ASSERT(first->ny == second->ny, "Frame dimensions should match");
        ASSERT(first->nz == second->nz, "Frame dimensions should match");
    }

    LES_free(handle);
}

//
// ADM Tests
//

TEST(adm_init_default) {
    SKIP_IF_NO_ADM_DATA();

    ADMHandle handle = ADM_init();
    ASSERT(handle != NULL, "ADM_init() should return valid handle");

    ADM_free(handle);
}

TEST(adm_data_path) {
    SKIP_IF_NO_ADM_DATA();

    ADMHandle handle = ADM_init();
    ASSERT(handle != NULL, "ADM_init() should succeed");

    char *path = ADM_data_path(handle);
    ASSERT(path != NULL, "ADM_data_path() should return valid path");
    ASSERT(strlen(path) > 0, "Path should not be empty");
    ASSERT(check_directory_exists(path), "Path should exist");

    ADM_free(handle);
}

TEST(adm_get_table_plate) {
    SKIP_IF_NO_ADM_DATA();

    ADMHandle handle = ADM_init();
    ASSERT(handle != NULL, "ADM_init() should succeed");

    ADMTable *table = ADM_get_table(handle, ADMConfigModelPlate);

    if (table != NULL) {
        // Validate table structure
        ASSERT(table->nb > 0, "Beta dimension should be positive");
        ASSERT(table->na > 0, "Alpha dimension should be positive");
        ASSERT(table->nn > 0, "Total grid points should be positive");
        ASSERT(table->nn == table->nb * table->na, "nn should equal nb * na");

        // Physical parameters should be set
        ASSERT(table->phys.mass > 0.0f, "Debris mass should be positive");
        ASSERT(table->phys.rho > 0.0f, "Density should be positive");

        // Data arrays should be allocated
        ASSERT(table->data.cdx != NULL, "Cdx data should be allocated");
        ASSERT(table->data.cdy != NULL, "Cdy data should be allocated");
        ASSERT(table->data.cdz != NULL, "Cdz data should be allocated");
        ASSERT(table->data.cmx != NULL, "Cmx data should be allocated");
        ASSERT(table->data.cmy != NULL, "Cmy data should be allocated");
        ASSERT(table->data.cmz != NULL, "Cmz data should be allocated");
    } else {
        printf("SKIP (plate data not found) ... ");
        tests_skipped++;
        tests_passed--;
    }

    ADM_free(handle);
}

TEST(adm_get_table_square_plate) {
    SKIP_IF_NO_ADM_DATA();

    ADMHandle handle = ADM_init();
    ASSERT(handle != NULL, "ADM_init() should succeed");

    ADMTable *table = ADM_get_table(handle, ADMConfigSquarePlate);

    if (table != NULL) {
        ASSERT(table->nb > 0, "Should have valid dimensions");
        ASSERT(table->na > 0, "Should have valid dimensions");
    } else {
        printf("SKIP (square_plate data not found) ... ");
        tests_skipped++;
        tests_passed--;
    }

    ADM_free(handle);
}

//
// RCS Tests
//

TEST(rcs_init_default) {
    SKIP_IF_NO_RCS_DATA();

    RCSHandle handle = RCS_init();
    ASSERT(handle != NULL, "RCS_init() should return valid handle");

    RCS_free(handle);
}

TEST(rcs_data_path) {
    SKIP_IF_NO_RCS_DATA();

    RCSHandle handle = RCS_init();
    ASSERT(handle != NULL, "RCS_init() should succeed");

    char *path = RCS_data_path(handle);
    ASSERT(path != NULL, "RCS_data_path() should return valid path");
    ASSERT(strlen(path) > 0, "Path should not be empty");
    ASSERT(check_directory_exists(path), "Path should exist");

    RCS_free(handle);
}

TEST(rcs_get_table_leaf) {
    SKIP_IF_NO_RCS_DATA();

    RCSHandle handle = RCS_init();
    ASSERT(handle != NULL, "RCS_init() should succeed");

    RCSTable *table = RCS_get_table(handle, RCSConfigLeaf);

    if (table != NULL) {
        // Validate table structure
        ASSERT(table->na > 0, "Alpha dimension should be positive");
        ASSERT(table->nb > 0, "Beta dimension should be positive");
        ASSERT(table->nn > 0, "Total grid points should be positive");
        ASSERT(table->nn == table->na * table->nb, "nn should equal na * nb");

        // Wavelength should be set
        ASSERT(table->lambda > 0.0f, "Wavelength should be positive");

        // Data arrays should be allocated (all polarizations, real and imag)
        ASSERT(table->data.hh_real != NULL, "HH real data should be allocated");
        ASSERT(table->data.hh_imag != NULL, "HH imag data should be allocated");
        ASSERT(table->data.vv_real != NULL, "VV real data should be allocated");
        ASSERT(table->data.vv_imag != NULL, "VV imag data should be allocated");
        ASSERT(table->data.hv_real != NULL, "HV real data should be allocated");
        ASSERT(table->data.hv_imag != NULL, "HV imag data should be allocated");
    } else {
        printf("SKIP (leaf data not found) ... ");
        tests_skipped++;
        tests_passed--;
    }

    RCS_free(handle);
}

TEST(rcs_get_table_plate) {
    SKIP_IF_NO_RCS_DATA();

    RCSHandle handle = RCS_init();
    ASSERT(handle != NULL, "RCS_init() should succeed");

    RCSTable *table = RCS_get_table(handle, RCSConfigPlate);

    if (table != NULL) {
        ASSERT(table->na > 0, "Should have valid dimensions");
        ASSERT(table->nb > 0, "Should have valid dimensions");
        ASSERT(table->lambda > 0.0f, "Wavelength should be set");
    } else {
        printf("SKIP (plate RCS data not found) ... ");
        tests_skipped++;
        tests_passed--;
    }

    RCS_free(handle);
}

//
// Cross-module Tests
//

TEST(data_handle_null_safety) {
    // Test that functions handle NULL gracefully
    LES_free(NULL);
    ADM_free(NULL);
    RCS_free(NULL);

    // This test passes if we get here without crashing
}

TEST(multiple_handle_init) {
    // Test that we can initialize multiple handles
    if (les_data_available) {
        LESHandle les1 = LES_init();
        LESHandle les2 = LES_init();
        ASSERT(les1 != NULL, "First LES handle should be valid");
        ASSERT(les2 != NULL, "Second LES handle should be valid");
        LES_free(les1);
        LES_free(les2);
    }

    if (adm_data_available) {
        ADMHandle adm1 = ADM_init();
        ADMHandle adm2 = ADM_init();
        ASSERT(adm1 != NULL, "First ADM handle should be valid");
        ASSERT(adm2 != NULL, "Second ADM handle should be valid");
        ADM_free(adm1);
        ADM_free(adm2);
    }

    if (rcs_data_available) {
        RCSHandle rcs1 = RCS_init();
        RCSHandle rcs2 = RCS_init();
        ASSERT(rcs1 != NULL, "First RCS handle should be valid");
        ASSERT(rcs2 != NULL, "Second RCS handle should be valid");
        RCS_free(rcs1);
        RCS_free(rcs2);
    }

    if (!les_data_available && !adm_data_available && !rcs_data_available) {
        printf("SKIP (no data available) ... ");
        tests_skipped++;
        tests_passed--;
    }
}

//
// Main test runner
//

int main(void) {
    printf("\n");
    printf("=================================\n");
    printf(" Data Loader Integration Tests\n");
    printf("=================================\n");
    printf("\n");

    // Check data availability
    printf("Checking data availability... ");
    check_data_available();
    printf("\n");
    printf("  LES data: %s\n", les_data_available ? "✓ Available" : "✗ Not found");
    printf("  ADM data: %s\n", adm_data_available ? "✓ Available" : "✗ Not found");
    printf("  RCS data: %s\n", rcs_data_available ? "✓ Available" : "✗ Not found");
    printf("\n");

    if (!les_data_available && !adm_data_available && !rcs_data_available) {
        printf("⚠️  No data files found. Install data to:\n");
        printf("   ~/Downloads/tables/{les,adm,rcs}/\n");
        printf("   ~/Documents/tables/{les,adm,rcs}/\n");
        printf("   ~/Desktop/tables/{les,adm,rcs}/\n");
        printf("\n");
        printf("Download: http://arrc.ou.edu/~boonleng/simradar/tables.zip (15GB)\n");
        printf("\n");
    }

    // Run all tests
    run_test_les_init_default();
    run_test_les_data_path();
    run_test_les_table_count();
    run_test_les_table_period();
    run_test_les_get_frame();
    run_test_les_multiple_frames();

    run_test_adm_init_default();
    run_test_adm_data_path();
    run_test_adm_get_table_plate();
    run_test_adm_get_table_square_plate();

    run_test_rcs_init_default();
    run_test_rcs_data_path();
    run_test_rcs_get_table_leaf();
    run_test_rcs_get_table_plate();

    run_test_data_handle_null_safety();
    run_test_multiple_handle_init();

    // Print summary
    printf("\n");
    printf("=================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests skipped: %d\n", tests_skipped);
    printf("Tests failed: %d\n", tests_run - tests_passed - tests_skipped);
    printf("=================================\n");
    printf("\n");

    if (tests_run == tests_passed + tests_skipped) {
        printf("✅ All tests passed (or skipped)!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
}
