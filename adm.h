//
//  adm.h
//  Radar Simulation Framework
//
//  Created by Boon Leng Cheong on 1/19/15.
//  Copyright (c) 2015-2016 Boon Leng Cheong. All rights reserved.
//

#ifndef _radarsim_adm_h
#define _radarsim_adm_h

#include "log.h"

#define ADMConfigModelPlate        "plate"
#define ADMConfigSquarePlate       "square_plate"
#define ADMConfigRoofTile          "roof_tile"

typedef void * ADMHandle;
typedef char * ADMConfig;

//typedef struct adm_grid {
//    uint32_t  rev;            // Revision number, perhaps?
//    uint32_t  nb;             // Number of cells in beta direction
//    uint32_t  na;             // Number of cells in alpha direction
//    float     *b;             // Beta values
//    float     *a;             // Alpha values
//} ADMGrid;

typedef struct _adm_base {
    float     x;              // Length in x (m) (local coordinate)
    float     y;              // Length in y (m) (local coordinate)
    float     z;              // Length in z (m) (local coordinate)
    float     rho;            // Density (kg / m^3)
    float     mass;           // Mass (kg)
    float     Ta;             // Tachikawa parameter
    float     inv_inln_x;     // X componenent of 1 / (In Ln)
    float     inv_inln_y;     // Y componenent of 1 / (In Ln)
    float     inv_inln_z;     // Z componenent of 1 / (In Ln)
} ADMBase;

typedef struct _adm_data {
    float *b;
    float *a;
    float *cdx;
    float *cdy;
    float *cdz;
    float *cmx;
    float *cmy;
    float *cmz;
} ADMData;

typedef struct _adm_table {
    uint32_t  nb;              // Number of cells in beta direction
    uint32_t  na;              // Number of cells in alpha direction
    uint32_t  nn;              // Number of cells in all directions combined
    ADMBase   phys;            // Physical description of the debris
    ADMData   data;
    char      name[1024];
    char      path[1024];
} ADMTable;

/**
 * @brief Initialize Air Drag Model (ADM) handler with custom path
 *
 * Loads ADM data tables from specified directory. ADM tables contain
 * drag coefficients (Cd) and moment coefficients (Cm) as functions of
 * debris orientation angles (alpha, beta) for realistic debris motion.
 *
 * @param path Directory containing ADM .adm files
 * @return ADM handle on success, NULL on failure
 *
 * @note ADM files contain 2D lookup tables (alpha × beta)
 * @see ADM_init(), ADM_free(), ADM_get_table()
 *
 * @code
 * ADMHandle adm = ADM_init_with_path("/data/tables/adm");
 * if (adm) {
 *     ADMTable *plate = ADM_get_table(adm, ADMConfigModelPlate);
 *     // ... use drag coefficients
 *     ADM_free(adm);
 * }
 * @endcode
 */
ADMHandle ADM_init_with_path(const char *path);

/**
 * @brief Initialize ADM handler with default path
 *
 * Searches standard locations for ADM data:
 * - ~/Downloads/tables/adm
 * - ~/Documents/tables/adm
 * - ~/Desktop/tables/adm
 *
 * @return ADM handle on success, NULL if tables not found
 * @see ADM_init_with_path()
 */
ADMHandle ADM_init(void);

/**
 * @brief Free ADM handle and release resources
 *
 * @param handle ADM handle to free
 * @see ADM_init()
 */
void ADM_free(ADMHandle);

/**
 * @brief Get ADM table for specific debris configuration
 *
 * Retrieves drag and moment coefficient lookup tables for a debris type.
 * Tables provide Cd (drag) and Cm (moment) as functions of orientation
 * angles alpha and beta, enabling realistic tumbling motion.
 *
 * @param handle ADM handle
 * @param config Debris configuration (e.g., ADMConfigModelPlate, ADMConfigRoofTile)
 * @return Pointer to ADM table with Cd/Cm data, or NULL if not found
 *
 * @note Available configurations:
 *   - ADMConfigModelPlate: "plate" - Generic plate model
 *   - ADMConfigSquarePlate: "square_plate" - Square plate
 *   - ADMConfigRoofTile: "roof_tile" - Roof tile debris
 *
 * @code
 * ADMTable *plate = ADM_get_table(adm, ADMConfigModelPlate);
 * if (plate) {
 *     printf("Table dimensions: %d × %d\n", plate->nb, plate->na);
 *     printf("Debris mass: %.3f kg\n", plate->phys.mass);
 *     // Access Cd/Cm via plate->data.cdx, cdy, cdz, cmx, cmy, cmz
 * }
 * @endcode
 */
ADMTable *ADM_get_table(const ADMHandle, const ADMConfig config);

/**
 * @brief Get path to ADM data directory
 *
 * @param handle ADM handle
 * @return String path to ADM data directory
 */
char *ADM_data_path(const ADMHandle);

void ADM_show_table_summary(const ADMTable *);
void ADM_transform_scale(ADMTable *T, const float x, const float y, const float z, const float r);
void ADM_dimension_set(ADMTable *T, const float x, const float y, const float z, const float r);

#endif
