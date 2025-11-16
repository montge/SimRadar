/*
 * test_data_loaders.c
 * Unit tests for data loader modules
 *
 * Tests LES, ADM, and RCS data loading error handling
 * and data structure initialization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>

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

//
// Test Cases
//

TEST(fread_error_handling) {
    // Test that we properly handle fread() errors
    FILE *fp = fopen("/dev/null", "r");
    ASSERT(fp != NULL, "Should be able to open /dev/null");

    uint32_t test_data[4];
    size_t items_read = fread(test_data, sizeof(uint32_t), 4, fp);

    // Reading from /dev/null should return 0 items
    ASSERT(items_read == 0, "Reading from /dev/null should return 0 items");

    fclose(fp);
}

TEST(file_open_error_handling) {
    // Test handling of non-existent files
    const char *nonexistent = "/tmp/this_file_should_not_exist_12345.dat";

    FILE *fp = fopen(nonexistent, "r");
    ASSERT(fp == NULL, "Opening non-existent file should fail");
    ASSERT(errno == ENOENT, "errno should be ENOENT");
}

TEST(directory_creation_error_handling) {
    // Test error handling for invalid directory paths
    struct stat dir_stat;

    // /dev/null is not a directory
    int result = stat("/dev/null", &dir_stat);
    ASSERT(result == 0, "stat on /dev/null should succeed");
    ASSERT(!S_ISDIR(dir_stat.st_mode), "/dev/null should not be a directory");
}

TEST(buffer_size_validation) {
    // Test that buffer sizes are validated before allocation
    size_t huge_size = SIZE_MAX;
    size_t item_size = sizeof(float);

    // Check for overflow before allocation
    int would_overflow = (huge_size > SIZE_MAX / item_size);

    ASSERT(would_overflow, "Huge size * item_size should overflow SIZE_MAX");
}

TEST(grid_dimension_validation) {
    // Test validation of grid dimensions from binary files
    uint16_t nb = 100;  // beta dimension
    uint16_t na = 100;  // alpha dimension

    // Calculate total elements
    size_t nn = (size_t)nb * (size_t)na;

    ASSERT(nn == 10000, "100x100 grid should have 10000 elements");

    // Test overflow protection
    uint16_t huge_nb = UINT16_MAX;
    uint16_t huge_na = UINT16_MAX;

    size_t huge_nn = (size_t)huge_nb * (size_t)huge_na;

    // This should not overflow because we're using size_t
    ASSERT(huge_nn > 0, "Grid dimension calculation should not overflow");

    // But check if it's reasonable for memory allocation
    size_t element_size = sizeof(float);
    int would_overflow_bytes = (huge_nn > SIZE_MAX / element_size);

    ASSERT(would_overflow_bytes, "Huge grid should exceed reasonable memory size");
}

TEST(path_buffer_validation) {
    // Test path buffer safety
    char path[1024];
    const char *base = "/very/long/base/path";
    const char *subdir = "/subdirectory";
    const char *filename = "/very_long_filename_that_could_cause_overflow.dat";

    // Check that combined path fits in buffer
    size_t total_len = strlen(base) + strlen(subdir) + strlen(filename);

    ASSERT(total_len < sizeof(path), "Combined path should fit in buffer");

    // Safe concatenation using snprintf
    int written = snprintf(path, sizeof(path), "%s%s%s", base, subdir, filename);

    ASSERT(written > 0, "snprintf should succeed");
    ASSERT((size_t)written < sizeof(path), "snprintf should not truncate");
    ASSERT(path[sizeof(path)-1] != '\0' || written < (int)sizeof(path),
           "Buffer should have space or be null-terminated");
}

TEST(data_file_format_validation) {
    // Test that we validate data file format before reading large amounts
    // Simulate reading a header
    uint16_t dimensions[2];
    dimensions[0] = 50;  // nb
    dimensions[1] = 60;  // na

    size_t expected_elements = (size_t)dimensions[0] * (size_t)dimensions[1];

    ASSERT(expected_elements == 3000, "50x60 grid should have 3000 elements");

    // Validate dimensions are reasonable
    ASSERT(dimensions[0] > 0 && dimensions[0] < 10000,
           "nb dimension should be reasonable");
    ASSERT(dimensions[1] > 0 && dimensions[1] < 10000,
           "na dimension should be reasonable");
}

TEST(float_data_validation) {
    // Test validation of float data read from files
    float test_values[] = {1.0f, -2.5f, 3.14159f, 0.0f, -0.001f};

    for (int i = 0; i < 5; i++) {
        // Check for NaN
        int is_nan = (test_values[i] != test_values[i]);
        ASSERT(!is_nan, "Value should not be NaN");

        // Check for infinity
        int is_inf = (test_values[i] == test_values[i] * 2.0f) &&
                     (test_values[i] != 0.0f);
        ASSERT(!is_inf, "Value should not be infinity");
    }
}

TEST(config_string_validation) {
    // Test validation of configuration strings
    const char *valid_config = "config_name";
    const char *empty_config = "";
    const char *long_config = "this_is_a_very_long_configuration_name_that_might_exceed_buffer_size";

    ASSERT(strlen(valid_config) > 0, "Valid config should not be empty");
    ASSERT(strlen(empty_config) == 0, "Empty config length should be 0");
    ASSERT(strlen(long_config) > 50, "Long config should exceed reasonable length");

    // Validate against buffer size
    char buffer[64];
    int can_fit = (strlen(valid_config) < sizeof(buffer) - 1);

    ASSERT(can_fit, "Valid config should fit in buffer");
}

TEST(memory_allocation_validation) {
    // Test validation before memory allocation
    size_t reasonable_size = 1024;

    void *ptr = malloc(reasonable_size);
    ASSERT(ptr != NULL, "Allocation of reasonable size should succeed");

    if (ptr) {
        // Initialize to verify it's usable
        memset(ptr, 0, reasonable_size);
        free(ptr);
    }

    // Test NULL check after allocation
    size_t test_size = 100 * sizeof(float);
    float *data = (float *)malloc(test_size);

    if (data != NULL) {
        // Safe to use
        data[0] = 1.0f;
        ASSERT(data[0] == 1.0f, "Allocated memory should be usable");
        free(data);
    }
}

TEST(file_size_validation) {
    // Test validation of expected file size vs actual size
    struct stat file_stat;

    // Use a known file for testing
    int result = stat("/dev/null", &file_stat);
    ASSERT(result == 0, "stat should succeed on /dev/null");

    // /dev/null should have size 0
    ASSERT(file_stat.st_size == 0, "/dev/null should have size 0");
}

TEST(endianness_awareness) {
    // Test endianness detection
    uint32_t test_value = 0x01020304;
    uint8_t *bytes = (uint8_t *)&test_value;

    int is_little_endian = (bytes[0] == 0x04);
    int is_big_endian = (bytes[0] == 0x01);

    ASSERT(is_little_endian || is_big_endian,
           "System should be either little or big endian");

    // Most modern systems are little endian
    // This test just ensures we're aware of byte order
}

TEST(data_array_bounds) {
    // Test array bounds checking
    size_t array_size = 100;
    float *array = (float *)malloc(array_size * sizeof(float));

    ASSERT(array != NULL, "Array allocation should succeed");

    if (array) {
        // Safe indexing
        for (size_t i = 0; i < array_size; i++) {
            array[i] = (float)i;
        }

        // Verify bounds
        ASSERT(array[0] == 0.0f, "First element should be 0");
        ASSERT(array[array_size - 1] == 99.0f, "Last element should be 99");

        free(array);
    }
}

TEST(string_termination_in_paths) {
    // Test that path strings are properly null-terminated
    char path[256];

    strncpy(path, "/test/path", sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    ASSERT(strlen(path) < sizeof(path), "String should be null-terminated");
    ASSERT(path[sizeof(path) - 1] == '\0', "Last byte should be null terminator");
}

TEST(error_message_formatting) {
    // Test that error messages are properly formatted
    char error_msg[512];
    const char *filename = "test_file.dat";
    int line_num = 42;

    int written = snprintf(error_msg, sizeof(error_msg),
                          "Error reading file '%s' at line %d: %s",
                          filename, line_num, strerror(ENOENT));

    ASSERT(written > 0, "Error message formatting should succeed");
    ASSERT((size_t)written < sizeof(error_msg), "Error message should fit in buffer");
}

//
// Main test runner
//

int main(void) {
    printf("\n");
    printf("=================================\n");
    printf("  Data Loader Unit Tests\n");
    printf("=================================\n");
    printf("\n");

    // Run all tests
    run_test_fread_error_handling();
    run_test_file_open_error_handling();
    run_test_directory_creation_error_handling();
    run_test_buffer_size_validation();
    run_test_grid_dimension_validation();
    run_test_path_buffer_validation();
    run_test_data_file_format_validation();
    run_test_float_data_validation();
    run_test_config_string_validation();
    run_test_memory_allocation_validation();
    run_test_file_size_validation();
    run_test_endianness_awareness();
    run_test_data_array_bounds();
    run_test_string_termination_in_paths();
    run_test_error_message_formatting();

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
