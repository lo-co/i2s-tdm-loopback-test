/** @file
 * @brief This module provides I2S functionality support
 *
 * This module supports setting up and tearing down I2S functionality using
 * Flexcomm drivers provided by NXP.  The I2S functionality is described in
 * detail in the specification document UM 1147.  Chapter 21 describes the
 * Flexcomm interface for serial communication.  Chapter 25 provides details
 * on I2S functionality
 *
 * @author Matt Richardson
 * @date October 26, 2023
*/

#ifndef _I2S_H_
#define _I2S_H_

#include "fsl_dma.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/** Callback for I2S DMA */
typedef void (*i2s_cb_func)(I2S_Type *,i2s_dma_handle_t *,status_t ,void *);

/** Context used by main application for I2S operations */
typedef struct i2s_context_s
{
    I2S_Type *base;                 ///< This is the peripheral base address
    dma_handle_t dma_handle;        ///< DMA handle for transfers
    i2s_dma_handle_t i2s_dma_handle;///< I2S DMA handle for transfers

} i2s_context_t;

/** @brief Enumerations for the Flexcomm ports that have I2S functionality
 *
 * The ports that provide I2S support are defined in UM 1147, Table 502. Only
 * ports 0-7 support I2S
*/
typedef enum flexcomm_port_e {
    FLEXCOMM_0 = 0,
    FLEXCOMM_1,
    FLEXCOMM_2,
    FLEXCOMM_3,
    FLEXCOMM_4,
    FLEXCOMM_5,
    FLEXCOMM_6,
    FLEXCOMM_7,
    FLEXCOMM_ND,
} flexcomm_port_t;

/** Defines which of the two shared sets a shared clock is occupying */
typedef enum shared_set_e {
    NO_SHARE = 0,
    SHARED_SET_1,
    SHARED_SET_2,
} shared_set_t;

/** I2S configuration structure */
typedef struct i2s_init_s
{
    flexcomm_port_t flexcomm_bus; ///< Bus that I2S instance uses
    bool is_transmit;             ///< True if the bus is a transmitter
    bool is_master;               ///< True if bus is master
    uint8_t active_channels;      ///< Active number of channels - must be in pairs between 2 and 8
    uint32_t sample_rate;         ///< Frame sync clock rate in Hz
    uint32_t datalength;          ///< Number of bits per channel
    i2s_cb_func callback;         ///< I2S complete callback
    i2s_context_t *context;       ///< I2S context
    bool share_clk;               ///< True if the clock is shared with other FC buses
    shared_set_t shared_clk_set;  ///< Set to value < FLEXCOMM_ND if intend to use a shared clock
} i2s_init_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** Initialization function for an I2S bus
 *
 * @param config Configuration for I2S bus initialization
*/
void i2s_init(i2s_init_t config);

/**
 * @brief Stop all DMA transfers and disable the bus
 *
 * @param config Configuration structure for requested bus
 */
void i2s_stop(i2s_init_t config);

#endif