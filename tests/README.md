# SimRadar Test Suite

This directory contains unit tests for the SimRadar framework.

## Test Organization

### Unit Tests (No Dependencies)
- `test_string_safety.c` - Buffer overflow fixes and safe string operations (9 tests)
- `test_rs_tables.c` - RS data structures, radar parameters, and calculations (13 tests)
- `test_data_loaders.c` - Data loading error handling patterns (15 tests)

### Integration Tests (Requires OpenCL + Library)
- `test_pos_parsing.c` - Scan pattern parsing with librs.a (5 tests)
- `test_rs_integration.c` - RS framework initialization and configuration (13 tests)
  - **Requires**: OpenCL headers/runtime, librs.a
  - **GPU Detection**: Skips GPU-dependent tests if no GPU available
- `test_data_integration.c` - LES/ADM/RCS data loader integration (16 tests)
  - **Requires**: OpenCL headers/runtime, librs.a, data tables (15GB)
  - **Data Detection**: Skips tests if data files not found

**Total: 71 tests** (37 unit + 34 integration)

## Running Tests

### Compile and Run All Tests

```bash
make test
```

### Run Individual Tests

```bash
# String safety tests (9 tests, no dependencies)
./tests/test_string_safety

# RS table and parameter tests (13 tests, no dependencies)
./tests/test_rs_tables

# Data loader tests (15 tests, no dependencies)
./tests/test_data_loaders

# Pattern parsing tests (5 tests, requires librs.a)
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

## Code Coverage

### Generating Coverage Reports

Run tests with coverage analysis:

```bash
# Generate coverage report (requires lcov)
make coverage

# View HTML report
open coverage/html/index.html  # macOS
xdg-open coverage/html/index.html  # Linux
```

The coverage target will:
1. Build all tests with coverage instrumentation (`--coverage` flag)
2. Run all tests and collect execution data (`.gcda` files)
3. Generate HTML report showing line-by-line coverage
4. Display summary statistics

### Installing lcov

**Ubuntu/Debian:**
```bash
sudo apt-get install lcov
```

**macOS:**
```bash
brew install lcov
```

### Coverage Goals

- **Phase 1 (Current):** ~15-20% - Core security fixes validated
- **Phase 2:** 30% line coverage - Core modules tested
- **Phase 3:** 60% line coverage with integration tests
- **Phase 4:** 80%+ coverage with comprehensive test suite

Current coverage tracked in CI/CD - see badge in main README.

## Continuous Integration

Tests are automatically run by GitHub Actions on:
- Every push to main/master
- Every pull request
- Weekly scheduled runs

### CI/CD Workflows

- `.github/workflows/build-and-test.yml` - Build and test on multiple platforms
- `.github/workflows/static-analysis.yml` - Cppcheck static analysis
- `.github/workflows/codeql.yml` - GitHub CodeQL security scanning
- `.github/workflows/coverage.yml` - Code coverage measurement and reporting

Coverage reports are generated on every PR and tracked over time.

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
