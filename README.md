# SimRadar - Polarimetric Radar Simulator

[![Build and Test](https://img.shields.io/github/actions/workflow/status/ouradar/simradar/build-and-test.yml?branch=main&label=build&logo=github)](https://github.com/ouradar/simradar/actions/workflows/build-and-test.yml)
[![Code Coverage](https://img.shields.io/github/actions/workflow/status/ouradar/simradar/coverage.yml?branch=main&label=coverage&logo=codecov)](https://github.com/ouradar/simradar/actions/workflows/coverage.yml)
[![CodeQL](https://img.shields.io/github/actions/workflow/status/ouradar/simradar/codeql.yml?branch=main&label=security&logo=github)](https://github.com/ouradar/simradar/actions/workflows/codeql.yml)
[![Static Analysis](https://img.shields.io/github/actions/workflow/status/ouradar/simradar/static-analysis.yml?branch=main&label=static%20analysis&logo=c)](https://github.com/ouradar/simradar/actions/workflows/static-analysis.yml)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A polarimetric radar time-series emulator utilizing air-drag models for particle motions and realistic radar cross-section libraries for backscattering calculations. Implemented with OpenCL for parallel computing on GPUs.

![SimRadar Screenshot](blob/screenshot.png)

## ✨ Features

- **GPU-Accelerated**: OpenCL-based parallel computing for real-time simulation
- **Polarimetric**: Full HH, VV, and HV channel simulation
- **Physics-Based**:
  - Large Eddy Simulation (LES) wind fields
  - Air Drag Model (ADM) for debris transport
  - Radar Cross Section (RCS) lookup tables
- **Flexible Scanning**: PPI, RHI, and DBS scan patterns
- **Production Ready**: Comprehensive testing, CI/CD, and security hardening

## 📋 Table of Contents

- [Quick Start](#quick-start)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Documentation](#documentation)
- [Testing](#testing)
- [Contributing](#contributing)
- [Publications](#publications)
- [License](#license)

## 🚀 Quick Start

```bash
# Clone the repository
git clone https://github.com/ouradar/simradar.git
cd simradar

# Build the project
make

# Run example simulation
./simradar
```

## 📦 Requirements

### Linux
- **GCC** 4.9+ or Clang 3.5+
- **OpenCL** 1.1 or 1.2 (headers and runtime)
- **Optional**: Doxygen (for API documentation), lcov (for coverage reports)

```bash
# Ubuntu/Debian
sudo apt-get install gcc opencl-headers ocl-icd-opencl-dev

# Optional tools
sudo apt-get install doxygen graphviz lcov
```

### macOS
- **Xcode** 6 or above (provides OpenCL and compiler)
- **Optional**: [Sparkle Framework](http://arrc.ou.edu/~boonleng/files/Sparkle.framework.zip) for GUI app

```bash
# Install optional tools
brew install doxygen graphviz lcov
```

### Data Files
Large simulation data tables (15GB):
- [LES, ADM & RCS Data](http://arrc.ou.edu/~boonleng/simradar/tables.zip)
- Extract to `~/Downloads/tables`, `~/Documents/tables`, or `~/Desktop/tables`

## 🔧 Installation

### Standard Build

```bash
make
```

This builds:
- `lib/librs.a` - Core radar simulation library
- `simradar` - Command-line simulator
- `simple_ppi`, `simple_dbs` - Example programs
- `lsiq` - Data file utility

### Running Tests

```bash
# Run all unit tests (42 tests)
make test

# Generate code coverage report
make coverage
open coverage/html/index.html
```

### Building Documentation

```bash
# Generate API documentation
doxygen Doxyfile
open docs/html/index.html
```

## 📖 Usage

### Basic Example

```c
#include "rs.h"

int main(void) {
    // Initialize simulator
    RSHandle *sim = RS_init();
    if (sim == NULL) {
        fprintf(stderr, "Failed to initialize\n");
        return EXIT_FAILURE;
    }

    // Configure radar parameters
    RS_set_wavelength(sim, 0.10f);           // S-band (10 cm)
    RS_set_prt(sim, 0.001f);                 // 1 ms PRT (1000 Hz PRF)
    RS_set_antenna_params(sim, 1.0f, 45.0f); // 1° beamwidth, 45 dBi gain
    RS_set_tx_params(sim, 1.0e-6f, 500e3f);  // 1 μs pulse, 500 kW

    // Set up scan pattern (PPI at 3° elevation)
    POSPattern *scan = POS_init_with_string("P:3.0,0,360,1.0");
    RS_set_scan_pattern(sim, scan);

    // Populate simulation domain
    RS_populate(sim);

    // Run simulation
    while (POS_get_next_angles(scan)) {
        RS_set_beam_pos(sim, scan->az, scan->el);
        RS_make_pulse(sim);
        RS_advance_time(sim);
    }

    // Cleanup
    RS_free(sim);
    POS_free(scan);

    return EXIT_SUCCESS;
}
```

### Compiling with SimRadar

**Linux:**
```bash
gcc -I. -L./lib -o my_sim my_sim.c -lrs -lOpenCL -lm -lpthread
```

**macOS:**
```bash
gcc -I. -L./lib -o my_sim my_sim.c -lrs -framework OpenCL -lm -lpthread
```

See [simple_ppi.c](simple_ppi.c) and [simple_dbs.c](simple_dbs.c) for complete examples.

## 📚 Documentation

- **[API Documentation](docs/html/index.html)** - Full Doxygen API reference (run `doxygen` to generate)
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Development workflow, coding standards, testing guide
- **[SECURITY.md](SECURITY.md)** - Security policy and vulnerability reporting
- **[CHANGELOG.md](CHANGELOG.md)** - Project history and release notes
- **[tests/README.md](tests/README.md)** - Testing documentation and coverage goals

### Key API Modules

- **`rs.h`** - Main simulation framework (initialization, radar parameters, pulse generation)
- **`les.h`** - Large Eddy Simulation wind field data
- **`adm.h`** - Air Drag Model for debris transport
- **`rcs.h`** - Radar Cross Section lookup tables
- **`pos.h`** - Scan pattern definitions (PPI, RHI, DBS)

## 🧪 Testing

SimRadar includes comprehensive testing infrastructure:

```bash
# Run all tests
make test

# Run individual test suites
./tests/test_string_safety    # Security fixes (9 tests)
./tests/test_rs_tables         # Radar calculations (13 tests)
./tests/test_data_loaders      # I/O validation (15 tests)
./tests/test_pos_parsing       # Scan patterns (5 tests)
```

**Test Coverage:** ~15-20% (Goal: 30% → 60% → 80%)

See [tests/README.md](tests/README.md) for details.

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Development workflow and branch strategy
- Coding standards and style guide
- Testing requirements
- Pull request process
- Security guidelines

**Quick Start for Contributors:**

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Write tests for your changes
4. Ensure all tests pass (`make test`)
5. Commit using [Conventional Commits](https://www.conventionalcommits.org/) format
6. Push to your fork and submit a pull request

## 📊 Architecture

SimRadar uses a master handler structure to manage simulation state:

```
┌─────────────────────────────────────────────────┐
│              RSHandle (Master)                  │
│  ┌──────────────┐  ┌──────────────┐            │
│  │ Radar Params │  │ Scan Pattern │            │
│  └──────────────┘  └──────────────┘            │
│  ┌──────────────┐  ┌──────────────┐            │
│  │  LES Wind    │  │ ADM Tables   │            │
│  │   Fields     │  │              │            │
│  └──────────────┘  └──────────────┘            │
│  ┌──────────────┐  ┌──────────────┐            │
│  │ RCS Tables   │  │  Scatter     │            │
│  │              │  │   Bodies     │            │
│  └──────────────┘  └──────────────┘            │
└─────────────────────────────────────────────────┘
                    ↓
        ┌───────────────────────┐
        │   OpenCL Runtime      │
        │  ┌─────────────────┐  │
        │  │  GPU Kernels    │  │
        │  ├─────────────────┤  │
        │  │ • Scatterer Pos │  │
        │  │ • RCS Lookup    │  │
        │  │ • Pulse Gen     │  │
        │  └─────────────────┘  │
        └───────────────────────┘
```

**Workflow:**
1. **Setup**: Configure radar parameters, load data tables
2. **Population**: Allocate GPU memory, compile kernels, distribute scatter bodies
3. **Simulation**: Iterate scan pattern, generate pulses, advance time
4. **Retrieval**: Download results from GPU

## 📄 Publications

**Primary Reference:**

B. L. Cheong, D. J. Bodine, C. J. Fulton, S. M. Torres, T. Maruyama, R. D. Palmer, "SimRadar: A Polarimetric Radar Time-Series Simulator for Tornadic Debris Studies," *IEEE Trans. Geosci. Remote Sens.*, vol. 55, no. 5, pp. 2858-2870, 2017.

**DOI:** [10.1109/TGRS.2017.2655363](https://doi.org/10.1109/TGRS.2017.2655363)

## 🔒 Security

Security is a priority. See [SECURITY.md](SECURITY.md) for:
- Vulnerability reporting process
- Security best practices
- Known issues and mitigations

**Security Achievements:**
- ✅ All critical and high-severity vulnerabilities resolved
- ✅ Automated security scanning (CodeQL, Cppcheck)
- ✅ Safe string handling throughout codebase
- ✅ Comprehensive input validation

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Authors and Contact

- **Boon Leng Cheong** - *Original Author* - <boonleng@ou.edu>
- Advanced Radar Research Center (ARRC)
- University of Oklahoma

## 🙏 Acknowledgments

- LES data provided by tornado simulation research
- OpenCL framework by Khronos Group
- Contributors and testers from the radar meteorology community

---

**Looking to contribute?** Check out our [good first issues](https://github.com/ouradar/simradar/labels/good%20first%20issue) or contact the maintainer at boonleng@ou.edu.
