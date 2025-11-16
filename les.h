//
//  les.h
//  Radar Simulation Framework
//
//  Created by Boon Leng Cheong on 4/7/14.
//  Copyright (c) 2014 Boon Leng Cheong. All rights reserved.
//

#ifndef _radarsim_les_h
#define _radarsim_les_h

#include "log.h"

#define LESConfigNull                  ""
#define LESConfigFlat                  "flat"
#define LESConfigTwoCell               "twocell"
#define LESConfigSuctionVortices       "suctvort"
#define LESConfigSuctionVorticesLarge  "suctvort_large"

typedef void * LESHandle;
typedef char * LESConfig;
typedef float LESFloat4[4];

typedef struct les_grid {
	uint32_t  rev;            // Revision number, perhaps?
	uint32_t  nx;             // Number of cells in x direction
	uint32_t  ny;             // Number of cells in y direction
	uint32_t  nz;             // Number of cells in z direction
	float     *x;             // X values
	float     *y;             // Y values
	float     *z;             // Z values
    bool      is_stretched;   // Uniform or stretched
} LESGrid;

typedef struct _les_value {
    float *a;
	float *x;
	float *y;
	float *z;
	float *u;
	float *v;
	float *w;
	float *p;
	float *t;
} LESValue;

typedef struct _les_table {
	uint32_t  nx;             // Number of cells in x direction
	uint32_t  ny;             // Number of cells in y direction
	uint32_t  nz;             // Number of cells in z direction
	uint32_t  nn;             // Number of cells in all directions combined
	uint32_t  nt;             // Number of time steps in a file
    uint32_t  nc;             // Number of cubes in this set
    bool      is_stretched;   // Uniform or stretched
	float     tr;             // Replenishing time constant
    float     tp;             // Time period of a table entry
    float     ax;             // Base value "a" in geometric series a r ^ n in x direction
    float     ay;             // Base value "a" in geometric series a r ^ n in y direction
    float     az;             // Base value "a" in geometric series a r ^ n in z direction
    float     rx;             // Ratio value "r" in the geometric series in x direction. Otherwise, this is delta x.
    float     ry;             // Ratio value "r" in the geometric series in y direction. Otherwise, this is delta y.
    float     rz;             // Ratio value "r" in the geometric series in z direction. Otherwise, this is delta z.
    LESValue  data;           // Raw data from LES table
    LESFloat4 *uvwt;          // Remapped (u, v, w, t) data for efficient transfer in RS framework
    LESFloat4 *cpxx;          // Remapped (cn2, p, _, _) data for efficient transfer in RS framework
    float     *flux;          // Flux PDF
} LESTable;


/**
 * @brief Initialize Large Eddy Simulation (LES) data handler with configuration
 *
 * Loads LES wind field data from specified path with given configuration.
 * LES data provides 3D wind velocity (u,v,w) and turbulence fields for
 * realistic debris transport simulation.
 *
 * @param config LES configuration identifier (e.g., "t0_00600")
 * @param path Directory path containing LES data files
 * @return LES handle on success, NULL on failure
 *
 * @note LES files are typically >1GB. Ensure sufficient memory
 * @see LES_init(), LES_free()
 *
 * @code
 * LESHandle les = LES_init_with_config_path("t0_00600", "/data/les");
 * if (les) {
 *     LESTable *frame = LES_get_frame(les, 0);
 *     // ... use wind field data
 *     LES_free(les);
 * }
 * @endcode
 */
LESHandle LES_init_with_config_path(const LESConfig config, const char *path);

/**
 * @brief Initialize LES handler with default configuration
 *
 * Searches standard paths for LES data and loads default configuration.
 *
 * @return LES handle on success, NULL on failure
 * @see LES_init_with_config_path()
 */
LESHandle LES_init(void);

/**
 * @brief Free LES handle and release resources
 *
 * @param handle LES handle to free
 * @see LES_init()
 */
void LES_free(LESHandle);

/**
 * @brief Enable delayed read mode for memory efficiency
 *
 * In delayed read mode, LES frames are loaded on-demand rather than
 * pre-loaded. This reduces memory usage but increases I/O operations.
 *
 * @param handle LES handle
 */
void LES_set_delayed_read(LESHandle);

/**
 * @brief Get initial LES frame without time interpolation
 *
 * @param handle LES handle
 * @param n Frame index
 * @return Pointer to LES table, or NULL if index out of range
 * @see LES_get_frame()
 */
LESTable *LES_get_frame_0(const LESHandle, const int n);

/**
 * @brief Get LES frame with time interpolation
 *
 * Returns wind field data for specified frame index. If between frames,
 * performs temporal interpolation for smooth time evolution.
 *
 * @param handle LES handle
 * @param n Frame index (can be fractional for interpolation)
 * @return Pointer to LES table with (u,v,w,t,cn2,p) fields
 *
 * @note Interpolation provides smooth debris motion between frames
 */
LESTable *LES_get_frame(const LESHandle, const int n);

/**
 * @brief Get path to LES data directory
 *
 * @param handle LES handle
 * @return String path to data directory
 */
char *LES_data_path(const LESHandle);

/**
 * @brief Get time period between LES frames
 *
 * @param handle LES handle
 * @return Time period in seconds
 */
float LES_get_table_period(const LESHandle);

/**
 * @brief Get total number of LES frames available
 *
 * @param handle LES handle
 * @return Number of frames
 */
size_t LES_get_table_count(const LESHandle);

void LES_show_table_summary(const LESTable *table);

void LES_show_handle_summary(const LESHandle);

#endif
