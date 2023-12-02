/**
 * @file serdes_amplifier.c
 * @author Matt Richardson (mattrichardson@meta.com)
 * @brief
 * @version 0.1
 * @date 2023-11-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "serdes_amplifier.h"
#include "../drivers/i2c/i2c.h"
#include "../drivers/max98388/max98388.h"

// Address is 7bit so shift left
#define AMPLIFIER_ADDRESS (0x70 >> 1)

static i2c_ctx_t amp_i2c_ctx = {0};
static uint8_t data[10] = {0};
static i2c_master_handle_t handle;
static i2c_master_transfer_t txfr;
static max98388_ctx_t amp_ctx = {0};


static uint8_t serdes_amp_reader(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len);
static uint8_t serdes_amp_writer(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len);
static void i2c_amp_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);

// Documented in .h
void serdes_amp_init()
{
    i2c_init_t init_cfg = {.address = AMPLIFIER_ADDRESS,
                           .baud_rate = 100000U,
                           .flexcomm_port = I2CFC3,
                           .is_master = true,
                           .transfer_cb = i2c_amp_callback,
                           .usrData = NULL};
    amp_i2c_ctx.handle.master = &handle;
    amp_i2c_ctx.transfer.master = &txfr;
    amp_i2c_ctx.transfer.master->slaveAddress = AMPLIFIER_ADDRESS;
    amp_i2c_ctx.transfer.master->data = data;
    i2c_init(&init_cfg, &amp_i2c_ctx);

    amp_ctx.initialized = true;
    amp_ctx.reader = serdes_amp_reader;
    amp_ctx.writer = serdes_amp_writer;

    max98388_get_hw_rev(&amp_ctx, data);
    uint32_t cfg_len = 0;
    max98388_reg_val_t *cfg_reg;
    cfg_reg = max98388_dump_configuration(&amp_ctx, &cfg_len);
    uint8_t reg_cfg_vals[] = {
        0x0A, 0x58, 0x08, 0x02, 0x01, 0xD9, 0x08, 0x88,
        0x10, 0x14, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0x00, 0x10, 0x01, 0x03, 0x01, 0x01,
        0x00, 0x02, 0x03, 0x01, 0x0F, 0x00, 0x01, 0x10,
        0x03, 0x04, 0x20, 0x06, 0x02, 0x33, 0x00, 0x00
    };

    for (uint8_t idx =0; idx < cfg_len; idx++)
    {
        max98388_write_raw_reg(&amp_ctx, cfg_reg[idx].reg, &reg_cfg_vals[idx], 1);
    }
    cfg_reg = max98388_dump_configuration(&amp_ctx, &cfg_len);

    // Clear the state
    max98388_return_state_status(&amp_ctx);

}

// FIXME: should really check to make sure that the PVDD power supply is up and running
// Documented in .h
serdes_amp_code_t serdes_amp_start()
{
    // uint16_t state = max98388_return_state_status(&amp_ctx);

    uint8_t retStatus = max98388_enable(&amp_ctx, true);

    if (retStatus)
    {
        return 1;
    }
    return 0;

}

void serdes_amp_stop()
{
    max98388_enable(&amp_ctx, false);
}

uint8_t serdes_amp_reader(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len)
{
    // Setup the transfer...
    amp_i2c_ctx.transfer.master->subaddress = (uint32_t)reg;
    amp_i2c_ctx.transfer.master->dataSize = len;
    amp_i2c_ctx.transfer.master->subaddressSize = 2;
    amp_i2c_ctx.transfer.master->flags = kI2C_TransferDefaultFlag;
    amp_i2c_ctx.transfer.master->direction = kI2C_Read;
    amp_i2c_ctx.transfer.master->data = data;
    status_t retVal = I2C_MasterTransferBlocking(amp_i2c_ctx.base, amp_i2c_ctx.transfer.master);

    return retVal != 0;
}

uint8_t serdes_amp_writer(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len)
{
    // Setup the transfer...
    amp_i2c_ctx.transfer.master->subaddress = (uint32_t)reg;
    amp_i2c_ctx.transfer.master->dataSize = len;
    amp_i2c_ctx.transfer.master->subaddressSize = 2;
    amp_i2c_ctx.transfer.master->flags = kI2C_TransferDefaultFlag;
    amp_i2c_ctx.transfer.master->data = data;
    amp_i2c_ctx.transfer.master->direction = kI2C_Write;
    // amp_i2c_ctx.handle.master->
    status_t retVal = I2C_MasterTransferBlocking(amp_i2c_ctx.base, amp_i2c_ctx.transfer.master);
    if (retVal)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void i2c_amp_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
    (void)base;

    *(int32_t *)data = status;
}