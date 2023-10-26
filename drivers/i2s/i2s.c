#include "i2s.h"
#include "fsl_gpio.h"
#include "fsl_iopctl.h"

#define IOPCTL_PIO_ANAMUX_DI 0x00u        /*!<@brief Analog mux is disabled */
#define IOPCTL_PIO_FULLDRIVE_DI 0x00u     /*!<@brief Normal drive */
#define IOPCTL_PIO_FULLDRIVE_EN 0x0100u   /*!<@brief Full drive enable */
#define IOPCTL_PIO_FUNC0 0x00u            /*!<@brief Selects pin function 0 */
#define IOPCTL_PIO_FUNC1 0x01u            /*!<@brief Selects pin function 1 */
#define IOPCTL_PIO_INBUF_EN 0x40u         /*!<@brief Enables input buffer function */
#define IOPCTL_PIO_INV_DI 0x00u           /*!<@brief Input function is not inverted */
#define IOPCTL_PIO_PSEDRAIN_DI 0x00u      /*!<@brief Pseudo Output Drain is disabled */
#define IOPCTL_PIO_PULLDOWN_EN 0x00u      /*!<@brief Enable pull-down function */
#define IOPCTL_PIO_PUPD_DI 0x00u          /*!<@brief Disable pull-up / pull-down function */
#define IOPCTL_PIO_SLEW_RATE_NORMAL 0x00u /*!<@brief Normal mode */

typedef struct i2s_pins_s {
    uint8_t port;
    uint8_t sck_pin;
    uint8_t ws_pin;
    uint8_t data_pin;
} i2s_pins_t;

/** Populate the peripheral base address
 *
 * @param config I2S configuration for bus
*/
static void set_peripheral_address(i2s_init_t config);

/** Pin definitions for the different I2S busses (FLEXCOMM 1 -8)*
 *
 * 0,0,0 indicates pin is not configured
 */
const i2s_pins_t i2s_pin_config[] =
{
    {.port = 0U, .sck_pin = 7U,  .ws_pin = 8U,  .data_pin = 9U},
    {.port = 0U, .sck_pin = 0U,  .ws_pin = 0U,  .data_pin = 0U},
    {.port = 0U, .sck_pin = 21U, .ws_pin = 22U, .data_pin = 23U},
    {.port = 0U, .sck_pin = 28U, .ws_pin = 29U, .data_pin = 30U},
    {.port = 1U, .sck_pin = 3U,  .ws_pin = 4U,  .data_pin = 5U},
    {.port = 0U, .sck_pin = 0U,  .ws_pin = 0U,  .data_pin = 0U},
    {.port = 0U, .sck_pin = 0U,  .ws_pin = 0U,  .data_pin = 0U},
    {.port = 0U, .sck_pin = 0U,  .ws_pin = 0U,  .data_pin = 0U},
};

/** Configure I2S pins
 *
 * All pins for I2S will have the following configuration:
 * FUNC: 1
 * PUPDENA: Disabled
 * PUPDSEL: 0
 * IBENA: enabled
 * SLEWRATE: standard
 * FULLDRIVE: normal
 * AMENA: disabled
 * ODENA: disabled
 * IIENA: not inverted
 * These settings are suggested for I2S functionality on pg 576 of the
 * manual, Table 581
 *
 * @param config I2S configuration structure
 *
*/
static void i2s_pin_init(i2s_init_t config);

static void i2s_init(i2s_init_t config)
{
    i2s_config_t i2sConfig;

    i2s_pin_init(config);

    if (config.is_transmit)
    {
        I2S_TxGetDefaultConfig(&i2sConfig);
    } else
    {
        I2S_RxGetDefaultConfig(&i2sConfig);
    }

    i2sConfig.masterSlave = config.is_master ?kI2S_MasterSlaveNormalMaster :kI2S_MasterSlaveNormalSlave;
    i2sConfig.mode = config.active_channels > 2 ? kI2S_ModeDspWsShort : kI2S_ModeI2sClassic;
    i2sConfig.dataLength = config.datalength;
    i2sConfig.frameLength = config.datalength * config.active_channels;

    set_peripheral_address(config);

    if (config.is_transmit)
    {
        I2S_TxInit(config.context->base, &i2sConfig);
    } else
    {
        I2S_RxInit(config.context->base, &i2sConfig);
    }

}

// Documented in .h
static void i2s_pin_init(i2s_init_t config)
{
    uint32_t pin_config = (
                        IOPCTL_PIO_FUNC1 |
                        IOPCTL_PIO_PUPD_DI |
                        IOPCTL_PIO_PULLDOWN_EN |
                        IOPCTL_PIO_INBUF_EN |
                        IOPCTL_PIO_SLEW_RATE_NORMAL |
                        IOPCTL_PIO_FULLDRIVE_EN |
                        IOPCTL_PIO_ANAMUX_DI |
                        IOPCTL_PIO_PSEDRAIN_DI |
                        IOPCTL_PIO_INV_DI);
    IOPCTL_PinMuxSet(IOPCTL, i2s_pin_config[config.flexcomm_bus].port, i2s_pin_config[config.flexcomm_bus].sck_pin,   pin_config);
    IOPCTL_PinMuxSet(IOPCTL, i2s_pin_config[config.flexcomm_bus].port, i2s_pin_config[config.flexcomm_bus].ws_pin,    pin_config);
    IOPCTL_PinMuxSet(IOPCTL, i2s_pin_config[config.flexcomm_bus].port, i2s_pin_config[config.flexcomm_bus].data_pin,  pin_config);
}

// Documented above
static void set_peripheral_address(i2s_init_t config)
{
    switch(config.flexcomm_bus)
    {
        case 0:
            config.context->base = (I2S_Type *)FLEXCOMM0;
            break;
        case 1:
            config.context->base = (I2S_Type *)FLEXCOMM1;
            break;
        case 2:
            config.context->base = (I2S_Type *)FLEXCOMM2;
            break;
        case 3:
            config.context->base = (I2S_Type *)FLEXCOMM3;
            break;
        case 4:
            config.context->base = (I2S_Type *)FLEXCOMM4;
            break;
        case 5:
            config.context->base = (I2S_Type *)FLEXCOMM5;
            break;
        case 6:
            config.context->base = (I2S_Type *)FLEXCOMM6;
            break;
        case 7:
            config.context->base = (I2S_Type *)FLEXCOMM7;
            break;
    }
}