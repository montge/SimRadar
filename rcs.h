//
//  rcs.h
//  Radar Simulation Framework
//
//  Created by Boon Leng Cheong on 3/16/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef _radarsim_rcs_h
#define _radarsim_rcs_h

#include "log.h"

#define RCSConfigLeaf              "leaf"
#define RCSConfigPlate             "plate"
#define RCSConfigBrick             "brick"
#define RCSConfigWoodBoard         "woodboard"
#define RCSConfigWoodBoardDish     "woodboardish"

typedef void * RCSHandle;
typedef char * RCSConfig;

typedef struct _rcs_data {
    float *a;
    float *b;
    float *hh_real;
    float *vv_real;
    float *hv_real;
    float *hh_imag;
    float *vv_imag;
    float *hv_imag;
} RCSData;

typedef struct _rcs_table {
    uint32_t  na;             // Number of cells in alpha direction
    uint32_t  nb;             // Number of cells in beta direction
    uint32_t  nn;             // Number of cells in all directions combined
    RCSData   data;
    char      name[1024];
    char      path[1024];
    float     lambda;
} RCSTable;

/**
 * @brief Initialize Radar Cross Section (RCS) handler with custom path
 *
 * Loads RCS lookup tables from specified directory. RCS tables contain
 * polarimetric backscattering coefficients (HH, VV, HV) as functions of
 * debris orientation angles (alpha, beta) for realistic radar returns.
 *
 * @param path Directory containing RCS .rcs files
 * @return RCS handle on success, NULL on failure
 *
 * @note RCS tables are wavelength-specific and contain complex values
 *       (real and imaginary parts) for each polarization channel
 * @see RCS_init(), RCS_free(), RCS_get_table()
 *
 * @code
 * RCSHandle rcs = RCS_init_with_path("/data/tables/rcs");
 * if (rcs) {
 *     RCSTable *leaf = RCS_get_table(rcs, RCSConfigLeaf);
 *     // ... use backscattering data
 *     RCS_free(rcs);
 * }
 * @endcode
 */
RCSHandle RCS_init_with_path(const char *path);

/**
 * @brief Initialize RCS handler with default path
 *
 * Searches standard locations for RCS data:
 * - ~/Downloads/tables/rcs
 * - ~/Documents/tables/rcs
 * - ~/Desktop/tables/rcs
 *
 * @return RCS handle on success, NULL if tables not found
 * @see RCS_init_with_path()
 */
RCSHandle RCS_init(void);

/**
 * @brief Free RCS handle and release resources
 *
 * @param handle RCS handle to free
 * @see RCS_init()
 */
void RCS_free(RCSHandle);

/**
 * @brief Get RCS table for specific debris configuration
 *
 * Retrieves polarimetric backscattering lookup tables for a debris type.
 * Tables provide complex scattering amplitudes (real + imaginary) for
 * HH, VV, and HV polarization channels as functions of orientation.
 *
 * @param handle RCS handle
 * @param config Debris configuration (e.g., RCSConfigLeaf, RCSConfigPlate)
 * @return Pointer to RCS table with polarimetric data, or NULL if not found
 *
 * @note Available configurations:
 *   - RCSConfigLeaf: "leaf" - Tree leaf
 *   - RCSConfigPlate: "plate" - Metal plate
 *   - RCSConfigBrick: "brick" - Brick debris
 *   - RCSConfigWoodBoard: "woodboard" - Wooden board
 *   - RCSConfigWoodBoardDish: "woodboardish" - Dish-shaped wood
 *
 * @note RCS values are wavelength-dependent (stored in table->lambda)
 *
 * @code
 * RCSTable *leaf = RCS_get_table(rcs, RCSConfigLeaf);
 * if (leaf) {
 *     printf("Table dimensions: %d × %d\n", leaf->na, leaf->nb);
 *     printf("Wavelength: %.3f m\n", leaf->lambda);
 *     // Access backscatter coefficients:
 *     // leaf->data.hh_real, hh_imag (HH polarization)
 *     // leaf->data.vv_real, vv_imag (VV polarization)
 *     // leaf->data.hv_real, hv_imag (HV cross-pol)
 * }
 * @endcode
 */
RCSTable *RCS_get_table(const RCSHandle, const RCSConfig config);

/**
 * @brief Get path to RCS data directory
 *
 * @param handle RCS handle
 * @return String path to RCS data directory
 */
char *RCS_data_path(const RCSHandle);

void RCS_show_table_summary(const RCSTable *);

#endif
