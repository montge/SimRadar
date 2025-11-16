//
//  pos.h
//  Radar Simulation Framework
//
//  Created by Boonleng Cheong 9/12/2018
//  Copyright (c) 2018 Boonleng Cheong. All rights reserved.
//
#ifndef _radarsim_pos_h
#define _radarsim_pos_h

#include "log.h"

#define POS_MAX_PATTERN_COUNT    10000
#define POS_MAX_SWEEP_COUNT      50

// Test with this:
// radarsim -p50 -SD:0,75,10/90,75,10/0,90,10 -N
//


typedef void * POSHandle;

typedef struct pos_pos {
    float       az;
    float       el;
    uint32_t    index;                                 // Iteration of this position
    uint32_t    count;                                 // Repetition of this position
} POSPosition;

typedef struct pos_sweep {
    float       azStart;
    float       azEnd;
    float       azDelta;
    float       elStart;
    float       elEnd;
    float       elDelta;
} POSSweep;

typedef struct pos_pattern {
    char        mode;
    char        reserved1;
    char        reserved2;
    char        reserved3;
    uint32_t    index;                                 // The index of POSPosition elements
    uint32_t    count;                                 // The count of POSPosition elements
    uint32_t    sweepIndex;                            // The index of POSSweep elements
    uint32_t    sweepCount;                            // The count of POSSweep elements
    POSPosition positions[POS_MAX_PATTERN_COUNT];      // Array of positions
    POSSweep    sweeps[POS_MAX_SWEEP_COUNT];           // For summary generation only
    float       az;                                    // Current azimuth to use
    float       el;                                    // Current elevation to use
    uint32_t    tic;
} POSPattern;

/**
 * @brief Initialize empty scan pattern
 *
 * Creates a scan pattern structure with no positions defined.
 * Use POS_parse_from_string() to populate with scan positions.
 *
 * @return Pointer to POSPattern on success, NULL on failure
 * @see POS_init_with_string(), POS_free()
 */
POSPattern *POS_init(void);

/**
 * @brief Initialize scan pattern from string description
 *
 * Creates and parses a scan pattern from string representation.
 * Supports PPI, RHI, and DBS scan patterns.
 *
 * @param string Scan pattern description (e.g., "P:0,10.0,360.0,1.0")
 * @return Pointer to POSPattern on success, NULL on parse error
 *
 * @note String format:
 *   - PPI: "P:elevation,az_start,az_end,az_delta"
 *   - RHI: "R:azimuth,el_start,el_end,el_delta"
 *   - DBS: "D:azimuth,elevation"
 *
 * @see POS_init(), POS_parse_from_string()
 *
 * @code
 * // 360° PPI at 1° elevation, 1° azimuth spacing
 * POSPattern *scan = POS_init_with_string("P:1.0,0,360,1.0");
 * if (scan) {
 *     while (POS_get_next_angles(scan)) {
 *         // Process beam at scan->az, scan->el
 *     }
 *     POS_free(scan);
 * }
 * @endcode
 */
POSPattern *POS_init_with_string(const char *);

/**
 * @brief Free scan pattern and release resources
 *
 * @param pattern Scan pattern to free
 * @see POS_init()
 */
void POS_free(POSPattern *);

/**
 * @brief Advance to next beam position in scan pattern
 *
 * Updates scan->az and scan->el to the next position in the pattern.
 * Call repeatedly to iterate through all beam positions.
 *
 * @param scan Scan pattern
 * @return 1 if advanced to next position, 0 if end of pattern reached
 *
 * @code
 * POSPattern *scan = POS_init_with_string("P:2.0,0,90,1.0");
 * while (POS_get_next_angles(scan)) {
 *     printf("Beam: az=%.1f el=%.1f\n", scan->az, scan->el);
 * }
 * @endcode
 */
int POS_get_next_angles(POSPattern *scan);

/**
 * @brief Parse scan pattern from string
 *
 * Populates existing scan pattern from string description.
 * Overwrites any existing pattern data.
 *
 * @param scan Scan pattern structure to populate
 * @param string Pattern description string
 * @return Number of positions parsed, or negative on error
 *
 * @see POS_init_with_string()
 */
int POS_parse_from_string(POSPattern *scan, const char *string);

/**
 * @brief Check if scan pattern is PPI (Plan Position Indicator)
 *
 * PPI scans sweep azimuth at constant elevation.
 *
 * @param scan Scan pattern
 * @return true if PPI scan, false otherwise
 */
bool POS_is_ppi(const POSPattern *scan);

/**
 * @brief Check if scan pattern is RHI (Range Height Indicator)
 *
 * RHI scans sweep elevation at constant azimuth.
 *
 * @param scan Scan pattern
 * @return true if RHI scan, false otherwise
 */
bool POS_is_rhi(const POSPattern *scan);

/**
 * @brief Check if scan pattern is DBS (Doppler Beam Swinging)
 *
 * DBS uses fixed beam positions for wind profiling.
 *
 * @param scan Scan pattern
 * @return true if DBS scan, false otherwise
 */
bool POS_is_dbs(const POSPattern *scan);

/**
 * @brief Check if scan pattern is empty (no positions)
 *
 * @param scan Scan pattern
 * @return true if empty, false otherwise
 */
bool POS_is_empty(const POSPattern *scan);

void POS_summary(POSHandle P);

#endif
