# SimRadar Test Suite

This directory contains unit tests for the SimRadar framework.

## Test Organization

- `test_string_safety.c` - Tests for buffer overflow fixes and safe string operations
- `test_pos_parsing.c` - Tests for scan pattern parsing
- `test_rs_framework.c` - Tests for core RS framework functions (TODO)
- `test_data_integrity.c` - Tests for data loading and validation (TODO)

## Running Tests

### Compile and Run All Tests

```bash
make test
```

### Run Individual Tests

```bash
# String safety tests
./tests/test_string_safety

# Pattern parsing tests
./tests/test_pos_parsing
```

## Test Framework

We use a lightweight testing framework with these macros:

- `TEST(name)` - Define a test function
- `ASSERT(condition, message)` - Assert a condition is true
- `ASSERT_EQ(expected, actual, message)` - Assert equality
- `ASSERT_STR_EQ(expected, actual, message)` - Assert string equality

## Writing New Tests

1. Create a new file: `tests/test_<module>.c`
2. Include the module header and test macros
3. Write test functions using `TEST(name)`
4. Add test runner in `main()`
5. Update Makefile to compile the new test

Example:

```c
#include "../my_module.h"

TEST(my_feature_works) {
    int result = my_function(42);
    ASSERT_EQ(42, result, "Function should return input");
}

int main(void) {
    run_test_my_feature_works();
    return (tests_passed == tests_run) ? 0 : 1;
}
```

## Test Coverage Goals

- **Phase 1 (Current):** Core security fixes validated
- **Phase 2:** 30% line coverage
- **Phase 3:** 60% line coverage with integration tests
- **Phase 4:** 80%+ coverage with comprehensive test suite

## Continuous Integration

Tests are automatically run by GitHub Actions on:
- Every push to main/master
- Every pull request
- Weekly scheduled runs

See `.github/workflows/build-and-test.yml` for CI configuration.

## Test Data

Some tests require data files. These should be placed in `tests/data/` and committed to the repository (for small files) or downloaded during test setup (for large files).

## Known Issues

- Tests requiring GPU/OpenCL are skipped in CI environments
- Large data table tests (LES, ADM, RCS) require >15GB download

## Contributing

When adding new features or fixing bugs:
1. Write tests first (TDD approach recommended)
2. Ensure all existing tests pass
3. Add tests for new functionality
4. Update this README if adding new test categories
