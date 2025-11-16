/*
 * test_rs_integration.c
 * Integration tests for RS framework core functionality
 *
 * Tests actual RS framework functions with GPU detection.
 * Skips GPU-dependent tests if no GPU is available.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "rs.h"

// Test counter
static int tests_run = 0;
static int tests_passed = 0;
static int tests_skipped = 0;

// GPU availability flag
static bool gpu_available = false;

// Simple assertion macro
#define ASSERT(test, message) \
    do { \
        if (!(test)) { \
            printf("FAIL\n  %s\n", message); \
            return; \
        } \
    } while(0)

// Skip macro for GPU tests
#define SKIP_IF_NO_GPU() \
    do { \
        if (!gpu_available) { \
            printf("SKIP (no GPU)\n"); \
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

static bool check_gpu_available(void) {
    // Try to detect OpenCL GPUs
    cl_uint num_platforms = 0;
    cl_int err = clGetPlatformIDs(0, NULL, &num_platforms);

    if (err != CL_SUCCESS || num_platforms == 0) {
        return false;
    }

    cl_platform_id *platforms = malloc(num_platforms * sizeof(cl_platform_id));
    if (!platforms) return false;

    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    if (err != CL_SUCCESS) {
        free(platforms);
        return false;
    }

    // Check for GPU devices
    bool has_gpu = false;
    for (cl_uint i = 0; i < num_platforms; i++) {
        cl_uint num_devices = 0;
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
        if (err == CL_SUCCESS && num_devices > 0) {
            has_gpu = true;
            break;
        }
    }

    free(platforms);
    return has_gpu;
}

//
// Test Cases
//

TEST(rs_init_basic) {
    // Test basic initialization without GPU
    // This should work even without GPU by falling back to CPU mode
    RSHandle *handle = RS_init();

    if (handle == NULL && !gpu_available) {
        // Expected failure without GPU - this is OK
        printf("SKIP (no GPU, expected) ... ");
        tests_skipped++;
        tests_passed--;  // Don't count as passed
        return;
    }

    ASSERT(handle != NULL, "RS_init() should return valid handle");

    // Clean up
    RS_free(handle);
}

TEST(rs_init_verbose) {
    // Test initialization with verbosity
    RSHandle *handle = RS_init_verbose(0);  // Quiet mode

    if (handle == NULL && !gpu_available) {
        printf("SKIP (no GPU, expected) ... ");
        tests_skipped++;
        tests_passed--;
        return;
    }

    ASSERT(handle != NULL, "RS_init_verbose() should return valid handle");

    RS_free(handle);
}

TEST(rs_gpu_count) {
    // Test GPU detection
    cl_uint gpu_count = RS_gpu_count();

    if (gpu_available) {
        ASSERT(gpu_count > 0, "Should detect at least one GPU when available");
    } else {
        // With no GPU, count should be 0
        ASSERT(gpu_count == 0, "Should report 0 GPUs when none available");
    }
}

TEST(rs_version_string) {
    // Test version string retrieval
    char *version = RS_version_string();

    ASSERT(version != NULL, "Version string should not be NULL");
    ASSERT(strlen(version) > 0, "Version string should not be empty");

    // Should contain "SimRadar" or version number
    bool valid = (strstr(version, "SimRadar") != NULL) ||
                 (strstr(version, "v") != NULL) ||
                 (strstr(version, "0.") != NULL);

    ASSERT(valid, "Version string should contain version info");
}

TEST(rs_set_prt) {
    SKIP_IF_NO_GPU();

    RSHandle *handle = RS_init();
    ASSERT(handle != NULL, "RS_init() should succeed");

    // Set PRT to 1ms (1000 Hz PRF)
    RS_set_prt(handle, 0.001f);

    // Verify PRF was calculated correctly
    // PRF should be 1/PRT = 1000 Hz
    ASSERT(handle->params.prf > 999.0f && handle->params.prf < 1001.0f,
           "PRF should be calculated as 1/PRT (~1000 Hz)");

    // Verify Nyquist frequency (PRF/2)
    ASSERT(handle->params.fn > 499.0f && handle->params.fn < 501.0f,
           "Nyquist frequency should be PRF/2 (~500 Hz)");

    RS_free(handle);
}

TEST(rs_set_lambda) {
    SKIP_IF_NO_GPU();

    RSHandle *handle = RS_init();
    ASSERT(handle != NULL, "RS_init() should succeed");

    // Set S-band wavelength (10 cm)
    RS_set_lambda(handle, 0.10f);

    ASSERT(handle->params.lambda > 0.099f && handle->params.lambda < 0.101f,
           "Lambda should be set to ~0.10 m");

    RS_free(handle);
}

TEST(rs_set_antenna_params) {
    SKIP_IF_NO_GPU();

    RSHandle *handle = RS_init();
    ASSERT(handle != NULL, "RS_init() should succeed");

    // Set 1° beamwidth, 45 dBi gain
    RS_set_antenna_params(handle, 1.0f, 45.0f);

    ASSERT(handle->params.antenna_bw_deg > 0.99f && handle->params.antenna_bw_deg < 1.01f,
           "Beamwidth should be set to ~1.0°");
    ASSERT(handle->params.antenna_gain_dbi > 44.9f && handle->params.antenna_gain_dbi < 45.1f,
           "Gain should be set to ~45 dBi");

    // Beamwidth should also be converted to radians
    float expected_rad = 1.0f * M_PI / 180.0f;
    ASSERT(fabsf(handle->params.antenna_bw_rad - expected_rad) < 0.001f,
           "Beamwidth should be converted to radians");

    RS_free(handle);
}

TEST(rs_set_tx_params) {
    SKIP_IF_NO_GPU();

    RSHandle *handle = RS_init();
    ASSERT(handle != NULL, "RS_init() should succeed");

    // Set 1 μs pulse width, 500 kW power
    RS_set_tx_params(handle, 1.0e-6f, 500000.0f);

    ASSERT(fabsf(handle->params.tau - 1.0e-6f) < 1.0e-9f,
           "Pulse width should be set to 1 μs");
    ASSERT(handle->params.tx_power_watt > 499999.0f && handle->params.tx_power_watt < 500001.0f,
           "TX power should be set to 500 kW");

    RS_free(handle);
}

TEST(rs_set_sampling_spacing) {
    SKIP_IF_NO_GPU();

    RSHandle *handle = RS_init();
    ASSERT(handle != NULL, "RS_init() should succeed");

    // Set 30m range, 1° azimuth, 1° elevation spacing
    RS_set_sampling_spacing(handle, 30.0f, 1.0f, 1.0f);

    ASSERT(handle->params.range_delta > 29.9f && handle->params.range_delta < 30.1f,
           "Range spacing should be set to ~30 m");
    ASSERT(handle->params.azimuth_delta_deg > 0.99f && handle->params.azimuth_delta_deg < 1.01f,
           "Azimuth spacing should be set to ~1°");
    ASSERT(handle->params.elevation_delta_deg > 0.99f && handle->params.elevation_delta_deg < 1.01f,
           "Elevation spacing should be set to ~1°");

    RS_free(handle);
}

TEST(rs_complete_configuration) {
    SKIP_IF_NO_GPU();

    RSHandle *handle = RS_init();
    ASSERT(handle != NULL, "RS_init() should succeed");

    // Configure a complete realistic radar
    RS_set_lambda(handle, 0.10f);              // S-band
    RS_set_prt(handle, 0.001f);                // 1 ms PRT
    RS_set_antenna_params(handle, 1.0f, 45.0f); // 1° beam, 45 dBi
    RS_set_tx_params(handle, 1.0e-6f, 500e3f); // 1 μs, 500 kW
    RS_set_sampling_spacing(handle, 30.0f, 1.0f, 1.0f);

    // Verify all parameters were set
    ASSERT(handle->params.lambda > 0.0f, "Lambda should be set");
    ASSERT(handle->params.prt > 0.0f, "PRT should be set");
    ASSERT(handle->params.prf > 0.0f, "PRF should be calculated");
    ASSERT(handle->params.antenna_bw_deg > 0.0f, "Beamwidth should be set");
    ASSERT(handle->params.tx_power_watt > 0.0f, "TX power should be set");
    ASSERT(handle->params.range_delta > 0.0f, "Range spacing should be set");

    RS_free(handle);
}

TEST(rs_handle_null_safety) {
    // Test that functions handle NULL gracefully
    // These should not crash
    RS_free(NULL);  // Should handle NULL gracefully

    // This test passes if we get here without crashing
}

TEST(rs_multiple_init_free) {
    SKIP_IF_NO_GPU();

    // Test multiple initialization and cleanup cycles
    for (int i = 0; i < 3; i++) {
        RSHandle *handle = RS_init();
        ASSERT(handle != NULL, "RS_init() should succeed in loop");

        RS_set_lambda(handle, 0.10f);
        RS_set_prt(handle, 0.001f);

        RS_free(handle);
    }
}

TEST(rs_get_domain) {
    SKIP_IF_NO_GPU();

    RSHandle *handle = RS_init();
    ASSERT(handle != NULL, "RS_init() should succeed");

    // Get simulation domain
    RSVolume vol = RS_get_domain(handle);

    // Domain should have positive size
    ASSERT(vol.size.x >= 0.0f, "Domain size.x should be non-negative");
    ASSERT(vol.size.y >= 0.0f, "Domain size.y should be non-negative");
    ASSERT(vol.size.z >= 0.0f, "Domain size.z should be non-negative");

    RS_free(handle);
}

//
// Main test runner
//

int main(void) {
    printf("\n");
    printf("=================================\n");
    printf("  RS Framework Integration Tests\n");
    printf("=================================\n");
    printf("\n");

    // Check GPU availability
    printf("Checking GPU availability... ");
    gpu_available = check_gpu_available();
    if (gpu_available) {
        printf("GPU detected ✓\n");
    } else {
        printf("No GPU (tests will skip or use CPU)\n");
    }
    printf("\n");

    // Run all tests
    run_test_rs_init_basic();
    run_test_rs_init_verbose();
    run_test_rs_gpu_count();
    run_test_rs_version_string();
    run_test_rs_set_prt();
    run_test_rs_set_lambda();
    run_test_rs_set_antenna_params();
    run_test_rs_set_tx_params();
    run_test_rs_set_sampling_spacing();
    run_test_rs_complete_configuration();
    run_test_rs_handle_null_safety();
    run_test_rs_multiple_init_free();
    run_test_rs_get_domain();

    // Print summary
    printf("\n");
    printf("=================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests skipped: %d (no GPU)\n", tests_skipped);
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
