#include "i2s.h"
#include "assert.h"
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

#define I2S_DMA DMA0

#define FLEXCOMM_CLOCK_SOURCE 24576000UL
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

/** Pin definitions for Flexcomm busses supporting I2S
 *
 * These pinouts are described in Table 4.  Each one of these
 * pinout designations are for use with FUNC1 in the pin mux
 * configuration.
 *
 */
const i2s_pins_t i2s_pin_config[] =
{
    {.port = 0U, .sck_pin = 0U,  .ws_pin = 1U,  .data_pin = 2U},
    {.port = 0U, .sck_pin = 7U,  .ws_pin = 8U,  .data_pin = 9U},
    {.port = 0U, .sck_pin = 14U, .ws_pin = 15U, .data_pin = 16U},
    {.port = 0U, .sck_pin = 21U, .ws_pin = 22U, .data_pin = 23U},
    {.port = 0U, .sck_pin = 28U, .ws_pin = 29U, .data_pin = 30U},
    {.port = 1U, .sck_pin = 3U,  .ws_pin = 4U,  .data_pin = 5U},
    {.port = 3U, .sck_pin = 25U, .ws_pin = 26U, .data_pin = 27U},
    {.port = 4U, .sck_pin = 0U,  .ws_pin = 1U,  .data_pin = 2U},
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

static void dma_init(i2s_init_t config);

void i2s_init(i2s_init_t config)
{
    // Must have between 2 and 8 channels and they must be in pairs
    assert(config.active_channels > 2 && config.active_channels <= 8
            && !(config.active_channels % 2));

    assert(config.flexcomm_bus >= FLEXCOMM_0 && config.flexcomm_bus < FLEXCOMM_ND);

    i2s_config_t i2sConfig;

    i2s_pin_init(config);

    if (config.is_transmit)
    {
        /* Default config:
        *   masterSlave = kI2S_MasterSlaveNormalMaster;
        *   mode = kI2S_ModeI2sClassic;
        *   rightLow = false;
        *   leftJust = false;
        *   pdmData = false;
        *   sckPol = false;
        *   wsPol = false;
        *   divider = 1;
        *   oneChannel = false;
        *   dataLength = 16;
        *   frameLength = 32;
        *   position = 0;
        *   watermark = 4;
        *   txEmptyZero = true;
        *   pack48 = false;
        */
        I2S_TxGetDefaultConfig(&i2sConfig);
    } else
    {
        /* Default config:
        *   masterSlave = kI2S_MasterSlaveNormalSlave;
        *   mode = kI2S_ModeI2sClassic;
        *   rightLow = false;
        *   leftJust = false;
        *   pdmData = false;
        *   sckPol = false;
        *   wsPol = false;
        *   divider = 1;
        *   oneChannel = false;
        *   dataLength = 16;
        *   frameLength = 32;
        *   position = 0;
        *   watermark = 4;
        *   txEmptyZero = false;
        *   pack48 = false;
        */
        I2S_RxGetDefaultConfig(&i2sConfig);
    }

    i2sConfig.position = 1;
    i2sConfig.masterSlave = config.is_master ? kI2S_MasterSlaveNormalMaster : kI2S_MasterSlaveNormalSlave;
    i2sConfig.mode = config.active_channels > 2 ? kI2S_ModeDspWsShort : kI2S_ModeI2sClassic;
    i2sConfig.dataLength = config.datalength;
    i2sConfig.frameLength = config.datalength * config.active_channels;
    i2sConfig.divider = FLEXCOMM_CLOCK_SOURCE / config.sample_rate / i2sConfig.frameLength;

    set_peripheral_address(config);

    if (config.is_transmit)
    {
        I2S_TxInit(config.context->base, &i2sConfig);
    } else
    {
        I2S_RxInit(config.context->base, &i2sConfig);
    }

    if (config.active_channels > 2)
    {
        // Divide the active number of channels by 2; we have already established the
        // channels are in pairs
        uint8_t channel_pairs = config.active_channels >> 1;
        for (uint8_t frame = 1; frame < channel_pairs; frame++)
        {
            I2S_EnableSecondaryChannel(config.context->base, frame-1, false,
                i2sConfig.position + frame * i2sConfig.frameLength);
        }
    }

    // I2S CONFIGURATION COMPLETE //



    // Set MCLK direction as output
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

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
        case FLEXCOMM_0:
            config.context->base = (I2S_Type *)FLEXCOMM0;
            break;
        case FLEXCOMM_1:
            config.context->base = (I2S_Type *)FLEXCOMM1;
            break;
        case FLEXCOMM_2:
            config.context->base = (I2S_Type *)FLEXCOMM2;
            break;
        case FLEXCOMM_3:
            config.context->base = (I2S_Type *)FLEXCOMM3;
            break;
        case FLEXCOMM_4:
            config.context->base = (I2S_Type *)FLEXCOMM4;
            break;
        case FLEXCOMM_5:
            config.context->base = (I2S_Type *)FLEXCOMM5;
            break;
        case FLEXCOMM_6:
            config.context->base = (I2S_Type *)FLEXCOMM6;
            break;
        case FLEXCOMM_7:
            config.context->base = (I2S_Type *)FLEXCOMM7;
            break;
        default:
            break;
    }

}

static void dma_init(i2s_init_t config)
{
    // I2S peripheral must be defined
    assert(config.context->base);
    DMA_Init(I2S_DMA);

    // DMA Channels are defined in Table 364;
    // channel is determined by whether it is receive or transmit
    uint32_t dma_channel = config.is_transmit ? 1 + 2 * config.flexcomm_bus : 2* config.flexcomm_bus;

    DMA_EnableChannel(I2S_DMA, dma_channel);
    DMA_SetChannelPriority(I2S_DMA, dma_channel, kDMA_ChannelPriority3);
    DMA_CreateHandle(config.context->dma_handle, I2S_DMA, dma_channel);

    if (config.is_transmit)
    {
        I2S_TxTransferCreateHandleDMA(config.context->base, config.context->i2s_dma_handle,
                                      config.context->dma_handle, config.callback, NULL);

    } else {
        I2S_RxTransferCreateHandleDMA(config.context->base, config.context->i2s_dma_handle,
                                      config.context->dma_handle, config.callback, NULL);
    }

}
