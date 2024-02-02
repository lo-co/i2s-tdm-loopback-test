/**
 * @file i2c.c
 * @author Matt Richardson (matt.richardson@msrconsults.com)
 * @brief
 * @version 0.1
 * @date 2023-11-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "i2c.h"
#include "fsl_clock.h"
#include "fsl_iopctl.h"

#include "../gpio/gpio_def.h"

typedef struct i2c_pins_s
{
    uint8_t port;
    uint8_t sda_pin;
    uint8_t scl_pin;
} i2c_pins_t;

const i2c_pins_t i2c_pin_config[] =
{
   {.port = 0, .sda_pin = 2,  .scl_pin = 1},    // FC 0
   {.port = 0, .sda_pin = 9,  .scl_pin = 8},    // FC 1
   {.port = 0, .sda_pin = 16, .scl_pin = 15},   // FC 2
   {.port = 0, .sda_pin = 24, .scl_pin = 25},   // FC 3
   {.port = 0, .sda_pin = 30, .scl_pin = 29},   // FC 4
   {.port = 1, .sda_pin = 5,  .scl_pin = 4},    // FC 5
   {.port = 3, .sda_pin = 27, .scl_pin = 26},   // FC 6
   {.port = 4, .sda_pin = 2,  .scl_pin = 1},    // FC 7
};

/**
 * @brief Initialize the pin mux for I2C
 *
 * @param i2c_init Initialization structure for current use case
 */
void i2c_pin_init(i2c_init_t *i2c_init);

void i2c_set_peripheral_address(i2c_init_t *i2c_init, i2c_ctx_t *ctx);

/**
 * @brief Attach the proper clock to the flexcomm port
 *
 * I2C on ports 0-7 use the internal 16 MHz oscillator (SFRO) labeled 16m_irc on
 * all clock diagrams.  This clock should be enabled in the boot sequence.
 *
 * @param init_cfg port initialization configuration structure
 */
static void i2c_clock_init(i2c_init_t *init_cfg);

// Documented in .h
void i2c_init(i2c_init_t *init_cfg, i2c_ctx_t *ctx)
{
    assert(init_cfg->flexcomm_port < I2CFCMAX);

    // FIXME: slave is not implemented...
    assert(init_cfg->is_master);

    i2c_clock_init(init_cfg);

    i2c_pin_init(init_cfg);
    i2c_set_peripheral_address(init_cfg, ctx);

    if (init_cfg->is_master)
    {
        i2c_master_config_t cfg;
        /*
         *   masterConfig->enableMaster  = true;
         *   masterConfig->baudRate_Bps  = 100000U;
         *   masterConfig->enableTimeout = false;
         *   masterConfig->timeout_Ms    = 35;
         */
        I2C_MasterGetDefaultConfig(&cfg);
        cfg.baudRate_Bps = init_cfg->baud_rate;

        I2C_MasterInit(ctx->base, &cfg, CLOCK_GetFlexCommClkFreq(init_cfg->flexcomm_port));

        // Set up the transfer handle
        I2C_MasterTransferCreateHandle(ctx->base, ctx->handle.master, init_cfg->transfer_cb, init_cfg->usrData);
        ctx->transfer.master->slaveAddress = init_cfg->address;
    }
}

void i2c_pin_init(i2c_init_t *i2c_init)
{
    i2c_pins_t pins = i2c_pin_config[i2c_init->flexcomm_port];

    const uint32_t i2c_pin_cfg = (IOPCTL_PIO_FUNC1 |
                                    // IOPCTL_PIO_PUPD_EN |
                                    // IOPCTL_PIO_PULLUP_EN |
                                    IOPCTL_PIO_INBUF_EN |
                                    IOPCTL_PIO_SLEW_RATE_NORMAL |
                                    IOPCTL_PIO_FULLDRIVE_DI |
                                    IOPCTL_PIO_ANAMUX_DI |
                                    IOPCTL_PIO_PSEDRAIN_EN |
                                    IOPCTL_PIO_INV_DI);

    IOPCTL_PinMuxSet(IOPCTL, pins.port, pins.scl_pin, i2c_pin_cfg);
    IOPCTL_PinMuxSet(IOPCTL, pins.port, pins.sda_pin, i2c_pin_cfg);
}

// TODO - remove this to a common file
void i2c_set_peripheral_address(i2c_init_t *i2c_init, i2c_ctx_t *ctx)
{
    assert(i2c_init->flexcomm_port < I2CFCMAX);
    switch(i2c_init->flexcomm_port)
    {
        case I2CFC0:
            ctx->base = (I2C_Type *)FLEXCOMM0;
            break;
        case I2CFC1:
            ctx->base = (I2C_Type *)FLEXCOMM1;
            break;
        case I2CFC2:
            ctx->base = (I2C_Type *)FLEXCOMM2;
            break;
        case I2CFC3:
            ctx->base = (I2C_Type *)FLEXCOMM3;
            break;
        case I2CFC4:
            ctx->base = (I2C_Type *)FLEXCOMM4;
            break;
        case I2CFC5:
            ctx->base = (I2C_Type *)FLEXCOMM5;
            break;
        case I2CFC6:
            ctx->base = (I2C_Type *)FLEXCOMM6;
            break;
        case I2CFC7:
            ctx->base = (I2C_Type *)FLEXCOMM7;
            break;
        default:
            break;
    }
}

static void i2c_clock_init(i2c_init_t *init_cfg)
{
    switch (init_cfg->flexcomm_port)
    {
        case I2CFC0:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM0);
            break;
        case I2CFC1:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM1);
            break;
        case I2CFC2:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM2);
            break;
        case I2CFC3:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM3);
            break;
        case I2CFC4:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM4);
            break;
        case I2CFC5:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM5);
            break;
        case I2CFC6:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM6);
            break;
        case I2CFC7:
            CLOCK_AttachClk(kSFRO_to_FLEXCOMM7);
            break;
        default:
            break;
    }
}

