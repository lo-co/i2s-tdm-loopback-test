/**
 * @file max98388.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-27
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _MAX98388_h
#define _MAX98388_h

#include <stdint.h>
#include <stdbool.h>

typedef struct max98388_ctx_s max98388_ctx_t;

typedef struct max98388_reg_val_s
{
    uint16_t reg;
    uint8_t val;
} max98388_reg_val_t;

typedef uint8_t (*max98388_read_fn)(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len);
typedef uint8_t (*max98388_write_fn)(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len);

typedef struct max98388_status_s
{
    uint8_t status_raw_1;
    uint8_t status_raw_2;
    uint8_t status_state_1;
    uint8_t status_state_2;
} max98388_status_t;

struct max98388_ctx_s
{
    max98388_read_fn reader;
    max98388_write_fn writer;
    bool initialized;
};

typedef enum max98388_rtn_status_e {
    MAX98388_STATUS = 0x0,
    MAX98388_VOL_OUT_OF_BOUNDS,
} max98388_rtn_status_t;


/**
 * @brief Initialize device after power on
 *
 * @param ctx Device context
 * @param init_seq Sequence of register address-value pairs
 * @return 0 on success
 */
uint8_t max98388_init(max98388_ctx_t *ctx, uint8_t *init_seq);

/**
 * @brief Reset device and return it to it's power on state
 *
 * @param ctx Device context
 */
void max98388_reset(max98388_ctx_t *ctx);

/**
 * @brief Get the device status.  Reads all device status registers and returns them
 *
 * @param ctx Device context
 * @return  Result of reading device registers
 */
max98388_status_t max98388_get_status(max98388_ctx_t *ctx);

/**
 * @brief Set the device volume
 *
 * @param ctx Device context
 * @param vol 8-bit integer representing the volume.
 */
max98388_rtn_status_t max98388_set_volume(max98388_ctx_t *ctx, uint8_t vol);

/**
 * @brief Mute the amp
 *
 * @param ctx Device context
 * @return uint8_t
 */
uint8_t max98388_mute(max98388_ctx_t *ctx);

uint8_t max98388_get_hw_rev(max98388_ctx_t *ctx, uint8_t *data);

uint8_t max98388_write_raw_reg(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len);

max98388_reg_val_t *max98388_dump_configuration(max98388_ctx_t *ctx, uint32_t *len);

uint8_t max98388_enable(max98388_ctx_t *ctx, bool enable);

#endif