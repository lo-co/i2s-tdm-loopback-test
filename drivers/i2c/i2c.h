/**
 * @file i2c.h
 * @author Matt Richardson (mattrichardson@meta.com)
 * @brief
 * @version 0.1
 * @date 2023-11-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _I2C_H
#define _I2C_H

#include <stdint.h>

#include "fsl_i2c.h"

// TODO: This should be moved to a general flexcomm driver package taht will allow the code
// to determine which ports are in use or not.
/**
 * @brief I2C port instance
 *
 */
typedef enum i2c_fcport_e
{
    I2CFC0 = 0,
    I2CFC1,
    I2CFC2,
    I2CFC3,
    I2CFC4,
    I2CFC5,
    I2CFC6,
    I2CFC7,
    I2CFCMAX
} i2c_fcport_t;

/**
 * @brief I2C context.  Stores base Flexcomm address and handles for use
 *
 */
typedef struct i2c_ctx_s
{
    I2C_Type *base;
    union{
        i2c_slave_handle_t *slave;
        i2c_master_handle_t *master;
    } handle;

    union {
        i2c_slave_transfer_t *slave;
        i2c_master_transfer_t *master;
    } transfer;


} i2c_ctx_t;

/**
 * @brief Initialization structure for I2C communications
 *
 */
typedef struct i2c_init_s
{
    uint8_t address;
    uint32_t baud_rate;
    i2c_fcport_t flexcomm_port;
    void *transfer_cb;
    void *usrData;
    bool is_master;
} i2c_init_t;

/**
 * @brief I2C initialization routine
 *
 * @param init_cfg Initialization configuration structure
 * @param ctx I2C context
 */
void i2c_init(i2c_init_t *init_cfg, i2c_ctx_t *ctx);

#endif