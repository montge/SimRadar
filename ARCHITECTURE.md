# SimRadar Architecture

This document provides a high-level overview of the SimRadar architecture, design decisions, and component interactions.

## Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Component Design](#component-design)
- [Data Flow](#data-flow)
- [GPU Acceleration](#gpu-acceleration)
- [Extension Points](#extension-points)

## Overview

SimRadar is a polarimetric radar simulator designed for realistic time-series generation with GPU acceleration. The architecture follows a **master-worker pattern** where a CPU-side master handler coordinates multiple GPU workers for parallel computation.

### Design Goals

1. **Performance**: Leverage GPU parallel computing for real-time simulation
2. **Accuracy**: Physics-based models (LES wind, ADM debris motion, RCS backscattering)
3. **Flexibility**: Support multiple scan patterns and debris types
4. **Portability**: OpenCL for vendor-neutral GPU access
5. **Maintainability**: Clean C API with clear separation of concerns

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐            │
│  │  simradar  │  │ simple_ppi │  │ simple_dbs │  (User App)│
│  └──────┬─────┘  └──────┬─────┘  └──────┬─────┘            │
│         │                │                │                  │
│         └────────────────┴────────────────┘                  │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────┴───────────────────────────────────┐
│                      RS Framework API                        │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              RSHandle (Master Handler)               │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌───────────┐  │   │
│  │  │ Radar Params │  │ Scan Pattern │  │   DSD     │  │   │
│  │  │  - PRT       │  │  - PPI       │  │  Profile  │  │   │
│  │  │  - λ (wave)  │  │  - RHI       │  │           │  │   │
│  │  │  - Antenna   │  │  - DBS       │  │           │  │   │
│  │  └──────────────┘  └──────────────┘  └───────────┘  │   │
│  │                                                       │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌───────────┐  │   │
│  │  │  LES Handle  │  │  ADM Handle  │  │ RCS Handle│  │   │
│  │  │  Wind Fields │  │  Drag Tables │  │Backscatter│  │   │
│  │  └──────┬───────┘  └──────┬───────┘  └─────┬─────┘  │   │
│  │         │                  │                │        │   │
│  │         └──────────────────┴────────────────┘        │   │
│  │                            │                          │   │
│  │                    ┌───────▼────────┐                │   │
│  │                    │  Scatter Bodies │                │   │
│  │                    │  (Position,     │                │   │
│  │                    │   Velocity,     │                │   │
│  │                    │   Orientation)  │                │   │
│  │                    └───────┬────────┘                │   │
│  └────────────────────────────┼───────────────────────────┘   │
└─────────────────────────────┬─┴───────────────────────────────┘
                              │
┌─────────────────────────────┴───────────────────────────────┐
│                    OpenCL Runtime Layer                      │
│  ┌──────────────────────────────────────────────────────┐   │
│  │                  GPU Workers                          │   │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐     │   │
│  │  │ Worker 0   │  │ Worker 1   │  │ Worker N   │     │   │
│  │  │ (GPU 0)    │  │ (GPU 1)    │  │ (GPU/CPU)  │     │   │
│  │  └────────────┘  └────────────┘  └────────────┘     │   │
│  │                                                       │   │
│  │  ┌─────────────────────────────────────────────┐     │   │
│  │  │           GPU Kernels                       │     │   │
│  │  │  • scat_pos_update  - Position integration  │     │   │
│  │  │  • scat_rcs_lookup  - RCS table lookup      │     │   │
│  │  │  • scat_tumble      - Orientation update    │     │   │
│  │  │  • make_pulse_p1    - Pulse gen (pass 1)    │     │   │
│  │  │  • make_pulse_p2    - Pulse gen (pass 2)    │     │   │
│  │  └─────────────────────────────────────────────┘     │   │
│  └──────────────────────────────────────────────────────┘   │
└───────────────────────────────────────────────────────────┘
```

## Component Design

### 1. Master Handler (RSHandle)

**Purpose**: Central coordination point for simulation state.

**Responsibilities**:
- Maintain radar parameters (wavelength, PRT, antenna characteristics)
- Manage scan pattern progression
- Coordinate data table access (LES, ADM, RCS)
- Orchestrate GPU workers
- Track simulation time

**Key Fields**:
```c
struct _rs_handle {
    RSParams     params;          // Radar parameters
    POSPattern   *scan_pattern;   // Scan definition
    LESHandle    vel_handle;      // Wind field data
    ADMHandle    adm_handle[N];   // Drag coefficient tables
    RCSHandle    rcs_handle[N];   // Backscattering tables
    size_t       num_scats;       // Total scatter bodies
    RSWorker     *workers[N];     // GPU worker handles
    // ... (see rs.h for complete definition)
};
```

### 2. Data Table Modules

#### LES (Large Eddy Simulation)
**Purpose**: Provide 3D wind field data for realistic debris motion.

**Key Features**:
- Time-series of 3D velocity grids (u, v, w)
- Turbulence fields (Cn², pressure, temperature)
- Temporal interpolation for smooth evolution
- Stretched grid support for efficiency

**Data Structure**:
```
LES Frame: [nx × ny × nz] grid
  - u: x-component wind velocity (m/s)
  - v: y-component wind velocity (m/s)
  - w: z-component wind velocity (m/s)
  - t: temperature (K)
  - cn2: refractive index structure parameter
  - p: pressure (Pa)
```

#### ADM (Air Drag Model)
**Purpose**: Provide drag and moment coefficients for debris motion.

**Key Features**:
- 2D lookup tables indexed by orientation angles (α, β)
- Drag coefficients (Cd) for force calculation
- Moment coefficients (Cm) for tumbling motion
- Physical parameters (mass, density, dimensions)

**Data Structure**:
```
ADM Table: [nβ × nα] grid
  - Cdx, Cdy, Cdz: Drag coefficients (3 components)
  - Cmx, Cmy, Cmz: Moment coefficients (3 components)
  - Physical: mass, density, dimensions, Tachikawa parameter
```

#### RCS (Radar Cross Section)
**Purpose**: Provide polarimetric backscattering coefficients.

**Key Features**:
- 2D lookup tables indexed by orientation angles (α, β)
- Complex values (real + imaginary) for phase information
- Wavelength-specific tables
- Polarimetric channels (HH, VV, HV)

**Data Structure**:
```
RCS Table: [nα × nβ] grid
  - HH: Co-polar horizontal (real, imag)
  - VV: Co-polar vertical (real, imag)
  - HV: Cross-polar (real, imag)
```

### 3. Scan Pattern (POS)

**Purpose**: Define radar beam positions over time.

**Scan Types**:
- **PPI** (Plan Position Indicator): Azimuth sweep at constant elevation
- **RHI** (Range Height Indicator): Elevation sweep at constant azimuth
- **DBS** (Doppler Beam Swinging): Fixed beam positions for wind profiling

**String Format**:
```
PPI: "P:elevation,az_start,az_end,az_delta"
RHI: "R:azimuth,el_start,el_end,el_delta"
DBS: "D:azimuth,elevation"
```

### 4. GPU Workers (RSWorker)

**Purpose**: Execute parallel computations on GPU devices.

**Responsibilities**:
- Manage OpenCL context and command queue
- Compile and cache GPU kernels
- Allocate and manage GPU memory buffers
- Execute kernels with appropriate work group sizing
- Transfer data between CPU and GPU

**Key GPU Buffers**:
```c
cl_mem scat_pos;   // [x, y, z, radius] for each scatter body
cl_mem scat_vel;   // [vx, vy, vz, _] velocity
cl_mem scat_ori;   // [q0, q1, q2, q3] orientation quaternion
cl_mem scat_rcs;   // [Ihh, Qhh, Ivv, Qvv] backscattering
cl_mem scat_sig;   // [Ihh, Qhh, Ivv, Qvv] radar signal
```

## Data Flow

### Initialization Phase

```
1. RS_init()
   ├─> Create master handler
   ├─> Detect available GPUs
   ├─> Initialize OpenCL context
   └─> Set default parameters

2. Configuration
   ├─> RS_set_prt(), RS_set_lambda(), etc.
   ├─> Load LES/ADM/RCS tables
   └─> Define scan pattern

3. RS_populate()
   ├─> Calculate scatter body count
   ├─> Allocate GPU memory
   ├─> Compile OpenCL kernels
   ├─> Distribute bodies across workers
   ├─> Upload table data to GPU
   └─> Initialize scatter positions/velocities
```

### Simulation Loop

```
for each radar pulse:
    1. RS_set_beam_pos(az, el)
       └─> Update beam parameters

    2. RS_make_pulse()
       ├─> Kernel: Update scatter positions (integrate motion equations)
       │   └─> Use LES wind field + ADM drag
       ├─> Kernel: Update orientations (tumbling)
       │   └─> Use ADM moment coefficients
       ├─> Kernel: Lookup RCS values
       │   └─> Index RCS tables by current orientation
       ├─> Kernel: Calculate radar returns (pass 1)
       │   └─> Sum contributions within range gates
       └─> Kernel: Reduce to final I/Q values (pass 2)
           └─> Parallel reduction for efficiency

    3. RS_advance_time()
       ├─> Increment simulation time
       └─> Update LES frame if needed
```

### Data Retrieval

```
RS_download()
├─> Transfer results from GPU to CPU
├─> Assemble I/Q time series
└─> Format for output (NetCDF, binary, etc.)
```

## GPU Acceleration

### Kernel Organization

**Position Update** (`kern_scat_pos`):
```c
// For each scatter body (parallel):
1. Interpolate wind velocity from LES grid
2. Lookup drag coefficient from ADM table (orientation-dependent)
3. Calculate drag force: F = 0.5 * ρ * Cd * A * v²
4. Integrate position: x(t+Δt) = x(t) + v*Δt + 0.5*a*Δt²
```

**Orientation Update** (`kern_scat_ori`):
```c
// For each scatter body (parallel):
1. Lookup moment coefficient from ADM table
2. Calculate torque from air resistance
3. Update angular velocity
4. Integrate orientation (quaternion)
```

**RCS Lookup** (`kern_rcs_lookup`):
```c
// For each scatter body (parallel):
1. Extract orientation angles (α, β) from quaternion
2. Bilinear interpolation in RCS table
3. Store complex backscattering coefficients (HH, VV, HV)
```

**Pulse Generation** (Two-pass reduction):
```c
// Pass 1: Range gate accumulation
for each scatter body (parallel):
    range = distance_to_radar(position)
    gate_index = (range - range_start) / range_delta
    if gate_index valid:
        atomic_add(pulse[gate_index], signal * beam_weight)

// Pass 2: Parallel reduction
Reduce partial sums to final I/Q values per range gate
```

### Memory Optimization

**Coalesced Access**:
- Scatter body data in Structure of Arrays (SoA) layout
- Sequential threads access sequential memory addresses
- Maximizes memory bandwidth utilization

**Texture Memory** (for table lookups):
- LES/ADM/RCS tables stored as textures
- Hardware interpolation for sub-index access
- Cached for faster repeated access

**Local Memory**:
- Reduction operations use local memory
- Minimize global memory traffic
- Work group synchronization for correctness

## Extension Points

### Adding New Debris Types

1. **Create RCS table**: Compute backscattering for new geometry
2. **Create ADM table**: Simulate aerodynamics in wind tunnel/CFD
3. **Define configuration**: Add to `adm.h` and `rcs.h`
4. **Register in framework**: Update loading code

### Adding New Scan Patterns

1. **Extend POS parser**: Add new pattern type to `pos.c`
2. **Define string format**: Document in `pos.h`
3. **Implement iteration**: Add to `POS_get_next_angles()`

### Custom Physics Models

**Replace LES wind**:
```c
// Implement custom wind field provider
MyWindHandle my_wind_init();
void RS_set_vel_data(RSHandle *H, float *u, float *v, float *w, ...);
```

**Replace ADM drag**:
```c
// Provide custom drag coefficients
MyADMHandle my_adm_init();
void RS_set_adm_data(RSHandle *H, float *cdx, float *cdy, ...);
```

### Output Formats

Current: Binary `.iq` files with custom header

**Add new format**:
1. Implement writer in `simradar.c`
2. Add command-line option
3. Document in README

Potential formats: NetCDF, HDF5, ODIM_H5, CF-Radial

## Performance Considerations

### Bottlenecks

1. **GPU-CPU data transfer**: Minimize by batching operations
2. **Kernel launch overhead**: Amortize over many scatter bodies
3. **Table lookup**: Use texture memory for caching
4. **Reduction operations**: Two-pass algorithm for efficiency

### Scalability

- **Strong scaling**: More GPUs → faster for fixed problem size
- **Weak scaling**: More GPUs → larger problem size at same speed
- **Multi-GPU**: Automatic work distribution across devices

### Typical Performance

- **Scatter bodies**: 1M-10M per simulation
- **Frame rate**: 100-500 FPS (depending on GPU)
- **Range gates**: 1000-2000
- **Polarizations**: 3 channels (HH, VV, HV)

## Design Patterns

### Resource Management
- **RAII-style**: `_init()` allocates, `_free()` deallocates
- **Handle-based**: Opaque pointers hide implementation details

### Error Handling
- **Return codes**: NULL for initialization failure
- **Logging**: Timestamped messages to stderr
- **Validation**: Parameter checks before GPU upload

### Modularity
- **Clean interfaces**: Each module has `.h` and `.c` files
- **Minimal dependencies**: Modules can be tested independently
- **API stability**: Public API separate from internal implementation

---

**Document Version**: 1.0
**Last Updated**: 2025
**Maintainer**: Boon Leng Cheong <boonleng@ou.edu>
