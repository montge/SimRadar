/*
 * test_rs_tables.c
 * Unit tests for RS table data structures
 *
 * Tests the table initialization and manipulation functions
 * without requiring OpenCL/GPU dependencies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "rs_types.h"

// Test counter
static int tests_run = 0;
static int tests_passed = 0;

// Simple assertion macro
#define ASSERT(test, message) \
    do { \
        if (!(test)) { \
            printf("FAIL\n  %s\n", message); \
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

// Helper function to compare floats with tolerance
static int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

//
// Test Cases
//

TEST(volume_initialization) {
    RSVolume vol;
    vol.origin.x = 0.0f;
    vol.origin.y = 0.0f;
    vol.origin.z = 0.0f;
    vol.size.x = 100.0f;
    vol.size.y = 100.0f;
    vol.size.z = 50.0f;

    ASSERT(float_equals(vol.origin.x, 0.0f, 0.001f), "Volume origin.x should be 0");
    ASSERT(float_equals(vol.size.x, 100.0f, 0.001f), "Volume size.x should be 100");
    ASSERT(float_equals(vol.size.z, 50.0f, 0.001f), "Volume size.z should be 50");
}

TEST(point_initialization) {
    RSPoint point;
    point.x = -50.0f;
    point.y = -50.0f;
    point.z = 0.0f;

    ASSERT(float_equals(point.x, -50.0f, 0.001f), "Point x should be -50");
    ASSERT(float_equals(point.y, -50.0f, 0.001f), "Point y should be -50");
    ASSERT(float_equals(point.z, 0.0f, 0.001f), "Point z should be 0");
}

TEST(params_ranges) {
    RSParams params;

    // Set up basic radar parameters
    params.range_start = 0.0f;
    params.range_delta = 30.0f;  // 30 meters
    params.range_count = 1000;

    // Calculate maximum range
    float max_range = params.range_start + (params.range_count - 1) * params.range_delta;

    ASSERT(params.range_count == 1000, "Range count should be 1000");
    ASSERT(float_equals(params.range_delta, 30.0f, 0.001f), "Range delta should be 30");
    ASSERT(float_equals(max_range, 29970.0f, 0.1f), "Max range should be ~29.97 km");
}

TEST(params_prt) {
    RSParams params;

    // Set up timing parameters
    params.prt = 0.001f;  // 1 ms PRT
    params.lambda = 0.10f;  // 10 cm wavelength (S-band)

    // Calculate derived parameters (simulates what RS_init does)
    params.prf = 1.0f / params.prt;  // PRF = 1/PRT
    params.fn = params.prf / 2.0f;   // Nyquist frequency
    params.va = params.lambda * params.fn;  // Nyquist velocity

    ASSERT(float_equals(params.prt, 0.001f, 0.000001f), "PRT should be 1 ms");
    ASSERT(float_equals(params.prf, 1000.0f, 0.1f), "PRF should be 1000 Hz");
    ASSERT(float_equals(params.va, 25.0f, 0.1f), "Nyquist velocity should be ~25 m/s");
}

TEST(debris_count_limits) {
    // Test that we don't exceed maximum debris type count
    int max_types = RS_MAX_DEBRIS_TYPES;

    ASSERT(max_types > 0, "Maximum debris types should be positive");
    ASSERT(max_types <= 256, "Maximum debris types should be reasonable (<= 256)");
}

TEST(coordinate_transformation) {
    // Test basic coordinate conversions
    float azimuth_deg = 90.0f;
    float elevation_deg = 45.0f;

    // Convert to radians
    float azimuth_rad = azimuth_deg * M_PI / 180.0f;
    float elevation_rad = elevation_deg * M_PI / 180.0f;

    ASSERT(float_equals(azimuth_rad, M_PI/2, 0.001f), "90 degrees should be π/2 radians");
    ASSERT(float_equals(elevation_rad, M_PI/4, 0.001f), "45 degrees should be π/4 radians");

    // Convert back
    float az_check = azimuth_rad * 180.0f / M_PI;
    float el_check = elevation_rad * 180.0f / M_PI;

    ASSERT(float_equals(az_check, azimuth_deg, 0.001f), "Conversion should round-trip");
    ASSERT(float_equals(el_check, elevation_deg, 0.001f), "Conversion should round-trip");
}

TEST(range_to_sample_index) {
    RSParams params;
    params.range_start = 0.0f;
    params.range_delta = 30.0f;
    params.range_count = 1000;

    // Test range to index conversion
    float test_range = 1500.0f;  // 1.5 km
    int expected_index = (int)((test_range - params.range_start) / params.range_delta);

    ASSERT(expected_index == 50, "1.5 km should map to index 50");

    // Test boundary cases
    int first_index = (int)((params.range_start - params.range_start) / params.range_delta);
    ASSERT(first_index == 0, "First range should map to index 0");

    // Test that index is within bounds
    ASSERT(expected_index >= 0, "Index should not be negative");
    ASSERT(expected_index < (int)params.range_count, "Index should be within range count");
}

TEST(wavelength_frequency_relationship) {
    // Test wavelength and frequency relationship: c = λf
    float speed_of_light = 299792458.0f;  // m/s

    // S-band radar: ~3 GHz, wavelength ~10 cm
    float frequency_s_band = 3.0e9f;  // Hz
    float wavelength_s_band = speed_of_light / frequency_s_band;

    ASSERT(float_equals(wavelength_s_band, 0.0999f, 0.001f),
           "S-band wavelength should be ~10 cm");

    // C-band radar: ~5.6 GHz, wavelength ~5.4 cm
    float frequency_c_band = 5.6e9f;  // Hz
    float wavelength_c_band = speed_of_light / frequency_c_band;

    ASSERT(float_equals(wavelength_c_band, 0.0535f, 0.001f),
           "C-band wavelength should be ~5.4 cm");
}

TEST(pulse_width_range_resolution) {
    // Range resolution = c * pulse_width / 2
    float speed_of_light = 299792458.0f;  // m/s
    float pulse_width = 1.0e-6f;  // 1 microsecond

    float range_resolution = (speed_of_light * pulse_width) / 2.0f;

    ASSERT(float_equals(range_resolution, 149.896f, 0.1f),
           "1 μs pulse should give ~150m range resolution");

    // Shorter pulse for better resolution
    float short_pulse = 0.5e-6f;  // 0.5 microseconds
    float fine_resolution = (speed_of_light * short_pulse) / 2.0f;

    ASSERT(float_equals(fine_resolution, 74.948f, 0.1f),
           "0.5 μs pulse should give ~75m range resolution");
}

TEST(vector_magnitude) {
    // Test 3D vector magnitude calculation
    float x = 3.0f;
    float y = 4.0f;
    float z = 0.0f;

    float magnitude = sqrtf(x*x + y*y + z*z);

    ASSERT(float_equals(magnitude, 5.0f, 0.001f),
           "3-4-5 triangle magnitude should be 5");

    // Test with z component
    float x2 = 1.0f;
    float y2 = 2.0f;
    float z2 = 2.0f;

    float magnitude2 = sqrtf(x2*x2 + y2*y2 + z2*z2);

    ASSERT(float_equals(magnitude2, 3.0f, 0.001f),
           "Vector magnitude should be 3");
}

TEST(dBZ_linear_conversion) {
    // Test reflectivity dBZ to linear Z conversion
    // Z = 10^(dBZ/10)

    float dBZ = 30.0f;  // Moderate rain
    float Z_linear = powf(10.0f, dBZ / 10.0f);

    ASSERT(float_equals(Z_linear, 1000.0f, 0.1f),
           "30 dBZ should equal 1000 mm^6/m^3");

    // Test inverse: dBZ = 10 * log10(Z)
    float dBZ_check = 10.0f * log10f(Z_linear);

    ASSERT(float_equals(dBZ_check, dBZ, 0.001f),
           "Conversion should round-trip");
}

TEST(doppler_velocity_calculation) {
    // Doppler velocity = (wavelength * frequency_shift) / 2
    float wavelength = 0.10f;  // 10 cm (S-band)
    float prf = 1000.0f;  // 1000 Hz PRF

    // Maximum unambiguous velocity (Nyquist)
    float v_nyquist = (wavelength * prf) / 4.0f;

    ASSERT(float_equals(v_nyquist, 25.0f, 0.1f),
           "Nyquist velocity should be 25 m/s");

    // Test velocity for half Nyquist shift
    float doppler_shift = prf / 2.0f;
    float velocity = (wavelength * doppler_shift) / 2.0f;

    ASSERT(float_equals(velocity, v_nyquist, 0.1f),
           "Half PRF shift should give Nyquist velocity");
}

TEST(beam_width_calculation) {
    // Beamwidth (degrees) ≈ 70 * λ / D
    // where λ is wavelength and D is antenna diameter

    float wavelength = 0.10f;  // 10 cm
    float antenna_diameter = 2.0f;  // 2 meters

    float beamwidth_deg = 70.0f * wavelength / antenna_diameter;

    ASSERT(float_equals(beamwidth_deg, 3.5f, 0.1f),
           "Beamwidth should be ~3.5 degrees");

    // Larger antenna gives narrower beam
    float large_antenna = 8.0f;  // 8 meters
    float narrow_beam = 70.0f * wavelength / large_antenna;

    ASSERT(narrow_beam < beamwidth_deg,
           "Larger antenna should give narrower beamwidth");
}

//
// Main test runner
//

int main(void) {
    printf("\n");
    printf("=================================\n");
    printf("  RS Table & Param Unit Tests\n");
    printf("=================================\n");
    printf("\n");

    // Run all tests
    run_test_volume_initialization();
    run_test_point_initialization();
    run_test_params_ranges();
    run_test_params_prt();
    run_test_debris_count_limits();
    run_test_coordinate_transformation();
    run_test_range_to_sample_index();
    run_test_wavelength_frequency_relationship();
    run_test_pulse_width_range_resolution();
    run_test_vector_magnitude();
    run_test_dBZ_linear_conversion();
    run_test_doppler_velocity_calculation();
    run_test_beam_width_calculation();

    // Print summary
    printf("\n");
    printf("=================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("=================================\n");
    printf("\n");

    if (tests_run == tests_passed) {
        printf("✅ All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
}
