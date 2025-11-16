# Contributing to SimRadar

Thank you for your interest in contributing to SimRadar! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Testing Requirements](#testing-requirements)
- [Security Guidelines](#security-guidelines)
- [Pull Request Process](#pull-request-process)
- [Issue Guidelines](#issue-guidelines)

---

## Code of Conduct

We are committed to providing a welcoming and inclusive environment. Please:

- Be respectful and constructive
- Focus on what is best for the community
- Show empathy towards other community members
- Accept constructive criticism gracefully

## Getting Started

### Prerequisites

**Linux:**
- GCC compiler
- OpenCL 1.1+ (headers and runtime)
- Make
- Git

**macOS:**
- Xcode 6+
- OpenCL (included with macOS)
- Make
- Git

### Setting Up Development Environment

1. **Fork and Clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/SimRadar.git
   cd SimRadar
   ```

2. **Install Dependencies**

   Ubuntu/Debian:
   ```bash
   sudo apt-get install build-essential opencl-headers ocl-icd-opencl-dev
   ```

   macOS:
   ```bash
   # OpenCL is included with Xcode
   xcode-select --install
   ```

3. **Build the Project**
   ```bash
   make clean
   make
   ```

4. **Run Tests**
   ```bash
   make test
   ```

5. **Verify Installation**
   ```bash
   ./simradar --help
   ```

---

## Development Workflow

### Branch Strategy

- `main/master` - Stable production code
- `develop` - Integration branch for features
- `feature/xxx` - New features
- `fix/xxx` - Bug fixes
- `security/xxx` - Security fixes

### Creating a Feature Branch

```bash
git checkout -b feature/my-new-feature main
```

### Making Changes

1. Write your code following our [coding standards](#coding-standards)
2. Add tests for new functionality
3. Ensure all tests pass
4. Update documentation
5. Commit with clear messages

### Commit Message Format

Follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style changes (formatting)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Build process or auxiliary tool changes
- `security`: Security-related changes (use `[SECURITY]` prefix)

**Examples:**
```
feat(radar): add support for dual-polarization

Implement dual-pol radar simulation with HH, VV, and HV channels.
Includes updated RCS calculations and data structures.

Closes #123
```

```
[SECURITY] fix: prevent buffer overflow in scan pattern parsing

Replaced strcpy with strncpy to prevent buffer overflow.
Added bounds checking and null termination.

CVE: None (proactive fix)
```

---

## Coding Standards

### C Code Style

**General Principles:**
- Clear, readable code over clever code
- Consistent formatting
- Comprehensive error handling
- Security-first mindset

**Formatting:**
- **Indentation:** 4 spaces (no tabs)
- **Line length:** 120 characters max
- **Braces:** K&R style (opening brace on same line)
- **Naming:**
  - Functions: `module_function_name()`
  - Types: `ModuleTypeName`
  - Constants: `MODULE_CONSTANT_NAME`
  - Variables: `descriptive_name`

**Example:**
```c
// Good
RSHandle *RS_init(void) {
    RSHandle *handle = (RSHandle *)malloc(sizeof(RSHandle));
    if (handle == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }

    memset(handle, 0, sizeof(RSHandle));
    return handle;
}

// Bad
RSHandle *RS_init(){  // Missing space before brace
RSHandle* handle=(RSHandle*)malloc(sizeof(RSHandle));  // Inconsistent spacing
if(!handle)return NULL;  // Missing braces, poor formatting
}
```

### Security Requirements

**ALWAYS:**
- ✅ Use `strncpy()`, never `strcpy()`
- ✅ Use `snprintf()`, never `sprintf()`
- ✅ Check all `malloc()` returns for NULL
- ✅ Validate all `fread()` return values
- ✅ Check `getenv()` returns for NULL
- ✅ Validate integer operations for overflow
- ✅ Sanitize all external input
- ✅ Close all file descriptors
- ✅ Free all allocated memory

**NEVER:**
- ❌ Use `strcpy()`, `strcat()`, `sprintf()`, `gets()`
- ❌ Use `system()` with user input
- ❌ Ignore return values from functions
- ❌ Assume memory allocation succeeds
- ❌ Leave file descriptors open
- ❌ Use uninitialized variables

### Error Handling

**Always handle errors:**
```c
// Good
FILE *fp = fopen(filename, "r");
if (fp == NULL) {
    fprintf(stderr, "Error: Cannot open file '%s': %s\n",
            filename, strerror(errno));
    return RS_ERROR_IO;
}

size_t items_read = fread(buffer, size, count, fp);
if (items_read != count) {
    fprintf(stderr, "Error: Failed to read from file '%s'\n", filename);
    fclose(fp);
    return RS_ERROR_IO;
}

fclose(fp);
```

---

## Testing Requirements

### Test Coverage

All new code must include tests:

- **Unit tests** for individual functions
- **Integration tests** for feature workflows
- **Edge case tests** for boundary conditions
- **Security tests** for validation logic

### Writing Tests

Tests live in the `tests/` directory:

```c
// tests/test_my_feature.c
#include <stdio.h>
#include <assert.h>
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

### Running Tests

```bash
# Run all tests
make test

# Run specific test
./tests/test_string_safety
```

### Test Requirements for PRs

- ✅ All existing tests must pass
- ✅ New features must include tests
- ✅ Bug fixes must include regression tests
- ✅ Coverage should not decrease

---

## Security Guidelines

### Reporting Security Issues

**DO NOT** open public issues for security vulnerabilities.

Instead:
1. Email: boonleng@ou.edu with `[SECURITY]` in subject
2. Provide detailed description and reproduction steps
3. Wait for acknowledgment before public disclosure

See [SECURITY.md](SECURITY.md) for full policy.

### Security Code Review Checklist

Before submitting, verify:

- [ ] No buffer overflows possible
- [ ] All input validated
- [ ] No integer overflow in calculations
- [ ] All errors handled properly
- [ ] No hardcoded secrets
- [ ] No command injection vectors
- [ ] All file operations validated
- [ ] Resource cleanup on all paths

---

## Pull Request Process

### Before Submitting

1. ✅ All tests pass (`make test`)
2. ✅ Code follows style guidelines
3. ✅ Documentation updated
4. ✅ Commit messages are clear
5. ✅ Branch is up-to-date with main

### Submitting a PR

1. **Push your branch**
   ```bash
   git push origin feature/my-feature
   ```

2. **Create Pull Request** on GitHub

3. **Fill out PR template** completely

4. **Link related issues** using `Closes #123`

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update
- [ ] Security fix

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] All tests passing

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No new warnings introduced
```

### Review Process

1. **Automated Checks** must pass (CI/CD)
2. **Code Review** by maintainer(s)
3. **Security Review** for sensitive changes
4. **Testing** in staging environment
5. **Approval** from at least one maintainer
6. **Merge** when all checks pass

### After Merge

- Branch will be deleted automatically
- Changes appear in next release
- You'll be credited in release notes

---

## Issue Guidelines

### Before Creating an Issue

1. **Search existing issues** for duplicates
2. **Check documentation** for answers
3. **Verify with latest version**

### Issue Types

**Bug Report:**
- Clear description
- Steps to reproduce
- Expected vs actual behavior
- Environment details
- Error messages/logs

**Feature Request:**
- Use case description
- Proposed solution
- Alternative solutions
- Additional context

**Security Issue:**
- **Email privately** (see Security Guidelines)
- Do NOT create public issue

### Issue Template

```markdown
## Description
Clear description of the issue

## Steps to Reproduce
1. Step one
2. Step two
3. See error

## Expected Behavior
What should happen

## Actual Behavior
What actually happens

## Environment
- OS: [Ubuntu 22.04]
- Compiler: [GCC 11.2]
- OpenCL: [1.2]
- SimRadar Version: [commit hash]

## Additional Context
Screenshots, logs, etc.
```

---

## Documentation

### When to Update Documentation

Update docs when you:
- Add new features
- Change APIs
- Fix bugs affecting usage
- Improve performance
- Change configuration

### Documentation Locations

- `README.md` - Project overview, quick start
- `SECURITY.md` - Security policy
- `CONTRIBUTING.md` - This file
- `tests/README.md` - Testing guide
- Code comments - Implementation details
- Function headers - API documentation

### Documentation Style

- Use clear, concise language
- Include code examples
- Explain "why" not just "what"
- Keep updated with code changes

### API Documentation

Generate API documentation with Doxygen:

```bash
# Install Doxygen
sudo apt-get install doxygen graphviz  # Ubuntu/Debian
brew install doxygen graphviz          # macOS

# Generate documentation
doxygen Doxyfile

# View documentation
open docs/html/index.html  # macOS
xdg-open docs/html/index.html  # Linux
```

Documentation is generated from:
- Header file comments (`.h` files)
- Function documentation blocks
- Structure and type definitions
- README, CONTRIBUTING, and SECURITY files

**Doxygen comment style:**

```c
/**
 * @brief Brief description of function
 *
 * Detailed description explaining what the function does,
 * any important algorithmic details, and usage notes.
 *
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 *
 * @note Any important notes or warnings
 * @see Related functions
 *
 * Example usage:
 * @code
 * RSHandle *handle = RS_init();
 * RS_set_wavelength(handle, 0.10f);
 * @endcode
 */
ReturnType function_name(Type param1, Type param2);
```

---

## Code Review Guidelines

### As a Reviewer

- Be respectful and constructive
- Explain your suggestions
- Approve quickly when possible
- Focus on important issues

### As an Author

- Respond to all comments
- Ask for clarification
- Make requested changes
- Mark conversations resolved

---

## Release Process

### Versioning

We use [Semantic Versioning](https://semver.org/):

- **MAJOR** - Incompatible API changes
- **MINOR** - Backwards-compatible features
- **PATCH** - Backwards-compatible bug fixes

### Release Checklist

- [ ] All tests passing
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] Version number bumped
- [ ] Security scan clean
- [ ] Performance verified
- [ ] Release notes prepared

---

## Getting Help

### Resources

- **Documentation:** README.md
- **Examples:** simple_ppi.c, simple_dbs.c
- **Issues:** GitHub Issues
- **Security:** SECURITY.md
- **Testing:** tests/README.md

### Contact

- **Maintainer:** Boonleng Cheong
- **Email:** boonleng@ou.edu
- **Issues:** GitHub Issues

### Response Times

- **Bug reports:** 2-3 business days
- **Feature requests:** 1 week
- **Security issues:** 48 hours
- **Pull requests:** 3-5 business days

---

## Recognition

Contributors will be:
- Listed in release notes
- Credited in commit history
- Thanked in acknowledgments
- Invited to join maintainer team (for significant contributions)

---

## License

By contributing, you agree that your contributions will be licensed under the same MIT License that covers the project.

---

Thank you for contributing to SimRadar! 🎉
