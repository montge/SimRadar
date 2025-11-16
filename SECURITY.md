# Security Policy

## Supported Versions

We take security seriously and are committed to addressing vulnerabilities promptly.

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

## Reporting a Vulnerability

If you discover a security vulnerability in SimRadar, please report it responsibly:

### How to Report

**DO NOT** open a public GitHub issue for security vulnerabilities.

Instead, please report security issues to:
- **Email:** boonleng@ou.edu
- **Subject Line:** [SECURITY] SimRadar Vulnerability Report

### What to Include

Please provide as much information as possible:

1. **Description** - Clear description of the vulnerability
2. **Type** - Category (e.g., command injection, buffer overflow, etc.)
3. **Location** - Affected file(s) and line number(s)
4. **Reproduction** - Step-by-step instructions to reproduce
5. **Impact** - Your assessment of the security impact
6. **Suggested Fix** - If you have ideas for remediation (optional)

### Response Timeline

- **Initial Response:** Within 48 hours
- **Status Update:** Within 7 days
- **Fix Timeline:** Varies by severity
  - **Critical:** Immediate (hours to days)
  - **High:** Within 2 weeks
  - **Medium:** Within 1 month
  - **Low:** Next release cycle

## Security Advisories

Security advisories will be published through:
- GitHub Security Advisories
- Release notes
- Git commit messages with `[SECURITY]` prefix

## Security Update Process

When a security issue is fixed:

1. **Private Fix** - Code is fixed in a private branch
2. **Testing** - Thorough testing to ensure the fix works
3. **Advisory Draft** - Security advisory is prepared
4. **Release** - New version is released with security patch
5. **Public Disclosure** - Advisory is published after users have time to update

## Known Security Issues

We maintain transparency about security issues. Current status:

### Recently Fixed (2025-11-16)

✅ **Command Injection Vulnerability (CRITICAL)**
- **Issue:** User-controlled paths passed to system() without sanitization
- **Files:** simradar.c:1134-1137
- **Fix:** Replaced system() call with mkdir() syscall
- **Status:** Fixed in commit [current]
- **Impact:** Prevented arbitrary command execution

✅ **NULL Pointer Dereference (HIGH)**
- **Issue:** Unchecked getenv() calls
- **Files:** simradar.c:1125, lsiq.c:48
- **Fix:** Added NULL checks with proper error handling
- **Status:** Fixed in commit [current]

✅ **Integer Overflow in malloc (HIGH)**
- **Issue:** User-controlled allocation size without overflow checks
- **Files:** simradar.c:1008
- **Fix:** Added overflow validation before allocation
- **Status:** Fixed in commit [current]

### Active Security Work

The following security improvements are in progress:

🔨 **Buffer Overflow Vulnerabilities (HIGH)**
- **Issue:** Unsafe use of strcpy(), sprintf(), strcat()
- **Files:** Multiple files throughout codebase
- **Status:** Work in progress
- **Tracking:** GitHub Issue #2

🔨 **Path Traversal (MEDIUM)**
- **Issue:** Insufficient path validation
- **Files:** simradar.c, lsiq.c
- **Status:** Work in progress
- **Tracking:** GitHub Issue #4

🔨 **Missing Error Checks (MEDIUM)**
- **Issue:** fread() return values not validated
- **Files:** les.c, adm.c, rcs.c
- **Status:** Work in progress
- **Tracking:** GitHub Issue #4

## Security Best Practices

When using SimRadar:

### Input Validation
- Validate all command-line arguments
- Sanitize file paths before use
- Check numeric parameters are within valid ranges

### File Operations
- Only read from trusted data sources
- Validate data table integrity
- Use absolute paths when possible

### GPU Operations
- Monitor GPU memory usage
- Validate OpenCL kernel compilation
- Handle GPU errors gracefully

### Environment Variables
- Ensure HOME is set if using default paths
- Validate environment variable contents
- Provide fallback values

## Security Tools and Testing

We use the following tools to maintain security:

### Static Analysis
- Clang Static Analyzer
- Cppcheck
- CodeQL (GitHub integration)

### Dynamic Analysis
- Valgrind (memory safety)
- AddressSanitizer (buffer overflows)
- ThreadSanitizer (race conditions)

### Testing
- Unit tests for input validation
- Fuzzing for command-line parsing
- Integration tests for file operations

## Security Hardening Recommendations

For production deployments:

### Compile-Time Hardening
```bash
# Enable stack protection
CFLAGS="-fstack-protector-strong"

# Enable position-independent code
CFLAGS="-fPIC -pie"

# Enable format string protection
CFLAGS="-Wformat -Wformat-security"

# Enable all warnings
CFLAGS="-Wall -Wextra -Werror"
```

### Runtime Hardening
- Run with minimal privileges
- Use resource limits (ulimit)
- Isolate in containers when possible
- Validate all input data files

### System Configuration
- Keep OpenCL drivers updated
- Monitor system resource usage
- Use firewall rules if network-accessible
- Enable system audit logs

## Acknowledgments

We appreciate security researchers who report vulnerabilities responsibly. Contributors will be acknowledged in:
- Security advisories
- Release notes
- Hall of fame (if they wish)

## Contact

For general security questions (non-vulnerability):
- GitHub Discussions: Use the Security category
- Email: boonleng@ou.edu

For urgent security matters:
- Email: boonleng@ou.edu with [SECURITY - URGENT] in subject

---

**Last Updated:** 2025-11-16
**Version:** 1.0.0
