/**
 * @file max98388.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-27
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <assert.h>
#include "max98388.h"
#include "max98388_registers.h"

const uint16_t cfg_list[] = {
    THERMAL_WRN, SPK_MON_THRESH, SPK_MON_LOAD, SPK_MON_DUR, ERROR_MON_CTL,
    PCM_MODE_CFG, PCM_CLK_SETUP, PCM_SAMPLE_RATE, PCM_TX_CTL1, PCM_TX_CTL2,
    PCM_HIZ_CTL1, PCM_HIZ_CTL2, PCM_HIZ_CTL3, PCM_HIZ_CTL4, PCM_HIZ_CTL5,
    PCM_HIZ_CTL6, PCM_HIZ_CTL7, PCM_HIZ_CTL8, PCM_RX_SRC1, PCM_RX_SRC2,
    PCM_TX_DRV_STR, PCM_TX_SRC_EN, PCM_RX_EN, PCM_TX_EN, SPK_VOL_CTL, SPK_CH_CFG, SPK_AMP_OUT_CFG,
    SPK_AMP_SSM_CFG, SPK_AMP_EDGE_RATE_CTL, SPK_CH_PINK_NOISE_EN, SPK_CH_AMP_EN,
    IV_DATA_DSP_CTL, IV_DATA_EN, BRWNOUT_PROT_ALC_THRESH, BRWNOUT_PROT_ALC_RATES,
    BRWNOUT_PROT_ALC_ATTN, BRWNOUT_PROT_ALC_RELEASE, BRWNOUT, BRWNOUT_PROT_ALC_INF_HOLD_RELEASE,
    BRWNOUT_PROT_ALC_EN
};

static uint32_t cfg_list_size = sizeof(cfg_list) / sizeof(cfg_list[0]);

static max98388_reg_val_t max98388_current_cfg[50] = {0};


uint8_t max98388_init(max98388_ctx_t *ctx, uint8_t *init_seq)
{
    return 0;
}

void max98388_reset(max98388_ctx_t *ctx)
{
    ;
}

max98388_status_t max98388_get_status(max98388_ctx_t *ctx)
{
    max98388_status_t status = {.status_raw_1 = 0, .status_raw_2 = 0, .status_state_1 = 0, .status_state_2 = 0};
    return status;
}

max98388_rtn_status_t max98388_set_volume(max98388_ctx_t *ctx, uint8_t vol)
{
    if (vol <= 0x7F)
    {
        return ctx->writer(ctx, SPK_VOL_CTL, &vol, 1);
    }
    else
    {
        return MAX98388_VOL_OUT_OF_BOUNDS;
    }
}

uint8_t max98388_mute(max98388_ctx_t *ctx)
{
    assert(ctx->initialized);
    uint8_t mute_vol = 0x7F;
    return ctx->writer(ctx, SPK_VOL_CTL, &mute_vol, 1);
}

uint8_t max98388_get_hw_rev(max98388_ctx_t *ctx, uint8_t *data)
{
    if (!ctx->reader(ctx, DEV_REV_ID, data, 1))
    {
        return *data;
    }
    else
    {
        return 1;
    }
}

uint8_t max98388_write_raw_reg(max98388_ctx_t *ctx, uint16_t reg, uint8_t *data, uint8_t len)
{
    return ctx->writer(ctx, reg, data, len);
}

uint16_t max98388_return_state_status(max98388_ctx_t *ctx)
{
    uint16_t raw_status = 0;
    uint8_t val = 0;
    ctx->reader(ctx, STAT_STATE1, &val, 1);
    raw_status = (uint16_t)val;
    ctx->reader(ctx, STAT_STATE2, &val, 1);

    raw_status |= ((uint16_t)val << 8);

    return raw_status;
}

max98388_reg_val_t *max98388_dump_configuration(max98388_ctx_t *ctx, uint32_t *len)
{
    uint8_t val = 0;
    uint8_t retVal = 0;
    *len = cfg_list_size;
    for (uint32_t idx = 0; idx < cfg_list_size; idx++)
    {
        retVal = ctx->reader(ctx, cfg_list[idx], &val, 1);
        if (!retVal)
        {
            max98388_current_cfg[idx].reg = cfg_list[idx];

            max98388_current_cfg[idx].val = val;
        }

    }
    return max98388_current_cfg;
}

// Documented in .h
uint8_t max98388_enable(max98388_ctx_t *ctx, bool enable)
{
    uint8_t global_enable = enable;
    return ctx->writer(ctx, GLOBAL_EN, &global_enable, 1);
}
